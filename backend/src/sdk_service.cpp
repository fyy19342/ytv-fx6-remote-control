#include "sdk_service.h"
#include "shutter_controller.h"

#include <algorithm>
#include <chrono>
#include <cmath>
#include <filesystem>
#include <future>
#include <iomanip>
#include <limits>
#include <memory>
#include <sstream>
#include <stdexcept>
#include <thread>

#if defined(__APPLE__) || defined(__linux__)
#include <unistd.h>
#endif

#include "sony_diagnostics.h"
#include "exposure_steps.h"
#include "network_target.h"

namespace {

int64_t monotonic_ms() {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        std::chrono::steady_clock::now().time_since_epoch()).count();
}

std::string sdk_error_to_string(SCRSDK::CrError err) {
    if (err == SCRSDK::CrError_None) return "CrError_None";
    return sdk_error_name(err) + " (0x" + [] (SCRSDK::CrError value) {
        std::ostringstream oss;
        oss << std::hex << value;
        return oss.str();
    }(err) + ")";
}

std::string comma_separated(uint64_t value) {
    std::string src = std::to_string(value);
    std::string out;
    int count = 0;
    for (auto it = src.rbegin(); it != src.rend(); ++it) {
        if (count == 3) {
            out.push_back(',');
            count = 0;
        }
        out.push_back(*it);
        ++count;
    }
    std::reverse(out.begin(), out.end());
    return out;
}

std::string format_decimal(double value, int decimals = 1) {
    std::ostringstream oss;
    oss << std::fixed << std::setprecision(decimals) << value;
    std::string s = oss.str();
    while (!s.empty() && s.back() == '0') s.pop_back();
    if (!s.empty() && s.back() == '.') s.pop_back();
    return s;
}

std::string current_working_directory() {
#if defined(__APPLE__)
    constexpr size_t kMaxPath = 1024;
    char buffer[kMaxPath] = {0};
    if (getcwd(buffer, sizeof(buffer) - 1) != nullptr) {
        return std::string(buffer);
    }
#endif
    return std::filesystem::current_path().string();
}

template <typename T>
std::vector<uint64_t> extract_array_values(const uint8_t* values, uint32_t size) {
    const auto count = size / sizeof(T);
    const auto* typed = reinterpret_cast<const T*>(values);
    std::vector<uint64_t> out;
    out.reserve(count);
    for (uint32_t i = 0; i < count; ++i) {
        out.push_back(static_cast<uint64_t>(typed[i]));
    }
    return out;
}

} // namespace

Fx6SdkService::Fx6SdkService(Logger& logger)
    : logger_(logger), callback_(*this) {
    sdk_initialized_ = SCRSDK::Init();
    if (!sdk_initialized_) {
        set_last_error("SCRSDK::Init failed");
        logger_.error(last_error_);
    } else {
        logger_.info("SCRSDK initialized");
    }
}

Fx6SdkService::~Fx6SdkService() {
    std::string ignored;
    disconnect_camera(ignored);
    release_sdk();
}

bool Fx6SdkService::ensure_initialized() {
    if (sdk_initialized_) return true;
    set_last_error("SDK is not initialized");
    return false;
}

void Fx6SdkService::release_sdk() {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    if (sdk_initialized_) {
        SCRSDK::Release();
        sdk_initialized_ = false;
        logger_.info("SCRSDK released");
    }
}

void Fx6SdkService::set_last_error(const std::string& message) {
    std::lock_guard<std::mutex> lock(error_mutex_);
    last_error_ = message;
}

std::string Fx6SdkService::build_camera_id(const SCRSDK::ICrCameraObjectInfo* info) const {
    if (!info) return {};
    auto mac = info->GetMACAddressChar();
    if (mac && *mac) return std::string(mac);
    auto guid = info->GetGuid();
    if (guid && *guid) return std::string(guid);
    auto ip = info->GetIPAddressChar();
    if (ip && *ip) return std::string(ip);
    return std::string(info->GetModel()) + "-unknown";
}

std::vector<CameraSummary> Fx6SdkService::enumerate_cameras() {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    std::vector<CameraSummary> result;
    if (!ensure_initialized()) return result;

    constexpr int kAttempts = 2;
    for (int attempt = 1; attempt <= kAttempts; ++attempt) {
        SCRSDK::ICrEnumCameraObjectInfo* camera_list = nullptr;
        const auto err = SCRSDK::EnumCameraObjects(&camera_list, 3);
        if (err == SCRSDK::CrError_None && camera_list != nullptr) {
            const auto count = camera_list->GetCount();
            for (uint32_t i = 0; i < count; ++i) {
                const auto* info = camera_list->GetCameraObjectInfo(i);
                if (!info) continue;
                CameraSummary camera;
                camera.index = static_cast<int>(i) + 1;
                camera.id = build_camera_id(info);
                camera.name = info->GetName() ? std::string(info->GetName()) : "";
                camera.model = info->GetModel() ? std::string(info->GetModel()) : "";
                camera.connection_type = info->GetConnectionTypeName() ? std::string(info->GetConnectionTypeName()) : "";
                camera.mac_address = info->GetMACAddressChar() ? std::string(info->GetMACAddressChar()) : "";
                camera.ip_address = info->GetIPAddressChar() ? std::string(info->GetIPAddressChar()) : "";
                camera.ssh_support = info->GetSSHsupport() == SCRSDK::CrSSHsupport_ON;
                if (camera.ssh_support) {
                    char fp[128] = {0};
                    CrInt32u len = 0;
                    if (SCRSDK::GetFingerprint(const_cast<SCRSDK::ICrCameraObjectInfo*>(info), fp, &len) == SCRSDK::CrError_None && len > 0 && len <= sizeof(fp)) {
                        camera.fingerprint.assign(fp, fp + len);
                    }
                }
                result.push_back(camera);
            }
            camera_list->Release();
            if (!result.empty()) {
                logger_.info("Enumerated " + std::to_string(result.size()) + " camera(s)");
                return result;
            }
            logger_.warn("EnumCameraObjects returned 0 camera(s) on attempt " + std::to_string(attempt));
        } else if (err == SCRSDK::CrError_None) {
            logger_.warn("EnumCameraObjects returned no camera list on attempt " + std::to_string(attempt));
        } else {
            const auto message = "EnumCameraObjects failed: " + sdk_error_to_string(err);
            set_last_error(message);
            logger_.warn(message + " (attempt " + std::to_string(attempt) + ")");
            if (camera_list) camera_list->Release();
        }
        if (attempt < kAttempts) std::this_thread::sleep_for(std::chrono::milliseconds(700));
    }

    logger_.warn("No camera detected after retries");
    return result;
}

SCRSDK::ICrCameraObjectInfo* Fx6SdkService::create_ip_camera_locked(const std::string& address, std::string& error) {
    auto target = parse_network_target(address);
    if (!target) {
        error = "Enter a valid unicast IPv4 address (example: 192.168.0.5).";
        return nullptr;
    }
    SCRSDK::ICrCameraObjectInfo* camera = nullptr;
    const auto err = SCRSDK::CreateCameraObjectInfoEthernetConnection(&camera,
        SCRSDK::CrCameraDeviceModel_ILME_FX6, target->sdk_ip, target->object_id.data(), SCRSDK::CrSSHsupport_ON);
    if (err != SCRSDK::CrError_None || !camera) {
        if (camera) camera->Release();
        error = "Create IP camera failed: " + sdk_error_to_string(err);
        return nullptr;
    }
    return camera;
}

bool Fx6SdkService::probe_camera_ip(const std::string& address, CameraSummary& camera, std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    error.clear();
    if (!ensure_initialized()) { error = "SDK is not initialized"; return false; }
    if (connected_) { error = "Disconnect the current camera before checking another IP."; return false; }
    direct_camera_.reset();
    const auto release = [](SCRSDK::ICrCameraObjectInfo* value) { if (value) value->Release(); };
    std::unique_ptr<SCRSDK::ICrCameraObjectInfo, decltype(release)> object(create_ip_camera_locked(address, error), release);
    if (!object) return false;
    char fingerprint[128] = {};
    CrInt32u length = 0;
    const auto err = SCRSDK::GetFingerprint(object.get(), fingerprint, &length);
    if (err != SCRSDK::CrError_None || length == 0 || length > sizeof(fingerprint)) {
        error = "IP fingerprint check failed: " + sdk_error_to_string(err) +
            ". Check the camera IP, Camera Remote Control setting and local network permission.";
        set_last_error(error);
        return false;
    }
    camera = {};
    camera.id = "ip:" + address;
    camera.name = "FX6 (IP direct)";
    camera.model = "ILME-FX6";
    camera.connection_type = "IP direct (SSH)";
    camera.ip_address = address;
    camera.ssh_support = true;
    camera.fingerprint.assign(fingerprint, length);
    direct_camera_ = camera;
    set_last_error("");
    logger_.info("IP camera fingerprint obtained; authentication has not run");
    return true;
}

bool Fx6SdkService::connect_camera(const std::string& camera_id, const std::string& user_id, const std::string& password,
                                 std::string& error, const std::string& expected_fingerprint) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    error.clear();
    if (!ensure_initialized()) {
        error = last_error_;
        return false;
    }
    if (connected_) {
        logger_.info("Disconnecting existing camera before reconnect");
        SCRSDK::Disconnect(device_handle_);
        connected_ = false;
        device_handle_ = 0;
    }

    const auto release_list = [](SCRSDK::ICrEnumCameraObjectInfo* p) { if (p) p->Release(); };
    const auto release_camera = [](SCRSDK::ICrCameraObjectInfo* p) { if (p) p->Release(); };
    std::unique_ptr<SCRSDK::ICrEnumCameraObjectInfo, decltype(release_list)> camera_list(nullptr, release_list);
    std::unique_ptr<SCRSDK::ICrCameraObjectInfo, decltype(release_camera)> direct_object(nullptr, release_camera);
    SCRSDK::ICrCameraObjectInfo* selected = nullptr;
    SCRSDK::CrError err = SCRSDK::CrError_None;
    if (camera_id.rfind("ip:", 0) == 0) {
        if (!direct_camera_ || direct_camera_->id != camera_id || expected_fingerprint.empty() ||
                direct_camera_->fingerprint != expected_fingerprint) {
            error = "Check the IP and displayed fingerprint again before connecting.";
            return false;
        }
        direct_object.reset(create_ip_camera_locked(direct_camera_->ip_address, error));
        if (!direct_object) return false;
        selected = direct_object.get();
    } else {
        SCRSDK::ICrEnumCameraObjectInfo* list = nullptr;
        err = SCRSDK::EnumCameraObjects(&list, 3);
        camera_list.reset(list);
        if (err != SCRSDK::CrError_None || !camera_list) {
            error = "Camera discovery failed: " + sdk_error_to_string(err) + ". Try IP direct connection.";
            set_last_error(error);
            return false;
        }
        for (uint32_t i = 0; i < camera_list->GetCount(); ++i) {
            auto* info = const_cast<SCRSDK::ICrCameraObjectInfo*>(camera_list->GetCameraObjectInfo(i));
            if (info && build_camera_id(info) == camera_id) { selected = info; break; }
        }
    }

    if (!selected) {
        error = "Requested camera was not found: " + camera_id;
        set_last_error(error);
        logger_.error(error);
        return false;
    }

    std::string fingerprint;
    if (selected->GetSSHsupport() == SCRSDK::CrSSHsupport_ON) {
        char fp[128] = {0};
        CrInt32u len = 0;
        err = SCRSDK::GetFingerprint(selected, fp, &len);
        if (err != SCRSDK::CrError_None || len == 0 || len > sizeof(fp)) {
            error = "GetFingerprint failed: " + sdk_error_to_string(err);
            set_last_error(error);
            logger_.error(error);
            return false;
        }
        fingerprint.assign(fp, fp + len);
        if (!expected_fingerprint.empty() && fingerprint != expected_fingerprint) {
            error = "Camera fingerprint changed. Recheck the camera identity before connecting.";
            return false;
        }
    }

    std::promise<void> event_promise;
    std::future<void> event_future = event_promise.get_future();
    {
        std::lock_guard<std::mutex> event_lock(event_mutex_);
        event_promise_ = &event_promise;
        pending_property_code_ = 0;
    }

    logger_.info("Connecting to camera: " + build_camera_id(selected));
    err = SCRSDK::Connect(
        selected,
        &callback_,
        &device_handle_,
        SCRSDK::CrSdkControlMode_Remote,
        SCRSDK::CrReconnecting_ON,
        user_id.c_str(),
        password.c_str(),
        fingerprint.c_str(),
        static_cast<CrInt32u>(fingerprint.size())
    );

    if (err != SCRSDK::CrError_None) {
        {
            std::lock_guard<std::mutex> event_lock(event_mutex_);
            event_promise_ = nullptr;
        }
        error = "Connect failed: " + sdk_error_to_string(err);
        set_last_error(error);
        logger_.error(error);
        return false;
    }

    if (event_future.wait_for(std::chrono::seconds(10)) != std::future_status::ready) {
        {
            std::lock_guard<std::mutex> event_lock(event_mutex_);
            event_promise_ = nullptr;
        }
        SCRSDK::Disconnect(device_handle_);
        connected_ = false;
        device_handle_ = 0;
        error = "Timed out waiting for camera connection.";
        set_last_error(error);
        return false;
    }
    try {
        event_future.get();
    } catch (const std::exception& ex) {
        {
            std::lock_guard<std::mutex> event_lock(event_mutex_);
            event_promise_ = nullptr;
        }
        error = std::string("Connection event failed: ") + ex.what();
        set_last_error(error);
        logger_.error(error);
        return false;
    }

    { std::lock_guard<std::mutex> lock(awb_mutex_); awb_progress_ = {}; }
    connected_camera_id_ = camera_id;
    connected_camera_model_ = selected->GetModel() ? std::string(selected->GetModel()) : "";
    connected_camera_name_ = selected->GetName() ? std::string(selected->GetName()) : connected_camera_model_;

    camera_list.reset();
    direct_object.reset();

    if (!set_save_info_locked(error)) {
        logger_.warn("SetSaveInfo skipped/failed: " + error);
        error.clear();
    }

    std::string defaults_error;
    if (!ensure_operational_defaults_locked(defaults_error)) {
        logger_.warn("Operational defaults could not be fully applied: " + defaults_error);
    }

    set_last_error("");
    logger_.info("Connected to " + connected_camera_model_ + " (" + connected_camera_id_ + ")");
    return true;
}

bool Fx6SdkService::disconnect_camera(std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    error.clear();
    if (!connected_) return true;

    const auto err = SCRSDK::Disconnect(device_handle_);
    if (err != SCRSDK::CrError_None) {
        error = "Disconnect failed: " + sdk_error_to_string(err);
        set_last_error(error);
        logger_.error(error);
        return false;
    }

    connected_ = false;
    device_handle_ = 0;
    connected_camera_id_.clear();
    connected_camera_model_.clear();
    connected_camera_name_.clear();
    logger_.info("Camera disconnected");
    return true;
}

bool Fx6SdkService::set_save_info_locked(std::string& error) {
    error.clear();
    const auto cwd = current_working_directory();
    auto err = SCRSDK::SetSaveInfo(device_handle_, const_cast<CrChar*>(cwd.c_str()), const_cast<CrChar*>("FX6"), SCRSDK::CrSETSAVEINFO_AUTO_NUMBER);
    if (err != SCRSDK::CrError_None) {
        error = "SetSaveInfo failed: " + sdk_error_to_string(err);
        return false;
    }
    return true;
}

bool Fx6SdkService::ensure_operational_defaults_locked(std::string& error) {
    error.clear();
    // The operator's Gain control is represented by ISO sensitivity.
    return set_property_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainUnitSetting,
                               static_cast<uint64_t>(SCRSDK::CrGainUnitSetting_ISO), error, true);
}

SCRSDK::CrError Fx6SdkService::get_property_locked(uint32_t code, SCRSDK::CrDeviceProperty& out_property) {
    if (!connected_) return SCRSDK::CrError_Connect_Disconnected;
    std::int32_t num_properties = 0;
    SCRSDK::CrDeviceProperty* property_list = nullptr;
    const auto err = SCRSDK::GetSelectDeviceProperties(device_handle_, 1, &code, &property_list, &num_properties);
    const bool found = property_list != nullptr && num_properties >= 1;
    if (err == SCRSDK::CrError_None && found) {
        out_property = property_list[0];
    }
    if (property_list) {
        SCRSDK::ReleaseDeviceProperties(device_handle_, property_list);
    }
    return err == SCRSDK::CrError_None && !found
        ? SCRSDK::CrError_Connect_GetProperty : err;
}

bool Fx6SdkService::set_property_locked(uint32_t code, uint64_t data, std::string& error, bool blocking, bool force) {
    error.clear();
    if (!connected_) {
        error = "Camera is not connected.";
        return false;
    }

    SCRSDK::CrDeviceProperty property;
    auto err = get_property_locked(code, property);
    if (err != SCRSDK::CrError_None) {
        error = "Get property failed for " + sdk_property_name(static_cast<SCRSDK::CrDevicePropertyCode>(code)) + ": " + sdk_error_to_string(err);
        set_last_error(error);
        logger_.error(error);
        return false;
    }
    if (!force && property.IsGetEnableCurrentValue() && property.GetCurrentValue() == data) {
        return true;
    }
    if (!property.IsSetEnableCurrentValue()) {
        error = sdk_property_name(static_cast<SCRSDK::CrDevicePropertyCode>(code)) + " is not writable";
        return false;
    }

    std::promise<void> event_promise;
    std::future<void> event_future = event_promise.get_future();
    if (blocking) {
        std::lock_guard<std::mutex> event_lock(event_mutex_);
        pending_property_code_ = code;
        event_promise_ = &event_promise;
    }

    property.SetCurrentValue(data);
    err = SCRSDK::SetDeviceProperty(device_handle_, &property);
    if (err != SCRSDK::CrError_None) {
        if (blocking) {
            std::lock_guard<std::mutex> event_lock(event_mutex_);
            event_promise_ = nullptr;
            pending_property_code_ = 0;
        }
        error = "Set property failed for " + sdk_property_name(static_cast<SCRSDK::CrDevicePropertyCode>(code)) + ": " + sdk_error_to_string(err);
        set_last_error(error);
        logger_.error(error);
        return false;
    }

    if (blocking) {
        const auto status = event_future.wait_for(std::chrono::milliseconds(3000));
        if (status != std::future_status::ready) {
            {
                std::lock_guard<std::mutex> event_lock(event_mutex_);
                event_promise_ = nullptr;
                pending_property_code_ = 0;
            }
            // No notification can be sent for an idempotent forced OFF. The
            // readback below remains authoritative, including after a timeout.
            logger_.warn("Property notification timed out; checking readback.");
        } else {
            try {
                event_future.get();
            } catch (const std::exception& ex) {
                error = std::string("Property change failed: ") + ex.what();
                set_last_error(error);
                logger_.error(error);
                return false;
            }
        }
    }

    if (blocking) {
        SCRSDK::CrDeviceProperty confirmed;
        if (get_property_locked(code, confirmed) != SCRSDK::CrError_None ||
            !confirmed.IsGetEnableCurrentValue() || confirmed.GetCurrentValue() != data) {
            error = "Property readback mismatch: " + sdk_property_name(static_cast<SCRSDK::CrDevicePropertyCode>(code));
            set_last_error(error);
            return false;
        }
    }
    logger_.info("Set " + sdk_property_name(static_cast<SCRSDK::CrDevicePropertyCode>(code)) + " -> " + std::to_string(data));
    return true;
}

std::vector<uint64_t> Fx6SdkService::extract_possible_values(const SCRSDK::CrDeviceProperty& property) const {
    const auto data_type = property.GetValueType();
    const auto* values = property.GetValues();
    const auto size = property.GetValueSize();
    if (!values || size == 0) return {};

    switch (data_type) {
    case SCRSDK::CrDataType_UInt8Array:
        return extract_array_values<uint8_t>(values, size);
    case SCRSDK::CrDataType_UInt16Array:
        return extract_array_values<uint16_t>(values, size);
    case SCRSDK::CrDataType_UInt32Array:
        return extract_array_values<uint32_t>(values, size);
    case SCRSDK::CrDataType_UInt64Array:
        return extract_array_values<uint64_t>(values, size);
    case SCRSDK::CrDataType_Int16Array: {
        auto raw = extract_array_values<int16_t>(values, size);
        return raw;
    }
    case SCRSDK::CrDataType_UInt32Range: {
        return extract_array_values<uint32_t>(values, size);
    }
    case SCRSDK::CrDataType_Int16Range: {
        return extract_array_values<int16_t>(values, size);
    }
    default:
        return {};
    }
}

PropertyView Fx6SdkService::read_property_view_locked(uint32_t code, const std::string&) {
    PropertyView view;
    if (!connected_) return view;

    SCRSDK::CrDeviceProperty property;
    const auto err = get_property_locked(code, property);
    if (err != SCRSDK::CrError_None) {
        return view;
    }

    view.supported = property.IsGetEnableCurrentValue();
    view.writable = property.IsSetEnableCurrentValue();
    view.raw = property.GetCurrentValue();
    view.label = label_for_property(code, view.raw);
    view.possible = extract_possible_values(property);
    return view;
}

std::string Fx6SdkService::label_for_property(uint32_t code, uint64_t raw) const {
    switch (code) {
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_FNumber: {
        if (raw == SCRSDK::CrFnumber_IrisClose) return "CLOSE";
        if (!valid_exposure(ExposureKind::Iris, raw)) return "—";
        const double value = static_cast<double>(raw) / 100.0;
        return "F" + format_decimal(value, 2);
    }
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_IsoSensitivity:
        if ((raw & 0x00ffffff) == SCRSDK::CrISO_AUTO) return "ISO AUTO";
        return "ISO " + comma_separated(exposure_number(ExposureKind::Iso, raw));
    case SCRSDK::CrDeviceProperty_ShutterSpeedValue:
        if (!valid_shutter_speed(raw)) return "—";
        if ((raw >> 32) >= (raw & 0xffffffff))
            return format_decimal(static_cast<double>(raw >> 32) / (raw & 0xffffffff), 3) + " s";
        return "1/" + format_decimal(static_cast<double>(raw & 0xffffffff) / (raw >> 32), 2) + " s";
    case SCRSDK::CrDeviceProperty_ShutterModeStatus:
        switch (raw) {
        case SCRSDK::CrShutterModeStatus_Speed: return "Speed";
        case SCRSDK::CrShutterModeStatus_Off: return "OFF";
        case SCRSDK::CrShutterModeStatus_Angle: return "Angle";
        case SCRSDK::CrShutterModeStatus_ECS: return "ECS";
        case SCRSDK::CrShutterModeStatus_Auto: return "Auto";
        default: return "—";
        }
    case SCRSDK::CrDeviceProperty_WhiteBalanceModeSetting:
        return raw == SCRSDK::CrWhiteBalanceModeSetting_Manual ? "Manual" :
            raw == SCRSDK::CrWhiteBalanceModeSetting_Automatic ? "ATW" : "—";
    case SCRSDK::CrDeviceProperty_Colortemp:
        return raw == 0 || raw >= 0xffff ? "—" : std::to_string(raw) + " K";
    case SCRSDK::CrDeviceProperty_AWB:
        return raw == SCRSDK::CrAWB_Up ? "Released" : raw == SCRSDK::CrAWB_Down ? "Pressed" : "—";
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainBaseIsoSensitivity:
        return raw == SCRSDK::CrGainBaseIsoSensitivity_High ? "High" : raw == SCRSDK::CrGainBaseIsoSensitivity_Low ? "Low" : std::to_string(raw);
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainUnitSetting:
        return raw == SCRSDK::CrGainUnitSetting_ISO ? "ISO" : raw == SCRSDK::CrGainUnitSetting_dB ? "dB" : std::to_string(raw);
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilter:
        return raw == SCRSDK::CrNDFilter_ON ? "ON" : raw == SCRSDK::CrNDFilter_OFF ? "OFF" : std::to_string(raw);
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilterModeSetting:
        return raw == SCRSDK::CrNDFilterModeSetting_Manual ? "Manual" : raw == SCRSDK::CrNDFilterModeSetting_Automatic ? "Automatic" : std::to_string(raw);
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilterSwitchingSetting:
        if (raw == SCRSDK::CrNDFilterSwitchingSetting_Step) return "Step";
        if (raw == SCRSDK::CrNDFilterSwitchingSetting_Variable) return "Variable";
        if (raw == SCRSDK::CrNDFilterSwitchingSetting_Preset) return "Preset";
        return std::to_string(raw);
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilterOpticalDensityValue:
        if (raw == 0 || raw >= 0xffff) return "ND value unavailable";
        return "1/~" + std::to_string(static_cast<long long>(std::llround(std::pow(10.0, static_cast<double>(raw) / 100.0))))
            + " (OD " + format_decimal(static_cast<double>(raw) / 100.0, 2) + ")";
    case SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilterValue: {
        const auto numerator = raw >> 32;
        const auto denominator = raw & 0xffffffff;
        if (numerator == 0 || numerator >= denominator) return "ND value unavailable";
        return "1/" + format_decimal(static_cast<double>(denominator) / numerator, 2);
    }
    default:
        return std::to_string(raw);
    }
}

StateSnapshot Fx6SdkService::get_state() {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    StateSnapshot snapshot;
    snapshot.sdk_initialized = sdk_initialized_;
    snapshot.connected = connected_;
    snapshot.camera_id = connected_camera_id_;
    snapshot.camera_model = connected_camera_model_;
    snapshot.camera_name = connected_camera_name_;
    {
        std::lock_guard<std::mutex> error_lock(error_mutex_);
        snapshot.last_error = last_error_;
    }
    snapshot.log_path = logger_.path();
    {
        std::lock_guard<std::mutex> lock(awb_mutex_);
        awb_progress_.expire(monotonic_ms());
        if (!connected_) awb_progress_.disconnect();
        snapshot.awb = awb_progress_;
        snapshot.awb_retry_after_ms = awb_progress_.retry_after_ms(monotonic_ms());
    }
    if (!connected_) return snapshot;

    snapshot.iris = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_FNumber);
    snapshot.iso = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_IsoSensitivity);
    snapshot.shutter_speed = read_property_view_locked(SCRSDK::CrDeviceProperty_ShutterSpeedValue);
    snapshot.shutter_mode = read_property_view_locked(SCRSDK::CrDeviceProperty_ShutterModeStatus);
    snapshot.white_balance_mode = read_property_view_locked(SCRSDK::CrDeviceProperty_WhiteBalanceModeSetting);
    snapshot.color_temperature = read_property_view_locked(SCRSDK::CrDeviceProperty_Colortemp);
    snapshot.awb_button = read_property_view_locked(SCRSDK::CrDeviceProperty_AWB);
    snapshot.iso_base = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainBaseIsoSensitivity);
    snapshot.gain_unit = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainUnitSetting);
    snapshot.nd_filter = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilter);
    snapshot.nd_mode_setting = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilterModeSetting);
    snapshot.nd_switching = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilterSwitchingSetting);
    snapshot.nd_optical_density = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilterOpticalDensityValue);
    snapshot.nd_value = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_NDFilterValue);

    return snapshot;
}

bool Fx6SdkService::step_array_property_locked(uint32_t code, int delta, std::string& error) {
    error.clear();
    if (delta == 0) return true;

    SCRSDK::CrDeviceProperty property;
    const auto err = get_property_locked(code, property);
    if (err != SCRSDK::CrError_None) {
        error = "Failed to read property: " + sdk_error_to_string(err);
        return false;
    }
    if (!property.IsSetEnableCurrentValue()) {
        error = sdk_property_name(static_cast<SCRSDK::CrDevicePropertyCode>(code)) + " is not writable";
        return false;
    }
    const auto kind = code == SCRSDK::CrDeviceProperty_FNumber ? ExposureKind::Iris : ExposureKind::Iso;
    const auto target = exposure_step(kind, extract_possible_values(property), property.GetCurrentValue(), delta);
    if (!target || !property.IsGetEnableCurrentValue()) {
        error = "No manual numeric value/steps available. Set Iris / Gain (ISO) to manual on the camera.";
        return false;
    }
    return set_property_locked(code, *target, error, true);
}

bool Fx6SdkService::step_iris(int delta, std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    // Positive delta opens iris by selecting a lower F number.
    return step_array_property_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_FNumber, delta > 0 ? -1 : delta < 0 ? 1 : 0, error);
}

bool Fx6SdkService::step_iso(int delta, std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    if (!set_property_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainUnitSetting,
                             static_cast<uint64_t>(SCRSDK::CrGainUnitSetting_ISO), error, true)) return false;
    return step_array_property_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_IsoSensitivity, delta, error);
}

bool Fx6SdkService::step_shutter(int delta, std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    if (!connected_) { error = "Camera is not connected."; return false; }
    auto code_for = [](ShutterProperty p) -> uint32_t {
        return p == ShutterProperty::Mode ? SCRSDK::CrDeviceProperty_ShutterModeStatus
                                         : SCRSDK::CrDeviceProperty_ShutterSpeedValue;
    };
    ShutterController controller([this, code_for](auto p) {
        const auto view = read_property_view_locked(code_for(p));
        return ShutterReading{view.supported, view.writable, view.raw, view.possible};
    }, [this, code_for](auto p, auto value, auto& reason) {
        return set_property_locked(code_for(p), value, reason, true);
    }, SCRSDK::CrShutterModeStatus_Speed);
    const bool ok = controller.step(delta, error);
    if (!ok) { set_last_error(error); logger_.error(error); }
    else set_last_error("");
    return ok;
}

bool Fx6SdkService::run_awb(std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    if (!connected_) { error = "Camera is not connected."; return false; }
    {
        std::lock_guard<std::mutex> state_lock(awb_mutex_);
        if (!awb_progress_.begin(monotonic_ms())) {
            error = "AWB は前回の実行から3秒後に再実行できます。";
            return false;
        }
    }
    set_last_error("");
    SCRSDK::CrDeviceProperty button_descriptor;
    AwbController controller([&](AwbProperty p) {
        if (p == AwbProperty::Mode) {
            const auto view = read_property_view_locked(SCRSDK::CrDeviceProperty_WhiteBalanceModeSetting);
            return AwbReading{view.supported, view.writable, view.raw, view.possible};
        }
        if (get_property_locked(SCRSDK::CrDeviceProperty_AWB, button_descriptor) != SCRSDK::CrError_None)
            return AwbReading{};
        return AwbReading{button_descriptor.IsGetEnableCurrentValue(), button_descriptor.IsSetEnableCurrentValue(),
                          button_descriptor.GetCurrentValue(), extract_possible_values(button_descriptor)};
    }, [&](AwbProperty p, uint64_t value, std::string& reason) {
        if (p == AwbProperty::Mode)
            return set_property_locked(SCRSDK::CrDeviceProperty_WhiteBalanceModeSetting, value, reason, true);
        // Keep the validated descriptor for Up: a new property query during
        // measurement must not prevent release of the button we just pressed.
        button_descriptor.SetCurrentValue(value);
        const auto result = SCRSDK::SetDeviceProperty(device_handle_, &button_descriptor);
        if (result != SCRSDK::CrError_None) {
            reason = "AWB: " + sdk_error_to_string(result);
            return false;
        }
        logger_.info(value == SCRSDK::CrAWB_Down ? "AWB Down sent" : "AWB Up sent");
        return true;
    }, [] { std::this_thread::sleep_for(std::chrono::milliseconds(100)); },
        SCRSDK::CrWhiteBalanceModeSetting_Manual, SCRSDK::CrAWB_Up, SCRSDK::CrAWB_Down);
    bool ok = controller.trigger(error);
    {
        std::lock_guard<std::mutex> state_lock(awb_mutex_);
        if (!ok) awb_progress_.fail(error);
        else if (awb_progress_.status == "failed") { error = awb_progress_.message; ok = false; }
    }
    if (!ok) { set_last_error(error); logger_.error(error); }
    return ok; // Accepted is not completion; /api/state carries the SDK result.
}

bool Fx6SdkService::toggle_iso_base(std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    auto current = read_property_view_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainBaseIsoSensitivity);
    if (!current.supported) {
        error = "Gain base ISO sensitivity is not supported.";
        return false;
    }
    const auto target = current.raw == SCRSDK::CrGainBaseIsoSensitivity_High
        ? static_cast<uint64_t>(SCRSDK::CrGainBaseIsoSensitivity_Low)
        : static_cast<uint64_t>(SCRSDK::CrGainBaseIsoSensitivity_High);
    return set_property_locked(SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainBaseIsoSensitivity, target, error, true);
}

bool Fx6SdkService::set_iso_base(bool high, std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    return set_property_locked(
        SCRSDK::CrDevicePropertyCode::CrDeviceProperty_GainBaseIsoSensitivity,
        high ? static_cast<uint64_t>(SCRSDK::CrGainBaseIsoSensitivity_High)
             : static_cast<uint64_t>(SCRSDK::CrGainBaseIsoSensitivity_Low),
        error,
        true
    );
}

NdController Fx6SdkService::nd_controller_locked() {
    auto code_for = [](NdProperty property) -> uint32_t {
        switch (property) {
        case NdProperty::Filter: return SCRSDK::CrDeviceProperty_NDFilter;
        case NdProperty::Mode: return SCRSDK::CrDeviceProperty_NDFilterModeSetting;
        case NdProperty::Switching: return SCRSDK::CrDeviceProperty_NDFilterSwitchingSetting;
        // FX6 supports transmittance. OpticalDensityValue is a different model's
        // property; apparent readback on FX6 can settle to a different value.
        case NdProperty::Density: return SCRSDK::CrDeviceProperty_NDFilterValue;
        }
        return 0;
    };
    return NdController(
        [this, code_for](NdProperty property) {
            const auto view = read_property_view_locked(code_for(property));
            return NdReading{view.supported, view.writable, view.raw, view.possible};
        },
        [this, code_for](NdProperty property, uint64_t value, std::string& error) {
            return set_property_locked(code_for(property), value, error, true,
                                       property == NdProperty::Filter && value == SCRSDK::CrNDFilter_OFF);
        },
        {SCRSDK::CrNDFilter_OFF, SCRSDK::CrNDFilter_ON,
         SCRSDK::CrNDFilterModeSetting_Manual, SCRSDK::CrNDFilterSwitchingSetting_Step,
         SCRSDK::CrNDFilterSwitchingSetting_Variable, NdValueFormat::Transmittance});
}

bool Fx6SdkService::set_nd(bool enabled, std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    if (!connected_) { error = "Camera is not connected."; return false; }
    const bool ok = nd_controller_locked().set(enabled, error);
    if (!ok) { set_last_error(error); logger_.error(error); }
    else set_last_error("");
    return ok;
}

bool Fx6SdkService::toggle_nd(std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    if (!connected_) { error = "Camera is not connected."; return false; }
    const bool ok = nd_controller_locked().toggle(error);
    if (!ok) { set_last_error(error); logger_.error(error); }
    else set_last_error("");
    return ok;
}

bool Fx6SdkService::step_nd(int delta, std::string& error) {
    std::lock_guard<std::mutex> lock(sdk_mutex_);
    if (!connected_) { error = "Camera is not connected."; return false; }
    const bool ok = nd_controller_locked().step(delta, error);
    if (!ok) { set_last_error(error); logger_.error(error); }
    else set_last_error("");
    return ok;
}

void Fx6SdkService::Callback::OnConnected(SCRSDK::DeviceConnectionVersioin) {
    owner_.connected_ = true;
    owner_.logger_.info("Callback: OnConnected");
    std::lock_guard<std::mutex> lock(owner_.event_mutex_);
    if (owner_.event_promise_ && owner_.pending_property_code_ == 0) {
        owner_.event_promise_->set_value();
        owner_.event_promise_ = nullptr;
    }
}

void Fx6SdkService::Callback::OnError(CrInt32u error) {
    const std::string message = std::string("Callback: OnError -> ") + sdk_error_to_string(error);
    owner_.set_last_error(message);
    owner_.logger_.error(message);
    std::lock_guard<std::mutex> lock(owner_.event_mutex_);
    if (owner_.event_promise_) {
        owner_.event_promise_->set_exception(std::make_exception_ptr(std::runtime_error(message)));
        owner_.event_promise_ = nullptr;
    }
}

void Fx6SdkService::Callback::OnDisconnected(CrInt32u error) {
    owner_.connected_ = false;
    { std::lock_guard<std::mutex> lock(owner_.awb_mutex_); owner_.awb_progress_.disconnect(); }
    const std::string message = std::string("Callback: OnDisconnected -> ") + sdk_error_to_string(error);
    owner_.logger_.warn(message);
    std::lock_guard<std::mutex> lock(owner_.event_mutex_);
    if (owner_.event_promise_) {
        owner_.event_promise_->set_exception(std::make_exception_ptr(std::runtime_error(message)));
        owner_.event_promise_ = nullptr;
    }
}

void Fx6SdkService::Callback::OnCompleteDownload(CrChar* filename, CrInt32u) {
    owner_.logger_.info(std::string("Callback: OnCompleteDownload -> ") + (filename ? filename : ""));
}

void Fx6SdkService::Callback::OnNotifyContentsTransfer(CrInt32u, SCRSDK::CrContentHandle, CrChar* filename) {
    owner_.logger_.info(std::string("Callback: OnNotifyContentsTransfer -> ") + (filename ? filename : ""));
}

void Fx6SdkService::Callback::OnWarning(CrInt32u warning) {
    owner_.logger_.warn(std::string("Callback: OnWarning -> ") + sdk_error_to_string(warning));
}

void Fx6SdkService::Callback::OnWarningExt(CrInt32u warning, CrInt32 param1, CrInt32 param2, CrInt32 param3) {
    owner_.logger_.warn(std::string("Callback: OnWarningExt -> ") + sdk_warning_text(warning, param1, param2, param3));
    if ((warning != SCRSDK::CrWarningExt_OperationResults && warning != SCRSDK::CrWarningExt_OperationInvalid) ||
        param1 != SCRSDK::CrSdkApi_SetDeviceProperty || param2 != SCRSDK::CrDeviceProperty_AWB) return;
    std::string message;
    const bool ok = param3 == SCRSDK::CrWarningExt_OperationResultsParam_OK;
    switch (param3) {
    case SCRSDK::CrWarningExt_OperationResultsParam_OK: message = "AWB 完了（カメラの結果通知を確認）。"; break;
    case SCRSDK::CrWarningExt_OperationResultsParam_LowBrightnessError: message = "AWB 失敗: 被写体が暗すぎます。"; break;
    case SCRSDK::CrWarningExt_OperationResultsParam_HighBrightnessError: message = "AWB 失敗: 被写体が明るすぎます。"; break;
    case SCRSDK::CrWarningExt_OperationResultsParam_ColorTempHighError: message = "AWB 失敗: 色温度が高すぎます。"; break;
    case SCRSDK::CrWarningExt_OperationResultsParam_ColorTempLowError: message = "AWB 失敗: 色温度が低すぎます。"; break;
    case SCRSDK::CrWarningExt_OperationResultsParam_TintOutOfRangeError: message = "AWB 失敗: 色かぶりが調整範囲外です。"; break;
    case SCRSDK::CrWarningExt_OperationResultsParam_PoorWhiteAreaError: message = "AWB 失敗: 白い領域が不足しています。"; break;
    case SCRSDK::CrWarningExt_OperationResultsParam_ExecuteCanceled: message = "AWB はカメラでキャンセルされました。"; break;
    default: message = "AWB 失敗: カメラの状態と WB メモリー A/B を確認してください (" + std::to_string(param3) + ")。"; break;
    }
    std::lock_guard<std::mutex> state_lock(owner_.awb_mutex_);
    owner_.awb_progress_.expire(monotonic_ms());
    owner_.awb_progress_.notify(ok, message);
}

void Fx6SdkService::Callback::OnPropertyChangedCodes(CrInt32u num, CrInt32u* codes) {
    std::lock_guard<std::mutex> lock(owner_.event_mutex_);
    for (uint32_t i = 0; i < num; ++i) {
        if (owner_.pending_property_code_ != 0 && owner_.pending_property_code_ == codes[i]) {
            owner_.pending_property_code_ = 0;
            if (owner_.event_promise_) {
                owner_.event_promise_->set_value();
                owner_.event_promise_ = nullptr;
            }
        }
    }
}

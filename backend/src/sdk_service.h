#pragma once

#include <atomic>
#include <future>
#include <mutex>
#include <optional>
#include <string>
#include <vector>

#include "logger.h"
#include "nd_state_machine.h"
#include "nd_controller.h"
#include "awb_controller.h"

#include "CameraRemote_SDK.h"
#include "IDeviceCallback.h"

struct CameraSummary {
    int index = 0;
    std::string id;
    std::string name;
    std::string model;
    std::string connection_type;
    std::string mac_address;
    std::string ip_address;
    bool ssh_support = false;
    std::string fingerprint;
};

struct PropertyView {
    bool supported = false;
    bool writable = false;
    uint64_t raw = 0;
    std::string label;
    std::vector<uint64_t> possible;
};

struct StateSnapshot {
    bool sdk_initialized = false;
    bool connected = false;
    std::string camera_id;
    std::string camera_model;
    std::string camera_name;
    std::string last_error;
    std::string log_path;

    PropertyView iris;
    PropertyView iso;
    PropertyView shutter_speed;
    PropertyView shutter_mode;
    PropertyView white_balance_mode;
    PropertyView color_temperature;
    PropertyView awb_button;
    AwbProgress awb;
    PropertyView iso_base;
    PropertyView gain_unit;
    PropertyView nd_filter;
    PropertyView nd_mode_setting;
    PropertyView nd_switching;
    PropertyView nd_optical_density;
    PropertyView nd_value;
};

class Fx6SdkService {
public:
    explicit Fx6SdkService(Logger& logger);
    ~Fx6SdkService();

    std::vector<CameraSummary> enumerate_cameras();
    bool probe_camera_ip(const std::string& address, CameraSummary& camera, std::string& error);
    bool connect_camera(const std::string& camera_id, const std::string& user_id, const std::string& password,
                        std::string& error, const std::string& expected_fingerprint = "");
    bool disconnect_camera(std::string& error);

    StateSnapshot get_state();

    bool step_iris(int delta, std::string& error);
    bool step_iso(int delta, std::string& error);
    bool step_shutter(int delta, std::string& error);
    bool run_awb(std::string& error);
    bool toggle_iso_base(std::string& error);
    bool set_iso_base(bool high, std::string& error);
    bool set_nd(bool enabled, std::string& error);
    bool toggle_nd(std::string& error);
    bool step_nd(int delta, std::string& error);

private:
    class Callback final : public SCRSDK::IDeviceCallback {
    public:
        explicit Callback(Fx6SdkService& owner) : owner_(owner) {}
        void OnConnected(SCRSDK::DeviceConnectionVersioin version) override;
        void OnError(CrInt32u error) override;
        void OnDisconnected(CrInt32u error) override;
        void OnCompleteDownload(CrChar* filename, CrInt32u type) override;
        void OnNotifyContentsTransfer(CrInt32u notify, SCRSDK::CrContentHandle contentHandle, CrChar* filename) override;
        void OnWarning(CrInt32u warning) override;
        void OnWarningExt(CrInt32u warning, CrInt32 param1, CrInt32 param2, CrInt32 param3) override;
        void OnLvPropertyChanged() override {}
        void OnLvPropertyChangedCodes(CrInt32u num, CrInt32u* codes) override {}
        void OnPropertyChanged() override {}
        void OnPropertyChangedCodes(CrInt32u num, CrInt32u* codes) override;
    private:
        Fx6SdkService& owner_;
    };

    bool ensure_initialized();
    SCRSDK::ICrCameraObjectInfo* create_ip_camera_locked(const std::string& address, std::string& error);
    void release_sdk();
    void set_last_error(const std::string& message);
    std::string build_camera_id(const SCRSDK::ICrCameraObjectInfo* info) const;

    bool set_save_info_locked(std::string& error);
    bool ensure_operational_defaults_locked(std::string& error);

    SCRSDK::CrError get_property_locked(uint32_t code, SCRSDK::CrDeviceProperty& out_property);
    bool set_property_locked(uint32_t code, uint64_t data, std::string& error, bool blocking = true, bool force = false);

    PropertyView read_property_view_locked(uint32_t code, const std::string& property_name_hint = "");
    std::vector<uint64_t> extract_possible_values(const SCRSDK::CrDeviceProperty& property) const;
    bool step_array_property_locked(uint32_t code, int delta, std::string& error);
    NdController nd_controller_locked();

    std::string label_for_property(uint32_t code, uint64_t raw) const;

    Logger& logger_;
    Callback callback_;
    std::mutex sdk_mutex_;
    bool sdk_initialized_ = false;
    std::optional<CameraSummary> direct_camera_;
    std::atomic<bool> connected_{false};
    SCRSDK::CrDeviceHandle device_handle_ = 0;
    std::string connected_camera_id_;
    std::string connected_camera_model_;
    std::string connected_camera_name_;
    std::string last_error_;
    std::mutex error_mutex_;
    std::mutex awb_mutex_;
    AwbProgress awb_progress_;

    std::mutex event_mutex_;
    std::promise<void>* event_promise_ = nullptr;
    uint32_t pending_property_code_ = 0;
};

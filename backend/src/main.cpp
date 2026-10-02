#include <csignal>
#include <cstdlib>
#include <filesystem>
#include <iostream>
#include <sstream>
#include <unistd.h>

#include "httplib.h"

#include "json_util.h"
#include "logger.h"
#include "sdk_service.h"

namespace {

std::string get_home_directory() {
    if (const char* home = std::getenv("HOME")) {
        return std::string(home);
    }
    return ".";
}

std::string executable_path(const char* argv0) {
    if (argv0 == nullptr || *argv0 == '\0') {
        return {};
    }
    std::error_code ec;
    auto absolute = std::filesystem::absolute(argv0, ec);
    if (ec) {
        return std::string(argv0);
    }
    return absolute.string();
}

std::string runtime_info_json(const std::string& log_path, const std::string& executable, const std::string& cwd) {
    std::ostringstream oss;
    oss << '{'
        << "\"service\":\"ytv-fx6-remote-control-backend\","
        << "\"appName\":" << json_util::quote(FX6_APP_NAME) << ','
        << "\"buildId\":" << json_util::quote(FX6_BUILD_ID) << ','
        << "\"version\":" << json_util::quote(FX6_VERSION) << ','
        << "\"port\":" << FX6_BACKEND_PORT << ','
        << "\"pid\":" << getpid() << ','
        << "\"logPath\":" << json_util::quote(log_path) << ','
        << "\"executablePath\":" << json_util::quote(executable) << ','
        << "\"cwd\":" << json_util::quote(cwd)
        << '}';
    return oss.str();
}

std::string success_response(const std::string& payload = "{}") {
    return std::string("{\"ok\":true,\"data\":") + payload + '}';
}

std::string error_response(const std::string& message) {
    return std::string("{\"ok\":false,\"error\":") + json_util::quote(message) + '}';
}

std::string camera_to_json(const CameraSummary& camera) {
    std::ostringstream oss;
    oss << '{'
        << "\"index\":" << camera.index << ','
        << "\"id\":" << json_util::quote(camera.id) << ','
        << "\"name\":" << json_util::quote(camera.name) << ','
        << "\"model\":" << json_util::quote(camera.model) << ','
        << "\"connectionType\":" << json_util::quote(camera.connection_type) << ','
        << "\"macAddress\":" << json_util::quote(camera.mac_address) << ','
        << "\"ipAddress\":" << json_util::quote(camera.ip_address) << ','
        << "\"sshSupport\":" << json_util::boolean(camera.ssh_support) << ','
        << "\"fingerprint\":" << json_util::quote(camera.fingerprint)
        << '}';
    return oss.str();
}

std::string property_to_json(const PropertyView& property) {
    std::ostringstream oss;
    oss << '{'
        << "\"supported\":" << json_util::boolean(property.supported) << ','
        << "\"writable\":" << json_util::boolean(property.writable) << ','
        << "\"raw\":" << property.raw << ','
        << "\"label\":" << json_util::quote(property.label) << ','
        << "\"possible\":[";
    for (size_t i = 0; i < property.possible.size(); ++i) {
        if (i != 0) oss << ',';
        oss << property.possible[i];
    }
    oss << "]}";
    return oss.str();
}

std::string state_to_json(const StateSnapshot& state) {
    std::ostringstream oss;
    oss << '{'
        << "\"sdkInitialized\":" << json_util::boolean(state.sdk_initialized) << ','
        << "\"connected\":" << json_util::boolean(state.connected) << ','
        << "\"cameraId\":" << json_util::quote(state.camera_id) << ','
        << "\"cameraModel\":" << json_util::quote(state.camera_model) << ','
        << "\"cameraName\":" << json_util::quote(state.camera_name) << ','
        << "\"lastError\":" << json_util::quote(state.last_error) << ','
        << "\"logPath\":" << json_util::quote(state.log_path) << ','
        << "\"iris\":" << property_to_json(state.iris) << ','
        << "\"iso\":" << property_to_json(state.iso) << ','
        << "\"shutterSpeed\":" << property_to_json(state.shutter_speed) << ','
        << "\"shutterMode\":" << property_to_json(state.shutter_mode) << ','
        << "\"isoBase\":" << property_to_json(state.iso_base) << ','
        << "\"gainUnit\":" << property_to_json(state.gain_unit) << ','
        << "\"ndFilter\":" << property_to_json(state.nd_filter) << ','
        << "\"ndModeSetting\":" << property_to_json(state.nd_mode_setting) << ','
        << "\"ndSwitching\":" << property_to_json(state.nd_switching) << ','
        << "\"ndOpticalDensity\":" << property_to_json(state.nd_optical_density) << ','
        << "\"ndValue\":" << property_to_json(state.nd_value)
        << '}';
    return oss.str();
}

int parse_int_param(const httplib::Request& req, const std::string& name, int fallback) {
    if (!req.has_param(name)) return fallback;
    try {
        return std::stoi(req.get_param_value(name));
    } catch (...) {
        return fallback;
    }
}

double parse_double_param(const httplib::Request& req, const std::string& name, double fallback) {
    if (!req.has_param(name)) return fallback;
    try {
        return std::stod(req.get_param_value(name));
    } catch (...) {
        return fallback;
    }
}

} // namespace

int main(int argc, char** argv) {
    const auto log_dir = std::getenv("FX6_LOG_DIR") ? std::string(std::getenv("FX6_LOG_DIR"))
        : get_home_directory() + "/Library/Logs/ytv-fx6-remote-control";
    Logger logger(log_dir);
    Fx6SdkService service(logger);
    const std::string backend_executable = executable_path(argc > 0 ? argv[0] : nullptr);
    const std::string backend_cwd = std::filesystem::current_path().string();

    httplib::Server server;
    server.new_task_queue = [] { return new httplib::ThreadPool(4); };

    server.Get("/api/health", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(
            success_response(runtime_info_json(logger.path(), backend_executable, backend_cwd)),
            "application/json; charset=utf-8"
        );
    });

    server.Get("/api/cameras", [&](const httplib::Request&, httplib::Response& res) {
        auto cameras = service.enumerate_cameras();
        std::ostringstream payload;
        payload << "[";
        for (size_t i = 0; i < cameras.size(); ++i) {
            if (i != 0) payload << ',';
            payload << camera_to_json(cameras[i]);
        }
        payload << "]";
        res.set_content(success_response(payload.str()), "application/json; charset=utf-8");
    });

    server.Post("/api/cameras/ip", [&](const httplib::Request& req, httplib::Response& res) {
        CameraSummary camera;
        std::string error;
        if (!service.probe_camera_ip(req.get_param_value("ipAddress"), camera, error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(camera_to_json(camera)), "application/json; charset=utf-8");
    });

    server.Post("/api/connect", [&](const httplib::Request& req, httplib::Response& res) {
        const auto camera_id = req.get_param_value("cameraId");
        const auto user = req.get_param_value("userId");
        const auto password = req.get_param_value("password");
        std::string error;
        if (!service.connect_camera(camera_id, user, password, error, req.get_param_value("fingerprint"))) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/disconnect", [&](const httplib::Request&, httplib::Response& res) {
        std::string error;
        if (!service.disconnect_camera(error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Get("/api/state", [&](const httplib::Request&, httplib::Response& res) {
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/iris/step", [&](const httplib::Request& req, httplib::Response& res) {
        const int delta = parse_int_param(req, "delta", 0);
        std::string error;
        if (!service.step_iris(delta, error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/iso/step", [&](const httplib::Request& req, httplib::Response& res) {
        const int delta = parse_int_param(req, "delta", 0);
        std::string error;
        if (!service.step_iso(delta, error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/shutter/step", [&](const httplib::Request& req, httplib::Response& res) {
        std::string error;
        if (!service.step_shutter(parse_int_param(req, "delta", 0), error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/nd/toggle", [&](const httplib::Request&, httplib::Response& res) {
        std::string error;
        if (!service.toggle_nd(error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/iso/base/toggle", [&](const httplib::Request&, httplib::Response& res) {
        std::string error;
        if (!service.toggle_iso_base(error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/iso/base/high", [&](const httplib::Request&, httplib::Response& res) {
        std::string error;
        if (!service.set_iso_base(true, error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/iso/base/low", [&](const httplib::Request&, httplib::Response& res) {
        std::string error;
        if (!service.set_iso_base(false, error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/nd/on", [&](const httplib::Request&, httplib::Response& res) {
        std::string error;
        if (!service.set_nd(true, error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/nd/off", [&](const httplib::Request&, httplib::Response& res) {
        std::string error;
        if (!service.set_nd(false, error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/nd/step", [&](const httplib::Request& req, httplib::Response& res) {
        const int delta = parse_int_param(req, "delta", 0);
        std::string error;
        if (!service.step_nd(delta, error)) {
            res.status = 400;
            res.set_content(error_response(error), "application/json; charset=utf-8");
            return;
        }
        res.set_content(success_response(state_to_json(service.get_state())), "application/json; charset=utf-8");
    });

    server.Post("/api/quit", [&](const httplib::Request&, httplib::Response& res) {
        std::string error;
        service.disconnect_camera(error);
        res.set_content(success_response("{}"), "application/json; charset=utf-8");
        std::thread([&server]() {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            server.stop();
        }).detach();
    });

    constexpr auto kPort = FX6_BACKEND_PORT;
    logger.info(std::string("Build ID: ") + FX6_BUILD_ID + " version " + FX6_VERSION);
    logger.info("Executable: " + backend_executable);
    logger.info("Working directory: " + backend_cwd);
    logger.info("Backend listening on http://127.0.0.1:" + std::to_string(kPort));
    std::cout << "ytv-fx6-remote-control backend listening on http://127.0.0.1:" << kPort << std::endl;
    if (!server.listen("127.0.0.1", kPort)) {
        logger.error("server.listen failed for 127.0.0.1:" + std::to_string(kPort));
        return 1;
    }
    return 0;
}

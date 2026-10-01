#include "logger.h"

#include <chrono>
#include <filesystem>
#include <iomanip>
#include <sstream>

namespace fs = std::filesystem;

Logger::Logger(const std::string& log_dir) {
    fs::create_directories(log_dir);
    file_path_ = (fs::path(log_dir) / make_filename()).string();
    stream_.open(file_path_, std::ios::out | std::ios::app);
    write("INFO", "logger started");
}

Logger::~Logger() {
    write("INFO", "logger stopped");
    if (stream_.is_open()) {
        stream_.flush();
        stream_.close();
    }
}

void Logger::info(const std::string& message) { write("INFO", message); }
void Logger::warn(const std::string& message) { write("WARN", message); }
void Logger::error(const std::string& message) { write("ERROR", message); }

std::string Logger::path() const { return file_path_; }

void Logger::write(const std::string& level, const std::string& message) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!stream_.is_open()) return;
    stream_ << '[' << make_timestamp() << "] [" << level << "] " << message << '\n';
    stream_.flush();
}

std::string Logger::make_timestamp() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(now.time_since_epoch()) % 1000;

    std::tm local_tm{};
#if defined(_WIN32)
    localtime_s(&local_tm, &time);
#else
    localtime_r(&time, &local_tm);
#endif

    std::ostringstream oss;
    oss << std::put_time(&local_tm, "%Y-%m-%d %H:%M:%S")
        << '.' << std::setfill('0') << std::setw(3) << ms.count();
    return oss.str();
}

std::string Logger::make_filename() {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);

    std::tm local_tm{};
#if defined(_WIN32)
    localtime_s(&local_tm, &time);
#else
    localtime_r(&time, &local_tm);
#endif

    std::ostringstream oss;
    oss << "fx6-operator-" << std::put_time(&local_tm, "%Y%m%d-%H%M%S") << ".txt";
    return oss.str();
}

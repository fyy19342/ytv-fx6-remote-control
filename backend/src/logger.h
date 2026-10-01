#pragma once

#include <fstream>
#include <mutex>
#include <string>

class Logger {
public:
    explicit Logger(const std::string& log_dir);
    ~Logger();

    void info(const std::string& message);
    void warn(const std::string& message);
    void error(const std::string& message);

    std::string path() const;

private:
    void write(const std::string& level, const std::string& message);
    static std::string make_timestamp();
    static std::string make_filename();

    std::string file_path_;
    std::ofstream stream_;
    mutable std::mutex mutex_;
};

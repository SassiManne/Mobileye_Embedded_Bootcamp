#pragma once
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

class Logger
{
private:
    std::shared_ptr<spdlog::logger> logger_;
    static std::unique_ptr<Logger> infoLog;
    static std::unique_ptr<Logger> errorLog;



public:

    Logger(const std::string& loggerName) {
        if (loggerName == "Info") {
            this->logger_ = spdlog::basic_logger_mt(loggerName, "Logs/Info.txt");

        }
        if (loggerName == "Error") {
            this->logger_ = spdlog::basic_logger_mt(loggerName + "exception_file", "Logs/Exception.txt");
        }
    }
    ~Logger() {
        this->logger_->flush();
        this->logger_->flush_on(spdlog::level::info);
    }

    static Logger& getInfoInstance();
    static Logger& getErrorInstance();
    static void Init();

    void setPattern(const std::string& pattern);
    void logInfo(const std::string& message);
    void logError(const std::string& message);
    void logWarn(const std::string& message);
    void logDebug(const std::string& message);
    void shutdown();

};

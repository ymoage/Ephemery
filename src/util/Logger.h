#pragma once

#include <string>
#include <fstream>
#include <mutex>

namespace Ephemery {

enum class LogLevel {
    Debug,
    Info,
    Warning,
    Error
};

class Logger {
public:
    static Logger& Instance();

    void SetLogLevel(LogLevel level);
    void SetLogFile(const std::wstring& path);

    void Debug(const std::wstring& message);
    void Info(const std::wstring& message);
    void Warning(const std::wstring& message);
    void Error(const std::wstring& message);

private:
    Logger();
    ~Logger();
    Logger(const Logger&) = delete;
    Logger& operator=(const Logger&) = delete;

    void Log(LogLevel level, const std::wstring& message);
    std::wstring GetTimestamp() const;
    std::wstring LevelToString(LogLevel level) const;

    LogLevel m_minLevel = LogLevel::Info;
    std::wofstream m_logFile;
    std::mutex m_mutex;
    bool m_consoleOutput = true;
};

// Convenience macros
#define LOG_DEBUG(msg) Ephemery::Logger::Instance().Debug(msg)
#define LOG_INFO(msg) Ephemery::Logger::Instance().Info(msg)
#define LOG_WARNING(msg) Ephemery::Logger::Instance().Warning(msg)
#define LOG_ERROR(msg) Ephemery::Logger::Instance().Error(msg)

} // namespace Ephemery

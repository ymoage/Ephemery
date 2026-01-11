#include "Logger.h"
#include <Windows.h>
#include <chrono>
#include <iomanip>
#include <sstream>

namespace Ephemery {

Logger& Logger::Instance() {
    static Logger instance;
    return instance;
}

Logger::Logger() {
}

Logger::~Logger() {
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
}

void Logger::SetLogLevel(LogLevel level) {
    std::lock_guard<std::mutex> lock(m_mutex);
    m_minLevel = level;
}

void Logger::SetLogFile(const std::wstring& path) {
    std::lock_guard<std::mutex> lock(m_mutex);
    if (m_logFile.is_open()) {
        m_logFile.close();
    }
    m_logFile.open(path, std::ios::app);
}

void Logger::Debug(const std::wstring& message) {
    Log(LogLevel::Debug, message);
}

void Logger::Info(const std::wstring& message) {
    Log(LogLevel::Info, message);
}

void Logger::Warning(const std::wstring& message) {
    Log(LogLevel::Warning, message);
}

void Logger::Error(const std::wstring& message) {
    Log(LogLevel::Error, message);
}

void Logger::Log(LogLevel level, const std::wstring& message) {
    if (level < m_minLevel) {
        return;
    }

    std::lock_guard<std::mutex> lock(m_mutex);

    std::wstringstream ss;
    ss << L"[" << GetTimestamp() << L"] "
       << L"[" << LevelToString(level) << L"] "
       << message << L"\n";

    std::wstring logLine = ss.str();

    if (m_consoleOutput) {
        OutputDebugStringW(logLine.c_str());
    }

    if (m_logFile.is_open()) {
        m_logFile << logLine;
        m_logFile.flush();
    }
}

std::wstring Logger::GetTimestamp() const {
    auto now = std::chrono::system_clock::now();
    auto time = std::chrono::system_clock::to_time_t(now);
    auto ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;

    std::tm tm;
    localtime_s(&tm, &time);

    std::wstringstream ss;
    ss << std::put_time(&tm, L"%Y-%m-%d %H:%M:%S")
       << L"." << std::setfill(L'0') << std::setw(3) << ms.count();

    return ss.str();
}

std::wstring Logger::LevelToString(LogLevel level) const {
    switch (level) {
        case LogLevel::Debug:   return L"DEBUG";
        case LogLevel::Info:    return L"INFO";
        case LogLevel::Warning: return L"WARN";
        case LogLevel::Error:   return L"ERROR";
        default:                return L"UNKNOWN";
    }
}

} // namespace Ephemery

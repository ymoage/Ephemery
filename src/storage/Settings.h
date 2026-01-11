#pragma once

#include "Types.h"
#include <string>

namespace Ephemery {

class Settings {
public:
    Settings();
    ~Settings();

    bool Load();
    bool Save() const;

    AppSettings& GetSettings();
    const AppSettings& GetSettings() const;

    void SetDefaults();

    static std::wstring GetSettingsFilePath();
    static std::wstring GetDefaultStoragePath();

private:
    bool EnsureDirectoryExists(const std::wstring& path) const;
    std::wstring ExpandEnvironmentPath(const std::wstring& path) const;

    AppSettings m_settings;
};

} // namespace Ephemery

#include "Settings.h"
#include "util/Logger.h"
#include "hotkey/HotkeyConfig.h"
#include <Windows.h>
#include <ShlObj.h>
#include <fstream>
#include <sstream>

namespace Ephemery {

Settings::Settings() {
    SetDefaults();
}

Settings::~Settings() = default;

bool Settings::Load() {
    std::wstring filePath = GetSettingsFilePath();

    std::ifstream file(filePath);
    if (!file.is_open()) {
        LOG_INFO(L"Settings file not found, using defaults");
        SetDefaults();
        return true;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();
    std::string content = buffer.str();

    // Simple JSON parsing (for MVP, no external library)
    // Format: {"version":1,"maxImages":10}

    auto findValue = [&content](const std::string& key) -> std::string {
        std::string searchKey = "\"" + key + "\":";
        size_t pos = content.find(searchKey);
        if (pos == std::string::npos) return "";

        pos += searchKey.length();
        while (pos < content.length() && (content[pos] == ' ' || content[pos] == '\t')) {
            pos++;
        }

        size_t end = pos;
        if (content[pos] == '"') {
            pos++;
            end = content.find('"', pos);
            return content.substr(pos, end - pos);
        } else {
            while (end < content.length() && content[end] != ',' && content[end] != '}') {
                end++;
            }
            return content.substr(pos, end - pos);
        }
    };

    std::string versionStr = findValue("version");
    std::string maxImagesStr = findValue("maxImages");
    std::string quoteStyleStr = findValue("quoteStyle");

    if (!versionStr.empty()) {
        m_settings.version = static_cast<uint32_t>(std::stoi(versionStr));
    }
    if (!maxImagesStr.empty()) {
        m_settings.maxImages = static_cast<uint32_t>(std::stoi(maxImagesStr));
        if (m_settings.maxImages < 1) m_settings.maxImages = 1;
        if (m_settings.maxImages > 100) m_settings.maxImages = 100;
    }
    if (!quoteStyleStr.empty()) {
        if (quoteStyleStr == "none") m_settings.quoteStyle = QuoteStyle::None;
        else if (quoteStyleStr == "double") m_settings.quoteStyle = QuoteStyle::Double;
        else if (quoteStyleStr == "single") m_settings.quoteStyle = QuoteStyle::Single;
        else if (quoteStyleStr == "backtick") m_settings.quoteStyle = QuoteStyle::Backtick;
    }

    // Load hotkey bindings (hk<id>_mods, hk<id>_key)
    for (auto& binding : m_settings.hotkeys) {
        std::string modsKey = "hk" + std::to_string(binding.id) + "_mods";
        std::string keyKey  = "hk" + std::to_string(binding.id) + "_key";
        std::string modsStr = findValue(modsKey);
        std::string keyStr  = findValue(keyKey);
        if (!modsStr.empty()) {
            binding.modifiers = static_cast<uint32_t>(std::stoi(modsStr));
        }
        if (!keyStr.empty()) {
            std::wstring wkey(keyStr.begin(), keyStr.end());
            uint32_t vk = HotkeyConfig::KeyToVirtualKey(wkey);
            if (vk != 0) {
                binding.virtualKey = vk;
            }
        }
    }

    LOG_INFO(L"Settings loaded from: " + filePath);
    return true;
}

bool Settings::Save() const {
    std::wstring filePath = GetSettingsFilePath();

    std::wstring dirPath = filePath.substr(0, filePath.rfind(L'\\'));
    if (!EnsureDirectoryExists(dirPath)) {
        LOG_ERROR(L"Failed to create settings directory");
        return false;
    }

    std::ofstream file(filePath);
    if (!file.is_open()) {
        LOG_ERROR(L"Failed to open settings file for writing");
        return false;
    }

    std::string quoteStyleStr = "none";
    switch (m_settings.quoteStyle) {
        case QuoteStyle::None: quoteStyleStr = "none"; break;
        case QuoteStyle::Double: quoteStyleStr = "double"; break;
        case QuoteStyle::Single: quoteStyleStr = "single"; break;
        case QuoteStyle::Backtick: quoteStyleStr = "backtick"; break;
    }

    file << "{\n";
    file << "  \"version\": " << m_settings.version << ",\n";
    file << "  \"maxImages\": " << m_settings.maxImages << ",\n";
    file << "  \"quoteStyle\": \"" << quoteStyleStr << "\"";

    // Save hotkey bindings (hk<id>_mods, hk<id>_key)
    for (const auto& binding : m_settings.hotkeys) {
        std::wstring wkey = HotkeyConfig::VirtualKeyToKey(binding.virtualKey);
        std::string key = wkey.empty() ? "" : std::string(1, static_cast<char>(wkey[0]));
        file << ",\n";
        file << "  \"hk" << binding.id << "_mods\": " << binding.modifiers << ",\n";
        file << "  \"hk" << binding.id << "_key\": \"" << key << "\"";
    }

    file << "\n}\n";

    LOG_INFO(L"Settings saved to: " + filePath);
    return true;
}

AppSettings& Settings::GetSettings() {
    return m_settings;
}

const AppSettings& Settings::GetSettings() const {
    return m_settings;
}

void Settings::SetDefaults() {
    m_settings.version = 1;
    m_settings.maxImages = 10;
    m_settings.storagePath = GetDefaultStoragePath();
    m_settings.hotkeys = HotkeyConfig::GetDefaultHotkeys();
}

std::wstring Settings::GetSettingsFilePath() {
    wchar_t appDataPath[MAX_PATH];
    if (SUCCEEDED(SHGetFolderPathW(nullptr, CSIDL_APPDATA, nullptr, 0, appDataPath))) {
        return std::wstring(appDataPath) + L"\\Ephemery\\settings.json";
    }
    return L".\\settings.json";
}

std::wstring Settings::GetDefaultStoragePath() {
    wchar_t tempPath[MAX_PATH];
    if (GetTempPathW(MAX_PATH, tempPath)) {
        return std::wstring(tempPath) + L"Ephemery";
    }
    return L".\\Ephemery";
}

bool Settings::EnsureDirectoryExists(const std::wstring& path) const {
    DWORD attr = GetFileAttributesW(path.c_str());
    if (attr != INVALID_FILE_ATTRIBUTES && (attr & FILE_ATTRIBUTE_DIRECTORY)) {
        return true;
    }

    return SHCreateDirectoryExW(nullptr, path.c_str(), nullptr) == ERROR_SUCCESS ||
           GetLastError() == ERROR_ALREADY_EXISTS;
}

std::wstring Settings::ExpandEnvironmentPath(const std::wstring& path) const {
    wchar_t expanded[MAX_PATH];
    if (ExpandEnvironmentStringsW(path.c_str(), expanded, MAX_PATH)) {
        return expanded;
    }
    return path;
}

} // namespace Ephemery

#include "HotkeyConfig.h"
#include <algorithm>
#include <cwctype>

namespace Ephemery {

std::array<HotkeyBinding, 4> HotkeyConfig::GetDefaultHotkeys() {
    return {
        GetDefaultHotkey(HotkeyAction::CaptureScreen),
        GetDefaultHotkey(HotkeyAction::SaveClipboard),
        GetDefaultHotkey(HotkeyAction::PastePaths),
        GetDefaultHotkey(HotkeyAction::ClearPaths)
    };
}

HotkeyBinding HotkeyConfig::GetDefaultHotkey(HotkeyAction action) {
    HotkeyBinding binding;
    binding.action = action;
    binding.modifiers = MOD_CONTROL | MOD_ALT;  // Ctrl+Alt 系（VS Code等との競合回避）

    switch (action) {
        case HotkeyAction::CaptureScreen:
            binding.virtualKey = 'S';  // Ctrl+Alt+S
            binding.id = HOTKEY_ID_CAPTURE_SCREEN;
            break;
        case HotkeyAction::SaveClipboard:
            binding.virtualKey = 'C';  // Ctrl+Alt+C
            binding.id = HOTKEY_ID_SAVE_CLIPBOARD;
            break;
        case HotkeyAction::PastePaths:
            binding.virtualKey = 'V';  // Ctrl+Alt+V
            binding.id = HOTKEY_ID_PASTE_PATHS;
            break;
        case HotkeyAction::ClearPaths:
            binding.virtualKey = 'X';  // Ctrl+Alt+X
            binding.id = HOTKEY_ID_CLEAR_PATHS;
            break;
    }

    return binding;
}

uint32_t HotkeyConfig::ModifiersToWin32(const std::vector<std::wstring>& modifiers) {
    uint32_t result = 0;
    for (const auto& mod : modifiers) {
        if (mod == L"Alt") result |= MOD_ALT;
        else if (mod == L"Ctrl") result |= MOD_CONTROL;
        else if (mod == L"Shift") result |= MOD_SHIFT;
        else if (mod == L"Win") result |= MOD_WIN;
    }
    return result;
}

std::vector<std::wstring> HotkeyConfig::Win32ToModifiers(uint32_t modifiers) {
    std::vector<std::wstring> result;
    if (modifiers & MOD_WIN) result.push_back(L"Win");
    if (modifiers & MOD_CONTROL) result.push_back(L"Ctrl");
    if (modifiers & MOD_ALT) result.push_back(L"Alt");
    if (modifiers & MOD_SHIFT) result.push_back(L"Shift");
    return result;
}

uint32_t HotkeyConfig::KeyToVirtualKey(const std::wstring& key) {
    if (key.empty()) return 0;

    // F-keys: "F1"-"F12"
    if ((key[0] == L'F' || key[0] == L'f') && key.length() >= 2) {
        int n = 0;
        for (size_t i = 1; i < key.length(); ++i) {
            if (key[i] < L'0' || key[i] > L'9') { n = 0; break; }
            n = n * 10 + (key[i] - L'0');
        }
        if (n >= 1 && n <= 12) {
            return static_cast<uint32_t>(VK_F1 + n - 1);
        }
    }

    wchar_t ch = std::towupper(key[0]);
    if (ch >= L'A' && ch <= L'Z') {
        return static_cast<uint32_t>(ch);
    }
    if (ch >= L'0' && ch <= L'9') {
        return static_cast<uint32_t>(ch);
    }

    return 0;
}

std::wstring HotkeyConfig::VirtualKeyToKey(uint32_t vk) {
    if (vk >= 'A' && vk <= 'Z') {
        return std::wstring(1, static_cast<wchar_t>(vk));
    }
    if (vk >= '0' && vk <= '9') {
        return std::wstring(1, static_cast<wchar_t>(vk));
    }
    if (vk >= VK_F1 && vk <= VK_F12) {
        return L"F" + std::to_wstring(vk - VK_F1 + 1);
    }
    return L"";
}

bool HotkeyConfig::IsValidHotkey(const HotkeyBinding& binding) {
    if (binding.modifiers == 0) {
        return false;
    }
    if (binding.virtualKey == 0) {
        return false;
    }
    if (binding.virtualKey < 0x01 || binding.virtualKey > 0xFE) {
        return false;
    }
    return true;
}

std::wstring HotkeyConfig::HotkeyToString(const HotkeyBinding& binding) {
    std::wstring result;

    auto modifiers = Win32ToModifiers(binding.modifiers);
    for (const auto& mod : modifiers) {
        if (!result.empty()) result += L"+";
        result += mod;
    }

    std::wstring keyStr = VirtualKeyToKey(binding.virtualKey);
    if (!keyStr.empty()) {
        if (!result.empty()) result += L"+";
        result += keyStr;
    }

    return result;
}

} // namespace Ephemery

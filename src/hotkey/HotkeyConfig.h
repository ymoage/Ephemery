#pragma once

#include <vector>
#include <array>
#include <Windows.h>
#include "storage/Types.h"

namespace Ephemery {

class HotkeyConfig {
public:
    static constexpr int HOTKEY_ID_CAPTURE_SCREEN = 1;
    static constexpr int HOTKEY_ID_SAVE_CLIPBOARD = 2;
    static constexpr int HOTKEY_ID_PASTE_PATHS = 3;

    static std::array<HotkeyBinding, 3> GetDefaultHotkeys();
    static HotkeyBinding GetDefaultHotkey(HotkeyAction action);

    static uint32_t ModifiersToWin32(const std::vector<std::wstring>& modifiers);
    static std::vector<std::wstring> Win32ToModifiers(uint32_t modifiers);

    static uint32_t KeyToVirtualKey(const std::wstring& key);
    static std::wstring VirtualKeyToKey(uint32_t vk);

    static bool IsValidHotkey(const HotkeyBinding& binding);
    static std::wstring HotkeyToString(const HotkeyBinding& binding);
};

} // namespace Ephemery

#pragma once

#include "storage/Types.h"
#include <Windows.h>
#include <functional>
#include <array>

namespace Ephemery {

using HotkeyCallback = std::function<void(HotkeyAction)>;

class HotkeyManager {
public:
    HotkeyManager();
    ~HotkeyManager();

    bool Initialize(HWND hwnd);
    void Shutdown();

    bool RegisterHotkeys(const std::array<HotkeyBinding, 3>& bindings);
    void UnregisterAllHotkeys();

    bool UpdateHotkey(const HotkeyBinding& binding);

    void SetCallback(HotkeyCallback callback);
    void HandleHotkey(int id);

    bool IsHotkeyAvailable(uint32_t modifiers, uint32_t virtualKey) const;

private:
    HWND m_hwnd = nullptr;
    HotkeyCallback m_callback;
    std::array<HotkeyBinding, 3> m_bindings;
    bool m_registered = false;
};

} // namespace Ephemery

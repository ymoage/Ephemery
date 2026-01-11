#include "HotkeyManager.h"
#include "HotkeyConfig.h"
#include "util/Logger.h"

namespace Ephemery {

HotkeyManager::HotkeyManager() = default;

HotkeyManager::~HotkeyManager() {
    Shutdown();
}

bool HotkeyManager::Initialize(HWND hwnd) {
    m_hwnd = hwnd;
    LOG_INFO(L"HotkeyManager initialized");
    return true;
}

void HotkeyManager::Shutdown() {
    UnregisterAllHotkeys();
    m_hwnd = nullptr;
}

bool HotkeyManager::RegisterHotkeys(const std::array<HotkeyBinding, 3>& bindings) {
    if (m_hwnd == nullptr) {
        LOG_ERROR(L"HotkeyManager not initialized");
        return false;
    }

    UnregisterAllHotkeys();

    m_bindings = bindings;
    bool allSuccess = true;

    for (const auto& binding : m_bindings) {
        if (!RegisterHotKey(m_hwnd, binding.id, binding.modifiers | MOD_NOREPEAT, binding.virtualKey)) {
            DWORD error = GetLastError();
            LOG_ERROR(L"Failed to register hotkey: " + HotkeyConfig::HotkeyToString(binding) +
                     L" (error: " + std::to_wstring(error) + L")");
            allSuccess = false;
        } else {
            LOG_INFO(L"Registered hotkey: " + HotkeyConfig::HotkeyToString(binding));
        }
    }

    m_registered = true;
    return allSuccess;
}

void HotkeyManager::UnregisterAllHotkeys() {
    if (m_hwnd == nullptr || !m_registered) {
        return;
    }

    for (const auto& binding : m_bindings) {
        UnregisterHotKey(m_hwnd, binding.id);
    }

    m_registered = false;
    LOG_INFO(L"Unregistered all hotkeys");
}

bool HotkeyManager::UpdateHotkey(const HotkeyBinding& binding) {
    if (m_hwnd == nullptr) {
        return false;
    }

    UnregisterHotKey(m_hwnd, binding.id);

    if (!RegisterHotKey(m_hwnd, binding.id, binding.modifiers | MOD_NOREPEAT, binding.virtualKey)) {
        LOG_ERROR(L"Failed to update hotkey: " + HotkeyConfig::HotkeyToString(binding));
        return false;
    }

    for (auto& b : m_bindings) {
        if (b.action == binding.action) {
            b = binding;
            break;
        }
    }

    LOG_INFO(L"Updated hotkey: " + HotkeyConfig::HotkeyToString(binding));
    return true;
}

void HotkeyManager::SetCallback(HotkeyCallback callback) {
    m_callback = std::move(callback);
}

void HotkeyManager::HandleHotkey(int id) {
    if (!m_callback) {
        return;
    }

    for (const auto& binding : m_bindings) {
        if (binding.id == id) {
            LOG_DEBUG(L"Hotkey triggered: " + HotkeyConfig::HotkeyToString(binding));
            m_callback(binding.action);
            return;
        }
    }
}

bool HotkeyManager::IsHotkeyAvailable(uint32_t modifiers, uint32_t virtualKey) const {
    if (m_hwnd == nullptr) {
        return false;
    }

    if (RegisterHotKey(m_hwnd, 9999, modifiers, virtualKey)) {
        UnregisterHotKey(m_hwnd, 9999);
        return true;
    }

    return false;
}

} // namespace Ephemery

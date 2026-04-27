#pragma once

#include "storage/Types.h"
#include "hotkey/HotkeyConfig.h"
#include <Windows.h>
#include <CommCtrl.h>
#include <array>
#include <functional>

namespace Ephemery {

using SettingsChangedCallback = std::function<void(const AppSettings&)>;

class SettingsDialog {
public:
    SettingsDialog();
    ~SettingsDialog();

    bool Show(HWND parent, AppSettings& settings, SettingsChangedCallback onChanged = nullptr);

private:
    static LRESULT CALLBACK DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool CreateDialogControls(HWND hwnd);
    void OnInitDialog(HWND hwnd);
    void OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam);
    void OnHotkeyChange(HWND hwnd, int controlId);
    bool OnOK(HWND hwnd);
    void OnCancel(HWND hwnd);

    void UpdateHotkeyDisplay(HWND hwnd, int controlId, const HotkeyBinding& binding);
    bool ValidateSettings(HWND hwnd);
    bool IsHotkeyConflict(const HotkeyBinding& binding, HotkeyAction excludeAction);

    static LRESULT CALLBACK HotkeyEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    AppSettings* m_settings = nullptr;
    AppSettings m_tempSettings;
    SettingsChangedCallback m_onChanged;

    HWND m_hwndCaptureHotkey = nullptr;
    HWND m_hwndClipboardHotkey = nullptr;
    HWND m_hwndPasteHotkey = nullptr;
    HWND m_hwndClearHotkey = nullptr;
    HWND m_hwndMaxImages = nullptr;
    HWND m_hwndQuoteNone = nullptr;
    HWND m_hwndQuoteDouble = nullptr;
    HWND m_hwndQuoteSingle = nullptr;
    HWND m_hwndQuoteBacktick = nullptr;

    std::array<HotkeyBinding, 4> m_tempHotkeys;
    int m_editingHotkeyIndex = -1;

    bool m_closed = false;
    INT_PTR m_result = IDCANCEL;

    static constexpr int IDC_CAPTURE_HOTKEY = 101;
    static constexpr int IDC_CLIPBOARD_HOTKEY = 102;
    static constexpr int IDC_PASTE_HOTKEY = 103;
    static constexpr int IDC_MAX_IMAGES = 104;
    static constexpr int IDC_MAX_IMAGES_SPIN = 105;
    static constexpr int IDC_QUOTE_NONE = 106;
    static constexpr int IDC_QUOTE_DOUBLE = 107;
    static constexpr int IDC_QUOTE_SINGLE = 108;
    static constexpr int IDC_QUOTE_BACKTICK = 109;
    static constexpr int IDC_CLEAR_HOTKEY = 110;

    static constexpr int DIALOG_WIDTH = 400;
    static constexpr int DIALOG_HEIGHT = 330;
};

} // namespace Ephemery

#include "SettingsDialog.h"
#include "util/Logger.h"
#include <windowsx.h>

#pragma comment(lib, "comctl32.lib")

namespace Ephemery {

static SettingsDialog* s_currentDialog = nullptr;
static WNDPROC s_originalEditProc = nullptr;

SettingsDialog::SettingsDialog() = default;

SettingsDialog::~SettingsDialog() = default;

bool SettingsDialog::Show(HWND parent, AppSettings& settings, SettingsChangedCallback onChanged) {
    LOG_INFO(L"Opening settings dialog");

    m_settings = &settings;
    m_tempSettings = settings;
    m_tempHotkeys = settings.hotkeys;
    m_onChanged = std::move(onChanged);
    m_closed = false;
    m_result = IDCANCEL;
    s_currentDialog = this;

    // Initialize common controls
    INITCOMMONCONTROLSEX icex;
    icex.dwSize = sizeof(INITCOMMONCONTROLSEX);
    icex.dwICC = ICC_UPDOWN_CLASS;
    InitCommonControlsEx(&icex);

    // Register dialog class
    const wchar_t* className = L"EphemerySettingsDialog";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = DialogProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_BTNFACE + 1);
    wc.lpszClassName = className;

    RegisterClassExW(&wc);

    // Calculate window size from desired client area
    RECT clientRect = {0, 0, DIALOG_WIDTH, DIALOG_HEIGHT};
    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    DWORD exStyle = WS_EX_DLGMODALFRAME | WS_EX_TOPMOST;
    AdjustWindowRectEx(&clientRect, style, FALSE, exStyle);
    int windowWidth = clientRect.right - clientRect.left;
    int windowHeight = clientRect.bottom - clientRect.top;

    // Calculate dialog position (center on parent)
    RECT parentRect;
    GetWindowRect(parent ? parent : GetDesktopWindow(), &parentRect);
    int x = parentRect.left + (parentRect.right - parentRect.left - windowWidth) / 2;
    int y = parentRect.top + (parentRect.bottom - parentRect.top - windowHeight) / 2;

    // Create dialog window
    HWND hwnd = CreateWindowExW(
        exStyle,
        className,
        L"Ephemery 設定",
        style,
        x, y, windowWidth, windowHeight,
        parent,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (!hwnd) {
        LOG_ERROR(L"Failed to create settings dialog");
        s_currentDialog = nullptr;
        return false;
    }

    // Create controls
    if (!CreateDialogControls(hwnd)) {
        DestroyWindow(hwnd);
        s_currentDialog = nullptr;
        return false;
    }

    OnInitDialog(hwnd);

    ShowWindow(hwnd, SW_SHOW);
    UpdateWindow(hwnd);

    // Disable parent
    if (parent) {
        EnableWindow(parent, FALSE);
    }

    // Modal message loop (use flag instead of PostQuitMessage)
    MSG msg;
    while (!m_closed && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    // Re-enable parent
    if (parent) {
        EnableWindow(parent, TRUE);
        SetForegroundWindow(parent);
    }

    s_currentDialog = nullptr;
    LOG_INFO(L"Settings dialog closed with result: " + std::to_wstring(m_result));
    return m_result == IDOK;
}

bool SettingsDialog::CreateDialogControls(HWND hwnd) {
    HINSTANCE hInst = GetModuleHandle(nullptr);
    HFONT hFont = reinterpret_cast<HFONT>(GetStockObject(DEFAULT_GUI_FONT));

    int y = 15;
    int labelWidth = 120;
    int editWidth = 180;
    int editHeight = 22;
    int rowHeight = 30;
    int leftMargin = 15;

    // Capture hotkey label and edit
    HWND hwndLabel = CreateWindowExW(0, L"STATIC", L"画面キャプチャ:",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        leftMargin, y + 3, labelWidth, 18, hwnd, nullptr, hInst, nullptr);
    SendMessage(hwndLabel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    m_hwndCaptureHotkey = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_READONLY,
        leftMargin + labelWidth + 10, y, editWidth, editHeight,
        hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_CAPTURE_HOTKEY)), hInst, nullptr);
    SendMessage(m_hwndCaptureHotkey, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    s_originalEditProc = reinterpret_cast<WNDPROC>(SetWindowLongPtrW(m_hwndCaptureHotkey, GWLP_WNDPROC,
        reinterpret_cast<LONG_PTR>(HotkeyEditProc)));

    y += rowHeight;

    // Clipboard hotkey label and edit
    hwndLabel = CreateWindowExW(0, L"STATIC", L"クリップボード保存:",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        leftMargin, y + 3, labelWidth, 18, hwnd, nullptr, hInst, nullptr);
    SendMessage(hwndLabel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    m_hwndClipboardHotkey = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_READONLY,
        leftMargin + labelWidth + 10, y, editWidth, editHeight,
        hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_CLIPBOARD_HOTKEY)), hInst, nullptr);
    SendMessage(m_hwndClipboardHotkey, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SetWindowLongPtrW(m_hwndClipboardHotkey, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HotkeyEditProc));

    y += rowHeight;

    // Paste hotkey label and edit
    hwndLabel = CreateWindowExW(0, L"STATIC", L"パス貼り付け:",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        leftMargin, y + 3, labelWidth, 18, hwnd, nullptr, hInst, nullptr);
    SendMessage(hwndLabel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    m_hwndPasteHotkey = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_READONLY,
        leftMargin + labelWidth + 10, y, editWidth, editHeight,
        hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_PASTE_HOTKEY)), hInst, nullptr);
    SendMessage(m_hwndPasteHotkey, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);
    SetWindowLongPtrW(m_hwndPasteHotkey, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(HotkeyEditProc));

    y += rowHeight + 10;

    // Max images label and spin control
    hwndLabel = CreateWindowExW(0, L"STATIC", L"保持上限:",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        leftMargin, y + 3, labelWidth, 18, hwnd, nullptr, hInst, nullptr);
    SendMessage(hwndLabel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    m_hwndMaxImages = CreateWindowExW(WS_EX_CLIENTEDGE, L"EDIT", L"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_NUMBER | ES_RIGHT,
        leftMargin + labelWidth + 10, y, 60, editHeight,
        hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_MAX_IMAGES)), hInst, nullptr);
    SendMessage(m_hwndMaxImages, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    HWND hwndSpin = CreateWindowExW(0, UPDOWN_CLASSW, L"",
        WS_CHILD | WS_VISIBLE | UDS_SETBUDDYINT | UDS_ALIGNRIGHT | UDS_ARROWKEYS | UDS_NOTHOUSANDS,
        0, 0, 0, 0,
        hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_MAX_IMAGES_SPIN)), hInst, nullptr);
    SendMessage(hwndSpin, UDM_SETBUDDY, reinterpret_cast<WPARAM>(m_hwndMaxImages), 0);
    SendMessage(hwndSpin, UDM_SETRANGE32, 1, 100);

    hwndLabel = CreateWindowExW(0, L"STATIC", L"枚 (1-100)",
        WS_CHILD | WS_VISIBLE,
        leftMargin + labelWidth + 80, y + 3, 80, 18, hwnd, nullptr, hInst, nullptr);
    SendMessage(hwndLabel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    y += rowHeight + 10;

    // Quote style label and radio buttons
    hwndLabel = CreateWindowExW(0, L"STATIC", L"引用符:",
        WS_CHILD | WS_VISIBLE | SS_RIGHT,
        leftMargin, y + 3, labelWidth, 18, hwnd, nullptr, hInst, nullptr);
    SendMessage(hwndLabel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    int radioX = leftMargin + labelWidth + 10;
    m_hwndQuoteNone = CreateWindowExW(0, L"BUTTON", L"なし",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON | WS_GROUP,
        radioX, y, 60, 20, hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_QUOTE_NONE)), hInst, nullptr);
    SendMessage(m_hwndQuoteNone, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    m_hwndQuoteDouble = CreateWindowExW(0, L"BUTTON", L"\"",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON,
        radioX + 65, y, 40, 20, hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_QUOTE_DOUBLE)), hInst, nullptr);
    SendMessage(m_hwndQuoteDouble, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    m_hwndQuoteSingle = CreateWindowExW(0, L"BUTTON", L"'",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON,
        radioX + 110, y, 40, 20, hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_QUOTE_SINGLE)), hInst, nullptr);
    SendMessage(m_hwndQuoteSingle, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    m_hwndQuoteBacktick = CreateWindowExW(0, L"BUTTON", L"`",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_AUTORADIOBUTTON,
        radioX + 155, y, 40, 20, hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_QUOTE_BACKTICK)), hInst, nullptr);
    SendMessage(m_hwndQuoteBacktick, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    y += rowHeight + 10;

    // OK and Cancel buttons
    int buttonWidth = 80;
    int buttonHeight = 28;
    int buttonY = DIALOG_HEIGHT - 55;

    HWND hwndOK = CreateWindowExW(0, L"BUTTON", L"OK",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
        DIALOG_WIDTH / 2 - buttonWidth - 10, buttonY, buttonWidth, buttonHeight,
        hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDOK)), hInst, nullptr);
    SendMessage(hwndOK, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    HWND hwndCancel = CreateWindowExW(0, L"BUTTON", L"キャンセル",
        WS_CHILD | WS_VISIBLE | WS_TABSTOP,
        DIALOG_WIDTH / 2 + 10, buttonY, buttonWidth, buttonHeight,
        hwnd, reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDCANCEL)), hInst, nullptr);
    SendMessage(hwndCancel, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), TRUE);

    return true;
}

void SettingsDialog::OnInitDialog(HWND hwnd) {
    // Set hotkey displays
    UpdateHotkeyDisplay(hwnd, IDC_CAPTURE_HOTKEY, m_tempHotkeys[0]);
    UpdateHotkeyDisplay(hwnd, IDC_CLIPBOARD_HOTKEY, m_tempHotkeys[1]);
    UpdateHotkeyDisplay(hwnd, IDC_PASTE_HOTKEY, m_tempHotkeys[2]);

    // Set max images
    SetDlgItemInt(hwnd, IDC_MAX_IMAGES, m_tempSettings.maxImages, FALSE);

    // Set quote style
    int quoteRadioId = IDC_QUOTE_NONE;
    switch (m_tempSettings.quoteStyle) {
        case QuoteStyle::None: quoteRadioId = IDC_QUOTE_NONE; break;
        case QuoteStyle::Double: quoteRadioId = IDC_QUOTE_DOUBLE; break;
        case QuoteStyle::Single: quoteRadioId = IDC_QUOTE_SINGLE; break;
        case QuoteStyle::Backtick: quoteRadioId = IDC_QUOTE_BACKTICK; break;
    }
    CheckRadioButton(hwnd, IDC_QUOTE_NONE, IDC_QUOTE_BACKTICK, quoteRadioId);

    LOG_INFO(L"Settings dialog initialized");
}

LRESULT CALLBACK SettingsDialog::DialogProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    SettingsDialog* dialog = nullptr;

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        dialog = static_cast<SettingsDialog*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(dialog));
    } else {
        dialog = reinterpret_cast<SettingsDialog*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (dialog) {
        return dialog->HandleMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT SettingsDialog::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_COMMAND:
            OnCommand(hwnd, wParam, lParam);
            return TRUE;

        case WM_CLOSE:
            OnCancel(hwnd);
            return TRUE;

        case WM_DESTROY:
            // Do NOT call PostQuitMessage here - it would terminate the main app
            return TRUE;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void SettingsDialog::OnCommand(HWND hwnd, WPARAM wParam, LPARAM lParam) {
    int id = LOWORD(wParam);
    int code = HIWORD(wParam);

    switch (id) {
        case IDOK:
            if (OnOK(hwnd)) {
                LOG_INFO(L"Settings saved, closing dialog");
                m_result = IDOK;
                m_closed = true;
                DestroyWindow(hwnd);
            }
            break;

        case IDCANCEL:
            OnCancel(hwnd);
            break;
    }
}

void SettingsDialog::OnHotkeyChange(HWND hwnd, int controlId) {
    int index = -1;
    switch (controlId) {
        case IDC_CAPTURE_HOTKEY: index = 0; break;
        case IDC_CLIPBOARD_HOTKEY: index = 1; break;
        case IDC_PASTE_HOTKEY: index = 2; break;
    }

    if (index >= 0) {
        UpdateHotkeyDisplay(hwnd, controlId, m_tempHotkeys[index]);
    }
}

bool SettingsDialog::OnOK(HWND hwnd) {
    if (!ValidateSettings(hwnd)) {
        return false;
    }

    // Get max images
    BOOL success;
    UINT maxImages = GetDlgItemInt(hwnd, IDC_MAX_IMAGES, &success, FALSE);
    if (!success || maxImages < 1 || maxImages > 100) {
        MessageBoxW(hwnd, L"保持上限は1-100の範囲で入力してください。", L"入力エラー", MB_OK | MB_ICONWARNING);
        SetFocus(m_hwndMaxImages);
        return false;
    }

    // Get quote style
    QuoteStyle quoteStyle = QuoteStyle::None;
    if (IsDlgButtonChecked(hwnd, IDC_QUOTE_DOUBLE) == BST_CHECKED) {
        quoteStyle = QuoteStyle::Double;
        LOG_INFO(L"Quote style: Double");
    } else if (IsDlgButtonChecked(hwnd, IDC_QUOTE_SINGLE) == BST_CHECKED) {
        quoteStyle = QuoteStyle::Single;
        LOG_INFO(L"Quote style: Single");
    } else if (IsDlgButtonChecked(hwnd, IDC_QUOTE_BACKTICK) == BST_CHECKED) {
        quoteStyle = QuoteStyle::Backtick;
        LOG_INFO(L"Quote style: Backtick");
    } else {
        LOG_INFO(L"Quote style: None");
    }

    // Update settings
    m_settings->hotkeys = m_tempHotkeys;
    m_settings->maxImages = maxImages;
    m_settings->quoteStyle = quoteStyle;

    // Notify callback
    if (m_onChanged) {
        m_onChanged(*m_settings);
    }

    LOG_INFO(L"Settings saved");
    return true;
}

void SettingsDialog::OnCancel(HWND hwnd) {
    LOG_INFO(L"Settings cancelled");
    m_result = IDCANCEL;
    m_closed = true;
    DestroyWindow(hwnd);
}

void SettingsDialog::UpdateHotkeyDisplay(HWND hwnd, int controlId, const HotkeyBinding& binding) {
    std::wstring text = HotkeyConfig::HotkeyToString(binding);
    SetDlgItemTextW(hwnd, controlId, text.c_str());
}

bool SettingsDialog::ValidateSettings(HWND hwnd) {
    // Check for hotkey conflicts
    for (int i = 0; i < 3; ++i) {
        if (IsHotkeyConflict(m_tempHotkeys[i], m_tempHotkeys[i].action)) {
            std::wstring msg = L"ホットキー「" + HotkeyConfig::HotkeyToString(m_tempHotkeys[i]) +
                              L"」は他の機能と重複しています。";
            MessageBoxW(hwnd, msg.c_str(), L"ホットキー競合", MB_OK | MB_ICONWARNING);
            return false;
        }
    }

    return true;
}

bool SettingsDialog::IsHotkeyConflict(const HotkeyBinding& binding, HotkeyAction excludeAction) {
    for (const auto& other : m_tempHotkeys) {
        if (other.action != excludeAction &&
            other.modifiers == binding.modifiers &&
            other.virtualKey == binding.virtualKey) {
            return true;
        }
    }
    return false;
}

LRESULT CALLBACK SettingsDialog::HotkeyEditProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    if (msg == WM_KEYDOWN || msg == WM_SYSKEYDOWN) {
        if (!s_currentDialog) {
            return CallWindowProcW(s_originalEditProc, hwnd, msg, wParam, lParam);
        }

        UINT vk = static_cast<UINT>(wParam);

        // Ignore modifier-only keys
        if (vk == VK_SHIFT || vk == VK_CONTROL || vk == VK_MENU || vk == VK_LWIN || vk == VK_RWIN) {
            return 0;
        }

        // Get current modifiers
        uint32_t modifiers = 0;
        if (GetKeyState(VK_SHIFT) & 0x8000) modifiers |= MOD_SHIFT;
        if (GetKeyState(VK_CONTROL) & 0x8000) modifiers |= MOD_CONTROL;
        if (GetKeyState(VK_MENU) & 0x8000) modifiers |= MOD_ALT;
        if ((GetKeyState(VK_LWIN) & 0x8000) || (GetKeyState(VK_RWIN) & 0x8000)) modifiers |= MOD_WIN;

        // Require at least one modifier
        if (modifiers == 0) {
            return 0;
        }

        // Determine which hotkey is being edited
        int controlId = GetDlgCtrlID(hwnd);
        int index = -1;
        switch (controlId) {
            case IDC_CAPTURE_HOTKEY: index = 0; break;
            case IDC_CLIPBOARD_HOTKEY: index = 1; break;
            case IDC_PASTE_HOTKEY: index = 2; break;
        }

        if (index >= 0) {
            s_currentDialog->m_tempHotkeys[index].modifiers = modifiers;
            s_currentDialog->m_tempHotkeys[index].virtualKey = vk;

            HWND hwndParent = GetParent(hwnd);
            s_currentDialog->UpdateHotkeyDisplay(hwndParent, controlId, s_currentDialog->m_tempHotkeys[index]);
        }

        return 0;
    }

    if (msg == WM_CHAR || msg == WM_SYSCHAR) {
        return 0; // Suppress character input
    }

    return CallWindowProcW(s_originalEditProc, hwnd, msg, wParam, lParam);
}

} // namespace Ephemery

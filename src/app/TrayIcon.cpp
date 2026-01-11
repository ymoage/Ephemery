#include "TrayIcon.h"
#include "util/Logger.h"

namespace Ephemery {

TrayIcon::TrayIcon() = default;

TrayIcon::~TrayIcon() {
    Shutdown();
}

bool TrayIcon::Initialize(HWND hwnd, UINT callbackMessage) {
    if (m_initialized) {
        return true;
    }

    if (!CreateTrayIcon(hwnd, callbackMessage)) {
        return false;
    }

    m_initialized = true;
    LOG_INFO(L"TrayIcon initialized");
    return true;
}

void TrayIcon::Shutdown() {
    if (m_initialized) {
        Shell_NotifyIconW(NIM_DELETE, &m_nid);
        m_initialized = false;
        LOG_INFO(L"TrayIcon shutdown");
    }
}

bool TrayIcon::CreateTrayIcon(HWND hwnd, UINT callbackMessage) {
    ZeroMemory(&m_nid, sizeof(m_nid));
    m_nid.cbSize = sizeof(NOTIFYICONDATAW);
    m_nid.hWnd = hwnd;
    m_nid.uID = 1;
    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
    m_nid.uCallbackMessage = callbackMessage;

    // Try to load custom icon first, fall back to system icon
    HINSTANCE hInst = GetModuleHandle(nullptr);
    m_nid.hIcon = LoadIconW(hInst, L"IDI_APP");
    if (!m_nid.hIcon) {
        m_nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(1));
    }
    if (!m_nid.hIcon) {
        // Use system default icon as fallback
        m_nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);
        LOG_INFO(L"Using system default icon");
    }

    if (!m_nid.hIcon) {
        LOG_ERROR(L"Failed to load any icon");
        return false;
    }

    wcscpy_s(m_nid.szTip, L"Ephemery - Capture to Path");

    if (!Shell_NotifyIconW(NIM_ADD, &m_nid)) {
        DWORD error = GetLastError();
        LOG_ERROR(L"Failed to add tray icon, error: " + std::to_wstring(error));
        return false;
    }

    // Set version for modern balloon support
    m_nid.uVersion = NOTIFYICON_VERSION_4;
    Shell_NotifyIconW(NIM_SETVERSION, &m_nid);

    return true;
}

void TrayIcon::ShowContextMenu(HWND hwnd) {
    POINT pt;
    GetCursorPos(&pt);

    HMENU hMenu = CreateContextMenu();
    if (hMenu == nullptr) {
        return;
    }

    SetForegroundWindow(hwnd);

    UINT cmd = TrackPopupMenu(hMenu,
        TPM_RETURNCMD | TPM_RIGHTBUTTON | TPM_NONOTIFY,
        pt.x, pt.y, 0, hwnd, nullptr);

    DestroyMenu(hMenu);

    if (cmd != 0 && m_callback) {
        m_callback(static_cast<TrayMenuCommand>(cmd));
    }
}

void TrayIcon::SetCallback(TrayMenuCallback callback) {
    m_callback = std::move(callback);
}

void TrayIcon::ShowBalloon(const std::wstring& title, const std::wstring& message) {
    if (!m_initialized) {
        return;
    }

    m_nid.uFlags = NIF_INFO;
    m_nid.dwInfoFlags = NIIF_INFO;

    wcsncpy_s(m_nid.szInfoTitle, title.c_str(), _TRUNCATE);
    wcsncpy_s(m_nid.szInfo, message.c_str(), _TRUNCATE);

    Shell_NotifyIconW(NIM_MODIFY, &m_nid);

    m_nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
}

HMENU TrayIcon::CreateContextMenu() {
    HMENU hMenu = CreatePopupMenu();
    if (hMenu == nullptr) {
        return nullptr;
    }

    AppendMenuW(hMenu, MF_STRING, static_cast<UINT_PTR>(TrayMenuCommand::OpenFolder),
                L"フォルダを開く(&O)");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, static_cast<UINT_PTR>(TrayMenuCommand::Settings),
                L"設定(&S)...");
    AppendMenuW(hMenu, MF_SEPARATOR, 0, nullptr);
    AppendMenuW(hMenu, MF_STRING, static_cast<UINT_PTR>(TrayMenuCommand::Exit),
                L"終了(&X)");

    return hMenu;
}

} // namespace Ephemery

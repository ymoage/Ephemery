#pragma once

#include <Windows.h>
#include <shellapi.h>
#include <functional>
#include <string>

namespace Ephemery {

enum class TrayMenuCommand {
    OpenFolder = 1001,
    Settings = 1002,
    Exit = 1003
};

using TrayMenuCallback = std::function<void(TrayMenuCommand)>;

class TrayIcon {
public:
    TrayIcon();
    ~TrayIcon();

    bool Initialize(HWND hwnd, UINT callbackMessage);
    void Shutdown();

    void ShowContextMenu(HWND hwnd);
    void SetCallback(TrayMenuCallback callback);

    void ShowBalloon(const std::wstring& title, const std::wstring& message);

    static constexpr UINT WM_TRAYICON = WM_USER + 1;

private:
    bool CreateTrayIcon(HWND hwnd, UINT callbackMessage);
    HMENU CreateContextMenu();

    NOTIFYICONDATAW m_nid = {};
    bool m_initialized = false;
    TrayMenuCallback m_callback;
};

} // namespace Ephemery

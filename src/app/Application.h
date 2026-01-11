#pragma once

#include "TrayIcon.h"
#include "storage/Settings.h"
#include "storage/ImageStorage.h"
#include "hotkey/HotkeyManager.h"
#include "input/PathInputter.h"
#include "capture/ScreenCapture.h"
#include "capture/RegionSelector.h"
#include "capture/WindowSelector.h"
#include "capture/ClipboardCapture.h"
#include "SettingsDialog.h"
#include <Windows.h>
#include <memory>

namespace Ephemery {

class Application {
public:
    Application();
    ~Application();

    bool Initialize(HINSTANCE hInstance);
    int Run();
    void Shutdown();

    static Application* GetInstance();

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool CreateMessageWindow(HINSTANCE hInstance);
    void HandleHotkeyAction(HotkeyAction action);
    void HandleTrayMenuCommand(TrayMenuCommand cmd);

    void OnCaptureScreen();
    void OnSaveClipboard();
    void OnPastePaths();
    void OnOpenFolder();
    void OnShowSettings();
    void OnExit();

    static Application* s_instance;

    HWND m_hwnd = nullptr;
    bool m_running = false;

    Settings m_settings;
    ImageStorage m_imageStorage;
    HotkeyManager m_hotkeyManager;
    PathInputter m_pathInputter;
    TrayIcon m_trayIcon;

    ScreenCapture m_screenCapture;
    RegionSelector m_regionSelector;
    WindowSelector m_windowSelector;
    ClipboardCapture m_clipboardCapture;

    void OnRegionSelected(const CaptureRect& rect);
    void OnWindowSelected(HWND hwnd);
    void OnSelectionCancelled();
    bool SaveCapturedBitmap(HBITMAP hBitmap, CaptureSource source);
    void OnSettingsChanged(const AppSettings& newSettings);

    SettingsDialog m_settingsDialog;
};

} // namespace Ephemery

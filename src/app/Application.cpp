#include "Application.h"
#include "util/Logger.h"
#include "util/PngEncoder.h"
#include <shellapi.h>

namespace Ephemery {

Application* Application::s_instance = nullptr;

Application::Application() {
    s_instance = this;
}

Application::~Application() {
    Shutdown();
    s_instance = nullptr;
}

Application* Application::GetInstance() {
    return s_instance;
}

bool Application::Initialize(HINSTANCE hInstance) {
    LOG_INFO(L"Initializing Ephemery...");

    // Initialize GDI+
    if (!PngEncoder::Initialize()) {
        LOG_ERROR(L"Failed to initialize GDI+");
        return false;
    }

    // Load settings
    m_settings.Load();

    // Initialize image storage
    const auto& appSettings = m_settings.GetSettings();
    if (!m_imageStorage.Initialize(appSettings.storagePath, appSettings.maxImages)) {
        LOG_ERROR(L"Failed to initialize image storage");
        return false;
    }

    // Load existing images
    m_imageStorage.LoadExistingImages();

    // Create message-only window
    if (!CreateMessageWindow(hInstance)) {
        LOG_ERROR(L"Failed to create message window");
        return false;
    }

    // Initialize tray icon first (so we can show notifications)
    if (!m_trayIcon.Initialize(m_hwnd, TrayIcon::WM_TRAYICON)) {
        LOG_ERROR(L"Failed to initialize tray icon");
        return false;
    }

    m_trayIcon.SetCallback([this](TrayMenuCommand cmd) {
        HandleTrayMenuCommand(cmd);
    });

    // Initialize hotkey manager
    if (!m_hotkeyManager.Initialize(m_hwnd)) {
        LOG_ERROR(L"Failed to initialize hotkey manager");
        return false;
    }

    m_hotkeyManager.SetCallback([this](HotkeyAction action) {
        HandleHotkeyAction(action);
    });

    // Register hotkeys (non-fatal if some fail)
    if (!m_hotkeyManager.RegisterHotkeys(appSettings.hotkeys)) {
        LOG_WARNING(L"Some hotkeys failed to register");
        m_trayIcon.ShowBalloon(L"Ephemery",
            L"一部のホットキーが登録できませんでした。\n他のアプリと競合している可能性があります。");
    }

    LOG_INFO(L"Ephemery initialized successfully");
    return true;
}

int Application::Run() {
    m_running = true;

    MSG msg;    while (m_running && GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return static_cast<int>(msg.wParam);
}

void Application::Shutdown() {
    m_running = false;

    m_trayIcon.Shutdown();
    m_hotkeyManager.Shutdown();
    m_imageStorage.Shutdown();
    m_settings.Save();
    PngEncoder::Shutdown();

    if (m_hwnd) {
        DestroyWindow(m_hwnd);
        m_hwnd = nullptr;
    }

    LOG_INFO(L"Ephemery shutdown complete");
}

bool Application::CreateMessageWindow(HINSTANCE hInstance) {
    const wchar_t* className = L"EphemeryMessageWindow";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.lpszClassName = className;

    if (!RegisterClassExW(&wc)) {
        if (GetLastError() != ERROR_CLASS_ALREADY_EXISTS) {
            return false;
        }
    }

    m_hwnd = CreateWindowExW(
        0,
        className,
        L"Ephemery",
        0,
        0, 0, 0, 0,
        HWND_MESSAGE,
        nullptr,
        hInstance,
        this
    );

    return m_hwnd != nullptr;
}

LRESULT CALLBACK Application::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    Application* app = nullptr;

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        app = static_cast<Application*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(app));
    } else {
        app = reinterpret_cast<Application*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (app) {
        return app->HandleMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT Application::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_HOTKEY:
            m_hotkeyManager.HandleHotkey(static_cast<int>(wParam));
            return 0;

        case TrayIcon::WM_TRAYICON:
            if (LOWORD(lParam) == WM_RBUTTONUP) {
                m_trayIcon.ShowContextMenu(hwnd);
            }
            return 0;

        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void Application::HandleHotkeyAction(HotkeyAction action) {
    switch (action) {
        case HotkeyAction::CaptureScreen:
            OnCaptureScreen();
            break;
        case HotkeyAction::SaveClipboard:
            OnSaveClipboard();
            break;
        case HotkeyAction::PastePaths:
            OnPastePaths();
            break;
    }
}

void Application::HandleTrayMenuCommand(TrayMenuCommand cmd) {
    switch (cmd) {
        case TrayMenuCommand::OpenFolder:
            OnOpenFolder();
            break;
        case TrayMenuCommand::Settings:
            OnShowSettings();
            break;
        case TrayMenuCommand::Exit:
            OnExit();
            break;
    }
}

void Application::OnCaptureScreen() {
    LOG_INFO(L"CaptureScreen triggered");

    if (m_regionSelector.IsActive() || m_windowSelector.IsActive()) {
        LOG_WARNING(L"Selection already in progress");
        return;
    }

    m_regionSelector.Start(
        [this](const CaptureRect& rect) { OnRegionSelected(rect); },
        [this]() { OnSelectionCancelled(); }
    );
}

void Application::OnRegionSelected(const CaptureRect& rect) {
    LOG_INFO(L"Region selected: " + std::to_wstring(rect.width) + L"x" + std::to_wstring(rect.height));

    // T039: パフォーマンス計測開始
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    HBITMAP hBitmap = m_screenCapture.CaptureRegion(rect);
    if (hBitmap) {
        SaveCapturedBitmap(hBitmap, CaptureSource::Screen);
        DeleteObject(hBitmap);
    }

    // T039: パフォーマンス計測終了
    QueryPerformanceCounter(&end);
    double elapsedMs = (end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    LOG_INFO(L"Capture completed in " + std::to_wstring(static_cast<int>(elapsedMs)) + L"ms (target: <1000ms)");
}

void Application::OnWindowSelected(HWND hwnd) {
    LOG_INFO(L"Window selected");

    HBITMAP hBitmap = m_screenCapture.CaptureWindow(hwnd);
    if (hBitmap) {
        SaveCapturedBitmap(hBitmap, CaptureSource::Screen);
        DeleteObject(hBitmap);
    }
}

void Application::OnSelectionCancelled() {
    LOG_INFO(L"Selection cancelled");
}

bool Application::SaveCapturedBitmap(HBITMAP hBitmap, CaptureSource source) {
    std::wstring filePath = m_imageStorage.GenerateFilePath();

    if (!PngEncoder::SaveBitmapAsPng(hBitmap, filePath)) {
        LOG_ERROR(L"Failed to save PNG");
        return false;
    }

    if (!m_imageStorage.AddImage(filePath, source)) {
        LOG_ERROR(L"Failed to add image to storage");
        DeleteFileW(filePath.c_str());
        return false;
    }

    m_trayIcon.ShowBalloon(L"キャプチャ完了", filePath);
    return true;
}

void Application::OnSaveClipboard() {
    LOG_INFO(L"SaveClipboard triggered");

    if (!m_clipboardCapture.HasImage()) {
        LOG_WARNING(L"No image in clipboard");
        m_trayIcon.ShowBalloon(L"Ephemery", L"クリップボードに画像がありません");
        return;
    }

    HBITMAP hBitmap = m_clipboardCapture.CaptureFromClipboard();
    if (hBitmap) {
        SaveCapturedBitmap(hBitmap, CaptureSource::Clipboard);
        DeleteObject(hBitmap);
    } else {
        m_trayIcon.ShowBalloon(L"Ephemery", L"クリップボードからの画像取得に失敗しました");
    }
}

void Application::OnPastePaths() {
    LOG_INFO(L"PastePaths triggered");

    auto paths = m_imageStorage.GetAllPaths();
    if (paths.empty()) {
        LOG_WARNING(L"No images to paste");
        return;
    }

    // T040: パフォーマンス計測開始
    LARGE_INTEGER freq, start, end;
    QueryPerformanceFrequency(&freq);
    QueryPerformanceCounter(&start);

    if (!m_pathInputter.TypePaths(paths)) {
        LOG_ERROR(L"Failed to type paths");
    } else {
        // パス貼り付け成功後、リストのみクリア（ファイルは残す）
        m_imageStorage.ClearList();
    }

    // T040: パフォーマンス計測終了
    QueryPerformanceCounter(&end);
    double elapsedMs = (end.QuadPart - start.QuadPart) * 1000.0 / freq.QuadPart;
    LOG_INFO(L"Path input completed in " + std::to_wstring(static_cast<int>(elapsedMs)) + L"ms (target: <500ms)");
}

void Application::OnOpenFolder() {
    std::wstring path = m_imageStorage.GetStoragePath();
    ShellExecuteW(nullptr, L"open", path.c_str(), nullptr, nullptr, SW_SHOWNORMAL);
}

void Application::OnShowSettings() {
    LOG_INFO(L"Settings dialog requested");

    m_settingsDialog.Show(nullptr, m_settings.GetSettings(),
        [this](const AppSettings& newSettings) {
            OnSettingsChanged(newSettings);
        });
}

void Application::OnSettingsChanged(const AppSettings& newSettings) {
    LOG_INFO(L"Settings changed, re-registering hotkeys");

    // Re-register hotkeys with new bindings
    if (!m_hotkeyManager.RegisterHotkeys(newSettings.hotkeys)) {
        LOG_WARNING(L"Some hotkeys failed to register");
        m_trayIcon.ShowBalloon(L"Ephemery", L"一部のホットキーの登録に失敗しました");
    }

    // Update image storage max limit
    m_imageStorage.SetMaxImages(newSettings.maxImages);

    // Save settings to file
    m_settings.Save();

    m_trayIcon.ShowBalloon(L"Ephemery", L"設定を保存しました");
}

void Application::OnExit() {
    m_running = false;
    PostMessage(m_hwnd, WM_CLOSE, 0, 0);
}

} // namespace Ephemery

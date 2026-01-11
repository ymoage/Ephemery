#include "ScreenCapture.h"
#include "util/Logger.h"

namespace Ephemery {

HBITMAP ScreenCapture::CaptureRegion(const CaptureRect& rect) {
    if (!rect.IsValid()) {
        LOG_ERROR(L"Invalid capture rect");
        return nullptr;
    }

    HDC hdcScreen = GetDC(nullptr);
    if (hdcScreen == nullptr) {
        LOG_ERROR(L"Failed to get screen DC");
        return nullptr;
    }

    HBITMAP hBitmap = CaptureFromDC(hdcScreen, rect.x, rect.y, rect.width, rect.height);

    ReleaseDC(nullptr, hdcScreen);

    return hBitmap;
}

HBITMAP ScreenCapture::CaptureWindow(HWND hwnd) {
    if (hwnd == nullptr || !IsWindow(hwnd)) {
        LOG_ERROR(L"Invalid window handle");
        return nullptr;
    }

    RECT windowRect;
    if (!::GetWindowRect(hwnd, &windowRect)) {
        LOG_ERROR(L"Failed to get window rect");
        return nullptr;
    }

    int width = windowRect.right - windowRect.left;
    int height = windowRect.bottom - windowRect.top;

    if (width <= 0 || height <= 0) {
        LOG_ERROR(L"Invalid window dimensions");
        return nullptr;
    }

    HDC hdcWindow = GetWindowDC(hwnd);
    if (hdcWindow == nullptr) {
        LOG_ERROR(L"Failed to get window DC");
        return nullptr;
    }

    HBITMAP hBitmap = CaptureFromDC(hdcWindow, 0, 0, width, height);

    ReleaseDC(hwnd, hdcWindow);

    return hBitmap;
}

HBITMAP ScreenCapture::CaptureFullScreen() {
    CaptureRect rect = GetFullScreenRect();
    return CaptureRegion(rect);
}

HBITMAP ScreenCapture::CaptureMonitor(HMONITOR hMonitor) {
    CaptureRect rect = GetMonitorRect(hMonitor);
    return CaptureRegion(rect);
}

CaptureRect ScreenCapture::GetFullScreenRect() {
    CaptureRect rect;
    rect.x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    rect.y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    rect.width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    rect.height = GetSystemMetrics(SM_CYVIRTUALSCREEN);
    return rect;
}

CaptureRect ScreenCapture::GetWindowRect(HWND hwnd) {
    CaptureRect result;

    RECT windowRect;
    if (::GetWindowRect(hwnd, &windowRect)) {
        result.x = windowRect.left;
        result.y = windowRect.top;
        result.width = windowRect.right - windowRect.left;
        result.height = windowRect.bottom - windowRect.top;
    }

    return result;
}

CaptureRect ScreenCapture::GetMonitorRect(HMONITOR hMonitor) {
    CaptureRect result;

    MONITORINFO mi;
    mi.cbSize = sizeof(MONITORINFO);

    if (GetMonitorInfoW(hMonitor, &mi)) {
        result.x = mi.rcMonitor.left;
        result.y = mi.rcMonitor.top;
        result.width = mi.rcMonitor.right - mi.rcMonitor.left;
        result.height = mi.rcMonitor.bottom - mi.rcMonitor.top;
    }

    return result;
}

HBITMAP ScreenCapture::CaptureFromDC(HDC hdcSource, int x, int y, int width, int height) {
    HDC hdcMem = CreateCompatibleDC(hdcSource);
    if (hdcMem == nullptr) {
        LOG_ERROR(L"Failed to create compatible DC");
        return nullptr;
    }

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcSource, width, height);
    if (hBitmap == nullptr) {
        LOG_ERROR(L"Failed to create compatible bitmap");
        DeleteDC(hdcMem);
        return nullptr;
    }

    HGDIOBJ hOldBitmap = SelectObject(hdcMem, hBitmap);

    if (!BitBlt(hdcMem, 0, 0, width, height, hdcSource, x, y, SRCCOPY)) {
        LOG_ERROR(L"BitBlt failed");
        SelectObject(hdcMem, hOldBitmap);
        DeleteObject(hBitmap);
        DeleteDC(hdcMem);
        return nullptr;
    }

    SelectObject(hdcMem, hOldBitmap);
    DeleteDC(hdcMem);

    LOG_DEBUG(L"Captured region: " + std::to_wstring(width) + L"x" + std::to_wstring(height));

    return hBitmap;
}

} // namespace Ephemery

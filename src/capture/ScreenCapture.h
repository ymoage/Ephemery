#pragma once

#include <Windows.h>
#include <string>

namespace Ephemery {

struct CaptureRect {
    int x = 0;
    int y = 0;
    int width = 0;
    int height = 0;

    bool IsValid() const { return width > 0 && height > 0; }
};

class ScreenCapture {
public:
    ScreenCapture() = default;
    ~ScreenCapture() = default;

    HBITMAP CaptureRegion(const CaptureRect& rect);
    HBITMAP CaptureWindow(HWND hwnd);
    HBITMAP CaptureFullScreen();
    HBITMAP CaptureMonitor(HMONITOR hMonitor);

    static CaptureRect GetFullScreenRect();
    static CaptureRect GetWindowRect(HWND hwnd);
    static CaptureRect GetMonitorRect(HMONITOR hMonitor);

private:
    HBITMAP CaptureFromDC(HDC hdcSource, int x, int y, int width, int height);
};

} // namespace Ephemery

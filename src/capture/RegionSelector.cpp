#include "RegionSelector.h"
#include "util/Logger.h"
#include <algorithm>

namespace Ephemery {

RegionSelector::RegionSelector() = default;

RegionSelector::~RegionSelector() {
    Cancel();
}

void RegionSelector::Start(RegionSelectedCallback onSelected, RegionCancelledCallback onCancelled) {
    if (m_active) {
        return;
    }

    m_onSelected = std::move(onSelected);
    m_onCancelled = std::move(onCancelled);

    CaptureScreenshot();

    if (!CreateOverlayWindow()) {
        LOG_ERROR(L"Failed to create overlay window");
        if (m_screenshotBitmap) {
            DeleteObject(m_screenshotBitmap);
            m_screenshotBitmap = nullptr;
        }
        if (m_onCancelled) {
            m_onCancelled();
        }
        return;
    }

    m_active = true;
    m_dragging = false;
    SetCapture(m_overlayWnd);
    SetCursor(LoadCursor(nullptr, IDC_CROSS));

    LOG_INFO(L"RegionSelector started");
}

void RegionSelector::Cancel() {
    if (!m_active) {
        return;
    }

    ReleaseCapture();
    DestroyOverlayWindow();

    if (m_screenshotBitmap) {
        DeleteObject(m_screenshotBitmap);
        m_screenshotBitmap = nullptr;
    }

    m_active = false;
    m_dragging = false;

    if (m_onCancelled) {
        m_onCancelled();
    }

    LOG_INFO(L"RegionSelector cancelled");
}

void RegionSelector::CaptureScreenshot() {
    m_offsetX = GetSystemMetrics(SM_XVIRTUALSCREEN);
    m_offsetY = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    HDC hdcScreen = GetDC(nullptr);
    HDC hdcMem = CreateCompatibleDC(hdcScreen);

    m_screenshotBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    HGDIOBJ hOld = SelectObject(hdcMem, m_screenshotBitmap);

    BitBlt(hdcMem, 0, 0, width, height, hdcScreen, m_offsetX, m_offsetY, SRCCOPY);

    SelectObject(hdcMem, hOld);
    DeleteDC(hdcMem);
    ReleaseDC(nullptr, hdcScreen);

    LOG_DEBUG(L"Screenshot captured: " + std::to_wstring(width) + L"x" + std::to_wstring(height));
}

bool RegionSelector::CreateOverlayWindow() {
    const wchar_t* className = L"EphemeryRegionSelector";

    WNDCLASSEXW wc = {};
    wc.cbSize = sizeof(WNDCLASSEXW);
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.hCursor = LoadCursor(nullptr, IDC_CROSS);
    wc.lpszClassName = className;

    RegisterClassExW(&wc);

    int x = GetSystemMetrics(SM_XVIRTUALSCREEN);
    int y = GetSystemMetrics(SM_YVIRTUALSCREEN);
    int width = GetSystemMetrics(SM_CXVIRTUALSCREEN);
    int height = GetSystemMetrics(SM_CYVIRTUALSCREEN);

    m_overlayWnd = CreateWindowExW(
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW,
        className,
        L"",
        WS_POPUP,
        x, y, width, height,
        nullptr,
        nullptr,
        GetModuleHandle(nullptr),
        this
    );

    if (m_overlayWnd == nullptr) {
        return false;
    }

    ShowWindow(m_overlayWnd, SW_SHOW);
    UpdateWindow(m_overlayWnd);

    return true;
}

void RegionSelector::DestroyOverlayWindow() {
    if (m_overlayWnd) {
        DestroyWindow(m_overlayWnd);
        m_overlayWnd = nullptr;
    }
}

LRESULT CALLBACK RegionSelector::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    RegionSelector* selector = nullptr;

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        selector = static_cast<RegionSelector*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(selector));
    } else {
        selector = reinterpret_cast<RegionSelector*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (selector) {
        return selector->HandleMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT RegionSelector::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            OnPaint(hdc);
            EndPaint(hwnd, &ps);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            POINT pt;
            GetCursorPos(&pt);
            OnMouseDown(pt.x - m_offsetX, pt.y - m_offsetY);
            return 0;
        }

        case WM_MOUSEMOVE: {
            POINT pt;
            GetCursorPos(&pt);
            OnMouseMove(pt.x - m_offsetX, pt.y - m_offsetY);
            return 0;
        }

        case WM_LBUTTONUP: {
            POINT pt;
            GetCursorPos(&pt);
            OnMouseUp(pt.x - m_offsetX, pt.y - m_offsetY);
            return 0;
        }

        case WM_KEYDOWN:
            OnKeyDown(wParam);
            return 0;

        case WM_RBUTTONDOWN:
            Cancel();
            return 0;
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

void RegionSelector::OnMouseDown(int x, int y) {
    m_dragging = true;
    m_startX = x;
    m_startY = y;
    m_currentX = x;
    m_currentY = y;
}

void RegionSelector::OnMouseMove(int x, int y) {
    if (!m_dragging) {
        return;
    }

    m_currentX = x;
    m_currentY = y;

    InvalidateRect(m_overlayWnd, nullptr, FALSE);
}

void RegionSelector::OnMouseUp(int x, int y) {
    if (!m_dragging) {
        return;
    }

    m_dragging = false;
    m_currentX = x;
    m_currentY = y;

    CaptureRect rect = NormalizeRect(m_startX, m_startY, m_currentX, m_currentY);

    if (rect.width < 5 || rect.height < 5) {
        LOG_WARNING(L"Selection too small, cancelled");
        Cancel();
        return;
    }

    // Convert to screen coordinates
    rect.x += m_offsetX;
    rect.y += m_offsetY;

    ReleaseCapture();
    DestroyOverlayWindow();

    if (m_screenshotBitmap) {
        DeleteObject(m_screenshotBitmap);
        m_screenshotBitmap = nullptr;
    }

    m_active = false;

    if (m_onSelected) {
        m_onSelected(rect);
    }

    LOG_INFO(L"Region selected: " + std::to_wstring(rect.width) + L"x" + std::to_wstring(rect.height));
}

void RegionSelector::OnKeyDown(WPARAM vk) {
    if (vk == VK_ESCAPE) {
        Cancel();
    }
}

void RegionSelector::OnPaint(HDC hdc) {
    RECT clientRect;
    GetClientRect(m_overlayWnd, &clientRect);

    int width = clientRect.right - clientRect.left;
    int height = clientRect.bottom - clientRect.top;

    // Create memory DC
    HDC hdcMem = CreateCompatibleDC(hdc);
    HBITMAP hBmpMem = CreateCompatibleBitmap(hdc, width, height);
    HGDIOBJ hOldBmp = SelectObject(hdcMem, hBmpMem);

    // Draw screenshot
    if (m_screenshotBitmap) {
        HDC hdcScreenshot = CreateCompatibleDC(hdc);
        HGDIOBJ hOldScreenshot = SelectObject(hdcScreenshot, m_screenshotBitmap);
        BitBlt(hdcMem, 0, 0, width, height, hdcScreenshot, 0, 0, SRCCOPY);
        SelectObject(hdcScreenshot, hOldScreenshot);
        DeleteDC(hdcScreenshot);
    }

    // Draw semi-transparent overlay
    BLENDFUNCTION bf = {};
    bf.BlendOp = AC_SRC_OVER;
    bf.SourceConstantAlpha = OVERLAY_ALPHA;

    HDC hdcOverlay = CreateCompatibleDC(hdc);
    HBITMAP hBmpOverlay = CreateCompatibleBitmap(hdc, width, height);
    HGDIOBJ hOldOverlay = SelectObject(hdcOverlay, hBmpOverlay);

    HBRUSH hBrush = CreateSolidBrush(OVERLAY_COLOR);
    FillRect(hdcOverlay, &clientRect, hBrush);
    DeleteObject(hBrush);

    AlphaBlend(hdcMem, 0, 0, width, height, hdcOverlay, 0, 0, width, height, bf);

    SelectObject(hdcOverlay, hOldOverlay);
    DeleteObject(hBmpOverlay);
    DeleteDC(hdcOverlay);

    // Draw selection rectangle
    if (m_dragging) {
        CaptureRect selRect = NormalizeRect(m_startX, m_startY, m_currentX, m_currentY);

        // Draw screenshot in selection area (clear overlay)
        if (m_screenshotBitmap && selRect.IsValid()) {
            HDC hdcScreenshot = CreateCompatibleDC(hdc);
            HGDIOBJ hOldScreenshot = SelectObject(hdcScreenshot, m_screenshotBitmap);
            BitBlt(hdcMem, selRect.x, selRect.y, selRect.width, selRect.height,
                   hdcScreenshot, selRect.x, selRect.y, SRCCOPY);
            SelectObject(hdcScreenshot, hOldScreenshot);
            DeleteDC(hdcScreenshot);
        }

        // Draw border
        HPEN hPen = CreatePen(PS_SOLID, BORDER_WIDTH, BORDER_COLOR);
        HBRUSH hNullBrush = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));
        HGDIOBJ hOldPen = SelectObject(hdcMem, hPen);
        HGDIOBJ hOldBrush2 = SelectObject(hdcMem, hNullBrush);

        Rectangle(hdcMem, selRect.x, selRect.y,
                  selRect.x + selRect.width, selRect.y + selRect.height);

        SelectObject(hdcMem, hOldPen);
        SelectObject(hdcMem, hOldBrush2);
        DeleteObject(hPen);
    }

    // Copy to window
    BitBlt(hdc, 0, 0, width, height, hdcMem, 0, 0, SRCCOPY);

    SelectObject(hdcMem, hOldBmp);
    DeleteObject(hBmpMem);
    DeleteDC(hdcMem);
}

CaptureRect RegionSelector::NormalizeRect(int x1, int y1, int x2, int y2) {
    CaptureRect rect;
    rect.x = (std::min)(x1, x2);
    rect.y = (std::min)(y1, y2);
    rect.width = std::abs(x2 - x1);
    rect.height = std::abs(y2 - y1);
    return rect;
}

} // namespace Ephemery

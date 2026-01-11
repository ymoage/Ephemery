#pragma once

#include "ScreenCapture.h"
#include <Windows.h>
#include <functional>

namespace Ephemery {

using RegionSelectedCallback = std::function<void(const CaptureRect&)>;
using RegionCancelledCallback = std::function<void()>;

class RegionSelector {
public:
    RegionSelector();
    ~RegionSelector();

    void Start(RegionSelectedCallback onSelected, RegionCancelledCallback onCancelled);
    void Cancel();

    bool IsActive() const { return m_active; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool CreateOverlayWindow();
    void DestroyOverlayWindow();
    void CaptureScreenshot();

    void OnMouseDown(int x, int y);
    void OnMouseMove(int x, int y);
    void OnMouseUp(int x, int y);
    void OnKeyDown(WPARAM vk);
    void OnPaint(HDC hdc);

    CaptureRect NormalizeRect(int x1, int y1, int x2, int y2);

    HWND m_overlayWnd = nullptr;
    HBITMAP m_screenshotBitmap = nullptr;
    bool m_active = false;
    bool m_dragging = false;

    int m_startX = 0;
    int m_startY = 0;
    int m_currentX = 0;
    int m_currentY = 0;

    int m_offsetX = 0;
    int m_offsetY = 0;

    RegionSelectedCallback m_onSelected;
    RegionCancelledCallback m_onCancelled;

    static constexpr COLORREF OVERLAY_COLOR = RGB(0, 0, 0);
    static constexpr BYTE OVERLAY_ALPHA = 100;
    static constexpr COLORREF BORDER_COLOR = RGB(0, 120, 215);
    static constexpr int BORDER_WIDTH = 2;
};

} // namespace Ephemery

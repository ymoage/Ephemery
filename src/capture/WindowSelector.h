#pragma once

#include <Windows.h>
#include <functional>

namespace Ephemery {

using WindowSelectedCallback = std::function<void(HWND)>;
using SelectionCancelledCallback = std::function<void()>;

class WindowSelector {
public:
    WindowSelector();
    ~WindowSelector();

    void Start(WindowSelectedCallback onSelected, SelectionCancelledCallback onCancelled);
    void Cancel();

    bool IsActive() const { return m_active; }

private:
    static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    LRESULT HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);

    bool CreateOverlayWindow();
    void DestroyOverlayWindow();

    void OnMouseMove(int x, int y);
    void OnMouseClick(int x, int y);
    void OnKeyDown(WPARAM vk);

    HWND GetWindowAtPoint(int x, int y);
    void HighlightWindow(HWND hwnd);
    void ClearHighlight();
    void DrawHighlight(HWND hwnd);

    HWND m_overlayWnd = nullptr;
    HWND m_highlightedWnd = nullptr;
    bool m_active = false;

    WindowSelectedCallback m_onSelected;
    SelectionCancelledCallback m_onCancelled;

    static constexpr COLORREF HIGHLIGHT_COLOR = RGB(0, 120, 215);
    static constexpr int HIGHLIGHT_BORDER = 3;
};

} // namespace Ephemery

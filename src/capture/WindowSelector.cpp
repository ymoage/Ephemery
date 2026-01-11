#include "WindowSelector.h"
#include "util/Logger.h"

namespace Ephemery {

WindowSelector::WindowSelector() = default;

WindowSelector::~WindowSelector() {
    Cancel();
}

void WindowSelector::Start(WindowSelectedCallback onSelected, SelectionCancelledCallback onCancelled) {
    if (m_active) {
        return;
    }

    m_onSelected = std::move(onSelected);
    m_onCancelled = std::move(onCancelled);

    if (!CreateOverlayWindow()) {
        LOG_ERROR(L"Failed to create overlay window");
        if (m_onCancelled) {
            m_onCancelled();
        }
        return;
    }

    m_active = true;
    SetCapture(m_overlayWnd);
    SetCursor(LoadCursor(nullptr, IDC_CROSS));

    LOG_INFO(L"WindowSelector started");
}

void WindowSelector::Cancel() {
    if (!m_active) {
        return;
    }

    ClearHighlight();
    ReleaseCapture();
    DestroyOverlayWindow();

    m_active = false;
    m_highlightedWnd = nullptr;

    if (m_onCancelled) {
        m_onCancelled();
    }

    LOG_INFO(L"WindowSelector cancelled");
}

bool WindowSelector::CreateOverlayWindow() {
    const wchar_t* className = L"EphemeryWindowSelector";

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
        WS_EX_TOPMOST | WS_EX_TOOLWINDOW | WS_EX_LAYERED,
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

    SetLayeredWindowAttributes(m_overlayWnd, 0, 1, LWA_ALPHA);
    ShowWindow(m_overlayWnd, SW_SHOW);

    return true;
}

void WindowSelector::DestroyOverlayWindow() {
    if (m_overlayWnd) {
        DestroyWindow(m_overlayWnd);
        m_overlayWnd = nullptr;
    }
}

LRESULT CALLBACK WindowSelector::WindowProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    WindowSelector* selector = nullptr;

    if (msg == WM_NCCREATE) {
        auto* cs = reinterpret_cast<CREATESTRUCT*>(lParam);
        selector = static_cast<WindowSelector*>(cs->lpCreateParams);
        SetWindowLongPtrW(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(selector));
    } else {
        selector = reinterpret_cast<WindowSelector*>(GetWindowLongPtrW(hwnd, GWLP_USERDATA));
    }

    if (selector) {
        return selector->HandleMessage(hwnd, msg, wParam, lParam);
    }

    return DefWindowProcW(hwnd, msg, wParam, lParam);
}

LRESULT WindowSelector::HandleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
        case WM_MOUSEMOVE: {
            POINT pt;
            GetCursorPos(&pt);
            OnMouseMove(pt.x, pt.y);
            return 0;
        }

        case WM_LBUTTONDOWN: {
            POINT pt;
            GetCursorPos(&pt);
            OnMouseClick(pt.x, pt.y);
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

void WindowSelector::OnMouseMove(int x, int y) {
    HWND hwnd = GetWindowAtPoint(x, y);
    if (hwnd != m_highlightedWnd) {
        ClearHighlight();
        HighlightWindow(hwnd);
    }
}

void WindowSelector::OnMouseClick(int x, int y) {
    ClearHighlight();
    ReleaseCapture();
    DestroyOverlayWindow();

    HWND hwnd = GetWindowAtPoint(x, y);
    m_active = false;

    if (hwnd && m_onSelected) {
        m_onSelected(hwnd);
    } else if (m_onCancelled) {
        m_onCancelled();
    }
}

void WindowSelector::OnKeyDown(WPARAM vk) {
    if (vk == VK_ESCAPE) {
        Cancel();
    }
}

HWND WindowSelector::GetWindowAtPoint(int x, int y) {
    POINT pt = { x, y };
    HWND hwnd = WindowFromPoint(pt);

    if (hwnd == m_overlayWnd) {
        ShowWindow(m_overlayWnd, SW_HIDE);
        hwnd = WindowFromPoint(pt);
        ShowWindow(m_overlayWnd, SW_SHOW);
    }

    if (hwnd) {
        HWND root = GetAncestor(hwnd, GA_ROOT);
        if (root) {
            hwnd = root;
        }
    }

    return hwnd;
}

void WindowSelector::HighlightWindow(HWND hwnd) {
    if (hwnd == nullptr) {
        return;
    }

    m_highlightedWnd = hwnd;
    DrawHighlight(hwnd);
}

void WindowSelector::ClearHighlight() {
    if (m_highlightedWnd) {
        RedrawWindow(m_highlightedWnd, nullptr, nullptr,
            RDW_INVALIDATE | RDW_UPDATENOW | RDW_FRAME);
        m_highlightedWnd = nullptr;
    }
}

void WindowSelector::DrawHighlight(HWND hwnd) {
    RECT rect;
    if (!::GetWindowRect(hwnd, &rect)) {
        return;
    }

    HDC hdc = GetWindowDC(hwnd);
    if (hdc == nullptr) {
        return;
    }

    HPEN hPen = CreatePen(PS_SOLID, HIGHLIGHT_BORDER, HIGHLIGHT_COLOR);
    HBRUSH hBrush = static_cast<HBRUSH>(GetStockObject(NULL_BRUSH));

    HGDIOBJ hOldPen = SelectObject(hdc, hPen);
    HGDIOBJ hOldBrush = SelectObject(hdc, hBrush);

    int width = rect.right - rect.left;
    int height = rect.bottom - rect.top;
    Rectangle(hdc, 0, 0, width, height);

    SelectObject(hdc, hOldPen);
    SelectObject(hdc, hOldBrush);
    DeleteObject(hPen);

    ReleaseDC(hwnd, hdc);
}

} // namespace Ephemery

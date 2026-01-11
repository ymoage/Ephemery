#include "ClipboardCapture.h"
#include "util/Logger.h"

namespace Ephemery {

bool ClipboardCapture::HasImage() const {
    return IsClipboardFormatAvailable(CF_BITMAP) ||
           IsClipboardFormatAvailable(CF_DIB) ||
           IsClipboardFormatAvailable(CF_DIBV5);
}

HBITMAP ClipboardCapture::CaptureFromClipboard() {
    if (!HasImage()) {
        LOG_WARNING(L"No image in clipboard");
        return nullptr;
    }

    if (!OpenClipboard(nullptr)) {
        LOG_ERROR(L"Failed to open clipboard");
        return nullptr;
    }

    HBITMAP result = nullptr;

    // Try CF_BITMAP first
    HANDLE hData = GetClipboardData(CF_BITMAP);
    if (hData) {
        HBITMAP hBitmapClip = static_cast<HBITMAP>(hData);

        BITMAP bm;
        if (GetObject(hBitmapClip, sizeof(BITMAP), &bm)) {
            HDC hdcScreen = GetDC(nullptr);
            HDC hdcSrc = CreateCompatibleDC(hdcScreen);
            HDC hdcDst = CreateCompatibleDC(hdcScreen);

            result = CreateCompatibleBitmap(hdcScreen, bm.bmWidth, bm.bmHeight);

            HGDIOBJ hOldSrc = SelectObject(hdcSrc, hBitmapClip);
            HGDIOBJ hOldDst = SelectObject(hdcDst, result);

            BitBlt(hdcDst, 0, 0, bm.bmWidth, bm.bmHeight, hdcSrc, 0, 0, SRCCOPY);

            SelectObject(hdcSrc, hOldSrc);
            SelectObject(hdcDst, hOldDst);

            DeleteDC(hdcSrc);
            DeleteDC(hdcDst);
            ReleaseDC(nullptr, hdcScreen);

            LOG_INFO(L"Captured CF_BITMAP from clipboard: " +
                     std::to_wstring(bm.bmWidth) + L"x" + std::to_wstring(bm.bmHeight));
        }
    }

    // Try CF_DIB if CF_BITMAP failed
    if (!result) {
        hData = GetClipboardData(CF_DIB);
        if (hData) {
            result = ConvertDIBToHBitmap(hData);
        }
    }

    // Try CF_DIBV5 if CF_DIB failed
    if (!result) {
        hData = GetClipboardData(CF_DIBV5);
        if (hData) {
            result = ConvertDIBToHBitmap(hData);
        }
    }

    CloseClipboard();

    if (!result) {
        LOG_ERROR(L"Failed to capture image from clipboard");
    }

    return result;
}

HBITMAP ClipboardCapture::ConvertDIBToHBitmap(HANDLE hDib) {
    if (!hDib) {
        return nullptr;
    }

    LPVOID pDib = GlobalLock(hDib);
    if (!pDib) {
        return nullptr;
    }

    const auto* bmi = static_cast<const BITMAPINFO*>(pDib);
    const BITMAPINFOHEADER& header = bmi->bmiHeader;

    int width = header.biWidth;
    int height = std::abs(header.biHeight);

    int colorTableSize = 0;
    if (header.biBitCount <= 8) {
        colorTableSize = (header.biClrUsed ? header.biClrUsed : (1 << header.biBitCount)) * sizeof(RGBQUAD);
    } else if (header.biCompression == BI_BITFIELDS) {
        colorTableSize = 3 * sizeof(DWORD);
    }

    const BYTE* bits = static_cast<const BYTE*>(pDib) + sizeof(BITMAPINFOHEADER) + colorTableSize;

    HDC hdcScreen = GetDC(nullptr);

    HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
    if (hBitmap) {
        HDC hdcMem = CreateCompatibleDC(hdcScreen);
        HGDIOBJ hOld = SelectObject(hdcMem, hBitmap);

        SetDIBitsToDevice(hdcMem, 0, 0, width, height, 0, 0, 0, height,
                          bits, bmi, DIB_RGB_COLORS);

        SelectObject(hdcMem, hOld);
        DeleteDC(hdcMem);

        LOG_INFO(L"Converted DIB to HBITMAP: " +
                 std::to_wstring(width) + L"x" + std::to_wstring(height));
    }

    ReleaseDC(nullptr, hdcScreen);
    GlobalUnlock(hDib);

    return hBitmap;
}

} // namespace Ephemery

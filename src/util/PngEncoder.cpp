#include "PngEncoder.h"
#include "Logger.h"
#include <objidl.h>
#include <gdiplus.h>

#pragma comment(lib, "gdiplus.lib")

namespace Ephemery {

bool PngEncoder::s_initialized = false;
ULONG_PTR PngEncoder::s_gdiplusToken = 0;

namespace {

int GetEncoderClsid(const WCHAR* format, CLSID* pClsid) {
    UINT num = 0;
    UINT size = 0;

    Gdiplus::GetImageEncodersSize(&num, &size);
    if (size == 0) return -1;

    auto* pImageCodecInfo = static_cast<Gdiplus::ImageCodecInfo*>(malloc(size));
    if (pImageCodecInfo == nullptr) return -1;

    Gdiplus::GetImageEncoders(num, size, pImageCodecInfo);

    for (UINT i = 0; i < num; ++i) {
        if (wcscmp(pImageCodecInfo[i].MimeType, format) == 0) {
            *pClsid = pImageCodecInfo[i].Clsid;
            free(pImageCodecInfo);
            return static_cast<int>(i);
        }
    }

    free(pImageCodecInfo);
    return -1;
}

} // anonymous namespace

bool PngEncoder::Initialize() {
    if (s_initialized) return true;

    Gdiplus::GdiplusStartupInput gdiplusStartupInput;
    Gdiplus::Status status = Gdiplus::GdiplusStartup(&s_gdiplusToken, &gdiplusStartupInput, nullptr);

    if (status != Gdiplus::Ok) {
        LOG_ERROR(L"Failed to initialize GDI+");
        return false;
    }

    s_initialized = true;
    LOG_INFO(L"GDI+ initialized successfully");
    return true;
}

void PngEncoder::Shutdown() {
    if (s_initialized) {
        Gdiplus::GdiplusShutdown(s_gdiplusToken);
        s_initialized = false;
        LOG_INFO(L"GDI+ shutdown");
    }
}

bool PngEncoder::SaveBitmapAsPng(HBITMAP hBitmap, const std::wstring& filePath) {
    if (!s_initialized) {
        LOG_ERROR(L"GDI+ not initialized");
        return false;
    }

    if (hBitmap == nullptr) {
        LOG_ERROR(L"Invalid bitmap handle");
        return false;
    }

    Gdiplus::Bitmap bitmap(hBitmap, nullptr);
    if (bitmap.GetLastStatus() != Gdiplus::Ok) {
        LOG_ERROR(L"Failed to create GDI+ bitmap from HBITMAP");
        return false;
    }

    CLSID pngClsid;
    if (GetEncoderClsid(L"image/png", &pngClsid) < 0) {
        LOG_ERROR(L"Failed to get PNG encoder CLSID");
        return false;
    }

    Gdiplus::Status status = bitmap.Save(filePath.c_str(), &pngClsid, nullptr);
    if (status != Gdiplus::Ok) {
        LOG_ERROR(L"Failed to save PNG file: " + filePath);
        return false;
    }

    LOG_INFO(L"Saved PNG: " + filePath);
    return true;
}

bool PngEncoder::SaveDIBAsPng(const void* dibData, const std::wstring& filePath) {
    if (!s_initialized) {
        LOG_ERROR(L"GDI+ not initialized");
        return false;
    }

    if (dibData == nullptr) {
        LOG_ERROR(L"Invalid DIB data");
        return false;
    }

    const auto* bmi = static_cast<const BITMAPINFO*>(dibData);
    const BITMAPINFOHEADER& header = bmi->bmiHeader;

    int colorTableSize = 0;
    if (header.biBitCount <= 8) {
        colorTableSize = (header.biClrUsed ? header.biClrUsed : (1 << header.biBitCount)) * sizeof(RGBQUAD);
    }

    const void* bits = reinterpret_cast<const BYTE*>(dibData) + sizeof(BITMAPINFOHEADER) + colorTableSize;

    Gdiplus::Bitmap bitmap(bmi, const_cast<void*>(bits));
    if (bitmap.GetLastStatus() != Gdiplus::Ok) {
        LOG_ERROR(L"Failed to create GDI+ bitmap from DIB");
        return false;
    }

    CLSID pngClsid;
    if (GetEncoderClsid(L"image/png", &pngClsid) < 0) {
        LOG_ERROR(L"Failed to get PNG encoder CLSID");
        return false;
    }

    Gdiplus::Status status = bitmap.Save(filePath.c_str(), &pngClsid, nullptr);
    if (status != Gdiplus::Ok) {
        LOG_ERROR(L"Failed to save PNG file: " + filePath);
        return false;
    }

    LOG_INFO(L"Saved PNG from DIB: " + filePath);
    return true;
}

} // namespace Ephemery

#pragma once

#include <string>
#include <Windows.h>

namespace Ephemery {

class PngEncoder {
public:
    static bool Initialize();
    static void Shutdown();

    static bool SaveBitmapAsPng(HBITMAP hBitmap, const std::wstring& filePath);
    static bool SaveDIBAsPng(const void* dibData, const std::wstring& filePath);

private:
    static bool s_initialized;
    static ULONG_PTR s_gdiplusToken;
};

} // namespace Ephemery

#pragma once

#include <Windows.h>

namespace Ephemery {

class ClipboardCapture {
public:
    ClipboardCapture() = default;
    ~ClipboardCapture() = default;

    bool HasImage() const;
    HBITMAP CaptureFromClipboard();

private:
    HBITMAP ConvertDIBToHBitmap(HANDLE hDib);
};

} // namespace Ephemery

#pragma once

#include <string>
#include <chrono>
#include <array>
#include <cstdint>

namespace Ephemery {

enum class CaptureSource {
    Screen,
    Clipboard
};

enum class HotkeyAction {
    CaptureScreen,
    SaveClipboard,
    PastePaths
};

struct CapturedImage {
    std::wstring filePath;
    std::chrono::system_clock::time_point capturedAt;
    uint32_t sequenceNumber = 0;
    CaptureSource source = CaptureSource::Screen;
};

struct HotkeyBinding {
    HotkeyAction action = HotkeyAction::CaptureScreen;
    uint32_t modifiers = 0;
    uint32_t virtualKey = 0;
    int id = 0;
};

struct AppSettings {
    uint32_t version = 1;
    uint32_t maxImages = 10;
    std::wstring storagePath;
    std::array<HotkeyBinding, 3> hotkeys;
};

} // namespace Ephemery

#include "PathInputter.h"
#include "util/Logger.h"

namespace Ephemery {

bool PathInputter::TypePaths(const std::vector<std::wstring>& paths, QuoteStyle quoteStyle) {
    if (paths.empty()) {
        LOG_WARNING(L"No paths to type");
        return false;
    }

    wchar_t quoteChar = L'\0';
    switch (quoteStyle) {
        case QuoteStyle::Double:   quoteChar = L'"';  break;
        case QuoteStyle::Single:   quoteChar = L'\''; break;
        case QuoteStyle::Backtick: quoteChar = L'`';  break;
        default: break;
    }

    std::wstring text;
    for (size_t i = 0; i < paths.size(); ++i) {
        if (i > 0) text += L"\n";
        if (quoteChar != L'\0') text += quoteChar;
        text += paths[i];
        if (quoteChar != L'\0') text += quoteChar;
    }

    LOG_INFO(L"Pasting paths via clipboard: " + text.substr(0, 50) + (text.length() > 50 ? L"..." : L""));

    // Wait for hotkey modifier keys to be physically released.
    // Prevents Ctrl+V from being interpreted as Ctrl+Alt+V (triggering our own hotkey).
    WaitForModifiersReleased();

    if (!SetClipboardText(text)) {
        LOG_ERROR(L"Failed to set clipboard text");
        return false;
    }

    return SendCtrlV();
}

void PathInputter::WaitForModifiersReleased() {
    // Poll up to 2 seconds for Ctrl/Alt/Shift to be physically released.
    // Necessary because WM_DEFERRED_PASTE can fire while the user still holds the hotkey.
    const DWORD kTimeoutMs = 2000;
    const DWORD deadline = GetTickCount() + kTimeoutMs;
    while (GetTickCount() < deadline) {
        const bool anyDown = (GetAsyncKeyState(VK_CONTROL) & 0x8000) ||
                             (GetAsyncKeyState(VK_MENU)    & 0x8000) ||
                             (GetAsyncKeyState(VK_SHIFT)   & 0x8000);
        if (!anyDown) break;
        Sleep(10);
    }
    Sleep(50); // Extra stability delay after modifier release
}

bool PathInputter::SetClipboardText(const std::wstring& text) {
    if (!OpenClipboard(nullptr)) {
        return false;
    }

    EmptyClipboard();

    const size_t byteCount = (text.size() + 1) * sizeof(wchar_t);
    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, byteCount);
    if (!hMem) {
        CloseClipboard();
        return false;
    }

    wchar_t* dst = static_cast<wchar_t*>(GlobalLock(hMem));
    wmemcpy(dst, text.c_str(), text.size() + 1);
    GlobalUnlock(hMem);

    // On success the system owns hMem; on failure we must free it.
    const bool ok = SetClipboardData(CF_UNICODETEXT, hMem) != nullptr;
    if (!ok) {
        GlobalFree(hMem);
    }

    CloseClipboard();
    return ok;
}

bool PathInputter::SendCtrlV() {
    INPUT inputs[4] = {};

    inputs[0].type = INPUT_KEYBOARD;
    inputs[0].ki.wVk = VK_CONTROL;                          // Ctrl down

    inputs[1].type = INPUT_KEYBOARD;
    inputs[1].ki.wVk = 'V';                                 // V down

    inputs[2].type = INPUT_KEYBOARD;
    inputs[2].ki.wVk = 'V';
    inputs[2].ki.dwFlags = KEYEVENTF_KEYUP;                 // V up

    inputs[3].type = INPUT_KEYBOARD;
    inputs[3].ki.wVk = VK_CONTROL;
    inputs[3].ki.dwFlags = KEYEVENTF_KEYUP;                 // Ctrl up

    const UINT sent = SendInput(4, inputs, sizeof(INPUT));
    LOG_INFO(L"Ctrl+V sent (" + std::to_wstring(sent) + L"/4 events)");
    return sent == 4;
}

} // namespace Ephemery

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
    LOG_DEBUG(L"Waiting for modifier keys to be released...");
    WaitForModifiersReleased();
    LOG_DEBUG(L"Modifiers released, proceeding with clipboard paste");

    if (!SetClipboardText(text)) {
        LOG_ERROR(L"Failed to set clipboard text");
        return false;
    }
    LOG_DEBUG(L"Clipboard set successfully");

    bool ok = SendCtrlV();
    LOG_DEBUG(L"SendCtrlV result: " + std::wstring(ok ? L"OK" : L"FAILED"));
    return ok;
}

void PathInputter::WaitForModifiersReleased() {
    // Poll up to 2 seconds for Ctrl/Alt/Shift to be physically released.
    // Necessary because WM_DEFERRED_PASTE can fire while the user still holds the hotkey.
    const DWORD kTimeoutMs = 2000;
    const DWORD start    = GetTickCount();
    const DWORD deadline = start + kTimeoutMs;
    while (GetTickCount() < deadline) {
        const bool ctrl  = (GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0;
        const bool alt   = (GetAsyncKeyState(VK_MENU)    & 0x8000) != 0;
        const bool shift = (GetAsyncKeyState(VK_SHIFT)   & 0x8000) != 0;
        if (!ctrl && !alt && !shift) break;
        Sleep(10);
    }
    DWORD waited = GetTickCount() - start;
    LOG_DEBUG(L"WaitForModifiersReleased: waited " + std::to_wstring(waited) + L"ms"
        + L" Ctrl=" + std::to_wstring((GetAsyncKeyState(VK_CONTROL) & 0x8000) != 0)
        + L" Alt="  + std::to_wstring((GetAsyncKeyState(VK_MENU)    & 0x8000) != 0)
        + L" Shift=" + std::to_wstring((GetAsyncKeyState(VK_SHIFT)  & 0x8000) != 0));
    Sleep(50); // Extra stability delay after modifier release
}

bool PathInputter::SetClipboardText(const std::wstring& text) {
    if (!OpenClipboard(nullptr)) {
        LOG_ERROR(L"OpenClipboard failed: " + std::to_wstring(GetLastError()));
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
        LOG_ERROR(L"SetClipboardData failed: " + std::to_wstring(GetLastError()));
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
    if (sent != 4) {
        LOG_ERROR(L"SendInput(Ctrl+V) failed: sent=" + std::to_wstring(sent)
            + L" error=" + std::to_wstring(GetLastError()));
    } else {
        LOG_DEBUG(L"SendInput(Ctrl+V) OK: 4/4 events sent");
    }
    return sent == 4;
}

} // namespace Ephemery

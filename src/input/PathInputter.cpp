#include "PathInputter.h"
#include "util/Logger.h"

namespace Ephemery {

bool PathInputter::TypePaths(const std::vector<std::wstring>& paths) {
    if (paths.empty()) {
        LOG_WARNING(L"No paths to type");
        return false;
    }

    std::wstring combined;
    for (size_t i = 0; i < paths.size(); ++i) {
        if (i > 0) {
            combined += L"\n";
        }
        combined += paths[i];
    }

    return TypeText(combined);
}

bool PathInputter::TypeText(const std::wstring& text) {
    if (text.empty()) {
        return false;
    }

    LOG_INFO(L"Typing text: " + text.substr(0, 50) + (text.length() > 50 ? L"..." : L""));

    // ホットキーの修飾キーが解放されるのを待つ
    ReleaseModifierKeys();
    Sleep(100);  // 修飾キー解放後の安定待ち

    for (wchar_t ch : text) {
        if (!SendUnicodeChar(ch)) {
            LOG_ERROR(L"Failed to send character");
            return false;
        }

        if (m_delayBetweenKeys > 0) {
            Sleep(m_delayBetweenKeys);
        }
    }

    return true;
}

void PathInputter::SetDelayBetweenKeys(DWORD delayMs) {
    m_delayBetweenKeys = delayMs;
}

bool PathInputter::SendUnicodeChar(wchar_t ch) {
    std::vector<INPUT> inputs;

    if (ch == L'\n') {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wVk = VK_RETURN;
        inputs.push_back(input);

        input.ki.dwFlags = KEYEVENTF_KEYUP;
        inputs.push_back(input);
    } else {
        INPUT input = {};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = ch;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        inputs.push_back(input);

        input.ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
        inputs.push_back(input);
    }

    return SendKeyInput(inputs);
}

bool PathInputter::SendKeyInput(const std::vector<INPUT>& inputs) {
    if (inputs.empty()) {
        return true;
    }

    UINT sent = SendInput(static_cast<UINT>(inputs.size()),
                          const_cast<INPUT*>(inputs.data()),
                          sizeof(INPUT));

    return sent == inputs.size();
}

void PathInputter::ReleaseModifierKeys() {
    // Ctrl, Alt, Shift, Win キーを解放
    std::vector<INPUT> inputs;
    WORD keys[] = { VK_CONTROL, VK_MENU, VK_SHIFT, VK_LWIN, VK_RWIN };

    for (WORD vk : keys) {
        if (GetAsyncKeyState(vk) & 0x8000) {
            INPUT input = {};
            input.type = INPUT_KEYBOARD;
            input.ki.wVk = vk;
            input.ki.dwFlags = KEYEVENTF_KEYUP;
            inputs.push_back(input);
        }
    }

    if (!inputs.empty()) {
        SendInput(static_cast<UINT>(inputs.size()), inputs.data(), sizeof(INPUT));
        LOG_INFO(L"Released " + std::to_wstring(inputs.size()) + L" modifier keys");
    }
}

} // namespace Ephemery

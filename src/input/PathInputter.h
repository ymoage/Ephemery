#pragma once

#include "storage/Types.h"
#include <string>
#include <vector>
#include <Windows.h>

namespace Ephemery {

class PathInputter {
public:
    PathInputter() = default;
    ~PathInputter() = default;

    bool TypePaths(const std::vector<std::wstring>& paths, QuoteStyle quoteStyle = QuoteStyle::None);
    bool TypeText(const std::wstring& text);

    void SetDelayBetweenKeys(DWORD delayMs);

private:
    bool SendUnicodeChar(wchar_t ch);
    bool SendKeyInput(const std::vector<INPUT>& inputs);
    void ReleaseModifierKeys();

    DWORD m_delayBetweenKeys = 0;
};

} // namespace Ephemery

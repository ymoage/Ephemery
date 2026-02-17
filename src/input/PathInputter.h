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

private:
    void WaitForModifiersReleased();
    bool SetClipboardText(const std::wstring& text);
    bool SendCtrlV();
};

} // namespace Ephemery

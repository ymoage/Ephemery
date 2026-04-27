# Ephemery v1.1.0

新しいホットキー「パスのクリア」を追加しました。

## What's New

✨ **パスのクリア機能** - 取得済みパスのリストを貼り付けずにクリアできるホットキーを追加
  - デフォルト: `Ctrl+Alt+X`
  - 設定ダイアログから変更可能
  - 間違ってキャプチャしたときに、貼り付けずにリストだけ取り消せます
  - ファイル本体は残るので、保持上限による自動削除に任せられます

## Download

- [Ephemery.exe (Release)](https://github.com/ymoage/Ephemery/releases/download/v1.1.0/Ephemery.exe)

---

**Full Changelog**: https://github.com/ymoage/Ephemery/compare/v1.0.2...v1.1.0

---

# Ephemery v1.0.1

Bug fix release with new quote style feature.

## What's New

✨ **Quote Style Option** - Wrap file paths with quotes (None/Double/Single/Backtick)

## Bug Fixes

🐛 **Settings Dialog** - Fixed OK/Cancel buttons not visible (incorrect window size calculation)
🐛 **Startup Behavior** - Removed unnecessary file loading on app startup
🐛 **Hotkey Persistence** - Fixed hardcoded hotkey values in settings file

## Download

- [Ephemery.exe (Release)](https://github.com/ymoage/Ephemery/releases/download/v1.0.1/Ephemery.exe)

---

**Full Changelog**: https://github.com/ymoage/Ephemery/compare/v1.0...v1.0.1

---

_Bug fixes verified by Claude Code - Ephemery's first user!_ 🐛✅

---

# Ephemery v1.0

First stable release of Ephemery - A lightweight Windows screenshot utility for quick file path access.

## What's Ephemery?

Ephemery captures screenshots and clipboard images to temporary files, allowing you to paste their file paths instantly via hotkeys.

**Perfect for AI workflows!** Share screenshots with AI assistants (Claude Code, ChatGPT, Gemini, etc.) without manual file management. Just capture, paste the path, and let your AI read it.

Also great for:
- Documentation tools that need file paths
- Bug reporting workflows
- Quick image sharing in chat/email (as file paths)
- Any scenario where you need temporary image files

## Features

✨ **Screen Capture** - Snipping Tool-like region/window selection
📋 **Clipboard Save** - Save clipboard images as files
⌨️ **Path Pasting** - Type file paths directly via hotkey
⚙️ **Customizable** - Configure hotkeys and retention limit
🔄 **Auto-cleanup** - FIFO management (default: 10 files)
🪶 **Lightweight** - Only 181KB

## Default Hotkeys

- `Ctrl+Alt+S` - Screen capture
- `Ctrl+Alt+C` - Save clipboard image
- `Ctrl+Alt+P` - Paste file paths

## Installation

1. Download `Ephemery.exe`
2. Run it (may require VC++ Redistributable 2022)
3. App appears in system tray

No installation or admin rights required.

## System Requirements

- Windows 10/11 (x64)
- Visual C++ Redistributable 2022 (usually pre-installed)

## Known Limitations

- Windows only
- Paths are typed using SendInput (requires focus in text input field)
- Images are stored in `%TEMP%\Ephemery\` (cleared on Windows cleanup)

## License

MIT License - Free for personal and commercial use.

---

**Full Changelog**: https://github.com/ymoage/Ephemery/commits/main

---

_This release note was written by Claude Code - proving that Ephemery works! Screenshot → `Ctrl+Alt+P` → AI reads it._ 🤖

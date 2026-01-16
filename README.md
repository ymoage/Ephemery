# Ephemery

A Windows tray application for capturing screenshots and clipboard images to temporary files, with hotkey-triggered path pasting.

**Ephemery** (ephemeral + memory) - Temporary screenshot storage for quick file path access.

> 💡 **Perfect for AI workflows!** Quickly capture and share screenshots with AI assistants (Claude, ChatGPT, etc.) by pasting file paths instead of uploading images manually.

## Why Ephemery?

Ever wanted to share a screenshot with an AI assistant but found copy-pasting images cumbersome? Ephemery solves this:

1. **Capture** - Take a screenshot with `Ctrl+Alt+S`
2. **Paste Path** - Press `Ctrl+Alt+P` to type the file path
3. **AI Ready** - Your AI assistant (Claude Code, ChatGPT, etc.) can now read the image

No more:
- ❌ Manual file saving and navigation
- ❌ Drag-and-drop gymnastics
- ❌ Uploading images one by one

Just capture, paste path, done.

## Features

- 📸 **Screen Capture** - Snipping Tool-like region/window selection
- 📋 **Clipboard Save** - Capture images from clipboard
- ⌨️ **Path Pasting** - Paste file paths via hotkey (SendInput)
- ⚙️ **Customizable Settings** - Configure hotkeys and file retention limit
- 🔄 **FIFO Management** - Automatic old file deletion (default: 10 files)
- 🪶 **Lightweight** - Only 181KB (Release build)

## Screenshot

<!-- TODO: Add screenshot here -->

## Installation

### Download

Download the latest release from [Releases](https://github.com/ymoage/Ephemery/releases) page.

### Requirements

- Windows 10/11 (x64)
- Visual C++ Redistributable 2022 (usually pre-installed)

## Usage

### Default Hotkeys

| Hotkey | Action |
|--------|--------|
| `Ctrl+Alt+S` | Screen capture (region/window selection) |
| `Ctrl+Alt+C` | Save clipboard image |
| `Ctrl+Alt+P` | Paste file paths |

### How It Works

1. **Capture**: Press `Ctrl+Alt+S` to capture screen, or `Ctrl+Alt+C` to save clipboard image
2. **Auto-save**: Images are saved to `%TEMP%\Ephemery\` as PNG files
3. **Paste**: Press `Ctrl+Alt+P` to type all file paths (newline-separated)
4. **Auto-cleanup**: Old files are deleted when limit is reached (FIFO)

### How Images Are Stored

Ephemery stores captured images in your system's temporary folder:

**Storage Location**: `%TEMP%\Ephemery\`

**File Naming**: `ephemery_YYYYMMDD_HHMMSS_N.png`
- Date/time stamp for easy identification
- Sequential number to prevent conflicts

**Lifecycle**:
1. Images are saved immediately after capture
2. File paths are tracked in memory
3. When retention limit is reached (default: 10), oldest files are deleted (FIFO)
4. On app restart, existing images in the folder are preserved but not loaded into the path list
5. Images remain on disk until overwritten or manually deleted

**Why Temporary Storage?**
- Fast access (no network/cloud dependency)
- Automatic cleanup by Windows (eventually)
- No cloud storage quota/privacy concerns

### Settings

Right-click the tray icon → **Settings** to:
- Change hotkeys
- Adjust file retention limit (1-100)

## Building from Source

### Prerequisites

- Visual Studio 2022
- CMake 3.20+
- Windows 10/11 SDK

### Build Steps

```bash
# Clone repository
git clone https://github.com/ymoage/Ephemery.git
cd Ephemery

# Build with CMake
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release

# Output: build/Release/Ephemery.exe
```

Or open the folder in Visual Studio 2022 as a CMake project.

## Technical Details

- **Language**: C++17
- **UI Framework**: Pure Win32 API
- **Graphics**: GDI+ (PNG encoding), BitBlt (screen capture)
- **Hotkeys**: RegisterHotKey API
- **Input**: SendInput API
- **Settings**: JSON (%APPDATA%\Ephemery\settings.json)

## License

MIT License - see [LICENSE](LICENSE) file for details.

## Acknowledgments

Built with assistance from Claude (Anthropic).

---

_This README was written by Claude Code - the very AI assistant this tool was designed to work with. Meta, isn't it?_ 🤖

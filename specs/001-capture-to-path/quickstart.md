# Quickstart: Ephemery - Capture to Path Tool

**Date**: 2026-01-11
**Branch**: `001-capture-to-path`

## Prerequisites

- **OS**: Windows 10/11 x64
- **IDE**: Visual Studio 2022 (Community以上)
- **Workload**: "Desktop development with C++" をインストール
- **CMake**: 3.20以上 (Visual Studio付属でOK)

## Setup

### 1. Clone & Checkout

```powershell
git clone <repository-url>
cd Ephemery
git checkout 001-capture-to-path
```

### 2. Create Build Directory

```powershell
mkdir build
cd build
```

### 3. Generate Project

```powershell
cmake .. -G "Visual Studio 17 2022" -A x64
```

### 4. Open in Visual Studio

```powershell
start Ephemery.sln
```

Or build from command line:

```powershell
cmake --build . --config Release
```

## Project Structure

```
Ephemery/
├── CMakeLists.txt           # Build configuration
├── src/
│   ├── main.cpp             # Entry point
│   ├── app/                 # Application core
│   ├── capture/             # Screen capture
│   ├── hotkey/              # Hotkey management
│   ├── storage/             # Image storage
│   ├── input/               # Keyboard input
│   └── util/                # Utilities
├── tests/                   # Unit tests
├── resources/               # Icons, manifests
└── specs/                   # Specifications
```

## Build Targets

| Target | Description |
|--------|-------------|
| `Ephemery` | Main executable |
| `EphemeryTests` | Unit tests (Google Test) |

## Development Workflow

### 1. Add New Feature

1. Create header/source in appropriate directory
2. Add to `CMakeLists.txt`
3. Write unit tests in `tests/unit/`
4. Build and test

### 2. Run Tests

```powershell
cd build
ctest -C Release --output-on-failure
```

### 3. Debug

1. Set `Ephemery` as startup project
2. F5 to start debugging
3. Tray icon appears in notification area

## Key APIs Reference

| Feature | API | Header |
|---------|-----|--------|
| Global Hotkey | `RegisterHotKey` | `<Windows.h>` |
| Screen Capture | `BitBlt`, `GetDC` | `<Windows.h>` |
| PNG Encoding | `Gdiplus::Bitmap` | `<gdiplus.h>` |
| Key Input | `SendInput` | `<Windows.h>` |
| System Tray | `Shell_NotifyIcon` | `<shellapi.h>` |
| Clipboard | `OpenClipboard`, `GetClipboardData` | `<Windows.h>` |

## Configuration Files

| File | Location | Purpose |
|------|----------|---------|
| `settings.json` | `%APPDATA%\Ephemery\` | User settings |
| Captured images | `%TEMP%\Ephemery\` | Temporary storage |

## Default Hotkeys

| Action | Default | Description |
|--------|---------|-------------|
| Capture Screen | `Win + Shift + S` | Start region/window selection |
| Save Clipboard | `Win + Shift + C` | Save clipboard image to file |
| Paste Paths | `Win + Shift + V` | Type all image paths |

## Troubleshooting

### Hotkey not working

1. Check if another app registered the same hotkey
2. Run as Administrator (some hotkeys require elevation)
3. Check Windows Event Viewer for errors

### Capture fails on certain windows

1. Some windows (DirectX games) require special capture methods
2. Try window capture mode instead of region selection

### PNG not saved

1. Check disk space
2. Verify `%TEMP%\Ephemery\` directory permissions
3. Check GDI+ initialization

## Next Steps

After setup:

1. Review `specs/001-capture-to-path/spec.md` for requirements
2. Review `specs/001-capture-to-path/data-model.md` for data structures
3. Review `specs/001-capture-to-path/contracts/interfaces.md` for interfaces
4. Run `/speckit.tasks` to generate implementation tasks

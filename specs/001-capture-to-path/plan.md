# Implementation Plan: Ephemery - Capture to Path Tool

**Branch**: `001-capture-to-path` | **Date**: 2026-01-11 | **Spec**: [spec.md](./spec.md)
**Input**: Feature specification from `/specs/001-capture-to-path/spec.md`

## Summary

画面キャプチャ/クリップボード画像を一時ファイルに保存し、ホットキーでファイルパスを直接入力できるWindows常駐アプリを実装する。Win32 APIを使用したシステムトレイアプリとして、グローバルホットキー、画面キャプチャ、クリップボード操作、キーストローク送信を実現する。

## Technical Context

**Language/Version**: C++17 (MSVC, Visual Studio 2022)
**Primary Dependencies**: Win32 API (RegisterHotKey, GDI+, Clipboard API, SendInput), libpng or GDI+ for PNG encoding
**Storage**: File system (%TEMP%\Ephemery), JSON/INI for settings
**Testing**: Google Test (gtest) for unit tests, manual integration tests
**Target Platform**: Windows 10/11 x64
**Project Type**: Single Windows desktop application
**Performance Goals**: Capture complete < 1 second, Path input < 500ms
**Constraints**: Memory < 50MB, Non-blocking UI during capture
**Scale/Scope**: Personal utility, single user

## Constitution Check

*GATE: Must pass before Phase 0 research. Re-check after Phase 1 design.*

Constitution is not yet defined (template state). No gates to check.
Proceeding with standard best practices for Windows C++ development.

## Project Structure

### Documentation (this feature)

```text
specs/001-capture-to-path/
├── plan.md              # This file
├── research.md          # Phase 0 output
├── data-model.md        # Phase 1 output
├── quickstart.md        # Phase 1 output
├── contracts/           # Phase 1 output (internal interfaces)
└── tasks.md             # Phase 2 output (/speckit.tasks command)
```

### Source Code (repository root)

```text
src/
├── main.cpp             # Entry point, message loop
├── app/
│   ├── Application.h/cpp    # Main application class
│   ├── TrayIcon.h/cpp       # System tray icon management
│   └── SettingsDialog.h/cpp # Settings UI
├── capture/
│   ├── ScreenCapture.h/cpp      # Screen capture logic
│   ├── RegionSelector.h/cpp     # Snipping tool-like UI
│   ├── WindowSelector.h/cpp     # Window selection logic
│   └── ClipboardCapture.h/cpp   # Clipboard image capture
├── hotkey/
│   ├── HotkeyManager.h/cpp      # Global hotkey registration
│   └── HotkeyConfig.h/cpp       # Hotkey configuration
├── storage/
│   ├── ImageStorage.h/cpp       # File management (save, delete, list)
│   └── Settings.h/cpp           # Settings persistence
├── input/
│   └── PathInputter.h/cpp       # SendInput-based path typing
└── util/
    ├── PngEncoder.h/cpp         # PNG encoding
    └── Logger.h/cpp             # Logging utility

tests/
├── unit/
│   ├── ImageStorageTest.cpp
│   ├── HotkeyConfigTest.cpp
│   └── SettingsTest.cpp
└── integration/
    └── CaptureFlowTest.cpp

resources/
├── icon.ico             # Tray icon
└── app.manifest         # Windows manifest (DPI awareness, UAC)
```

**Structure Decision**: Single project structure with modular separation by feature domain (capture, hotkey, storage, input). No external frameworks for UI - pure Win32 for minimal footprint.

## Complexity Tracking

No constitution violations to justify. Design follows YAGNI principles with minimal dependencies.

| Aspect | Decision | Rationale |
|--------|----------|-----------|
| UI Framework | Pure Win32 | Minimal footprint, no external dependencies |
| PNG Encoding | GDI+ | Built into Windows, no external library needed |
| Settings Format | JSON | Human-readable, easy to parse with lightweight library |

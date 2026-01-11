# Research: Ephemery - Capture to Path Tool

**Date**: 2026-01-11
**Branch**: `001-capture-to-path`

## Overview

Windows C++でのスクリーンキャプチャツール実装に必要な技術調査結果。

---

## 1. Global Hotkey Registration

**Decision**: Win32 `RegisterHotKey` API を使用

**Rationale**:
- Windows標準APIで追加依存なし
- フォーカス外でもキー入力を検出可能
- MOD_ALT, MOD_CONTROL, MOD_SHIFT, MOD_WIN を組み合わせ可能

**Implementation Pattern**:
```cpp
// 登録
RegisterHotKey(hwnd, HOTKEY_ID_CAPTURE, MOD_WIN | MOD_SHIFT, 'S');

// メッセージループで処理
case WM_HOTKEY:
    if (wParam == HOTKEY_ID_CAPTURE) { StartCapture(); }
    break;

// 解除
UnregisterHotKey(hwnd, HOTKEY_ID_CAPTURE);
```

**Alternatives Considered**:
- Low-level keyboard hook (`SetWindowsHookEx`): より柔軟だが複雑、パフォーマンスオーバーヘッド
- Raw Input API: ゲーム向け、この用途には過剰

---

## 2. Screen Capture

**Decision**: GDI `BitBlt` + `GetDC(NULL)` for desktop capture

**Rationale**:
- Windows全バージョンで動作
- マルチモニター対応 (`GetSystemMetrics(SM_XVIRTUALSCREEN)` 等)
- DirectX/OpenGLアプリはキャプチャ不可だが、一般用途には十分

**Implementation Pattern**:
```cpp
HDC hdcScreen = GetDC(NULL);
HDC hdcMem = CreateCompatibleDC(hdcScreen);
HBITMAP hBitmap = CreateCompatibleBitmap(hdcScreen, width, height);
SelectObject(hdcMem, hBitmap);
BitBlt(hdcMem, 0, 0, width, height, hdcScreen, x, y, SRCCOPY);
// HBITMAPをPNGとして保存
```

**Alternatives Considered**:
- Windows Graphics Capture API (Windows 10 1803+): モダンだが複雑、UWP依存
- DXGI Desktop Duplication: 高パフォーマンスだが複雑

---

## 3. Region Selection UI (Snipping Tool Style)

**Decision**: 全画面オーバーレイウィンドウ + GDI描画

**Rationale**:
- フルスクリーン半透明ウィンドウでオーバーレイ
- マウスドラッグで矩形選択
- 選択領域をリアルタイム描画

**Implementation Pattern**:
```cpp
// 1. 全画面透明ウィンドウ作成 (WS_EX_LAYERED, WS_EX_TOPMOST)
// 2. SetLayeredWindowAttributes で半透明背景
// 3. WM_LBUTTONDOWN で開始点記録
// 4. WM_MOUSEMOVE で選択矩形を描画 (XOR描画またはダブルバッファ)
// 5. WM_LBUTTONUP で領域確定
// 6. ESCキーでキャンセル
```

**Key Considerations**:
- マルチモニター: 仮想デスクトップ全体をカバー
- DPI対応: Per-monitor DPI awareness が必要
- クロスヘアカーソル表示

---

## 4. Window Selection

**Decision**: `EnumWindows` + マウス位置のウィンドウ検出

**Rationale**:
- `WindowFromPoint` で基本検出
- `GetAncestor(hwnd, GA_ROOT)` でトップレベルウィンドウ取得
- ホバー時にウィンドウ枠をハイライト

**Implementation Pattern**:
```cpp
// マウス位置からウィンドウ取得
POINT pt;
GetCursorPos(&pt);
HWND hwnd = WindowFromPoint(pt);
hwnd = GetAncestor(hwnd, GA_ROOT);

// ウィンドウ矩形取得
RECT rc;
GetWindowRect(hwnd, &rc);

// キャプチャ
BitBlt(..., rc.left, rc.top, rc.right - rc.left, rc.bottom - rc.top, ...);
```

**Edge Cases**:
- 最小化ウィンドウは除外 (`IsIconic`)
- 非表示ウィンドウは除外 (`IsWindowVisible`)

---

## 5. Clipboard Image Capture

**Decision**: Win32 Clipboard API (`CF_DIB` format)

**Rationale**:
- `CF_BITMAP` より `CF_DIB` の方がポータブル
- ほとんどのアプリが DIB 形式でコピー

**Implementation Pattern**:
```cpp
if (OpenClipboard(hwnd)) {
    if (IsClipboardFormatAvailable(CF_DIB)) {
        HANDLE hData = GetClipboardData(CF_DIB);
        BITMAPINFO* pBmi = (BITMAPINFO*)GlobalLock(hData);
        // DIBからHBITMAP作成、PNG保存
        GlobalUnlock(hData);
    }
    CloseClipboard();
}
```

**Alternatives Considered**:
- `CF_BITMAP`: 互換性は低い
- `CF_PNG`: 一部アプリのみサポート、フォールバックとして検討

---

## 6. PNG Encoding

**Decision**: GDI+ (`Gdiplus::Bitmap::Save`)

**Rationale**:
- Windows標準、追加DLL不要
- PNG, JPEG, BMP など複数形式対応
- HBITMAP からの変換が容易

**Implementation Pattern**:
```cpp
#include <gdiplus.h>
#pragma comment(lib, "gdiplus.lib")

// 初期化 (アプリ起動時)
Gdiplus::GdiplusStartupInput input;
ULONG_PTR token;
Gdiplus::GdiplusStartup(&token, &input, NULL);

// HBITMAP → PNG保存
Gdiplus::Bitmap bitmap(hBitmap, NULL);
CLSID pngClsid;
GetEncoderClsid(L"image/png", &pngClsid);
bitmap.Save(L"C:\\path\\to\\file.png", &pngClsid, NULL);
```

---

## 7. Keystroke Injection (Path Input)

**Decision**: `SendInput` API

**Rationale**:
- 現在のフォーカスウィンドウにキーストローク送信
- Unicode文字対応 (`KEYEVENTF_UNICODE`)
- 高速、信頼性が高い

**Implementation Pattern**:
```cpp
void TypeString(const std::wstring& text) {
    std::vector<INPUT> inputs;
    for (wchar_t ch : text) {
        INPUT input = {0};
        input.type = INPUT_KEYBOARD;
        input.ki.wScan = ch;
        input.ki.dwFlags = KEYEVENTF_UNICODE;
        inputs.push_back(input);

        input.ki.dwFlags |= KEYEVENTF_KEYUP;
        inputs.push_back(input);
    }
    SendInput((UINT)inputs.size(), inputs.data(), sizeof(INPUT));
}
```

**Considerations**:
- 改行は `\n` を `VK_RETURN` として送信
- 日本語パスも Unicode で対応可能

---

## 8. System Tray Icon

**Decision**: `Shell_NotifyIcon` API

**Rationale**:
- Windows標準のタスクトレイ実装
- コンテキストメニュー、バルーン通知対応

**Implementation Pattern**:
```cpp
NOTIFYICONDATA nid = {0};
nid.cbSize = sizeof(nid);
nid.hWnd = hwnd;
nid.uID = 1;
nid.uFlags = NIF_ICON | NIF_MESSAGE | NIF_TIP;
nid.uCallbackMessage = WM_TRAYICON;
nid.hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_ICON));
wcscpy_s(nid.szTip, L"Ephemery");
Shell_NotifyIcon(NIM_ADD, &nid);

// コンテキストメニュー (WM_RBUTTONUPで表示)
HMENU hMenu = CreatePopupMenu();
AppendMenu(hMenu, MF_STRING, ID_SETTINGS, L"設定");
AppendMenu(hMenu, MF_SEPARATOR, 0, NULL);
AppendMenu(hMenu, MF_STRING, ID_EXIT, L"終了");
TrackPopupMenu(hMenu, TPM_RIGHTBUTTON, pt.x, pt.y, 0, hwnd, NULL);
```

---

## 9. Settings Persistence

**Decision**: JSON file with nlohmann/json or RapidJSON

**Rationale**:
- 人間が読める形式
- 構造化データに適している
- 軽量ライブラリで十分

**Alternative**: INI file with `GetPrivateProfileString` (Win32標準だがフラット構造)

**Settings Location**: `%APPDATA%\Ephemery\settings.json`

**Schema**:
```json
{
  "version": 1,
  "hotkeys": {
    "capture": { "modifiers": ["Win", "Shift"], "key": "S" },
    "clipboardSave": { "modifiers": ["Win", "Shift"], "key": "C" },
    "pastePaths": { "modifiers": ["Win", "Shift"], "key": "V" }
  },
  "maxImages": 10
}
```

---

## 10. File Naming Strategy

**Decision**: Timestamp-based naming with counter

**Rationale**:
- 一意性を保証
- ソート順が時系列に
- 連番管理よりシンプル

**Pattern**: `ephemery_YYYYMMDD_HHMMSS_NNN.png`
- 例: `ephemery_20260111_143052_001.png`

**Storage Location**: `%TEMP%\Ephemery\`

---

## Summary of Technology Stack

| Component | Technology | Notes |
|-----------|------------|-------|
| Language | C++17 | MSVC/Visual Studio 2022 |
| UI | Pure Win32 | No external framework |
| Hotkeys | RegisterHotKey | Win32 API |
| Screen Capture | GDI (BitBlt) | Multi-monitor aware |
| Region Selection | Overlay window | Full-screen layered window |
| Clipboard | Win32 Clipboard API | CF_DIB format |
| PNG Encoding | GDI+ | Built into Windows |
| Key Input | SendInput | Unicode support |
| Tray Icon | Shell_NotifyIcon | Standard system tray |
| Settings | JSON (RapidJSON) | Lightweight, header-only |
| Build | CMake | Cross-IDE support |

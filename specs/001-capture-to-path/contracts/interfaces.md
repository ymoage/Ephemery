# Internal Interfaces: Ephemery

**Date**: 2026-01-11
**Branch**: `001-capture-to-path`

## Overview

Ephemeryアプリケーションのモジュール間インターフェース定義。

---

## 1. ICaptureProvider

画面キャプチャ機能のインターフェース。

```cpp
class ICaptureProvider {
public:
    virtual ~ICaptureProvider() = default;

    // 矩形領域をキャプチャ
    // @param rect キャプチャする領域 (スクリーン座標)
    // @return キャプチャしたビットマップのハンドル (呼び出し元が解放責任)
    virtual HBITMAP CaptureRegion(const RECT& rect) = 0;

    // ウィンドウ全体をキャプチャ
    // @param hwnd キャプチャ対象のウィンドウハンドル
    // @return キャプチャしたビットマップのハンドル
    virtual HBITMAP CaptureWindow(HWND hwnd) = 0;
};
```

**Implementations**: `ScreenCapture`

---

## 2. IRegionSelector

領域選択UIのインターフェース。

```cpp
// 選択結果
struct SelectionResult {
    enum class Type { Region, Window, Cancelled };
    Type type;
    RECT region;      // Type::Region の場合に有効
    HWND window;      // Type::Window の場合に有効
};

class IRegionSelector {
public:
    virtual ~IRegionSelector() = default;

    // 領域選択UIを開始 (ブロッキング)
    // @return 選択結果
    virtual SelectionResult StartSelection() = 0;

    // 選択をキャンセル
    virtual void Cancel() = 0;
};
```

**Implementations**: `RegionSelector`

---

## 3. IClipboardReader

クリップボード読み取りのインターフェース。

```cpp
class IClipboardReader {
public:
    virtual ~IClipboardReader() = default;

    // クリップボードに画像があるか確認
    virtual bool HasImage() const = 0;

    // クリップボードから画像を取得
    // @return ビットマップハンドル (画像がない場合は NULL)
    virtual HBITMAP GetImage() = 0;
};
```

**Implementations**: `ClipboardCapture`

---

## 4. IImageEncoder

画像エンコーダーのインターフェース。

```cpp
class IImageEncoder {
public:
    virtual ~IImageEncoder() = default;

    // ビットマップをPNGファイルとして保存
    // @param hBitmap 保存するビットマップ
    // @param filePath 保存先パス
    // @return 成功した場合 true
    virtual bool SaveAsPng(HBITMAP hBitmap, const std::wstring& filePath) = 0;
};
```

**Implementations**: `PngEncoder` (GDI+ベース)

---

## 5. IImageStorage

画像ファイル管理のインターフェース。

```cpp
class IImageStorage {
public:
    virtual ~IImageStorage() = default;

    // 画像を保存し、ストアに追加
    // @param hBitmap 保存するビットマップ
    // @param source キャプチャ元
    // @return 保存したファイルのパス (失敗時は空文字列)
    virtual std::wstring SaveImage(HBITMAP hBitmap, CaptureSource source) = 0;

    // 保持している全画像のパスを取得
    // @return ファイルパスのリスト (古い順)
    virtual std::vector<std::wstring> GetAllPaths() const = 0;

    // 画像数を取得
    virtual size_t GetCount() const = 0;

    // 全画像をクリア
    virtual void Clear() = 0;
};
```

**Implementations**: `ImageStorage`

---

## 6. IHotkeyManager

ホットキー管理のインターフェース。

```cpp
// ホットキーイベントハンドラ
using HotkeyHandler = std::function<void(HotkeyAction)>;

class IHotkeyManager {
public:
    virtual ~IHotkeyManager() = default;

    // ホットキーを登録
    // @param binding ホットキー設定
    // @return 成功した場合 true
    virtual bool Register(const HotkeyBinding& binding) = 0;

    // 全ホットキーを解除
    virtual void UnregisterAll() = 0;

    // ホットキー設定を更新
    // @param bindings 新しいホットキー設定
    // @return 成功した場合 true
    virtual bool UpdateBindings(const std::array<HotkeyBinding, 3>& bindings) = 0;

    // イベントハンドラを設定
    virtual void SetHandler(HotkeyHandler handler) = 0;

    // WM_HOTKEYメッセージを処理
    // @param wParam WM_HOTKEYのwParam (ホットキーID)
    virtual void HandleMessage(WPARAM wParam) = 0;
};
```

**Implementations**: `HotkeyManager`

---

## 7. IPathInputter

パス入力のインターフェース。

```cpp
class IPathInputter {
public:
    virtual ~IPathInputter() = default;

    // パス文字列を現在のフォーカスウィンドウに入力
    // @param paths パスのリスト
    // @param separator 区切り文字 (デフォルトは改行)
    virtual void TypePaths(const std::vector<std::wstring>& paths,
                           const std::wstring& separator = L"\n") = 0;
};
```

**Implementations**: `PathInputter` (SendInputベース)

---

## 8. ISettingsManager

設定管理のインターフェース。

```cpp
class ISettingsManager {
public:
    virtual ~ISettingsManager() = default;

    // 設定を読み込み
    // @return 読み込んだ設定 (失敗時はデフォルト値)
    virtual AppSettings Load() = 0;

    // 設定を保存
    // @param settings 保存する設定
    // @return 成功した場合 true
    virtual bool Save(const AppSettings& settings) = 0;

    // デフォルト設定を取得
    virtual AppSettings GetDefaults() const = 0;
};
```

**Implementations**: `Settings` (JSONベース)

---

## 9. IApplication

アプリケーションコアのインターフェース。

```cpp
class IApplication {
public:
    virtual ~IApplication() = default;

    // 初期化
    virtual bool Initialize(HINSTANCE hInstance) = 0;

    // メインループ実行
    virtual int Run() = 0;

    // 終了処理
    virtual void Shutdown() = 0;

    // 設定画面を表示
    virtual void ShowSettings() = 0;

    // キャプチャを開始
    virtual void StartCapture() = 0;

    // クリップボード画像を保存
    virtual void SaveClipboardImage() = 0;

    // パスを入力
    virtual void PastePaths() = 0;
};
```

**Implementations**: `Application`

---

## Dependency Graph

```
Application
    ├── IHotkeyManager
    ├── ISettingsManager
    ├── IImageStorage
    │       └── IImageEncoder
    ├── IRegionSelector
    ├── ICaptureProvider
    ├── IClipboardReader
    └── IPathInputter
```

---

## Event Flow

### Capture Flow
```
[Hotkey: CaptureScreen]
    → IHotkeyManager.HandleMessage()
    → Application.StartCapture()
    → IRegionSelector.StartSelection()
    → ICaptureProvider.CaptureRegion() or CaptureWindow()
    → IImageStorage.SaveImage()
```

### Clipboard Save Flow
```
[Hotkey: SaveClipboard]
    → IHotkeyManager.HandleMessage()
    → Application.SaveClipboardImage()
    → IClipboardReader.GetImage()
    → IImageStorage.SaveImage()
```

### Path Paste Flow
```
[Hotkey: PastePaths]
    → IHotkeyManager.HandleMessage()
    → Application.PastePaths()
    → IImageStorage.GetAllPaths()
    → IPathInputter.TypePaths()
```

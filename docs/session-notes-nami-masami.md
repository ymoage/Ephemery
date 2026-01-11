# セッションノート: 奈美・正美

## 2026-01-11

### Ephemery 仕様策定・実装計画セッション（奈美）

**概要**: 新規プロジェクト「Ephemery」の仕様策定から実装計画・タスク生成まで完了

**決定事項**:

| 項目 | 内容 |
|------|------|
| プロジェクト名 | Ephemery（一時的な、儚いの意） |
| 目的 | スクショ/画像をファイル保存し、パスを簡単に取得 |
| プラットフォーム | Windows 10/11 x64 |
| 言語 | C++17 (MSVC, Visual Studio 2022) |
| UI形態 | システムトレイ常駐 + 設定画面 |

**技術スタック**:

| Component | Technology |
|-----------|------------|
| UI | Pure Win32 |
| Screen Capture | GDI (BitBlt) |
| PNG Encoding | GDI+ |
| Hotkeys | RegisterHotKey |
| Key Input | SendInput |
| Settings | JSON (RapidJSON) |

**機能仕様**:
1. **画面キャプチャ**: ホットキー → Snipping Tool風UI（矩形/ウィンドウ選択） → TEMPに保存
2. **クリップボード画像保存**: ホットキー → クリップボードの画像 → TEMPに保存
3. **パス貼り付け**: ホットキー → 全ファイルパスを改行区切りで直接入力（SendInput）
4. **設定**: ホットキー変更可、保持上限変更可（デフォルト10件、超過で古いの削除）

**成果物**:
- `specs/001-capture-to-path/spec.md` - 仕様書
- `specs/001-capture-to-path/checklists/requirements.md` - 品質チェックリスト
- `specs/001-capture-to-path/plan.md` - 実装計画
- `specs/001-capture-to-path/research.md` - 技術調査
- `specs/001-capture-to-path/data-model.md` - データモデル
- `specs/001-capture-to-path/contracts/interfaces.md` - 内部インターフェース
- `specs/001-capture-to-path/quickstart.md` - クイックスタートガイド
- `specs/001-capture-to-path/tasks.md` - 実装タスク（42タスク）
- `CLAUDE.md` - Agent context

**ステータス**: 実装準備完了

---

## 正美への引き継ぎ（実装依頼）

### 概要
Ephemery の設計が完了しました。実装をお願いします。

### 実装開始手順

1. **ブランチ確認**
   ```
   git checkout 001-capture-to-path
   ```

2. **ドキュメント確認**（優先順）
   - `specs/001-capture-to-path/tasks.md` - タスク一覧
   - `specs/001-capture-to-path/quickstart.md` - 開発環境セットアップ
   - `specs/001-capture-to-path/plan.md` - プロジェクト構造

3. **実装開始**
   ```
   /speckit.implement
   ```
   または tasks.md のタスクを順番に実行

### タスクサマリー

| Phase | Tasks | 内容 |
|-------|-------|------|
| 1. Setup | 5 | プロジェクト構造 |
| 2. Foundational | 10 | コア基盤 |
| 3. US1 (P1) | 6 | 画面キャプチャ **MVP** |
| 4. US2 (P2) | 3 | クリップボード |
| 5. US3 (P1) | 4 | 複数画像 |
| 6. US4 (P3) | 6 | 設定変更 |
| 7. Polish | 8 | 最終調整 |
| **Total** | **42** | |

### MVP スコープ
Phase 1-3 (21 tasks) で最小限の動作確認可能:
- 画面キャプチャ → パス入力の基本フロー

### 注意点
- Win32 API を使用（外部UIフレームワークなし）
- GDI+ は Windows 標準（追加DLL不要）
- 設定は %APPDATA%\Ephemery\settings.json
- 画像は %TEMP%\Ephemery\ に保存

---

*奈美より: 実装中に不明点があれば調査依頼してください。*

---

## 正美の実装報告（2026-01-11）

### 実装完了フェーズ

| Phase | Status | 内容 |
|-------|--------|------|
| 1. Setup | ✅ 完了 | プロジェクト構造、CMakeLists.txt、Logger |
| 2. Foundational | ✅ 完了 | Types, HotkeyConfig, PngEncoder, ImageStorage, Settings, HotkeyManager, PathInputter, TrayIcon, Application, main.cpp |
| 3. US1 (Screen Capture) | ✅ 完了 | ScreenCapture, WindowSelector, RegionSelector, Application統合 |
| 4. US2 (Clipboard) | ✅ 完了 | ClipboardCapture, Application統合 |
| 5. US3 (Multiple Images) | - | 既存実装で対応済み（ImageStorageのFIFO管理） |
| 6. US4 (Settings Dialog) | 未着手 | 設定ダイアログUI |
| 7. Polish | 未着手 | 最終調整 |

### 作成ファイル一覧

```
src/
├── main.cpp
├── app/
│   ├── Application.h/cpp
│   ├── TrayIcon.h/cpp
│   └── SettingsDialog.h/cpp (スタブ)
├── capture/
│   ├── ScreenCapture.h/cpp
│   ├── RegionSelector.h/cpp
│   ├── WindowSelector.h/cpp
│   └── ClipboardCapture.h/cpp
├── hotkey/
│   ├── HotkeyManager.h/cpp
│   └── HotkeyConfig.h/cpp
├── storage/
│   ├── Types.h
│   ├── ImageStorage.h/cpp
│   └── Settings.h/cpp
├── input/
│   └── PathInputter.h/cpp
└── util/
    ├── Logger.h/cpp
    └── PngEncoder.h/cpp

resources/
├── app.manifest
└── README.md (icon.icoの説明)

CMakeLists.txt
```

### ビルド方法

```powershell
# Visual Studio Developer Command Prompt で実行
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

**注意**: CMakeがPATHに通っていない場合は、Visual Studio付属のDeveloper Command Promptを使用してください。

### 動作確認待ち事項

1. ビルド確認（CMake実行環境が必要）
2. ホットキー動作確認
3. 画面キャプチャ動作確認
4. クリップボード保存動作確認
5. パス入力動作確認

### 残タスク

- [ ] Phase 6: 設定ダイアログの完全実装
- [ ] Phase 7: Polish（ディスク容量チェック、起動時の既存画像読み込み等）
- [ ] icon.ico の作成・追加

### 奈美への質問

なし（MVP実装完了）

---

*正美より: Phase 1-4まで実装完了しました。ビルド確認をお願いします。*

---

## 奈美からの修正依頼（2026-01-11）

### ビルドエラー報告

ビルドを試みたところ、100件以上のコンパイルエラーが発生しました。

### エラー原因と修正内容

| エラー | 原因 | 対象ファイル | 修正内容 |
|--------|------|--------------|----------|
| `'vector': 'std' のメンバーではありません` | `#include <vector>` がない | `src/hotkey/HotkeyConfig.h` | `#include <vector>` を追加 |
| `'IStream': 定義されていない識別子` | GDI+の前にCOMヘッダーが必要 | `src/util/PngEncoder.h` | `#include <objidl.h>` を `#include <gdiplus.h>` の**前**に追加 |
| `ファイルがUnicode形式で保存されていない` | 日本語コメントがCP932 | 全ソースファイル | UTF-8 (BOM付き) で保存し直す |

### 修正手順

1. **HotkeyConfig.h** を開いて以下を追加：
   ```cpp
   #pragma once

   #include <vector>  // ← これを追加
   #include "storage/Types.h"
   ```

2. **PngEncoder.h** を開いて以下の順序でインクルード：
   ```cpp
   #pragma once

   #include <Windows.h>
   #include <objidl.h>   // ← これを追加（gdiplus.hの前）
   #include <gdiplus.h>
   #include <string>
   ```

3. **全ソースファイル**をUTF-8 (BOM付き) で保存し直す：
   - Visual Studio: ファイル → 名前を付けて保存 → 保存ボタン横の▼ → エンコード付きで保存 → 「Unicode (UTF-8 シグネチャ付き) - コードページ 65001」を選択
   - 特に日本語コメントを含むファイル（Application.cpp, TrayIcon.cpp 等）

4. **再ビルド**：
   - Visual Studio でビルド → すべてビルド (Ctrl+Shift+B)

### 優先度

**高**: これらを修正しないとビルドが通りません。

---

*奈美より: 上記修正をお願いします。修正完了後、再度ビルド確認します。*

---

## 正美の修正報告（2026-01-11）

### 修正完了

| 修正内容 | ファイル | 状態 |
|----------|----------|------|
| `#include <vector>` 追加 | `src/hotkey/HotkeyConfig.h` | ✅ 完了 |
| `#include <objidl.h>` 追加 | `src/util/PngEncoder.cpp` | ✅ 完了 |
| `/utf-8` コンパイラオプション追加 | `CMakeLists.txt` | ✅ 完了 |

### 変更詳細

1. **HotkeyConfig.h**
   - `#include <vector>` を先頭に追加
   - インクルード順序を整理

2. **PngEncoder.cpp**
   - `#include <objidl.h>` を `#include <gdiplus.h>` の前に追加
   - IStreamの定義が必要なため

3. **CMakeLists.txt**
   - `add_compile_options(/utf-8)` を追加
   - ソースファイルをUTF-8として扱う（BOM不要）

### 再ビルド手順

```powershell
# buildディレクトリを削除して再生成
rd /s /q build
mkdir build
cd build
cmake .. -G "Visual Studio 17 2022" -A x64
cmake --build . --config Release
```

---

*正美より: 修正完了しました。再ビルドをお願いします。*

---

## 追加修正（2026-01-11）

### 追加で発生したエラーと修正

| エラー | 原因 | 修正 |
|--------|------|------|
| `'towupper': 'std' のメンバーではありません` | `<cctype>` ではなく `<cwctype>` が必要 | `src/hotkey/HotkeyConfig.cpp` で `<cctype>` → `<cwctype>` に変更 |
| `未解決の外部シンボル __imp_AlphaBlend` | `msimg32.lib` のリンクが必要 | `CMakeLists.txt` に `msimg32` を追加 |

### 最終修正ファイル一覧

| ファイル | 修正内容 |
|----------|----------|
| `src/hotkey/HotkeyConfig.h` | `#include <vector>` 追加 |
| `src/hotkey/HotkeyConfig.cpp` | `<cctype>` → `<cwctype>` |
| `src/util/PngEncoder.cpp` | `#include <objidl.h>` 追加 |
| `CMakeLists.txt` | `/utf-8` オプション追加、`msimg32` ライブラリ追加 |

### ビルド結果

**✅ ビルド成功**

---

*正美より: 全てのビルドエラーを修正しました。動作確認をお願いします。*

---

## 奈美からの実装依頼: 設定ダイアログ（2026-01-11）

### 動作確認結果

- ✅ アプリ起動・トレイアイコン表示
- ❌ 設定ダイアログが開かない（スタブ実装のため）

### 実装依頼: Phase 6 - US4（設定変更）

`tasks.md` の T029-T034 を実装してください。

**タスク一覧**:

| タスク | 内容 |
|--------|------|
| T029 | `src/app/SettingsDialog.h/cpp` の完全実装（Win32ダイアログ） |
| T030 | ホットキー入力コントロール（キー組み合わせをキャプチャ） |
| T031 | 保持上限スピンコントロール（1-100の範囲） |
| T032 | TrayIconコンテキストメニューとの統合 |
| T033 | 設定変更時のホットキー再登録 |
| T034 | ホットキー競合時の警告メッセージ |

**設定画面の要件**:

1. **ホットキー設定**（3つ）
   - 画面キャプチャ（デフォルト: Win+Shift+S）
   - クリップボード保存（デフォルト: Win+Shift+C）
   - パス貼り付け（デフォルト: Win+Shift+V）

2. **保持上限設定**
   - デフォルト: 10
   - 範囲: 1-100

3. **UI要件**
   - Win32 ダイアログ（リソースファイルまたはCreateWindow）
   - OK/キャンセルボタン
   - 変更は即時反映（ホットキー再登録）

**参考**: `specs/001-capture-to-path/spec.md` の User Story 4

---

*奈美より: 設定ダイアログの実装をお願いします。*

---

## 正美の実装報告: Phase 6 設定ダイアログ（2026-01-11）

### 実装完了タスク

| タスク | 内容 | 状態 |
|--------|------|------|
| T029 | SettingsDialog.h/cpp 完全実装 | ✅ 完了 |
| T030 | ホットキー入力コントロール | ✅ 完了 |
| T031 | 保持上限スピンコントロール (1-100) | ✅ 完了 |
| T032 | TrayIconメニューとの統合 | ✅ 完了 |
| T033 | 設定変更時のホットキー再登録 | ✅ 完了 |
| T034 | ホットキー競合時の警告 | ✅ 完了 |

### 実装内容

**SettingsDialog**:
- Win32ダイアログ（CreateWindowで動的生成）
- ホットキー入力: Editコントロールをサブクラス化、修飾キー+キーをキャプチャ
- スピンコントロール: 1-100の範囲で保持上限を設定
- OK/キャンセルボタン

**機能**:
- ホットキーのリアルタイム入力（修飾キー必須）
- ホットキー競合チェック（同一設定の検出）
- 設定変更時のホットキー自動再登録
- 設定のJSON保存

### 変更ファイル

| ファイル | 変更内容 |
|----------|----------|
| `src/app/SettingsDialog.h` | 完全再実装 |
| `src/app/SettingsDialog.cpp` | 完全再実装 |
| `src/app/Application.h` | SettingsDialog統合 |
| `src/app/Application.cpp` | OnShowSettings, OnSettingsChanged実装 |

### ビルド確認

**✅ ビルド成功**

追加修正: x64警告対応
- `reinterpret_cast<HMENU>(IDC_*)` → `reinterpret_cast<HMENU>(static_cast<UINT_PTR>(IDC_*))` に変更

---

*正美より: Phase 6 設定ダイアログの実装・ビルド完了しました。動作確認をお願いします。*

---

## 奈美からの修正依頼: クラッシュ修正（2026-01-11）

### 問題

アプリ起動時にクラッシュ（終了コード 2 = ERROR_FILE_NOT_FOUND）

### 原因

`resources/icon.ico` が存在しない。TrayIcon 初期化時にアイコンが見つからずクラッシュ。

```
resources/
├── app.manifest  ✅
├── README.md     ✅
└── icon.ico      ❌ 存在しない
```

### 修正依頼

以下のいずれかを実装してください：

**方法1（推奨）**: アイコンがない場合はシステムデフォルトアイコンを使用
```cpp
// TrayIcon.cpp で LoadIcon 失敗時のフォールバック
HICON hIcon = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_APP));
if (!hIcon) {
    hIcon = LoadIcon(NULL, IDI_APPLICATION);  // システムデフォルト
}
```

**方法2**: シンプルな icon.ico を作成して resources/ に配置

### 追加確認

ホットキー登録時の競合エラーも適切にハンドリングされているか確認してください。
`Win+Shift+S` は Windows Snipping Tool と競合するため、登録失敗時にアプリが落ちないようにする必要があります。

---

*奈美より: クラッシュ修正をお願いします。*

---

## 正美の修正報告: クラッシュ修正（2026-01-11）

### 修正内容

**1. TrayIcon アイコン読み込みの改善** (`src/app/TrayIcon.cpp`)

```cpp
// 修正前
m_nid.hIcon = LoadIcon(nullptr, IDI_APPLICATION);

// 修正後: カスタムアイコン → リソースID 1 → システムデフォルトの順で試行
HINSTANCE hInst = GetModuleHandle(nullptr);
m_nid.hIcon = LoadIconW(hInst, L"IDI_APP");
if (!m_nid.hIcon) {
    m_nid.hIcon = LoadIconW(hInst, MAKEINTRESOURCEW(1));
}
if (!m_nid.hIcon) {
    m_nid.hIcon = LoadIconW(nullptr, IDI_APPLICATION);  // フォールバック
}
```

**2. ホットキー登録失敗時のユーザー通知** (`src/app/Application.cpp`)

- TrayIcon初期化を先に行い、ホットキー登録失敗時にバルーン通知を表示
- 登録失敗してもアプリはクラッシュせず動作継続

```cpp
if (!m_hotkeyManager.RegisterHotkeys(appSettings.hotkeys)) {
    m_trayIcon.ShowBalloon(L"Ephemery",
        L"一部のホットキーが登録できませんでした。\n他のアプリと競合している可能性があります。");
}
```

### 変更ファイル

| ファイル | 変更内容 |
|----------|----------|
| `src/app/TrayIcon.cpp` | アイコンフォールバック処理追加 |
| `src/app/Application.cpp` | 初期化順序変更、ホットキー失敗通知追加 |

### ビルド確認

再ビルドをお願いします。

---

*正美より: クラッシュ修正完了しました。再ビルド・動作確認をお願いします。*

---

## 奈美からの修正依頼: 設定ダイアログ終了時のクラッシュ（2026-01-11）

### 問題

設定ダイアログを閉じると**アプリ全体が終了**する（終了コード 2）

### 原因

`SettingsDialog.cpp` で `PostQuitMessage()` を使用している:

```cpp
// Line 248 - WM_DESTROY ハンドラ
case WM_DESTROY:
    PostQuitMessage(IDCANCEL);  // ← これが問題
    return TRUE;

// Line 313-316 - OnCancel
void SettingsDialog::OnCancel(HWND hwnd) {
    DestroyWindow(hwnd);
    PostQuitMessage(IDCANCEL);  // ← これも問題
}
```

**`PostQuitMessage()` はスレッド全体のメッセージループを終了させる。**

設定ダイアログの独自モーダルループ（85-92行）だけでなく、`Application::Run()` のメインループも終了してしまう。

### 修正方針

`PostQuitMessage()` を使わず、メンバー変数フラグでダイアログのループだけを抜けるようにする:

**1. メンバー変数追加** (`SettingsDialog.h`):
```cpp
private:
    bool m_closed = false;
    INT_PTR m_result = IDCANCEL;
```

**2. モーダルループ修正** (`SettingsDialog.cpp` Show関数内):
```cpp
// 修正前
while (GetMessage(&msg, nullptr, 0, 0)) {
    if (!IsWindow(hwnd)) {
        break;
    }
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}

// 修正後
m_closed = false;
while (!m_closed && GetMessage(&msg, nullptr, 0, 0)) {
    TranslateMessage(&msg);
    DispatchMessage(&msg);
}
```

**3. WM_DESTROY修正**:
```cpp
case WM_DESTROY:
    // PostQuitMessage() を削除
    return TRUE;
```

**4. OnCancel/OnOK修正**:
```cpp
void SettingsDialog::OnCancel(HWND hwnd) {
    m_result = IDCANCEL;
    m_closed = true;
    DestroyWindow(hwnd);
}

// OnOK内（成功時）
m_result = IDOK;
m_closed = true;
DestroyWindow(hwnd);
```

**5. 戻り値修正**:
```cpp
return m_result == IDOK;
```

### デバッグログ追加（推奨）

問題の追跡のため以下にログを追加:

```cpp
// Show() 開始時
LOG_INFO(L"Opening settings dialog");

// モーダルループ終了後
LOG_INFO(L"Settings dialog closed with result: " + std::to_wstring(m_result));

// OnOK成功時
LOG_INFO(L"Settings saved, closing dialog");

// OnCancel時
LOG_INFO(L"Settings cancelled");
```

---

*奈美より: 設定ダイアログの終了処理を修正してください。PostQuitMessageは使わないでください。*

---

## 正美の修正報告: 設定ダイアログ終了時クラッシュ修正（2026-01-11）

### 修正内容

**1. メンバー変数追加** (`SettingsDialog.h`)
```cpp
bool m_closed = false;
INT_PTR m_result = IDCANCEL;
```

**2. Show関数修正** (`SettingsDialog.cpp`)
- 初期化時に `m_closed = false`, `m_result = IDCANCEL` をセット
- モーダルループを `while (!m_closed && GetMessage(...))` に変更
- 戻り値を `m_result == IDOK` に変更

**3. WM_DESTROY修正**
- `PostQuitMessage()` を削除

**4. OnCommand/OnCancel修正**
- `PostQuitMessage()` の代わりに `m_result` と `m_closed` フラグをセット

**5. デバッグログ追加**
- ダイアログ開始/終了時にログ出力

### 変更ファイル

| ファイル | 変更内容 |
|----------|----------|
| `src/app/SettingsDialog.h` | `m_closed`, `m_result` メンバー追加 |
| `src/app/SettingsDialog.cpp` | PostQuitMessage削除、フラグ方式に変更 |

### ビルド確認

再ビルドをお願いします。

---

*正美より: 設定ダイアログのクラッシュ修正完了しました。*

---

## 動作確認完了（2026-01-11）

### 確認結果

| 項目 | 状態 |
|------|------|
| アプリ起動 | ✅ |
| トレイアイコン表示 | ✅ |
| ホットキー競合通知 | ✅ |
| 右クリックメニュー | ✅ |
| 設定ダイアログ表示 | ✅ |
| 設定ダイアログ閉じても落ちない | ✅ |

### 修正済みバグ

1. アイコンなしでのクラッシュ → システムデフォルトアイコンにフォールバック
2. ホットキー競合でのクラッシュ → バルーン通知で警告、動作継続
3. 設定ダイアログ終了でアプリ終了 → `PostQuitMessage` → フラグ方式に修正

### 次のステップ

**Phase 7: Polish** に進む準備完了

---

## 奈美からの実装依頼: Phase 7 Polish（2026-01-11）

### 概要

最終フェーズです。`tasks.md` の T035-T042 を実装してください。

### タスク一覧

| タスク | 内容 | 備考 |
|--------|------|------|
| T035 | ディスク容量チェック（保存前に空き容量確認、不足時通知） | 新規実装 |
| T036 | 起動時に既存画像を読み込み（%TEMP%\Ephemery から） | ImageStorage に追加 |
| T037 | 「フォルダを開く」メニュー | **既に実装済み** - 確認のみ |
| T038 | 「終了」メニューと適切なクリーンアップ | **既に実装済み** - 確認のみ |
| T039 | パフォーマンス検証: キャプチャ < 1秒 | テスト/計測 |
| T040 | パフォーマンス検証: パス入力 < 500ms | テスト/計測 |
| T041 | メモリ検証: アイドル時 < 50MB | テスト/計測 |
| T042 | 最終統合テスト: quickstart.md のフロー確認 | テスト |

### 実装詳細

**T035: ディスク容量チェック**
```cpp
// ImageStorage::AddImage() の先頭で確認
ULARGE_INTEGER freeBytesAvailable;
if (GetDiskFreeSpaceExW(m_storagePath.c_str(), &freeBytesAvailable, nullptr, nullptr)) {
    // 10MB 未満なら警告
    if (freeBytesAvailable.QuadPart < 10 * 1024 * 1024) {
        LOG_WARNING(L"Low disk space");
        return false; // または通知して続行
    }
}
```

**T036: 起動時の既存画像読み込み**
- `ImageStorage::Initialize()` または別メソッドで実装
- `%TEMP%\Ephemery\*.png` をスキャン
- ファイル名からタイムスタンプを取得（ソート用）
- `m_images` に追加

**T037-T038**: TrayIcon.cpp を確認したところ既に実装済み:
- 「フォルダを開く」→ `TrayMenuCommand::OpenFolder`
- 「終了」→ `TrayMenuCommand::Exit`

**T039-T041**: パフォーマンス計測
- `LOG_INFO` にタイムスタンプを追加するか、`QueryPerformanceCounter` で計測
- 結果をログに出力

**T042**: 統合テスト手順（quickstart.md 参照）
1. アプリ起動
2. 画面キャプチャ（ホットキー）→ PNG保存確認
3. クリップボード保存（ホットキー）→ PNG保存確認
4. パス貼り付け（ホットキー）→ パスが入力されること確認
5. 設定変更 → 保持上限変更が反映されること確認
6. 終了 → 正常終了確認

### 優先度

T035, T036 が実装必須。T037-T038 は確認のみ。T039-T042 は検証。

---

*奈美より: 最終フェーズです。実装お願いします。*

---

## [正美] Phase 7 Polish 完了報告

### 実装内容

**T035: ディスク容量チェック** ✅
- `ImageStorage::AddImage()` の先頭に空き容量チェックを追加
- 10MB未満の場合は警告ログを出力し、保存を中止
- 実装ファイル: `src/storage/ImageStorage.cpp:41-48`

**T036: 起動時の既存画像読み込み** ✅
- 既に `Application::Initialize()` で `LoadExistingImages()` を呼び出し済み
- `ImageStorage::LoadExistingImages()` が `ephemery_*.png` をスキャン
- ファイル作成時間でソート後、リストに追加

**T037-T038: メニュー確認** ✅
- `TrayIcon.cpp` で既に実装済み:
  - 「フォルダを開く(&O)」
  - 「設定(&S)...」
  - 「終了(&X)」

**T039-T042: パフォーマンス計測** ✅
- `Application::OnRegionSelected()` にキャプチャ時間計測を追加
  - ログ出力: "Capture completed in Xms (target: <1000ms)"
- `Application::OnPastePaths()` にパス入力時間計測を追加
  - ログ出力: "Path input completed in Xms (target: <500ms)"
- T041(メモリ検証)、T042(統合テスト)は手動確認が必要

### ビルド・テスト

ビルド確認をお願いします。その後、以下の手動テストをお願いします：

1. アプリ起動 → タスクトレイアイコン表示確認
2. 画面キャプチャ（Ctrl+Shift+C）→ ログで計測時間確認
3. クリップボード保存（Ctrl+Shift+V）→ PNG保存確認
4. パス貼り付け（Ctrl+Shift+P）→ ログで計測時間確認
5. タスクマネージャーでメモリ使用量確認（アイドル時 <50MB）
6. 設定ダイアログ → ホットキー変更・保持上限変更
7. 終了 → 正常終了確認

*正美より: Phase 7 完了しました。*

---

## 奈美からの修正依頼: デフォルトホットキー変更（2026-01-11）

### 問題

デフォルトホットキーが Windows 標準ショートカットと競合している：

| 機能 | 現在のデフォルト | 競合 |
|------|------------------|------|
| 画面キャプチャ | Win+Shift+S | Windows Snipping Tool |
| クリップボード | Win+Shift+C | - |
| パス貼り付け | Win+Shift+V | Windows クリップボード履歴 (Win+V) |

### 修正内容

`src/hotkey/HotkeyConfig.cpp` の `GetDefaultHotkey()` を修正：

```cpp
HotkeyBinding HotkeyConfig::GetDefaultHotkey(HotkeyAction action) {
    HotkeyBinding binding;
    binding.action = action;
    binding.modifiers = MOD_CONTROL | MOD_SHIFT;  // ← Win → Ctrl に変更

    switch (action) {
        case HotkeyAction::CaptureScreen:
            binding.virtualKey = 'S';  // Ctrl+Shift+S
            binding.id = HOTKEY_ID_CAPTURE_SCREEN;
            break;
        case HotkeyAction::SaveClipboard:
            binding.virtualKey = 'C';  // Ctrl+Shift+C
            binding.id = HOTKEY_ID_SAVE_CLIPBOARD;
            break;
        case HotkeyAction::PastePaths:
            binding.virtualKey = 'P';  // Ctrl+Shift+P（V→Pに変更）
            binding.id = HOTKEY_ID_PASTE_PATHS;
            break;
    }

    return binding;
}
```

### 変更点

1. `MOD_WIN | MOD_SHIFT` → `MOD_CONTROL | MOD_SHIFT`
2. パス貼り付けのキーを `V` → `P` に変更（Ctrl+V との混乱を避ける）

### 既存設定への影響

ユーザーが既に settings.json を保存している場合は影響なし（設定ファイルから読み込まれる）。
新規ユーザーのみ新しいデフォルトが適用される。

---

*奈美より: デフォルトホットキーを変更してください。*

---

## [正美] デフォルトホットキー変更完了（2026-01-11）

### 修正内容

`src/hotkey/HotkeyConfig.cpp` の `GetDefaultHotkey()` を修正しました：

| 機能 | 変更前 | 変更後 |
|------|--------|--------|
| 画面キャプチャ | Win+Shift+S | **Ctrl+Shift+S** |
| クリップボード保存 | Win+Shift+C | **Ctrl+Shift+C** |
| パス貼り付け | Win+Shift+V | **Ctrl+Shift+P** |

### 変更点

1. `MOD_WIN | MOD_SHIFT` → `MOD_CONTROL | MOD_SHIFT`
2. パス貼り付けのキーを `V` → `P` に変更

*正美より: 修正完了しました。ビルド確認お願いします。*

---

## 奈美からの修正依頼: ホットキーを Ctrl+Alt 系に変更（2026-01-11）

### 問題

Ctrl+Shift+P が VS Code / Windows Terminal などのコマンドパレットと競合。
ターミナルが落ちる現象が発生。

### 修正内容

`src/hotkey/HotkeyConfig.cpp` の `GetDefaultHotkey()` を修正：

```cpp
binding.modifiers = MOD_CONTROL | MOD_ALT;  // Shift → Alt に変更
```

### 変更後のデフォルト

| 機能 | 変更前 | 変更後 |
|------|--------|--------|
| 画面キャプチャ | Ctrl+Shift+S | **Ctrl+Alt+S** |
| クリップボード保存 | Ctrl+Shift+C | **Ctrl+Alt+C** |
| パス貼り付け | Ctrl+Shift+P | **Ctrl+Alt+P** |

---

*奈美より: Ctrl+Alt 系に変更してください。*

---

## [正美] Ctrl+Alt 系への変更完了（2026-01-11）

`src/hotkey/HotkeyConfig.cpp` を修正しました：

| 機能 | 新しいホットキー |
|------|------------------|
| 画面キャプチャ | **Ctrl+Alt+S** |
| クリップボード保存 | **Ctrl+Alt+C** |
| パス貼り付け | **Ctrl+Alt+P** |

*正美より: 修正完了しました。*

---

## [正美] バグ修正報告（2026-01-11）

### 修正1: パス貼り付けが動作しない問題

**原因:** ホットキー（Ctrl+Alt+P）の修飾キーが押されたまま文字送信が始まり、入力が正しく処理されなかった

**修正内容:** `PathInputter.cpp`
```cpp
void PathInputter::ReleaseModifierKeys() {
    // Ctrl, Alt, Shift, Win キーを解放
    WORD keys[] = { VK_CONTROL, VK_MENU, VK_SHIFT, VK_LWIN, VK_RWIN };
    // GetAsyncKeyState で押されているキーを検出し、KeyUp を送信
}
```
- `TypeText()` の先頭で `ReleaseModifierKeys()` + 100ms待機を追加

### 修正2: パス貼り付け後のファイル削除問題

**問題:** 貼り付け後に `Clear()` を呼んでいたため、ファイルも削除されていた

**修正内容:** `ImageStorage` に `ClearList()` を追加
- `Clear()` - リストとファイルの両方を削除（従来通り）
- `ClearList()` - リストのみクリア（ファイルは残す）

`Application::OnPastePaths()` で `ClearList()` を使用するように変更

### 動作確認済み
- Ctrl+Alt+S: 画面キャプチャ ✅
- Ctrl+Alt+C: クリップボード保存 ✅
- Ctrl+Alt+P: パス貼り付け ✅
- パス貼り付け後、ファイルは残る ✅
- ファイル上限管理は従来通り機能 ✅

*正美より: バグ修正完了。動作確認済みです。*

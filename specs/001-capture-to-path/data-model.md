# Data Model: Ephemery - Capture to Path Tool

**Date**: 2026-01-11
**Branch**: `001-capture-to-path`

## Overview

Ephemeryアプリケーションで使用するデータ構造の定義。

---

## Entities

### 1. CapturedImage

保存された一時画像ファイルを表すエンティティ。

| Field | Type | Description |
|-------|------|-------------|
| `filePath` | `std::wstring` | PNG画像の絶対パス |
| `capturedAt` | `std::chrono::system_clock::time_point` | キャプチャ日時 |
| `sequenceNumber` | `uint32_t` | セッション内の連番 |
| `source` | `CaptureSource` | キャプチャ元 (Screen/Clipboard) |

**Enum: CaptureSource**
```cpp
enum class CaptureSource {
    Screen,      // 画面キャプチャ
    Clipboard    // クリップボードから保存
};
```

**Validation Rules**:
- `filePath` は存在する有効なファイルパスであること
- `filePath` の拡張子は `.png` であること
- `sequenceNumber` は 1 以上の正の整数

**State Transitions**:
```
[Created] → (file saved) → [Active] → (max limit exceeded) → [Deleted]
                                    → (manual clear) → [Deleted]
```

---

### 2. HotkeyBinding

ホットキーの設定を表すエンティティ。

| Field | Type | Description |
|-------|------|-------------|
| `action` | `HotkeyAction` | 割り当てられた機能 |
| `modifiers` | `uint32_t` | 修飾キー (MOD_ALT, MOD_CONTROL, etc.) |
| `virtualKey` | `uint32_t` | 仮想キーコード |
| `id` | `int` | RegisterHotKey用の一意ID |

**Enum: HotkeyAction**
```cpp
enum class HotkeyAction {
    CaptureScreen,    // 画面キャプチャ開始
    SaveClipboard,    // クリップボード画像を保存
    PastePaths        // パスを入力
};
```

**Validation Rules**:
- `modifiers` は少なくとも1つの修飾キーを含むこと (単独キーは不可)
- `virtualKey` は有効な仮想キーコード (0x01-0xFE)
- 同一の modifiers + virtualKey の組み合わせは複数の action に割り当て不可

**Default Values**:
| Action | Default Hotkey |
|--------|---------------|
| CaptureScreen | Win + Shift + S |
| SaveClipboard | Win + Shift + C |
| PastePaths | Win + Shift + V |

---

### 3. AppSettings

アプリケーション全体の設定を表すエンティティ。

| Field | Type | Description |
|-------|------|-------------|
| `version` | `uint32_t` | 設定ファイルバージョン |
| `maxImages` | `uint32_t` | 保持する最大画像数 |
| `hotkeys` | `std::vector<HotkeyBinding>` | ホットキー設定 (3つ) |
| `storagePath` | `std::wstring` | 画像保存先ディレクトリ |

**Validation Rules**:
- `version` は現在のアプリバージョンと互換性があること
- `maxImages` は 1 以上 100 以下
- `hotkeys` は正確に3つの要素を持つこと (各アクションに1つ)
- `storagePath` は書き込み可能なディレクトリであること

**Default Values**:
| Field | Default |
|-------|---------|
| version | 1 |
| maxImages | 10 |
| storagePath | `%TEMP%\Ephemery` |

---

### 4. ImageStore (Runtime)

画像ファイルのコレクションを管理するランタイムエンティティ。

| Field | Type | Description |
|-------|------|-------------|
| `images` | `std::deque<CapturedImage>` | 保持中の画像リスト (FIFO) |
| `nextSequence` | `uint32_t` | 次の連番 |

**Operations**:
- `Add(CapturedImage)`: 画像を追加、上限超過時は最古を削除
- `GetAll()`: 全画像を取得 (パス一覧生成用)
- `Clear()`: 全画像を削除
- `Count()`: 現在の画像数を取得

**Invariants**:
- `images.size() <= AppSettings.maxImages`
- `images` は `capturedAt` の昇順でソートされていること

---

## Relationships

```
AppSettings
    ├── 1:N → HotkeyBinding (exactly 3)
    └── 1:1 → ImageStore (runtime reference)

ImageStore
    └── 1:N → CapturedImage (0 to maxImages)
```

---

## Persistence

### Settings File

**Location**: `%APPDATA%\Ephemery\settings.json`

**Schema**:
```json
{
  "$schema": "http://json-schema.org/draft-07/schema#",
  "type": "object",
  "required": ["version", "maxImages", "hotkeys"],
  "properties": {
    "version": { "type": "integer", "minimum": 1 },
    "maxImages": { "type": "integer", "minimum": 1, "maximum": 100 },
    "storagePath": { "type": "string" },
    "hotkeys": {
      "type": "object",
      "properties": {
        "captureScreen": { "$ref": "#/definitions/hotkey" },
        "saveClipboard": { "$ref": "#/definitions/hotkey" },
        "pastePaths": { "$ref": "#/definitions/hotkey" }
      },
      "required": ["captureScreen", "saveClipboard", "pastePaths"]
    }
  },
  "definitions": {
    "hotkey": {
      "type": "object",
      "properties": {
        "modifiers": {
          "type": "array",
          "items": { "enum": ["Alt", "Ctrl", "Shift", "Win"] }
        },
        "key": { "type": "string", "pattern": "^[A-Z0-9]$" }
      },
      "required": ["modifiers", "key"]
    }
  }
}
```

### Image Files

**Location**: `%TEMP%\Ephemery\`
**Naming**: `ephemery_YYYYMMDD_HHMMSS_NNN.png`
**Lifecycle**:
- Created on capture
- Deleted when exceeding maxImages (FIFO)
- Persisted across app restarts (loaded from disk on startup)

---

## C++ Type Definitions

```cpp
// Forward declarations
struct CapturedImage;
struct HotkeyBinding;
struct AppSettings;
class ImageStore;

// Enumerations
enum class CaptureSource { Screen, Clipboard };
enum class HotkeyAction { CaptureScreen, SaveClipboard, PastePaths };

// Structures
struct CapturedImage {
    std::wstring filePath;
    std::chrono::system_clock::time_point capturedAt;
    uint32_t sequenceNumber;
    CaptureSource source;
};

struct HotkeyBinding {
    HotkeyAction action;
    uint32_t modifiers;  // MOD_WIN | MOD_SHIFT etc.
    uint32_t virtualKey; // 'S', 'C', 'V' etc.
    int id;              // For RegisterHotKey
};

struct AppSettings {
    uint32_t version = 1;
    uint32_t maxImages = 10;
    std::wstring storagePath;
    std::array<HotkeyBinding, 3> hotkeys;
};
```

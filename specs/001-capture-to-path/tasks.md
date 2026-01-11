# Tasks: Ephemery - Capture to Path Tool

**Input**: Design documents from `/specs/001-capture-to-path/`
**Prerequisites**: plan.md, spec.md, research.md, data-model.md, contracts/

**Tests**: Not explicitly requested. Manual integration tests will be used.

**Organization**: Tasks are grouped by user story to enable independent implementation and testing.

## Format: `[ID] [P?] [Story] Description`

- **[P]**: Can run in parallel (different files, no dependencies)
- **[Story]**: Which user story this task belongs to (e.g., US1, US2, US3, US4)
- Include exact file paths in descriptions

---

## Phase 1: Setup (Shared Infrastructure)

**Purpose**: Project initialization and basic structure

- [ ] T001 Create project directory structure per plan.md (src/, tests/, resources/)
- [ ] T002 Create CMakeLists.txt with C++17, Win32, GDI+ configuration
- [ ] T003 [P] Create resources/icon.ico (placeholder tray icon)
- [ ] T004 [P] Create resources/app.manifest (DPI awareness, UAC)
- [ ] T005 [P] Create src/util/Logger.h and src/util/Logger.cpp (basic logging utility)

---

## Phase 2: Foundational (Blocking Prerequisites)

**Purpose**: Core infrastructure that MUST be complete before ANY user story can be implemented

**CRITICAL**: No user story work can begin until this phase is complete

### Data Structures

- [ ] T006 [P] Create src/storage/Types.h with CapturedImage, HotkeyBinding, AppSettings structs per data-model.md
- [ ] T007 [P] Create src/hotkey/HotkeyConfig.h and src/hotkey/HotkeyConfig.cpp (hotkey configuration and defaults)

### Core Services

- [ ] T008 Create src/util/PngEncoder.h and src/util/PngEncoder.cpp (GDI+ PNG encoding, HBITMAP to PNG file)
- [ ] T009 Create src/storage/ImageStorage.h and src/storage/ImageStorage.cpp (save, list, delete, FIFO management)
- [ ] T010 Create src/storage/Settings.h and src/storage/Settings.cpp (JSON load/save to %APPDATA%\Ephemery\settings.json)
- [ ] T011 Create src/hotkey/HotkeyManager.h and src/hotkey/HotkeyManager.cpp (RegisterHotKey, UnregisterHotKey, WM_HOTKEY handling)
- [ ] T012 Create src/input/PathInputter.h and src/input/PathInputter.cpp (SendInput-based path typing with Unicode support)

### Application Framework

- [ ] T013 Create src/app/TrayIcon.h and src/app/TrayIcon.cpp (Shell_NotifyIcon, context menu)
- [ ] T014 Create src/app/Application.h and src/app/Application.cpp (main app class skeleton, message loop, GDI+ init)
- [ ] T015 Create src/main.cpp (WinMain entry point, Application instantiation)

**Checkpoint**: Foundation ready - user story implementation can now begin

---

## Phase 3: User Story 1 - 画面キャプチャしてパスを取得 (Priority: P1) MVP

**Goal**: ユーザーが画面の領域またはウィンドウをキャプチャし、そのファイルパスを取得できる

**Independent Test**:
1. キャプチャ用ホットキー (Win+Shift+S) を押す
2. 領域選択UIが表示される
3. 矩形選択またはウィンドウクリックでキャプチャ
4. パス取得ホットキー (Win+Shift+V) を押す
5. テキストエディタにファイルパスが入力される

### Implementation for User Story 1

- [ ] T016 [P] [US1] Create src/capture/ScreenCapture.h and src/capture/ScreenCapture.cpp (BitBlt-based screen capture)
- [ ] T017 [P] [US1] Create src/capture/WindowSelector.h and src/capture/WindowSelector.cpp (WindowFromPoint, GetWindowRect, highlight)
- [ ] T018 [US1] Create src/capture/RegionSelector.h and src/capture/RegionSelector.cpp (full-screen overlay, mouse drag selection, Esc cancel)
- [ ] T019 [US1] Integrate ScreenCapture, RegionSelector, WindowSelector in Application for CaptureScreen hotkey handling
- [ ] T020 [US1] Implement PastePaths handler in Application (get all paths from ImageStorage, send via PathInputter)
- [ ] T021 [US1] Handle edge cases: folder auto-creation, Esc cancel, multi-monitor support

**Checkpoint**: User Story 1 should be fully functional - can capture screen regions/windows and paste paths

---

## Phase 4: User Story 2 - クリップボードの画像を保存してパスを取得 (Priority: P2)

**Goal**: クリップボードにコピーされた画像を一時ファイルとして保存できる

**Independent Test**:
1. ブラウザで画像を右クリック→コピー
2. クリップボード保存用ホットキー (Win+Shift+C) を押す
3. パス取得ホットキーでパスが入力される

### Implementation for User Story 2

- [ ] T022 [US2] Create src/capture/ClipboardCapture.h and src/capture/ClipboardCapture.cpp (CF_DIB/CF_BITMAP reading)
- [ ] T023 [US2] Integrate ClipboardCapture in Application for SaveClipboard hotkey handling
- [ ] T024 [US2] Handle edge case: clipboard has no image (no-op or notification)

**Checkpoint**: User Story 2 should be fully functional - can save clipboard images

---

## Phase 5: User Story 3 - 複数画像のパスを一括取得 (Priority: P1)

**Goal**: 複数のキャプチャ画像のパスをまとめて改行区切りで入力できる

**Independent Test**:
1. 3回連続でキャプチャ
2. パス取得ホットキーを押す
3. 3つのパスが改行区切りで入力される

### Implementation for User Story 3

- [ ] T025 [US3] Verify ImageStorage correctly maintains multiple files (FIFO order)
- [ ] T026 [US3] Verify PathInputter correctly outputs multiple paths with newline separator
- [ ] T027 [US3] Implement max image limit enforcement (delete oldest when exceeding limit)
- [ ] T028 [US3] Test 10 images capture and path output (SC-005 validation)

**Checkpoint**: User Story 3 should be fully functional - multiple paths work correctly

---

## Phase 6: User Story 4 - 設定変更 (Priority: P3)

**Goal**: ホットキーと保持上限をUIから変更できる

**Independent Test**:
1. トレイアイコン右クリック→設定
2. ホットキーを変更して保存
3. 新しいホットキーで機能が動作する

### Implementation for User Story 4

- [ ] T029 [US4] Create src/app/SettingsDialog.h and src/app/SettingsDialog.cpp (Win32 dialog for settings)
- [ ] T030 [US4] Implement hotkey input control (capture key combination)
- [ ] T031 [US4] Implement max images spin control (1-100 range)
- [ ] T032 [US4] Integrate SettingsDialog with TrayIcon context menu ("設定" item)
- [ ] T033 [US4] Implement hotkey re-registration on settings change
- [ ] T034 [US4] Handle edge case: hotkey already in use (warning message)

**Checkpoint**: User Story 4 should be fully functional - settings can be changed

---

## Phase 7: Polish & Cross-Cutting Concerns

**Purpose**: Improvements that affect multiple user stories

- [ ] T035 [P] Add disk space check before saving image (notify user if low)
- [ ] T036 [P] Load existing images from %TEMP%\Ephemery on startup
- [ ] T037 [P] Add "フォルダを開く" option to tray context menu
- [ ] T038 [P] Add "終了" option to tray context menu with proper cleanup
- [ ] T039 Performance validation: capture < 1 second (SC-001)
- [ ] T040 Performance validation: path input < 500ms (SC-002)
- [ ] T041 Memory validation: < 50MB during idle (SC-003)
- [ ] T042 Final integration test: full workflow per quickstart.md

---

## Dependencies & Execution Order

### Phase Dependencies

- **Setup (Phase 1)**: No dependencies - can start immediately
- **Foundational (Phase 2)**: Depends on Setup completion - BLOCKS all user stories
- **User Stories (Phase 3-6)**: All depend on Foundational phase completion
- **Polish (Phase 7)**: Depends on all user stories being complete

### User Story Dependencies

| Story | Depends On | Can Run In Parallel With |
|-------|------------|--------------------------|
| US1 (画面キャプチャ) | Foundational only | US2, US4 (if staffed) |
| US2 (クリップボード) | Foundational only | US1, US4 (if staffed) |
| US3 (複数画像) | US1 (uses same infrastructure) | - |
| US4 (設定変更) | Foundational only | US1, US2 (if staffed) |

### Within Each User Story

1. Models/headers before implementation
2. Core implementation before integration
3. Edge cases last

### Parallel Opportunities

**Phase 1**:
- T003, T004, T005 can run in parallel

**Phase 2**:
- T006, T007 can run in parallel (data structures)
- After data structures: T008-T012 services have some parallelism

**Phase 3 (US1)**:
- T016, T017 can run in parallel (ScreenCapture, WindowSelector)

---

## Parallel Example: Phase 2 Foundational

```bash
# Launch data structures together:
Task: "Create src/storage/Types.h with CapturedImage, HotkeyBinding, AppSettings"
Task: "Create src/hotkey/HotkeyConfig.h and .cpp"

# After data structures, launch independent services:
Task: "Create src/util/PngEncoder.h and .cpp"
Task: "Create src/input/PathInputter.h and .cpp"
```

---

## Implementation Strategy

### MVP First (User Story 1 Only)

1. Complete Phase 1: Setup
2. Complete Phase 2: Foundational
3. Complete Phase 3: User Story 1 (画面キャプチャ)
4. **STOP and VALIDATE**: Test キャプチャ → パス入力 flow
5. Deploy/demo if ready

### Incremental Delivery

1. Setup + Foundational → Foundation ready
2. Add US1 (画面キャプチャ) → Test → **MVP!**
3. Add US2 (クリップボード) → Test → Deploy
4. Add US3 (複数画像) → Test → Deploy
5. Add US4 (設定変更) → Test → Deploy
6. Polish → Final release

---

## Summary

| Phase | Tasks | Description |
|-------|-------|-------------|
| 1. Setup | T001-T005 (5) | Project structure |
| 2. Foundational | T006-T015 (10) | Core infrastructure |
| 3. US1 (P1) | T016-T021 (6) | 画面キャプチャ MVP |
| 4. US2 (P2) | T022-T024 (3) | クリップボード |
| 5. US3 (P1) | T025-T028 (4) | 複数画像 |
| 6. US4 (P3) | T029-T034 (6) | 設定変更 |
| 7. Polish | T035-T042 (8) | 最終調整 |
| **Total** | **42 tasks** | |

---

## Notes

- [P] tasks = different files, no dependencies
- [Story] label maps task to specific user story
- Each user story should be independently testable
- Commit after each task or logical group
- Stop at any checkpoint to validate story independently

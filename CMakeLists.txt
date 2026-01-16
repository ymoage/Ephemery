cmake_minimum_required(VERSION 3.20)
project(Ephemery LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Windows-specific settings
if(WIN32)
    add_definitions(-DUNICODE -D_UNICODE -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
endif()

# MSVC UTF-8 source encoding
if(MSVC)
    add_compile_options(/utf-8)
    # Use MultiThreadedDLL runtime for Release, MultiThreadedDebugDLL for Debug
    set(CMAKE_MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>DLL")
endif()

# Main executable
add_executable(Ephemery WIN32
    src/main.cpp
    src/app/Application.cpp
    src/app/TrayIcon.cpp
    src/app/SettingsDialog.cpp
    src/capture/ScreenCapture.cpp
    src/capture/RegionSelector.cpp
    src/capture/WindowSelector.cpp
    src/capture/ClipboardCapture.cpp
    src/hotkey/HotkeyManager.cpp
    src/hotkey/HotkeyConfig.cpp
    src/storage/ImageStorage.cpp
    src/storage/Settings.cpp
    src/input/PathInputter.cpp
    src/util/PngEncoder.cpp
    src/util/Logger.cpp
    resources/app.manifest
    resources/app.rc
)

target_include_directories(Ephemery PRIVATE
    ${CMAKE_SOURCE_DIR}/src
)

# Link Windows libraries
target_link_libraries(Ephemery PRIVATE
    kernel32
    gdiplus
    shell32
    user32
    gdi32
    ole32
    comdlg32
    msimg32
)

# Set Windows subsystem
set_target_properties(Ephemery PROPERTIES
    WIN32_EXECUTABLE TRUE
    VS_DEBUGGER_WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
)

# Copy icon to output directory
if(EXISTS "${CMAKE_SOURCE_DIR}/resources/icon.ico")
    configure_file(
        "${CMAKE_SOURCE_DIR}/resources/icon.ico"
        "${CMAKE_BINARY_DIR}/icon.ico"
        COPYONLY
    )
endif()

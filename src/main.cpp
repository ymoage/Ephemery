#include "app/Application.h"
#include "util/Logger.h"
#include <Windows.h>
#include <ShlObj.h>
#include <strsafe.h>
#include <csignal>

// ---------------------------------------------------------------------------
// Crash handler helpers
// ---------------------------------------------------------------------------

// Write crash info using only Win32 APIs (safe from inside exception handlers)
static void WriteCrashLog(const wchar_t* reason, DWORD code, void* address) {
    wchar_t appDataPath[MAX_PATH] = {};
    DWORD len = GetEnvironmentVariableW(L"APPDATA", appDataPath, MAX_PATH);
    if (len == 0 || len >= MAX_PATH) return;

    wchar_t logPath[MAX_PATH] = {};
    StringCchPrintfW(logPath, MAX_PATH, L"%s\\Ephemery\\crash.log", appDataPath);

    HANDLE hFile = CreateFileW(logPath, GENERIC_WRITE, 0, nullptr,
                               OPEN_ALWAYS, FILE_ATTRIBUTE_NORMAL, nullptr);
    if (hFile == INVALID_HANDLE_VALUE) return;

    SetFilePointer(hFile, 0, nullptr, FILE_END);

    wchar_t buf[512] = {};
    StringCchPrintfW(buf, _countof(buf),
        L"[CRASH] %s  code=0x%08X  addr=0x%p\r\n",
        reason, code, address);

    DWORD written = 0;
    WriteFile(hFile, buf, static_cast<DWORD>(wcslen(buf) * sizeof(wchar_t)), &written, nullptr);
    CloseHandle(hFile);
}

// SEH unhandled exception filter
static LONG WINAPI OnUnhandledException(EXCEPTION_POINTERS* ep) {
    DWORD code    = ep->ExceptionRecord->ExceptionCode;
    void* address = ep->ExceptionRecord->ExceptionAddress;
    WriteCrashLog(L"SEH unhandled exception", code, address);
    return EXCEPTION_CONTINUE_SEARCH;
}

// C++ terminate handler (called when exception escapes all catch blocks)
static void OnTerminate() {
    WriteCrashLog(L"std::terminate called", 0, nullptr);
    _exit(1);
}

// SIGABRT handler (called by abort())
static void OnAbortSignal(int) {
    WriteCrashLog(L"abort() called (SIGABRT)", 0, nullptr);
    _exit(1);
}

// ---------------------------------------------------------------------------
// Entry point helpers
// ---------------------------------------------------------------------------

// Separate function so __try/__except doesn't conflict with C++ object unwinding
static int RunApp(Ephemery::Application* app) {
    __try {
        return app->Run();
    } __except(
        (WriteCrashLog(L"SEH exception in Run()", GetExceptionCode(),
            GetExceptionInformation()->ExceptionRecord->ExceptionAddress),
         EXCEPTION_EXECUTE_HANDLER)) {
        return 1;
    }
}

// ---------------------------------------------------------------------------
// Entry point
// ---------------------------------------------------------------------------

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

    // Install crash handlers as early as possible
    SetUnhandledExceptionFilter(OnUnhandledException);
    std::set_terminate(OnTerminate);
    signal(SIGABRT, OnAbortSignal);

    // Check for single instance
    HANDLE hMutex = CreateMutexW(nullptr, TRUE, L"EphemeryMutex");
    if (GetLastError() == ERROR_ALREADY_EXISTS) {
        MessageBoxW(nullptr, L"Ephemery is already running.", L"Ephemery", MB_OK | MB_ICONINFORMATION);
        return 0;
    }

    Ephemery::Application app;

    if (!app.Initialize(hInstance)) {
        MessageBoxW(nullptr, L"Failed to initialize Ephemery.", L"Error", MB_OK | MB_ICONERROR);
        CloseHandle(hMutex);
        return 1;
    }

    int result = RunApp(&app);

    app.Shutdown();
    CloseHandle(hMutex);

    return result;
}

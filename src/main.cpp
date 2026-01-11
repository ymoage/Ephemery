#include "app/Application.h"
#include "util/Logger.h"
#include <Windows.h>

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    (void)hPrevInstance;
    (void)lpCmdLine;
    (void)nCmdShow;

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

    int result = app.Run();

    app.Shutdown();
    CloseHandle(hMutex);

    return result;
}

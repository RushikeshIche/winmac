#include <windows.h>
#include <iostream>

typedef bool (*CustomizeWindowFunc)(HWND);

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    // Phase 2: Find the external target application
    HWND target = FindWindow(L"TestAppWindowClass", NULL);
    
    if (!target) {
        MessageBox(NULL, L"Please open test_app.exe first so WinMac can find it!", L"WinMac", MB_OK);
        return 0;
    }

    // Load our injection DLL
    HMODULE hDll = LoadLibrary(L"winmac_hook.dll");
    if (!hDll) {
        MessageBox(NULL, L"Could not find winmac_hook.dll!", L"WinMac", MB_OK);
        return 0;
    }

    // Find the injection function
    CustomizeWindowFunc customize = (CustomizeWindowFunc)GetProcAddress(hDll, "CustomizeWindow");
    if (customize) {
        if (customize(target)) {
            // MessageBox(NULL, L"Successfully applied macOS title bar to Test App!", L"WinMac", MB_OK);
        } else {
            MessageBox(NULL, L"Failed to inject the custom title bar.", L"WinMac", MB_OK);
            return 0;
        }
    }

    // Keep winmac.exe running silently in the background
    // If we close, the DLL might be orphaned or cause issues depending on how we handle it later.
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

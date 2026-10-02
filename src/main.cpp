#include <windows.h>
#include <windowsx.h>

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
            if (hit == HTCLIENT) {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hwnd, &pt);
                
                // If mouse is in our custom title bar, pretend it's the native caption bar
                if (pt.y >= 0 && pt.y < 32) {
                    return HTCAPTION;
                }
            }
            return hit;
        }
        case WM_NCCALCSIZE: {
            if (wParam == TRUE) {
                return 0;
            }
            break;
        }
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hwnd, &ps);
            
            // Draw main background
            FillRect(hdc, &ps.rcPaint, (HBRUSH) (COLOR_WINDOW+1));
            
            // Draw macOS-style title bar
            RECT titleBarRect;
            GetClientRect(hwnd, &titleBarRect);
            titleBarRect.bottom = 32;
            
            HBRUSH titleBarBrush = CreateSolidBrush(RGB(240, 240, 240));
            FillRect(hdc, &titleBarRect, titleBarBrush);
            DeleteObject(titleBarBrush);
            
            // Draw macOS traffic lights on the right side
            HPEN oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
            
            int rightEdge = titleBarRect.right;
            int size = 13; // 13x13 bounding box often looks more symmetrical in GDI
            int yTop = 10;
            int yBottom = yTop + size;
            
            int redLeft = rightEdge - 28;
            int yellowLeft = redLeft - 20;
            int greenLeft = yellowLeft - 20;

            // Red button
            HBRUSH redBrush = CreateSolidBrush(RGB(255, 95, 86));
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, redBrush);
            Ellipse(hdc, redLeft, yTop, redLeft + size, yBottom);
            
            // Yellow button
            HBRUSH yellowBrush = CreateSolidBrush(RGB(255, 189, 46));
            SelectObject(hdc, yellowBrush);
            Ellipse(hdc, yellowLeft, yTop, yellowLeft + size, yBottom);
            
            // Green button
            HBRUSH greenBrush = CreateSolidBrush(RGB(39, 201, 63));
            SelectObject(hdc, greenBrush);
            Ellipse(hdc, greenLeft, yTop, greenLeft + size, yBottom);
            
            // Cleanup GDI objects
            SelectObject(hdc, oldBrush);
            SelectObject(hdc, oldPen);
            DeleteObject(redBrush);
            DeleteObject(yellowBrush);
            DeleteObject(greenBrush);
            
            EndPaint(hwnd, &ps);
            return 0;
        }
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

int WINAPI wWinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PWSTR pCmdLine, int nCmdShow) {
    const wchar_t CLASS_NAME[] = L"WinMacWindowClass";

    WNDCLASS wc = { };
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,                              // Optional window styles
        CLASS_NAME,                     // Window class
        L"WinMac Test Window",          // Window text
        WS_OVERLAPPEDWINDOW,            // Window style
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, // Size and position
        NULL,                           // Parent window
        NULL,                           // Menu
        hInstance,                      // Instance handle
        NULL                            // Additional application data
    );

    if (hwnd == NULL) {
        return 0;
    }

    // Force Windows to recalculate the non-client area immediately
    SetWindowPos(hwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);

    ShowWindow(hwnd, nCmdShow);

    // Run the message loop
    MSG msg = { };
    while (GetMessage(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }

    return 0;
}

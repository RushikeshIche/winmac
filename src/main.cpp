#include <windows.h>
#include <windowsx.h>

static int hoveredButton = 0; // 0=none, 1=red, 2=yellow, 3=green
static int pressedButton = 0; // 0=none, 1=red, 2=yellow, 3=green

// Helper to centralize button position math
void GetButtonRects(HWND hwnd, RECT* red, RECT* yellow, RECT* green) {
    RECT rc;
    GetClientRect(hwnd, &rc);
    int rightEdge = rc.right;
    int size = 13;
    int yTop = 10;
    
    int redLeft = rightEdge - 28;
    int yellowLeft = redLeft - 20;
    int greenLeft = yellowLeft - 20;
    
    SetRect(red, redLeft, yTop, redLeft + size, yTop + size);
    SetRect(yellow, yellowLeft, yTop, yellowLeft + size, yTop + size);
    SetRect(green, greenLeft, yTop, greenLeft + size, yTop + size);
}

LRESULT CALLBACK WindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
        case WM_NCHITTEST: {
            LRESULT hit = DefWindowProc(hwnd, uMsg, wParam, lParam);
            if (hit == HTCLIENT) {
                POINT pt;
                pt.x = GET_X_LPARAM(lParam);
                pt.y = GET_Y_LPARAM(lParam);
                ScreenToClient(hwnd, &pt);
                
                RECT rc;
                GetClientRect(hwnd, &rc);
                
                int borderX = GetSystemMetrics(SM_CXSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);
                int borderY = GetSystemMetrics(SM_CYSIZEFRAME) + GetSystemMetrics(SM_CXPADDEDBORDER);

                bool isLeft = pt.x < borderX;
                bool isRight = pt.x >= (rc.right - borderX);
                bool isTop = pt.y < borderY;
                bool isBottom = pt.y >= (rc.bottom - borderY);

                if (isTop && isLeft) return HTTOPLEFT;
                if (isTop && isRight) return HTTOPRIGHT;
                if (isBottom && isLeft) return HTBOTTOMLEFT;
                if (isBottom && isRight) return HTBOTTOMRIGHT;
                if (isLeft) return HTLEFT;
                if (isRight) return HTRIGHT;
                if (isTop) return HTTOP;
                if (isBottom) return HTBOTTOM;
                
                // If mouse is over buttons, handle it as client area for hover events
                RECT red, yellow, green;
                GetButtonRects(hwnd, &red, &yellow, &green);
                if (PtInRect(&red, pt) || PtInRect(&yellow, pt) || PtInRect(&green, pt)) {
                    return HTCLIENT;
                }
                
                // If mouse is in our custom title bar but not on buttons, pretend it's the native caption
                if (pt.y >= 0 && pt.y < 32) {
                    return HTCAPTION;
                }
            }
            return hit;
        }
        case WM_MOUSEMOVE: {
            POINT pt;
            pt.x = GET_X_LPARAM(lParam);
            pt.y = GET_Y_LPARAM(lParam);
            
            RECT red, yellow, green;
            GetButtonRects(hwnd, &red, &yellow, &green);
            
            int newHover = 0;
            if (PtInRect(&red, pt)) newHover = 1;
            else if (PtInRect(&yellow, pt)) newHover = 2;
            else if (PtInRect(&green, pt)) newHover = 3;
            
            if (newHover != hoveredButton) {
                hoveredButton = newHover;
                
                // Request WM_MOUSELEAVE to know when the mouse leaves the window
                TRACKMOUSEEVENT tme = {};
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
                
                InvalidateRect(hwnd, NULL, FALSE); // Force redraw for hover colors
            }
            break;
        }
        case WM_MOUSELEAVE: {
            hoveredButton = 0;
            pressedButton = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
        }
        case WM_LBUTTONDOWN: {
            if (hoveredButton != 0) {
                pressedButton = hoveredButton;
                InvalidateRect(hwnd, NULL, FALSE); // Redraw for pressed state
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (pressedButton != 0 && pressedButton == hoveredButton) {
                // Execute button actions!
                if (pressedButton == 1) { // Red / Close
                    PostMessage(hwnd, WM_CLOSE, 0, 0);
                } else if (pressedButton == 2) { // Yellow / Minimize
                    ShowWindow(hwnd, SW_MINIMIZE);
                } else if (pressedButton == 3) { // Green / Maximize-Restore
                    WINDOWPLACEMENT wp = { sizeof(wp) };
                    GetWindowPlacement(hwnd, &wp);
                    if (wp.showCmd == SW_MAXIMIZE) ShowWindow(hwnd, SW_RESTORE);
                    else ShowWindow(hwnd, SW_MAXIMIZE);
                }
            }
            pressedButton = 0;
            InvalidateRect(hwnd, NULL, FALSE);
            break;
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
            
            // Draw macOS traffic lights
            HPEN oldPen = (HPEN)SelectObject(hdc, GetStockObject(NULL_PEN));
            
            RECT rRed, rYellow, rGreen;
            GetButtonRects(hwnd, &rRed, &rYellow, &rGreen);

            // Red button (lighten if hovered, darken if pressed)
            COLORREF cRed = (pressedButton == 1) ? RGB(200, 70, 60) : (hoveredButton == 1 ? RGB(255, 120, 110) : RGB(255, 95, 86));
            HBRUSH redBrush = CreateSolidBrush(cRed);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdc, redBrush);
            Ellipse(hdc, rRed.left, rRed.top, rRed.right, rRed.bottom);
            
            // Yellow button
            COLORREF cYellow = (pressedButton == 2) ? RGB(200, 150, 30) : (hoveredButton == 2 ? RGB(255, 210, 80) : RGB(255, 189, 46));
            HBRUSH yellowBrush = CreateSolidBrush(cYellow);
            SelectObject(hdc, yellowBrush);
            Ellipse(hdc, rYellow.left, rYellow.top, rYellow.right, rYellow.bottom);
            
            // Green button
            COLORREF cGreen = (pressedButton == 3) ? RGB(30, 150, 45) : (hoveredButton == 3 ? RGB(70, 230, 90) : RGB(39, 201, 63));
            HBRUSH greenBrush = CreateSolidBrush(cGreen);
            SelectObject(hdc, greenBrush);
            Ellipse(hdc, rGreen.left, rGreen.top, rGreen.right, rGreen.bottom);
            
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
    wc.style         = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc   = WindowProc;
    wc.hInstance     = hInstance;
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

    RegisterClass(&wc);

    HWND hwnd = CreateWindowEx(
        0,
        CLASS_NAME,
        L"WinMac Test Window",
        WS_OVERLAPPEDWINDOW,
        CW_USEDEFAULT, CW_USEDEFAULT, 800, 600,
        NULL,
        NULL,
        hInstance,
        NULL
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

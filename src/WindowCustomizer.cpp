#define BUILD_DLL
#include "WindowCustomizer.h"
#include <windowsx.h>
#include <commctrl.h>

#pragma comment(lib, "comctl32.lib")

static int hoveredButton = 0;
static int pressedButton = 0;

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

LRESULT CALLBACK WinMacSubclassProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam, UINT_PTR uIdSubclass, DWORD_PTR dwRefData) {
    switch (uMsg) {
        case WM_NCHITTEST: {
            LRESULT hit = DefSubclassProc(hwnd, uMsg, wParam, lParam);
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
                
                RECT red, yellow, green;
                GetButtonRects(hwnd, &red, &yellow, &green);
                if (PtInRect(&red, pt) || PtInRect(&yellow, pt) || PtInRect(&green, pt)) {
                    return HTCLIENT;
                }
                
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
                TRACKMOUSEEVENT tme = {};
                tme.cbSize = sizeof(tme);
                tme.dwFlags = TME_LEAVE;
                tme.hwndTrack = hwnd;
                TrackMouseEvent(&tme);
                InvalidateRect(hwnd, NULL, FALSE);
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
                InvalidateRect(hwnd, NULL, FALSE);
            }
            break;
        }
        case WM_LBUTTONUP: {
            if (pressedButton != 0 && pressedButton == hoveredButton) {
                if (pressedButton == 1) PostMessage(hwnd, WM_CLOSE, 0, 0);
                else if (pressedButton == 2) ShowWindow(hwnd, SW_MINIMIZE);
                else if (pressedButton == 3) {
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
                return 0; // Hide native title bar
            }
            break;
        }
        case WM_PAINT: {
            // First, let the target app paint its own stuff
            LRESULT res = DefSubclassProc(hwnd, uMsg, wParam, lParam);
            
            // Then, grab a Device Context and draw our custom title bar OVER it
            HDC hdcOverlay = GetDC(hwnd);
            
            RECT titleBarRect;
            GetClientRect(hwnd, &titleBarRect);
            titleBarRect.bottom = 32;
            
            HBRUSH titleBarBrush = CreateSolidBrush(RGB(240, 240, 240));
            FillRect(hdcOverlay, &titleBarRect, titleBarBrush);
            DeleteObject(titleBarBrush);
            
            HPEN oldPen = (HPEN)SelectObject(hdcOverlay, GetStockObject(NULL_PEN));
            
            RECT rRed, rYellow, rGreen;
            GetButtonRects(hwnd, &rRed, &rYellow, &rGreen);

            COLORREF cRed = (pressedButton == 1) ? RGB(200, 70, 60) : (hoveredButton == 1 ? RGB(255, 120, 110) : RGB(255, 95, 86));
            HBRUSH redBrush = CreateSolidBrush(cRed);
            HBRUSH oldBrush = (HBRUSH)SelectObject(hdcOverlay, redBrush);
            Ellipse(hdcOverlay, rRed.left, rRed.top, rRed.right, rRed.bottom);
            
            COLORREF cYellow = (pressedButton == 2) ? RGB(200, 150, 30) : (hoveredButton == 2 ? RGB(255, 210, 80) : RGB(255, 189, 46));
            HBRUSH yellowBrush = CreateSolidBrush(cYellow);
            SelectObject(hdcOverlay, yellowBrush);
            Ellipse(hdcOverlay, rYellow.left, rYellow.top, rYellow.right, rYellow.bottom);
            
            COLORREF cGreen = (pressedButton == 3) ? RGB(30, 150, 45) : (hoveredButton == 3 ? RGB(70, 230, 90) : RGB(39, 201, 63));
            HBRUSH greenBrush = CreateSolidBrush(cGreen);
            SelectObject(hdcOverlay, greenBrush);
            Ellipse(hdcOverlay, rGreen.left, rGreen.top, rGreen.right, rGreen.bottom);
            
            SelectObject(hdcOverlay, oldBrush);
            SelectObject(hdcOverlay, oldPen);
            DeleteObject(redBrush);
            DeleteObject(yellowBrush);
            DeleteObject(greenBrush);
            
            ReleaseDC(hwnd, hdcOverlay);
            return res;
        }
    }
    return DefSubclassProc(hwnd, uMsg, wParam, lParam);
}

// Shared memory section for variables between processes
#pragma data_seg(".shared")
HHOOK g_hook = NULL;
HWND g_targetHwnd = NULL;
#pragma data_seg()
#pragma comment(linker, "/SECTION:.shared,RWS")

// This is per-process, so the target process tracks if it has subclassed
bool g_subclassed = false;

// This function runs INSIDE the target application's process when a message is processed
LRESULT CALLBACK CallWndProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HC_ACTION && !g_subclassed) {
        CWPSTRUCT* cwp = (CWPSTRUCT*)lParam;
        if (cwp->hwnd == g_targetHwnd) {
            // We are now inside the target process! Subclass the window.
            SetWindowSubclass(g_targetHwnd, WinMacSubclassProc, 1, 0);
            
            // Force redraw to apply our title bar immediately
            SetWindowPos(g_targetHwnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
            
            g_subclassed = true;
            
            // PIN the DLL in the target process memory so it doesn't unload when we unhook!
            // If the DLL unloads, the subclass pointer becomes invalid and crashes the app.
            HMODULE hMod;
            GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS | GET_MODULE_HANDLE_EX_FLAG_PIN, (LPCWSTR)WinMacSubclassProc, &hMod);
            
            // Remove the hook once subclassed to avoid slowing down the target app
            if (g_hook) {
                UnhookWindowsHookEx(g_hook);
                g_hook = NULL;
            }
        }
    }
    return CallNextHookEx(g_hook, nCode, wParam, lParam);
}

// This function is called by our winmac.exe to start the injection
extern "C" DLL_API bool CustomizeWindow(HWND hwnd) {
    g_targetHwnd = hwnd;
    g_subclassed = false;
    
    DWORD threadId = GetWindowThreadProcessId(hwnd, NULL);
    
    HMODULE hModule = GetModuleHandle(L"winmac_hook.dll");
    
    // Inject our DLL into the target application's thread
    g_hook = SetWindowsHookEx(WH_CALLWNDPROC, CallWndProc, hModule, threadId);
    
    if (g_hook == NULL) return false;
    
    // Send a dummy message to wake up the target application and force the hook to fire
    SendMessage(hwnd, WM_NULL, 0, 0);
    return true;
}

BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved) {
    return TRUE;
}

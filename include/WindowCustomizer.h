#pragma once
#include <windows.h>

#ifdef BUILD_DLL
#define DLL_API __declspec(dllexport)
#else
#define DLL_API __declspec(dllimport)
#endif

extern "C" DLL_API bool CustomizeWindow(HWND hwnd);

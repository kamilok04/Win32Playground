#include <windows.h>
#include <cmath>
#include <CommCtrl.h>
#include "resource.h"
#include <strsafe.h>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")


// wiele typ�w ju� jest pisane W_TAKI_SPOS�B, wi�c te te� mog�
#define STRUCT struct
#define STATIC static


LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GraphicsDemoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TrigonometryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR DefDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM ZarejestrujKlase(WNDCLASSEX*);

[[noreturn]] void SEPPUKU(LPCWSTR lpszFunc);
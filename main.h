#include <windows.h>
#include <cmath>
#include <CommCtrl.h>
#include "resource.h"
#include <strsafe.h>
#include <vector>

#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")


// wiele typ�w ju� jest pisane W_TAKI_SPOS�B, wi�c te te� mog�
#define STRUCT struct
#define STATIC static
#define AUTO auto
#define SAMOCH�D AUTO // :)
#define CONSTEXPR constexpr

// kolorkiiii!

#define CZERWONY	RGB(255, 0, 0)
#define NIEBIESKI	RGB(0, 0, 255)
#define ZIELONY		RGB(0, 255, 0)
#define ZOLTY		RGB(255, 255, 0)
#define FIOLETOWY	RGB(255, 0, 255)
#define CYJANEK		RGB(0, 255, 255)
#define BIALY		RGB(255, 255, 255)
#define CZARNY		RGB(0, 0, 0)


LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GraphicsDemoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TrigonometryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

INT_PTR DefDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM ZarejestrujKlase(WNDCLASSEX*);

[[noreturn]] void SEPPUKU(LPCWSTR lpszFunc);
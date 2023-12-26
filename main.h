#include <windows.h>
#include <cmath>
#include <CommCtrl.h>
#include "resource.h"
#include <strsafe.h>
#include <vector>
#include <winerror.h>
#include <windowsx.h>

// mimo nag³ówków parê plików nie chcia³o siê automatycznie wczytaæ
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

// odkomentuj w celu dodatkowej funkcjonalnoœci
// #define DEBUG

// wiele typów ju¿ jest pisane W_TAKI_SPOSÓB, wiêc te te¿ mog¹
// w dodatku Windows u¿ywa notacji wêgierskiej (np. char lpszTekst[20] = {})
// constexpr nie dzia³a na nazwy typów, a #define tak
#define STRUCT struct
#define STATIC static
#define AUTO auto
#define SAMOCHÓD AUTO // :)
#define CONSTEXPR constexpr
#define ENUM enum
#define CLASS class
#define WEKTOR_DO_LISCIA std::vector<struct Leaf>
#define TEMPLATE template
#define TYPENAME typename

#define TESTMSG MessageBox(NULL, TEXT("okno wrum"), TEXT("wrum"), MB_OK);

constexpr CONST TCHAR* COMPARE_STRING_FLOAT = L"%f\0";
constexpr CONST TCHAR* COMPARE_STRING_DOUBLE = L"%lf\0";
constexpr CONST TCHAR* COMPARE_STRING_LONG_DOUBLE = L"%Lf\0";
constexpr CONST TCHAR* COMPARE_STRING_STRING = L"%s\0";
constexpr CONST TCHAR* COMPARE_STRING_LONG = L"%ld\0";
constexpr CONST TCHAR* COMPARE_STRING_LONG_LONG = L"%lld\0";
constexpr CONST TCHAR* COMPARE_STRING_CHAR = L"%c\0";
constexpr CONST TCHAR* COMPARE_STRING_INT = L"%d\0"; 
constexpr CONST TCHAR* COMPARE_STRING_UINT = L"%u\0"; // tak, ale nie
													// bez dodatkowego formatowania zwraca liczbê U2
constexpr CONST TCHAR* COMPARE_STRING_DEFAULT = COMPARE_STRING_INT;

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
HWND GetGrandParent(HWND); 

struct HWNDWrapper {
	HWND hwnd;
};

// [[noreturn]] ucisza kompilator
// bez tego jest ostrze¿enie, ¿e zmienna powoduj¹ca b³¹d mo¿e mieæ wartoœæ NULL i spowodowaæ inne b³êdy
// mo¿e i to jest tu przechwytywane, ta funkcja zawsze koñczy program

[[noreturn]] void AWARIA(LPCWSTR lpszFunc);
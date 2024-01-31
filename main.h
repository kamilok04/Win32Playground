// <windows.h> musi być wezwane przed każdą inną biblioteką systemu
#include <Windows.h>
#include "resource.h"
#include <CommCtrl.h>
#include <strsafe.h>
#include <windowsx.h>
#include <winerror.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <dwrite.h>

// standard
#include <array>
#include <cmath>
#include <set>
#include <vector>
#include <algorithm>
#include <string>
#include <map>

// mimo #include parę plików nie chciało się automatycznie wczytać
#pragma comment(lib, "winmm.lib")
#pragma comment(lib, "comctl32.lib")

// odkomentuj w celu dodatkowej funkcjonalności
// (chyba) 
// #define DEBUG

// wyrzuć najstarszą funkcjonalność; zabiera miejsce, a jest nieużywana
#define WIN32_LEAN_AND_MEAN

// wiele typów już jest pisane W_TAKI_SPOSÓB, więc te też mogą
// w dodatku Windows używa notacji węgierskiej (np. char lpszTekst[20] = {})
// ten projekt też jej w miarę możliwości przestrzega
// 
// za robienie wszystkiego inaczej
// Microsoftowi wielkie HWND
#define STATIC static
#define AUTO auto
#define STRUCT struct
#define CONSTEXPR constexpr
#define ENUM enum
#define CLASS class
#define LEAF_VECTOR std::vector<struct LEAF>
#define TEMPLATE template
#define TYPENAME typename
#define TYPEDEF typedef
#define UNION union
#define cot(x) (1 / tan(x))
#define DOUBLE double
#define LDOUBLE long double
#define DOPOKI_AUTORA_NIE_BEDZIE_STAC_NA_PIWO for(;;)


// :)
#define SAMOCHÓD AUTO
#define OOLONG ULONG    // są tu koneserzy herbaty?

// do debugowania
#define TESTMSG MessageBox(NULL, TEXT("Wiadomość testowa: kontynuuj"), TEXT("test"), MB_OK | MB_ICONINFORMATION);

// kolorki!
#define RED		RGB(255, 0, 0)
#define BLUE	RGB(0, 0, 255)
#define GREEN	RGB(0, 255, 0)
#define YELLOW	RGB(255, 255, 0)
#define PURPLE	RGB(255, 0, 255)
#define CYAN	RGB(0, 255, 255)
#define WHITE	RGB(255, 255, 255)
#define BLACK	RGB(0, 0, 0)

// stałe
ENUM CLASS DIMENSIONS : INT{ X = 0, Y = 1, DIMENSIONS_MAX };

// Funkcje używane w main.cpp
LRESULT CALLBACK MainWndProc(HWND, UINT, WPARAM, LPARAM);
LRESULT CALLBACK GraphicsDemoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TrigonometryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
INT_PTR DefDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
ATOM ZarejestrujKlase(WNDCLASSEX*);

// Funkcje używane w więcej niż jednym pliku
HWND GetGrandParent(HWND); 
HBITMAP CaptureScreenPart(HDC, INT x, INT y, INT cX, INT cY);
BOOL DrawScreenPart(HDC, INT x, INT y, HBITMAP);
LONG GetRectDimension(RECT*, DIMENSIONS);

// szablony użytkowe
// ============================================================

// zwraca najbliższą potęgę dwójki
// jawna specjalizacja dla typów niecałkowitych
TEMPLATE<TYPENAME T>
TYPENAME std::enable_if<!std::is_integral<T>::value, T>::type ToLowerTwoPower(T t)
{
    return static_cast<T>(std::pow(2, std::floor(std::log2(t))));
};

// Dokumentacja: operator wgniotki
// x>>n<<n zwraca najbliższą wielokrotność 2^(n-1) mniejszą lub równą x
TEMPLATE<TYPENAME T>
TYPENAME std::enable_if<std::is_integral<T>::value, T>::type ToLowerTwoPower(T t)
{
	return t>>2<<2; 
};

// Miało być szybko, więc jest szybko
// Miało być ładnie, więc jest szybko
TEMPLATE<TYPENAME T>
TYPENAME std::enable_if<std::is_integral<T>::value, T>::type ToHigherTwoPower(T t)
{
    return ToLowerTwoPower<T>(t) << 1;
}

// prawdziwe w każdej podstawie a:
// ⌊log_a(x) = ilość cyfr x w podstawie a
// ⌈log_a(x) = 1 + ilość cyfr (x - 1) w podstawie a
TEMPLATE<TYPENAME T>
TYPENAME std::enable_if<std::is_integral<T>::value, T>::type ilog2(T t)
{
    return 8*sizeof(T) - __lzcnt(t);
};




// [[noreturn]] - atrybut C++11, który w tym przypadku ucisza kompilator :)
// TODO:    podmienić wywołania AWARIA() na jakieś mniej toporne funkcje obsługi błędów
//          jakieś osobne okienko może? 
[[noreturn]] void AWARIA(LPCWSTR lpszFunc); 

/**
 * Funkcja do zwalniania interfejsów/bloków/czegoś jeszcze
 * Nie ma jej w żadnej bibliotece, ale dokumentacja udostępnia jej kod
 * https://learn.microsoft.com/en-us/windows/win32/medfound/saferelease
 * 
 * \param ppT
 */
template <class T> void SafeRelease(T** ppT)
{
    if (*ppT)
    {
        (*ppT)->Release();
        *ppT = NULL;
    }
}

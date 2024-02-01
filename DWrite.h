#pragma once
#include <Windows.h>
#include <d2d1.h>
#include <dwrite.h>

template <class T> VOID SafeRelease(T** ppT);

// Interfejs DirectX do rysowania czcionek
// https://docs.microsoft.com/en-us/windows/win32/directwrite/direct-write-portal
class DWrite
{
public:
	~DWrite() {
		DiscardRecources();
		SafeRelease(&pDWriteFactory);
		SafeRelease(&pTextFormat);
		SafeRelease(&pD2DFactory);
	};
	IDWriteFactory* pDWriteFactory = NULL;
	IDWriteTextFormat* pTextFormat = NULL;
	ID2D1Factory* pD2DFactory = NULL;
	ID2D1HwndRenderTarget* pRT = NULL;
	ID2D1SolidColorBrush* pBrush = NULL;
	UINT uWindowDPI = 0;
	RECT rc = {};
	HRESULT CreateIndependentResources();
	HRESULT CreateDependentResources(HWND hwnd);
	VOID DiscardRecources();
	HRESULT DrawText(LPCWSTR wszText, UINT32 cTextLength); // nareszcie 
	HRESULT DrawContent(HWND hwnd, LPCWSTR wszText, UINT32 cTextLength, BOOL bForceUpdate = FALSE);
	
	// Powoli przygotowuj aplikacjê do skalowania DPI
	// funkcja jest z dokumentacji
	// https://docs.microsoft.com/en-us/windows/win32/learnwin32/dpi-and-device-independent-pixels
	FLOAT g_DPIScale = 1.0f;
	VOID InitializeDPIScale(HWND hwnd) {
		FLOAT dpi =(FLOAT)  GetDpiForWindow(hwnd);
		g_DPIScale = dpi / USER_DEFAULT_SCREEN_DPI;
	}

	template <typename T>
	FLOAT PixelsToDipsX(T x)
	{
		return static_cast<FLOAT>(x) / g_DPIScale;
	}

	template <typename T>
	FLOAT PixelsToDips(T y)
	{
		return static_cast<FLOAT>(y) / g_DPIScale;
	}
};

/**
 * Funkcja do zwalniania interfejsów/bloków/czegoœ jeszcze
 * Nie ma jej w ¿adnej bibliotece, ale dokumentacja udostêpnia jej kod
 * https://learn.microsoft.com/en-us/windows/win32/medfound/saferelease
 *
 * \param ppT
 */
template <class T> VOID SafeRelease(T** ppT)
{
	if (*ppT)
	{
		(*ppT)->Release();
		*ppT = NULL;
	}
}


/* TODO
	- dorobi� adnotacje SAL 
	- podmieni� wi�kszo�� wywo�a� AWARIA na co� mniej agresywnego
*/

#include "main.h"

// (alleluja) i do przodu!
/**
 * g��wna funkcja programu
 * 
 * \param hInstance		- uchwyt do instancji programu (pocz�tek pami�ci programu)
 * \param hPrevInstance - relikt z 16-bit Windows, obecnie zawsze NULL
 * \param lpCmdLine		- wska�nik do ci�gu znak�w z argumentami wiersza polece�
 * \param iCmdShow		- parametr okre�laj�cy jak ma by� pokazane okno
 * \return kod wyj�cia (0 przy sukcesie, co� innego przy pora�ce)
 */
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int iCmdShow) {

	// utw�rz g��wne okno programu
	WNDCLASSEX wc = {};
	wc.cbSize = sizeof(WNDCLASSEX);
	wc.lpfnWndProc = MainWndProc;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(WNDCLASSEX*);
	wc.hInstance = hInstance;
	wc.hIcon = reinterpret_cast<HICON>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_PIVO_256), IMAGE_ICON, 32, 26, NULL));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_INFOBK + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
	wc.lpszClassName = TEXT("CPPDemo");

	// zapisz identyfikator klasy (nazwa te� jest poprawnym identyfikatorem)
	ATOM aAtom = RegisterClassEx(&wc);

	// wy��cz wype�nianie bufora znakami debugowania (0xFE)
	_CrtSetDebugFillThreshold(NULL);

#ifdef DEBUG
	LPCWSTR lpszTitle = TEXT("BIGOS (DEBUG)");
	
	
#else
	LPCWSTR lpszTitle = TEXT("BIGOS");
#endif

	// w�asciwo�ci okna:
	// - przerysuj je przy zmianie rozmiaru w pionie
	// - przerysuj je przy zmianie rozmiaru w poziomie
	// - widoczne od razu po utworzeniu
	// - ma kraw�dzie, pasek tytu�u, mo�e wy�wietla� menu i ma dobrze zdefiniowane zachowania domy�lne
	HWND hwnd = CreateWindowEx(0, reinterpret_cast<LPCTSTR>(aAtom), lpszTitle,
		CS_HREDRAW | CS_VREDRAW | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		640,
		480,
		NULL,
		NULL,
		NULL,
		NULL);
	if (hwnd == NULL) {
		AWARIA(TEXT("Tworzenie g��wnego okna"));
	}
	// przywr�� poprzedni kolor, inaczej wszystkie modu�y mia�yby ��tawe t�o 
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	// zapisz referencj� do klasy w danych okna, "na marginesie"
	// b�dzie wykorzystywana ponownie
	// Win32: u�ywanie GWLP_USERDATA jest mo�liwe, ale traktowane jako antypraktyka
	// GWLP_USERDATA jest przeznaczone dla u�ytkownik�w kodu, nie autor�w
	SetWindowLongPtr(hwnd, 0, reinterpret_cast<LONG_PTR>(&wc));

	// wczytaj akceleratory
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_AKCELERATOR_CZASTECZEK));
	if (hAccel == NULL) AWARIA(TEXT("akcelerator"));

	UpdateWindow(hwnd);

	// tu jest miejsce na ostatnie rzeczy do zainicjalizowania
	// przed uruchomieniem widocznej cz�ci programu

	// w��cz co bardziej zaawansowane, ale nadal standardowe, kontrolki
	INITCOMMONCONTROLSEX iccex = {};
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_COOL_CLASSES; // paski narz�dzi, statusu, suwaki, wskaz�wki, rebary
	if(!InitCommonControlsEx(&iccex)) AWARIA(TEXT("nie ma kontrolek!"));

	// g��wna p�tla
	MSG msg;
	while (GetMessage(&msg, hwnd, 0, 0) > 0) {
		if (!TranslateAccelerator(hwnd, hAccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}

	// koniec programu
	return 0;

}

/**
 * Funkcja do obs�ugi g��wnego okna programu
 * 
 * \param hwnd - uchwyt do okna
 * \param uMsg - wiadomo�� otrzymana w winmain
 * \param wParam - parametr wiadomo�ci
 * \param lParam - parametr wiadomo�ci
 * \return kod zwrotu
 */
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect{};
	// do tekstu na ekranie powitalnym
	STATIC TCHAR lpIntro[256] = {};
	// nie lubi� D2D
	STATIC DWrite dw;
	STATIC BOOL bFastMode = FALSE;

	switch (uMsg) {
	case WM_CREATE: {
		
		LoadString(GetModuleHandle(NULL), IDS_INTRO, lpIntro, 256);
		// zdob�d� informacje o systemowej czcionce
		// domy�lnie: Marlett, 8pt
		hdc = GetDC(hwnd);
		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hwnd, hdc);

		return 0;
	}
	case WM_PAINT: {
		// narysuj startowy tekst
		hdc = BeginPaint(hwnd, &ps);
		FillRect(hdc, &ps.rcPaint, reinterpret_cast<HBRUSH>(COLOR_INFOBK + 1));
	if(bFastMode){
		SetBkMode(hdc, TRANSPARENT);
		STATIC HFONT hFont = CreateFont(34, 0, 0, 0, FW_REGULAR, FALSE, FALSE, FALSE,
			DEFAULT_CHARSET, OUT_TT_ONLY_PRECIS, NULL, CLEARTYPE_QUALITY,
			DEFAULT_PITCH | FF_SWISS, L"Comic Sans MS");
		HFONT hOldFont = (HFONT)SelectObject(hdc, hFont);
		STATIC RECT rText = {};
		GetClientRect(hwnd, &rText);
		//ExtTextOut(hdc, 0, 0, ETO_CLIPPED, &rText , lpIntro, lstrlen(lpIntro), NULL);
		DrawTextEx(hdc, lpIntro, lstrlen(lpIntro), &rText, DT_NOCLIP | DT_WORDBREAK, NULL);
		// SelectObject(hdc, hOldFont);
	}
	else {
		dw.DrawContent(hwnd, lpIntro, lstrlen(lpIntro), TRUE);
	}
		EndPaint(hwnd, &ps);
		return 0;
	}
	case WM_SIZE: {
		InvalidateRect(hwnd, NULL, TRUE);
	}
	/*case WM_EXITSIZEMOVE: {
		InvalidateRect(hwnd, NULL, TRUE);
		return 0;
	}*/
	case WM_COMMAND:
		// komendy, tak te z menu, jak i te z akcelerator�w
		switch (LOWORD(wParam)) {
		case IDM__ZAKO_CZ_TALT_F1: {
			DestroyWindow(hwnd);
			// papa
		}
		case IDM_O_PROGRAMIE____TF1:
		case IDR_HELP_SPEEDRUN: {
			PlaySound(TEXT("tada.wav"), NULL, SND_FILENAME | SND_ASYNC);
#ifdef _WIN64
			DialogBox(NULL, MAKEINTRESOURCE(IDD_O_PROGRAMIE), hwnd, DefDialogProc);
#else
			DialogBox(NULL, MAKEINTRESOURCE(IDD_O_PROGRAMIE), hwnd, (DLGPROC) DefDialogProc);
#endif
			break;
		}

		// zacznij modu� Tree
		case IDM_DRZEWKO___1:
		case IDR_TREE_SPEEDRUN:
		{
			static ATOM atom = 0;
			if (atom == 0) {
				WNDCLASSEX* pwc = reinterpret_cast<WNDCLASSEX*>(GetWindowLongPtr(hwnd, 0));
				pwc->lpfnWndProc = TreeWndProc;
				pwc->cbClsExtra = sizeof(HWND);
			 	//pwc->style =  CS_HREDRAW | CS_VREDRAW;
				pwc->style = NULL;
				pwc->lpszClassName = TEXT("Tree");
				pwc->lpszMenuName = NULL;
				atom = RegisterClassEx(pwc);
			}

			HWND childHwnd = CreateWindowEx(NULL, reinterpret_cast<LPCWSTR>(atom), 
				TEXT("Kodowanie Huffmana"), WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN,
				CW_USEDEFAULT, CW_USEDEFAULT, 1024, 768,
				hwnd, NULL, NULL, NULL);
			UpdateWindow(childHwnd);
			ShowWindow(childHwnd, 1);

			MSG msg;
			while (GetMessage(&msg, childHwnd, 0, 0) > 0) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			break;
		}

		// zacznij modu� GraphicsDemo
		case IDM_W_A_CIWO_CI_GRAFIKI___1: {
			static ATOM atom = 0;
			if (atom == 0) {
				WNDCLASSEX* pwc = reinterpret_cast<WNDCLASSEX*>(GetWindowLongPtr(hwnd, 0));
				pwc->lpfnWndProc = GraphicsDemoWndProc;
				pwc->cbWndExtra = 0;
				pwc->style = CS_HREDRAW | CS_VREDRAW;
				pwc->lpszClassName = TEXT("GraphicsDemo");
				pwc->lpszMenuName = NULL;
				pwc->hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
				atom = RegisterClassEx(pwc);
			}

			HWND childHwnd = CreateWindowEx(0, reinterpret_cast<LPCWSTR>(atom), TEXT("Mo�liwo�ci graficzne systemu"), WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
				hwnd, NULL, NULL, NULL);

			UpdateWindow(childHwnd);
			ShowWindow(childHwnd, SW_SHOW);

			MSG msg;
			while (GetMessage(&msg, childHwnd, 0, 0) > 0) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
		
			break;
		}

		// zacznij modu� Trigonometry
		case IDM__POKAZ_TRYGONOMETRII___1:
		case IDR_TRIG_SPEEDRUN:{
			static ATOM atom;
			if (atom == 0) {
				WNDCLASSEX* pwc = reinterpret_cast<WNDCLASSEX*>(GetWindowLongPtr(hwnd, 0));
				pwc->lpfnWndProc = TrigonometryWndProc;
				//pwc->cbWndExtra = sizeof(HACCEL*);
				pwc->cbWndExtra = NULL;
				pwc->style = CS_HREDRAW | CS_VREDRAW;
				//pwc->style = NULL;
				pwc->lpszClassName = TEXT("TrigDemo");
				pwc->lpszMenuName = MAKEINTRESOURCE(IDR_TRIG);
			
				atom = RegisterClassEx(pwc);;
			}
			HWND childHwnd = CreateWindowEx(0, reinterpret_cast<LPCWSTR>(atom), TEXT("Pokaz rysowania trygonometrii"),
				WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 800,
				hwnd, NULL, NULL, NULL);
			SetWindowLongPtr(childHwnd, GWL_STYLE, GetWindowLong(childHwnd, GWL_STYLE)&~WS_MINIMIZEBOX);
			// HACCEL hAccel = LoadAccelerators(GetModuleHandle(NULL), MAKEINTRESOURCE(IDR_AKCELERATOR_CZASTECZEK));
			// SetWindowLongPtr(childHwnd, 0, (LONG_PTR)&hAccel);
			UpdateWindow(childHwnd);
			ShowWindow(childHwnd, 1);
			MSG msg;
			while (GetMessage(&msg, childHwnd, 0, 0) > 0) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			break;
		}		

		// prze��cz tryb szybki/wolny
		case IDR_TOGGLE_EFF_MODE:
		case IDM_TOGGLE_EFF_MODE:{
			bFastMode = !bFastMode;
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		}

		// Nie ma tego w menu: Shift + F4 wyrzuca klas�, zmniejszaj�c zu�ycie pami�ci ~10-krotnie
		case IDR_DESTROY_D2D1: {
			bFastMode = TRUE;
			dw.~DWrite();
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		}
		}
	default: return DefWindowProc(hwnd, uMsg, wParam, lParam); 
	}
	return 0;
	
}
INT_PTR DefDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	switch (uMsg)
	{
	case WM_INITDIALOG:
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			EndDialog(hwnd, IDOK);
			break;
		case IDCANCEL:
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		break;
	default:
		return FALSE;
	}
	return TRUE;
}


/**
 * Bardzo kr�tka funkcja - zwr�� uchwyt dziadka na podstawie uchwytu wnuka.
 * 
 * \param hwnd - uchwyt do wnuka
 * \return 
 */
HWND GetGrandParent(HWND hwnd) { return GetParent(GetParent(hwnd)); }


/**
 * Funkcja do przechwytywania kawa�ka ekranu.
 * 
 * \param hdc	- kontekst urz�dzenia
 * \param x		- wsp�rz�dna x lewego g�rnego rogu
 * \param y		- wsp�rz�dna y lewego g�rnego rogu
 * \param cX	- szeroko�� przechwytywanego obszaru
 * \param cY	- wysoko�� przechwytywanego obszaru
 * \return		- uchwyt do bitmapy, je�li si� uda�o
 */

HBITMAP CaptureScreenPart(HDC hdc, INT x, INT y, INT cX, INT cY) {
	HDC hdcMem = CreateCompatibleDC(hdc);
	INT capX = GetDeviceCaps(hdc, HORZRES);
	INT capY = GetDeviceCaps(hdc, VERTRES);
	HBITMAP hbmMem = CreateCompatibleBitmap(hdc, cX, cY);
	HBITMAP hbmOld = (HBITMAP)SelectObject(hdcMem, hbmMem);
	BitBlt(hdcMem, 0, 0, cX, cY, hdc, x, y, SRCCOPY);
	hbmMem = (HBITMAP)SelectObject(hdcMem, hbmOld);
	DeleteDC(hdcMem);
	return hbmMem;
	
}

/**
 * DrawScreenPart - rysuje cz�� ekranu (lub dowoln� inn� bitmap�, na dobr� spraw�).
 * Zaleca si� u�ywanie tej funkcji w zestawie z CaptureScreenPart.
 * 
 * \param hdc - uchwyt do kontekstu urz�dzenia
 * \param x - wsp�rz�dna x lewego g�rnego rogu
 * \param y - wsp�rz�dna y lewego g�rnego rogu
 * \param hBitmap - uchwyt do bitmapy do narysowania
 * \return - czy si� uda�o
 */
BOOL DrawScreenPart(HDC hdc, INT x, INT y, HBITMAP hBitmap) {
	
	BITMAP bmp = {};
	HDC hdcMem = CreateCompatibleDC(hdc);
	GetObject(hBitmap, sizeof(BITMAP), &bmp);
	SelectObject(hdcMem, hBitmap);
	BOOL bResult = BitBlt(hdc, x, y, bmp.bmWidth, bmp.bmHeight, hdcMem, 0, 0, SRCCOPY);
	DeleteDC(hdcMem);
	return bResult;
}

/**  
 * Prosta funkcja zwracaj�ca wymiar prostok�ta.
 * 
 * \param rect	- prostok�t
 * \param x		- TRUE, je�li chcemy szeroko�� (domy�lnie), FALSE, je�li wysoko��
 * \return		- wyliczona d�ugo��
 */
LONG GetRectDimension(RECT* lpRect, DIMENSIONS dDim = DIMENSIONS::X) {
	return dDim == DIMENSIONS::X ? lpRect->right - lpRect->left : lpRect->bottom - lpRect->top;
}


/**
 * Funkcja owijaj�ca b��dy w spos�b przyja�niejszy u�ytkownikowi.
 *  
 * \param lpszFunc - nazwa funkcji do wy�wietlenia, w praktyce dowolny tekst
 * \return NIE! Co wi�cej, ko�czy program
 */
[[noreturn]] void AWARIA(LPCWSTR lpszFunc)
{
	LPVOID lpMsgBuf = nullptr;
	LPVOID lpDisplayBuf = nullptr;
	DWORD kod = GetLastError();
	FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER |
		FORMAT_MESSAGE_FROM_SYSTEM |
		FORMAT_MESSAGE_IGNORE_INSERTS,
		NULL,
		kod,
		MAKELANGID(LANG_POLISH, SUBLANG_DEFAULT),
		(LPTSTR)&lpMsgBuf,
		0, NULL);
	lpDisplayBuf = (LPVOID)LocalAlloc(LMEM_ZEROINIT,
		(lstrlen((LPCTSTR)lpMsgBuf) + lstrlen((LPCTSTR)lpszFunc) + 200) * sizeof(TCHAR));

	if (lpDisplayBuf == NULL) ExitProcess(2137); // jest bardzo �le

	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s zjeba�o si� z kodem: %d: %s."),
		lpszFunc, kod, reinterpret_cast<STRSAFE_LPCWSTR>(lpMsgBuf));
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("o ni"), MB_ICONINFORMATION | MB_CANCELTRYCONTINUE);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(kod);
}


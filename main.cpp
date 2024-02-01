/* TODO
	- dorobiæ adnotacje SAL 
	- podmieniæ wiêkszoœæ wywo³añ AWARIA na coœ mniej agresywnego
*/

#include "main.h"

// (alleluja) i do przodu!
/**
 * g³ówna funkcja programu
 * 
 * \param hInstance		- uchwyt do instancji programu (pocz¹tek pamiêci programu)
 * \param hPrevInstance - relikt z 16-bit Windows, obecnie zawsze NULL
 * \param lpCmdLine		- wskaŸnik do ci¹gu znaków z argumentami wiersza poleceñ
 * \param iCmdShow		- parametr okreœlaj¹cy jak ma byæ pokazane okno
 * \return kod wyjœcia (0 przy sukcesie, coœ innego przy pora¿ce)
 */
int WINAPI wWinMain(_In_ HINSTANCE hInstance, _In_opt_ HINSTANCE hPrevInstance, _In_ LPWSTR lpCmdLine, _In_ int iCmdShow) {

	// utwórz g³ówne okno programu
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

	// zapisz identyfikator klasy (nazwa te¿ jest poprawnym identyfikatorem)
	ATOM aAtom = RegisterClassEx(&wc);

	// wy³¹cz wype³nianie bufora znakami debugowania (0xFE)
	_CrtSetDebugFillThreshold(NULL);

#ifdef DEBUG
	LPCWSTR lpszTitle = TEXT("BIGOS (DEBUG)");
	
	
#else
	LPCWSTR lpszTitle = TEXT("BIGOS");
#endif

	// w³asciwoœci okna:
	// - przerysuj je przy zmianie rozmiaru w pionie
	// - przerysuj je przy zmianie rozmiaru w poziomie
	// - widoczne od razu po utworzeniu
	// - ma krawêdzie, pasek tytu³u, mo¿e wyœwietlaæ menu i ma dobrze zdefiniowane zachowania domyœlne
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
		AWARIA(TEXT("Tworzenie g³ównego okna"));
	}
	// przywróæ poprzedni kolor, inaczej wszystkie modu³y mia³yby ¿ó³tawe t³o 
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	// zapisz referencjê do klasy w danych okna, "na marginesie"
	// bêdzie wykorzystywana ponownie
	// Win32: u¿ywanie GWLP_USERDATA jest mo¿liwe, ale traktowane jako antypraktyka
	// GWLP_USERDATA jest przeznaczone dla u¿ytkowników kodu, nie autorów
	SetWindowLongPtr(hwnd, 0, reinterpret_cast<LONG_PTR>(&wc));

	// wczytaj akceleratory
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_AKCELERATOR_CZASTECZEK));
	if (hAccel == NULL) AWARIA(TEXT("akcelerator"));

	UpdateWindow(hwnd);

	// tu jest miejsce na ostatnie rzeczy do zainicjalizowania
	// przed uruchomieniem widocznej czêœci programu

	// w³¹cz co bardziej zaawansowane, ale nadal standardowe, kontrolki
	INITCOMMONCONTROLSEX iccex = {};
	iccex.dwSize = sizeof(INITCOMMONCONTROLSEX);
	iccex.dwICC = ICC_BAR_CLASSES | ICC_TAB_CLASSES | ICC_COOL_CLASSES; // paski narzêdzi, statusu, suwaki, wskazówki, rebary
	if(!InitCommonControlsEx(&iccex)) AWARIA(TEXT("nie ma kontrolek!"));

	// g³ówna pêtla
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
 * Funkcja do obs³ugi g³ównego okna programu
 * 
 * \param hwnd - uchwyt do okna
 * \param uMsg - wiadomoœæ otrzymana w winmain
 * \param wParam - parametr wiadomoœci
 * \param lParam - parametr wiadomoœci
 * \return kod zwrotu
 */
LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc;
	PAINTSTRUCT ps;
	RECT rect{};
	// do tekstu na ekranie powitalnym
	STATIC TCHAR lpIntro[256] = {};
	// nie lubiê D2D
	STATIC DWrite dw;
	STATIC BOOL bFastMode = FALSE;

	switch (uMsg) {
	case WM_CREATE: {
		
		LoadString(GetModuleHandle(NULL), IDS_INTRO, lpIntro, 256);
		// zdob¹dŸ informacje o systemowej czcionce
		// domyœlnie: Marlett, 8pt
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
		// komendy, tak te z menu, jak i te z akceleratorów
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

		// zacznij modu³ Tree
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

		// zacznij modu³ GraphicsDemo
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

			HWND childHwnd = CreateWindowEx(0, reinterpret_cast<LPCWSTR>(atom), TEXT("Mo¿liwoœci graficzne systemu"), WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
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

		// zacznij modu³ Trigonometry
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

		// prze³¹cz tryb szybki/wolny
		case IDR_TOGGLE_EFF_MODE:
		case IDM_TOGGLE_EFF_MODE:{
			bFastMode = !bFastMode;
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		}

		// Nie ma tego w menu: Shift + F4 wyrzuca klasê, zmniejszaj¹c zu¿ycie pamiêci ~10-krotnie
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
 * Bardzo krótka funkcja - zwróæ uchwyt dziadka na podstawie uchwytu wnuka.
 * 
 * \param hwnd - uchwyt do wnuka
 * \return 
 */
HWND GetGrandParent(HWND hwnd) { return GetParent(GetParent(hwnd)); }


/**
 * Funkcja do przechwytywania kawa³ka ekranu.
 * 
 * \param hdc	- kontekst urz¹dzenia
 * \param x		- wspó³rzêdna x lewego górnego rogu
 * \param y		- wspó³rzêdna y lewego górnego rogu
 * \param cX	- szerokoœæ przechwytywanego obszaru
 * \param cY	- wysokoœæ przechwytywanego obszaru
 * \return		- uchwyt do bitmapy, jeœli siê uda³o
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
 * DrawScreenPart - rysuje czêœæ ekranu (lub dowoln¹ inn¹ bitmapê, na dobr¹ sprawê).
 * Zaleca siê u¿ywanie tej funkcji w zestawie z CaptureScreenPart.
 * 
 * \param hdc - uchwyt do kontekstu urz¹dzenia
 * \param x - wspó³rzêdna x lewego górnego rogu
 * \param y - wspó³rzêdna y lewego górnego rogu
 * \param hBitmap - uchwyt do bitmapy do narysowania
 * \return - czy siê uda³o
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
 * Prosta funkcja zwracaj¹ca wymiar prostok¹ta.
 * 
 * \param rect	- prostok¹t
 * \param x		- TRUE, jeœli chcemy szerokoœæ (domyœlnie), FALSE, jeœli wysokoœæ
 * \return		- wyliczona d³ugoœæ
 */
LONG GetRectDimension(RECT* lpRect, DIMENSIONS dDim = DIMENSIONS::X) {
	return dDim == DIMENSIONS::X ? lpRect->right - lpRect->left : lpRect->bottom - lpRect->top;
}


/**
 * Funkcja owijaj¹ca b³êdy w sposób przyjaŸniejszy u¿ytkownikowi.
 *  
 * \param lpszFunc - nazwa funkcji do wyœwietlenia, w praktyce dowolny tekst
 * \return NIE! Co wiêcej, koñczy program
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

	if (lpDisplayBuf == NULL) ExitProcess(2137); // jest bardzo Ÿle

	StringCchPrintf((LPTSTR)lpDisplayBuf,
		LocalSize(lpDisplayBuf) / sizeof(TCHAR),
		TEXT("%s zjeba³o siê z kodem: %d: %s."),
		lpszFunc, kod, reinterpret_cast<STRSAFE_LPCWSTR>(lpMsgBuf));
	MessageBox(NULL, (LPCTSTR)lpDisplayBuf, TEXT("o ni"), MB_ICONINFORMATION | MB_CANCELTRYCONTINUE);

	LocalFree(lpMsgBuf);
	LocalFree(lpDisplayBuf);
	ExitProcess(kod);
}


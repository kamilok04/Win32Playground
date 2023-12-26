#include "main.h"

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
	wc.hbrBackground = reinterpret_cast<HBRUSH>(COLOR_WINDOW + 1);
	wc.lpszMenuName = MAKEINTRESOURCE(IDR_MAIN_MENU);
	wc.lpszClassName = TEXT("CPPDemo");

	ATOM atom = RegisterClassEx(&wc); // zapisz identyfikator klasy (nazwa te¿ jest poprawnym identyfikatorem)
	// w³asciwoœci okna:
	// - przerysuj je przy jakiejkolwiek zmianie
	// - widoczne od razu po utworzeniu
	// - ma krawêdzie, pasek tytu³u, mo¿e wyœwietlaæ menu i ma dobrze zdefiniowane zachowania domyœlne
#if defined DEBUG
		LPCWSTR title = TEXT("Aplikacja (DEBUG)");
#else
		LPCWSTR title = TEXT("Aplikacja");
#endif
	HWND hwnd = CreateWindowEx(0, reinterpret_cast<LPCTSTR>(atom), title, CS_HREDRAW | CS_VREDRAW | WS_VISIBLE | WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		NULL,
		NULL,
		NULL,
		NULL);
	if (hwnd == NULL) {
		AWARIA(TEXT("Tworzenie g³ównego okna"));
	}


	SetWindowLongPtr(hwnd, 0, reinterpret_cast<LONG_PTR>(& wc));

	// wczytaj akceleratory
	HACCEL hAccel = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDR_AKCELERATOR_CZASTECZEK));
	if (hAccel == NULL) AWARIA(TEXT("akcelerator"));

	UpdateWindow(hwnd);

	MSG msg;
	while (GetMessage(&msg, hwnd, 0, 0) > 0) {
		if (!TranslateAccelerator(hwnd, hAccel, &msg)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
	}
	return 0;

}


LRESULT CALLBACK MainWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	HDC hdc = nullptr;
	static PAINTSTRUCT ps{};
	static RECT rect{};
	switch (uMsg) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		TEXTMETRIC tm;
		GetTextMetrics(hdc, &tm);
		ReleaseDC(hwnd, hdc);
		hdc = nullptr;
		return 0;
	case WM_PAINT:
		if(hdc == nullptr) hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rect);
		DrawText(hdc, TEXT("CHUJ"), -1, &rect, DT_SINGLELINE);
		EndPaint(hwnd, &ps);
		return 0;
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDM_O_PROGRAMIE____TF1:
		case IDR_HELP_SPEEDRUN: {
			DialogBox(NULL, MAKEINTRESOURCE(IDD_O_PROGRAMIE), hwnd, DefDialogProc);
			break;
		}
		case IDM_DRZEWKO___1:
		case IDR_TREE_SPEEDRUN:
		{
			static ATOM atom = 0;
			if (atom == 0) {
				WNDCLASSEX* pwc = reinterpret_cast<WNDCLASSEX*>(GetWindowLongPtr(hwnd, 0));
				pwc->lpfnWndProc = TreeWndProc;
				pwc->cbClsExtra = sizeof(HWND);
			 	pwc->style = CS_HREDRAW | CS_VREDRAW;
				pwc->lpszClassName = TEXT("Tree");
				pwc->lpszMenuName = NULL;
				atom = RegisterClassEx(pwc);
			}

			HWND childHwnd = CreateWindowEx(NULL, reinterpret_cast<LPCWSTR>(atom), TEXT("Kodowanie Huffmana"), WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
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
		case IDM_W_A_CIWO_CI_GRAFIKI___1: {
			static ATOM atom = 0;
			if (atom == 0) {
				WNDCLASSEX* pwc = reinterpret_cast<WNDCLASSEX*>(GetWindowLongPtr(hwnd, 0));
				pwc->lpfnWndProc = GraphicsDemoWndProc;
				pwc->cbWndExtra = 0;
				pwc->style = CS_HREDRAW | CS_VREDRAW;
				pwc->lpszClassName = TEXT("GraphicsDemo");
				pwc->lpszMenuName = NULL;
				atom = RegisterClassEx(pwc);
			}

			HWND childHwnd = CreateWindowEx(0, reinterpret_cast<LPCWSTR>(atom), TEXT("Mo¿liwoœci graficzne systemu"), WS_OVERLAPPEDWINDOW | WS_VSCROLL | WS_HSCROLL, CW_USEDEFAULT, CW_USEDEFAULT, 640, 480,
				hwnd, NULL, NULL, NULL);

			UpdateWindow(childHwnd);
			ShowWindow(childHwnd, 1);

			MSG msg;
			while (GetMessage(&msg, childHwnd, 0, 0) > 0) {
				TranslateMessage(&msg);
				DispatchMessage(&msg);
			}
			// chuj->atom = 0;
			break;
		}
		case IDM__POKAZ_TRYGONOMETRII___1:
		case IDR_TRIG_SPEEDRUN:{
			static ATOM atom;
			if (atom == 0) {
				WNDCLASSEX* pwc = reinterpret_cast<WNDCLASSEX*>(GetWindowLongPtr(hwnd, 0));
				pwc->lpfnWndProc = TrigonometryWndProc;
				pwc->cbWndExtra = 0;
				pwc->style = CS_HREDRAW | CS_VREDRAW;
				pwc->lpszClassName = TEXT("TrigDemo");
				pwc->lpszMenuName = MAKEINTRESOURCE(IDR_TRIG);
				atom = RegisterClassEx(pwc);;
			}
			HWND childHwnd = CreateWindowEx(0, reinterpret_cast<LPCWSTR>(atom), TEXT("Pokaz rysowania trygonometrii"),
				WS_OVERLAPPEDWINDOW, CW_USEDEFAULT, CW_USEDEFAULT, 1200, 480,
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
		}
	default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	
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

HWND GetGrandParent(HWND hwnd) { return GetParent(GetParent(hwnd)); }

[[noreturn]] void AWARIA(LPCWSTR lpszFunc)
{
	LPVOID lpMsgBuf;
	LPVOID lpDisplayBuf;
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
#include "Trigonometry.h"
#define NPI(n) n*PI
static const int num = 1000;
static const double PI = 3.141592;

LRESULT CALLBACK TrigonometryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int cxClient = 0;
	static int cyClient = 0;
	PAINTSTRUCT ps{};
	POINT punkty[num];
	HDC hdc = NULL;
	switch (uMsg) {
	case WM_SIZE: {
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;
	}
	case WM_PAINT: {

		hdc = BeginPaint(hwnd, &ps);
		MoveToEx(hdc, 0, cyClient / 2, NULL);
		LineTo(hdc, cxClient, cyClient / 2);
		for (size_t i = 0; i < num; i++) {
			punkty[i].x = static_cast<LONG>(i * cxClient / num);
			punkty[i].y = static_cast<LONG>(cyClient / 2 * (1 - sin(static_cast<long double>(i * 4 * PI / num)))); 
			// tu jest du¿o rzutów, podziêkowania proszê s³aæ Microsoftowi
			// i ich >40-letniemu API
		}
		Polyline(hdc, punkty, num);
		return 0;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
			case IDM_NIC1: {
				DialogBox(NULL, MAKEINTRESOURCE(IDD_TRIG), hwnd, TrigDialogProc);
				break;
			}
		}
	}


	case WM_DESTROY: {
		return 0;
	}
	default: break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR TrigDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam); // usuñ ostrze¿enie
	HWND lista;
	static const HWND suwak = GetDlgItem(hwnd, IDC_TRIG_PRECISION);
	static HWND zegar;
	LRESULT dlg;
	HFONT czcionka;
	static HBITMAP hImage;
	static DWORD pos;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		lista = GetDlgItem(hwnd, IDC_TRIG_FUNCTION_SELECT);
		InitTrigList(lista);
		hImage = reinterpret_cast<HBITMAP>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PIVO), IMAGE_BITMAP, 0, 0, NULL));
		if (hImage == NULL) SEPPUKU(TEXT("piwko"));
		dlg = SendDlgItemMessage(hwnd, IDC_PIVO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hImage);
		zegar = GetDlgItem(hwnd, IDC_TRIG_INDICATOR);
		czcionka =  CreateFont(30, 70, 100, 100, FW_DONTCARE, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FF_DONTCARE, TEXT("Consolas"));
		if (czcionka == NULL) SEPPUKU(TEXT("czci¹ka"));
		SendMessage(zegar, WM_SETFONT, (WPARAM) czcionka, FALSE );
		SetWindowText(zegar, TEXT("2137"));
		return TRUE;
	case WM_CTLCOLORSTATIC:
		HBRUSH brush;
		if ((HWND)lParam == GetDlgItem(hwnd, IDC_TRIG_PRECISION))
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC)wParam, RGB(255, 0, 0));
			brush = GetSysColorBrush(COLOR_MENU);
			return brush != NULL;
		}
		if ((HWND)lParam == GetDlgItem(hwnd, IDC_TRIG_INDICATOR))
		{
			SetBkMode((HDC)wParam, TRANSPARENT);
			SetTextColor((HDC) wParam, RGB(255, 0, 0));
			brush = GetSysColorBrush(COLOR_MENU);
			return brush != NULL;
		}
		break;
	case WM_HSCROLL: // pasek
		pos = static_cast<DWORD>(SendDlgItemMessage(hwnd, IDC_TRIG_PRECISION, TBM_GETPOS, 0, 0));
		TCHAR posStr[10];
		_itow_s(pos, posStr, 10);
		posStr[9] = 0;
		SetWindowText(zegar, posStr);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			// odczytaj wartoœci
			TCHAR bufor[20];
			TCHAR bufor2[20];
			TCHAR bufor3[20];
			GetDlgItemText(hwnd, IDC_TRIG_FUNCTION_SELECT, bufor, 20);
			GetDlgItemText(hwnd, IDC_TRIG_RANGE_FROM, bufor2, 20);
			GetDlgItemText(hwnd, IDC_TRIG_RANGE_TO, bufor3, 20);
			TCHAR str[200];
			StringCchPrintf(str, 200, TEXT("Funkcja: %s\r\nZakres od: %s\r\nZakres do: %s\r\nPrecyzja: %d\r\n"), bufor, bufor2, bufor3, pos);
			MessageBox(hwnd, str, TEXT("wiedza tajemna"), MB_ICONINFORMATION | MB_OK);

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

void InitTrigList(HWND hwnd) {
	TCHAR tcTmp[16];
	LoadString(NULL, IDS_SINE, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
	SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	LoadString(NULL, IDS_COSINE, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
	SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	LoadString(NULL, IDS_TANGENT, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
	SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	LoadString(NULL, IDS_COTANGENT, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
	SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	SendMessage(hwnd, CB_SETCURSEL, 2, 0);
	return;
}
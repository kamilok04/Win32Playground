#include "Trigonometry.h"
// problemy:
// - funkcje o mniejszym zakresie nie powinny siê rysowaæ w [0, cxClient], tylko w swoim zakresie
// - jak dobrze narysowaæ tangensa/cotangensa?
// - to s¹ funkcje OKRESOWE, naprawdê da siê to zoptymalizowaæ lepiej
// - zmiana koloru funkcji (jakaœ paletka jak z painta)


// wstêpne parametry funkcji
static struct TrigData TrigFunctions[4] = {
	{TRUE	, KPI(0), KPI(6), 50},
	{TRUE	, 0, KPI(6), 50},
	{FALSE	, 0, KPI(6), 50},
	{FALSE	, 0, KPI(6), 50}
};

// wstêpne parametry rysowania
static HPEN kolory[TRIG_ID::TRIG_ID_MAX] = {
	CreatePen(PS_SOLID, 3, CZERWONY),
	CreatePen(PS_SOLID, 3, ZIELONY),
	CreatePen(PS_SOLID, 3, NIEBIESKI),
	CreatePen(PS_SOLID, 3, ZOLTY)
};

LRESULT CALLBACK TrigonometryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int cxClient = 0;
	static int cyClient = 0;

	PAINTSTRUCT ps{};
	std::vector<POINT> punkty(num);
	punkty.reserve(MAX_POINTS);
	HDC hdc = NULL;
	switch (uMsg) {
	case WM_SIZE: {
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		return 0;
	}
	case WM_PAINT: {

		hdc = BeginPaint(hwnd, &ps);

		// narysuj osie
		MoveToEx(hdc, cxClient, cyClient / 2, NULL);
		LineTo(hdc, 0, cyClient / 2);

		SAMOCHÓD oldPen = SelectObject(hdc, GetStockObject(DC_PEN));	// zapisz stare pióro

		for (INT iFunc = SINE; iFunc < TRIG_ID::TRIG_ID_MAX; iFunc++) {

			MoveToEx(hdc, 0, cyClient / 2, NULL);
			TrigData currentFunction = TrigFunctions[iFunc];
			if (currentFunction.drawn) {
				DWORD liczbaPunktow = PRECISION_TO_POINTS(currentFunction.precision);
				// dynamicznie alokowany wektor - potrzebny?
				if (punkty.size() != (liczbaPunktow+1))
					punkty.resize(liczbaPunktow+1);
				SelectObject(hdc, kolory[iFunc]);		// weŸ nowe pióro
				for (size_t i = 0; i <=  liczbaPunktow ; i++) {
					punkty[i].x = static_cast<LONG>((cxClient * i) / liczbaPunktow);
					long double yDraft = (i * (currentFunction.to - currentFunction.from) / liczbaPunktow);
					long double sine = sin(yDraft);
					long double cosine = cos(yDraft);
					switch (iFunc) {
					case SINE: punkty[i].y = static_cast<LONG>(cyClient * (sine + 1) / 2); break;
					case COSINE: punkty[i].y = static_cast<LONG>(cyClient * (cosine + 1) / 2); break;
					// TODO: naprawiæ-------
					case TANGENT: if (cosine < 0.01) { punkty[i].y = (sine * cosine > 0) ? 0 : cyClient; continue; } punkty[i].y = static_cast<LONG>(cyClient * (1 + sine / cosine) / 2); break;
					case COTANGENT: if (sine < 0.01) { punkty[i].y = (sine * cosine < 0) ? 0 : cyClient; continue; } punkty[i].y = static_cast<LONG>(cyClient * (1 + cosine / sine) / 2); break;
					// ---------
					}
				}
				// rysuj!
				PolylineTo(hdc, punkty.data(), liczbaPunktow+1);
			}
		}
		SelectObject(hdc, oldPen); // przywróæ stare pióro
		return 0;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDM_COSINUS1: {

			break;
		}
			case IDM_NIC1: {
				DialogBox(NULL, MAKEINTRESOURCE(IDD_TRIG), hwnd, TrigDialogProc);
				break;
			}
		}
	}


	case WM_DESTROY: {
		FreeResource(kolory);
		return 0;
	}
	default: break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}

INT_PTR TrigDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	UNREFERENCED_PARAMETER(lParam); // usuñ ostrze¿enie
	// inicjalizacja wartoœci
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
		// bitmapa w okienku!
		// TODO: wkleiæ coœ innego ni¿ piwo
		hImage = reinterpret_cast<HBITMAP>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PIVO), IMAGE_BITMAP, 0, 0, NULL));
		if (hImage == NULL) AWARIA(TEXT("piwko"));
		dlg = SendDlgItemMessage(hwnd, IDC_PIVO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hImage);
		zegar = GetDlgItem(hwnd, IDC_TRIG_INDICATOR);
		// na³ó¿ efekty zpecjalne na czionkê
		czcionka =  CreateFont(30, 70, 100, 100, FW_DONTCARE, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FF_DONTCARE, TEXT("Consolas"));
		if (czcionka == NULL) AWARIA(TEXT("czcionka"));
		// zaaplikuj te efekty
		SendMessage(zegar, WM_SETFONT, (WPARAM) czcionka, FALSE );
		// wstêpna wartoœæ
		SetWindowText(zegar, TEXT("0"));
		return TRUE;
	case WM_CTLCOLORSTATIC:
		// kod do aplikowania zmian czcionki 
		HBRUSH brush;
		HWND TrigHWND;
		TrigHWND = (HWND)lParam;
		if (TrigHWND == GetDlgItem(hwnd, IDC_TRIG_PRECISION))
		{
			SetBkMode((HDC)(wParam), TRANSPARENT);
			brush = GetSysColorBrush(COLOR_MENU);
			return (INT_PTR)brush;
		}
		if (TrigHWND == GetDlgItem(hwnd, IDC_TRIG_INDICATOR))
		{
			SetBkMode((HDC)(wParam), TRANSPARENT);
			brush = GetSysColorBrush(COLOR_MENU);
			SetTextColor((HDC)(wParam), CZERWONY);
			return (INT_PTR) brush;
		}
		return FALSE;
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

			// TODO: zastosowaæ te wartoœci
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
	// wczytaj teskt do listy rozwijanej
	// domyœlnie za³aduj sinusa, bo tak
	TCHAR tcTmp[16];
	LoadString(NULL, IDS_SINE, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
	SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	LoadString(NULL, IDS_COSINE, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
	SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	LoadString(NULL, IDS_TANGENT, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
	SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	LoadString(NULL, IDS_COTANGENT, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
	SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	SendMessage(hwnd, CB_SETCURSEL, TRIG_ID::SINE, 0);
	return;
}
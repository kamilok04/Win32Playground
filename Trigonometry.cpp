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
static HPEN hpColors[TRIG_ID::TRIG_ID_MAX] = {
	CreatePen(PS_SOLID, 3, RED),
	CreatePen(PS_SOLID, 3, GREEN),
	CreatePen(PS_SOLID, 3, BLUE),
	CreatePen(PS_SOLID, 3, YELLOW)
};

LRESULT CALLBACK TrigonometryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int cxClient = 0;
	static int cyClient = 0;

	PAINTSTRUCT ps{};
	std::vector<POINT> vPoints(num);
	vPoints.reserve(MAX_POINTS);
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

		AUTO hpOldPen = SelectObject(hdc, GetStockObject(DC_PEN));	// zapisz stare pióro

		for (INT iFunc = SINE; iFunc < TRIG_ID::TRIG_ID_MAX; iFunc++) {

			MoveToEx(hdc, 0, cyClient / 2, NULL);
			TrigData currentFunction = TrigFunctions[iFunc];
			if (currentFunction.drawn) {
				DWORD cPoints = PRECISION_TO_POINTS(currentFunction.precision);
				// dynamicznie alokowany wektor - potrzebny?
				if (vPoints.size() != cPoints+1)
					vPoints.resize(cPoints+1);
				SelectObject(hdc, hpColors[iFunc]);		// weŸ nowe pióro
				for (size_t i = 0; i <=  cPoints ; i++) {
					vPoints[i].x = static_cast<LONG>((cxClient * i) / cPoints);
					long double ldYDraft = (i * (currentFunction.to - currentFunction.from) / cPoints);
					long double ldSine = sin(ldYDraft);
					long double ldCosine = cos(ldYDraft);
					switch (iFunc) {
					case SINE: vPoints[i].y = static_cast<LONG>(cyClient * (ldSine + 1) / 2); break;
					case COSINE: vPoints[i].y = static_cast<LONG>(cyClient * (ldCosine + 1) / 2); break;
					// TODO: naprawiæ-------
					case TANGENT: if (ldCosine < 0.01) { vPoints[i].y = (ldSine * ldCosine > 0) ? 0 : cyClient; continue; } vPoints[i].y = static_cast<LONG>(cyClient * (1 + ldSine / ldCosine) / 2); break;
					case COTANGENT: if (ldSine < 0.01) { vPoints[i].y = (ldSine * ldCosine < 0) ? 0 : cyClient; continue; } vPoints[i].y = static_cast<LONG>(cyClient * (1 + ldCosine / ldSine) / 2); break;
					// ---------
					}
				}
				// rysuj!
				PolylineTo(hdc, vPoints.data(), cPoints+1);
			}
		}
		SelectObject(hdc, hpOldPen); // przywróæ stare pióro
		return 0;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDM_COSINUS1: {

			break;
		}
			case IDM_NIC1: {
#ifdef _WIN64
				DialogBox(NULL, MAKEINTRESOURCE(IDD_TRIG), hwnd, TrigDialogProc);
#else 
				DialogBox(NULL, MAKEINTRESOURCE(IDD_TRIG), hwnd, (DLGPROC) TrigDialogProc);
#endif
				break;
			}
		}
	}


	case WM_DESTROY: {
		FreeResource(hpColors);
		return 0;
	}
	default: break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


INT_PTR TrigDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// inicjalizacja wartoœci
	HWND hList;
	static const HWND hTrackbar = GetDlgItem(hwnd, IDC_TRIG_PRECISION);
	static HWND hIndicator;
	LRESULT lDlg;
	HFONT hFont;
	static HBITMAP hImage;
	static DWORD dPos;
	switch (uMsg)
	{
	case WM_INITDIALOG:
		hList = GetDlgItem(hwnd, IDC_TRIG_FUNCTION_SELECT);
		InitTrigList(hList);
		// bitmapa w okienku!
		// TODO: wkleiæ coœ innego ni¿ piwo
		hImage = reinterpret_cast<HBITMAP>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PIVO), IMAGE_BITMAP, 0, 0, NULL));
		if (hImage == NULL) AWARIA(TEXT("piwko"));
		lDlg = SendDlgItemMessage(hwnd, IDC_PIVO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM) hImage);
		hIndicator = GetDlgItem(hwnd, IDC_TRIG_INDICATOR);
		// na³ó¿ efekty zpecjalne na czionkê
		hFont =  CreateFont(30, 70, 100, 100, FW_DONTCARE, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FF_DONTCARE, TEXT("Consolas"));
		if (hFont == NULL) AWARIA(TEXT("hFont"));
		// zaaplikuj te efekty
		SendMessage(hIndicator, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), FALSE );
		// wstêpna wartoœæ
		SetWindowText(hIndicator, TEXT("0"));
		return TRUE;
	case WM_CTLCOLORSTATIC:
		// kod do aplikowania zmian czcionki 
		HBRUSH hBrush;
		HWND hTrig;
		hTrig = reinterpret_cast<HWND>(lParam);
		if (hTrig == GetDlgItem(hwnd, IDC_TRIG_PRECISION))
		{
			SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
			hBrush = GetSysColorBrush(COLOR_MENU);
			return reinterpret_cast<INT_PTR>(hBrush);
		}
		if (hTrig == GetDlgItem(hwnd, IDC_TRIG_INDICATOR))
		{
			SetBkMode(reinterpret_cast<HDC>(wParam), TRANSPARENT);
			hBrush = GetSysColorBrush(COLOR_MENU);
			SetTextColor(reinterpret_cast<HDC>(wParam), RED);
			return (INT_PTR) (hBrush);
		}
		return FALSE;
	case WM_HSCROLL: // pasek
		dPos = static_cast<DWORD>(SendDlgItemMessage(hwnd, IDC_TRIG_PRECISION, TBM_GETPOS, 0, 0));
		TCHAR posStr[10];
		_itow_s(dPos, posStr, 10);
		posStr[9] = 0;
		SetWindowText(hIndicator, posStr);
		return TRUE;
	case WM_COMMAND:
		switch (LOWORD(wParam))
		{
		case IDOK:
			// odczytaj wartoœci
			TCHAR lpszBufor[20];
			TCHAR lpszBufor2[20];
			TCHAR lpszBufor3[20];
			GetDlgItemText(hwnd, IDC_TRIG_FUNCTION_SELECT, lpszBufor, 20);
			GetDlgItemText(hwnd, IDC_TRIG_RANGE_FROM, lpszBufor2, 20);
			GetDlgItemText(hwnd, IDC_TRIG_RANGE_TO, lpszBufor3, 20);
			TCHAR lpszAggregate[200];
			StringCchPrintf(lpszAggregate, 200, TEXT("Funkcja: %s\r\nZakres od: %s\r\nZakres do: %s\r\nPrecyzja: %d\r\n"), lpszBufor, lpszBufor2, lpszBufor3, dPos);
			MessageBox(hwnd, lpszAggregate, TEXT("wiedza tajemna"), MB_ICONINFORMATION | MB_OK);
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
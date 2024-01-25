#include "Trigonometry.h"
// problemy:
// - funkcje o mniejszym zakresie nie powinny siê rysowaæ w [0, cxClient], tylko w swoim zakresie
// - jak dobrze narysowaæ tangensa/cotangensa?
// - to s¹ funkcje OKRESOWE, naprawdê da siê to zoptymalizowaæ lepiej
// - zmiana koloru funkcji (jakaœ paletka jak z painta)

#define NUM_CYCLES 4

// wstêpne parametry funkcji
static struct TrigData TrigFunctions[4] = {
	{SINE,		TRUE	, KPI(-1)	, KPI(1), 20},
	{COSINE,	TRUE	, KPI(-1)	, KPI(1), 20},
	{TANGENT,	FALSE	, -3.1		, 3.1, 20},
	{COTANGENT, FALSE	, -3.1		, 3.1, 20}
};

// wstêpne parametry rysowania
static HPEN hpColors[TRIG_ID::TRIG_ID_MAX] = {
	CreatePen(PS_SOLID, 3, RED),
	CreatePen(PS_SOLID, 3, GREEN),
	CreatePen(PS_SOLID, 3, BLUE),
	CreatePen(PS_SOLID, 3, PURPLE)
};

// ?
STATIC STRUCT DrawParamPackage dpp = {};

LRESULT CALLBACK TrigonometryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static int cxClient = 0;
	static int cyClient = 0;
	STATIC HANDLE hThread = 0;
	STATIC PTP_WORK pWork = NULL;
	STATIC PTP_WORK pAnimate = NULL;
	STATIC HMENU hMenu = GetMenu(hwnd);
	STATIC BOOL animRunning = FALSE;

	switch (uMsg) {
	case WM_CREATE: {
		UINT uID = 0;
		for (auto& aFunc : TrigFunctions) {
			switch (aFunc.type) {
			case SINE: uID = IDM_SINUS1; break;
			case COSINE: uID = IDM_COSINUS1; break;
			case TANGENT: uID = IDM_TANGENS1; break;
			case COTANGENT: uID = IDM_COTANGENS1; break;
			}
			if (aFunc.drawn)
				CheckMenuItem(hMenu, uID, MF_CHECKED);
		}

		dpp.hwnd = hwnd;
		dpp.ClientRect = RECT{ 2, 1, 3, 7 };
		// utwórz pulê w¹tków (skromn¹, bo o pojemnoœci 1)
		// bêd¹ wykonywaæ zadania, a nie bêdê musia³ ci¹gle u¿ywaæ kosztownego
		// CreateThread()
		PTP_POOL pPool = CreateThreadpool(NULL);
		if (pPool == NULL) AWARIA(TEXT("w baseniku nie ma powietrza"));
		SetThreadpoolThreadMinimum(pPool, 2);
		SetThreadpoolThreadMaximum(pPool, 2);
		// zatrudnij sprz¹taczkê
		PTP_CLEANUP_GROUP pPoolCleanup = CreateThreadpoolCleanupGroup();
		TP_CALLBACK_ENVIRON tpcbe = {};
		InitializeThreadpoolEnvironment(&tpcbe);
		SetThreadpoolCallbackPool(&tpcbe, pPool);
		SetThreadpoolCallbackCleanupGroup(&tpcbe, pPoolCleanup, NULL);
		pWork = CreateThreadpoolWork((PTP_WORK_CALLBACK)DrawTrig, nullptr, &tpcbe);
		pAnimate = CreateThreadpoolWork((PTP_WORK_CALLBACK)AnimateTrig, nullptr, &tpcbe);
		if(pAnimate == NULL) AWARIA(TEXT("nie uda³o siê utworzyæ pracy"));

		// zabezpiecz siê na wypadek bezrobocia
		// szkoda, ¿e nie mogê tak poza C++
		if (pWork == NULL) AWARIA(TEXT("nie uda³o siê utworzyæ pracy"));
	}


	case WM_SIZE: {
		dpp.ClientRect.right = LOWORD(lParam);
		dpp.ClientRect.bottom = HIWORD(lParam);
		
		return 0;
	}
	case WM_PAINT: {
		
		SubmitThreadpoolWork(pWork);
		if (!animRunning) {
			SubmitThreadpoolWork(pAnimate);
			animRunning = TRUE;
		}
		return 0;
	}
	case WM_COMMAND: {
		// weŸ informacje o pozycji w menu i odpowiednio zareaguj 
		MENUITEMINFO mii = {};
		mii.cbSize = sizeof(MENUITEMINFO);
		// wa¿ne: ustaw maskê! inaczej ca³a struktura siê wyzeruje i bêdzie przykro
		mii.fMask |= MIIM_STATE;
		if (GetMenuItemInfo(hMenu, LOWORD(wParam), FALSE, &mii) == 0) AWARIA(TEXT("gdzie s¹ TE funkcje?"));
		CheckMenuItem(hMenu, LOWORD(wParam), (mii.fState & MFS_CHECKED) ? MF_UNCHECKED : MF_CHECKED);
		ToggleTrigDraw(LOWORD(wParam), hwnd);
		switch (LOWORD(wParam)) {
		case IDM_NIC1: {
#ifdef _WIN64
			DialogBox(NULL, MAKEINTRESOURCE(IDD_TRIG), hwnd, TrigDialogProc);
#else 
			DialogBox(NULL, MAKEINTRESOURCE(IDD_TRIG), hwnd, (DLGPROC)TrigDialogProc);
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
		lDlg = SendDlgItemMessage(hwnd, IDC_PIVO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImage);
		hIndicator = GetDlgItem(hwnd, IDC_TRIG_INDICATOR);
		// na³ó¿ efekty specjalne na czionkê
		hFont = CreateFont(30, 70, 100, 100, FW_DONTCARE, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FF_DONTCARE, TEXT("Consolas"));
		if (hFont == NULL) AWARIA(TEXT("hFont"));
		// zaaplikuj te efekty
		SendMessage(hIndicator, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), FALSE);
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
			return (INT_PTR)(hBrush);
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

/**
 * Inicjalizuje listê rozwijan¹ funkcji trygonometrycznych.
 * *
 * * \param HWND hwnd - uchwyt do okienka
 * * \return nie
 **/
VOID InitTrigList(HWND hwnd) {
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

/**
 * Prze³¹cza rysowanie funkcji po wybraniu odpowiedzniej opcji w menu.
 *
 * \param s³owo wID - identyfikator menu
 * \return nie
 */
STATIC VOID ToggleTrigDraw(WORD wID, HWND hwnd) {
	// brzydkie? brzydkie.
	// ale chyba nie mam lepszego sposobu na przemapowanie
	// ID pozycji w menu (>40000, niekoniecznie po kolei) ==> ID funkcji (<10)
	switch (wID) {
	case IDM_SINUS1:
		TrigFunctions[TRIG_ID::SINE].drawn = !TrigFunctions[TRIG_ID::SINE].drawn;
		break;
	case IDM_COSINUS1:
		TrigFunctions[TRIG_ID::COSINE].drawn = !TrigFunctions[TRIG_ID::COSINE].drawn;
		break;
	case IDM_TANGENS1:
		TrigFunctions[TRIG_ID::TANGENT].drawn = !TrigFunctions[TRIG_ID::TANGENT].drawn;
		break;
	case IDM_COTANGENS1:
		TrigFunctions[TRIG_ID::COTANGENT].drawn = !TrigFunctions[TRIG_ID::COTANGENT].drawn;
		break;

	}
	// odœwie¿ okno
	// UWAGA: rêczne wysy³anie WM_PAINT jest odradzane przez dokumentacjê
	// prawdopodobnie powoduje samozap³on
	RECT rect = {};
	GetClientRect(hwnd, &rect);
	InvalidateRect(hwnd, &rect, TRUE);
	UpdateWindow(hwnd);
	return;
}

DWORD WINAPI DrawTrig() {
	PAINTSTRUCT ps{};
	RECT rect{};
	std::vector<POINT> vPoints(num);
	vPoints.reserve(MAX_POINTS);
	HDC hdc = NULL;
	hdc = BeginPaint(dpp.hwnd, &ps);
	rect = {dpp.ClientRect.top, dpp.ClientRect.left - 100, dpp.ClientRect.right, dpp.ClientRect.bottom};
	SetBoundsRect(hdc, &rect, DCB_RESET | DCB_ENABLE);
	// narysuj osie
	MoveToEx(hdc, dpp.ClientRect.right, dpp.ClientRect.bottom / 2,NULL);
	LineTo(hdc, 0, dpp.ClientRect.bottom / 2);
	AUTO hpOldPen = SelectObject(hdc, GetStockObject(DC_PEN));	// zapisz stare pióro
	FillRect(hdc, &(dpp.ClientRect), (HBRUSH)GetStockObject(WHITE_BRUSH));
	for (auto& aFunc : TrigFunctions) {
		for (LONG iCycle = 0; iCycle < NUM_CYCLES; iCycle++) {
			// MoveToEx(hdc, iCycle * cxClient / NUM_CYCLES, dpp->ClientRect.bottom / 2, NULL);
			if (aFunc.drawn) {
				DWORD cPoints = PRECISION_TO_POINTS(aFunc.precision);
				STATIC DOUBLE dImprecision;
				vPoints.reserve(sizeof(POINT) * cPoints + 1);
				SelectObject(hdc, hpColors[aFunc.type]);		// weŸ nowe pióro
				for (size_t register i = 0; i <= cPoints; i++) {
					SIZE_T iOffset = dpp.ClientRect.right / NUM_CYCLES / cPoints;
					dImprecision += static_cast<DOUBLE>((DOUBLE)dpp.ClientRect.right / (DOUBLE)NUM_CYCLES / (DOUBLE)cPoints) - iOffset;
					if (dImprecision >= 1) {
						iOffset++;
						dImprecision--;
					}
					vPoints[i].x = static_cast<LONG>(i ? (vPoints[i - 1].x + iOffset) : (iCycle * dpp.ClientRect.right / NUM_CYCLES));
					if (iCycle == 0) {
						long double ldYDraft = (i * abs(aFunc.to - aFunc.from) / cPoints);
						long double ldSine = sin(ldYDraft);
						long double ldCosine = cos(ldYDraft);
						switch (aFunc.type) {
						case SINE: vPoints[i].y = static_cast<LONG>(i ? (dpp.ClientRect.bottom * (1 - ldSine) / 2) : dpp.ClientRect.bottom / 2); break;
						case COSINE: vPoints[i].y = static_cast<LONG>(i ? (dpp.ClientRect.bottom * (1 - ldCosine) / 2) : 0); break;

							// TODO: naprawiæ-------
						case TANGENT: vPoints[i].y = static_cast<LONG>(dpp.ClientRect.bottom * (1 + ldSine / ldCosine) / 2); break;
						case COTANGENT: vPoints[i].y = static_cast<LONG>(dpp.ClientRect.bottom * (1 + ldCosine / ldSine) / 2); break;
							// ---------
						}
					}
				}
				// rysuj!
				PolylineTo(hdc, vPoints.data(), cPoints);
			}

		}
		// cofnij siê
		MoveToEx(hdc, 0, dpp.ClientRect.bottom / 2, NULL);
	}
	SelectObject(hdc, hpOldPen); // przywróæ stare pióro}
	EndPaint(dpp.hwnd, &ps);
	return 0;
}


VOID AnimateTrig(){
	while (1) {
		HDC hdc = GetDC(dpp.hwnd);
		HBITMAP hBm = CaptureScreenPart(hdc,dpp.ClientRect.right- 10, 0, 4, dpp.ClientRect.bottom - dpp.ClientRect.top);
		ScrollWindowEx(dpp.hwnd, 1, 0, NULL, NULL, NULL, NULL, SW_INVALIDATE);
		DrawScreenPart(hdc, 0, 0, hBm);
		DeleteObject(hBm);
		ReleaseDC(dpp.hwnd, hdc);
		Sleep(1);

	}
}


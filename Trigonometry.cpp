#include "Trigonometry.h"
// problemy:
// - funkcje o mniejszym zakresie nie powinny siê rysowaæ w [0, cxClient], tylko w swoim zakresie
// - jak dobrze narysowaæ tangensa/cotangensa?
// - to s¹ funkcje OKRESOWE, naprawdê da siê to zoptymalizowaæ lepiej
// - zmiana koloru funkcji (jakaœ paletka jak z painta)

#define NUM_CYCLES 4

// wstêpne parametry funkcji
STATIC STRUCT TrigData TrigFunctions[4] = {
	{SINE,		TRUE	, KPI(-1)	, KPI(1), 20},
	{COSINE,	TRUE	, KPI(-1)	, KPI(1), 20},
	{TANGENT,	FALSE	, KPI(-0.8)	, KPI(0.8), 40},
	{COTANGENT, FALSE	, KPI(-1)	, KPI(1), 20}
};

// wstêpne parametry rysowania
STATIC COLORREF cTrigColors[TRIG_ID::TRIG_ID_MAX] = {
	RED, GREEN, BLUE, PURPLE
};

STATIC HPEN hpColors[TRIG_ID::TRIG_ID_MAX] = {
	CreatePen(PS_SOLID, 3, cTrigColors[TRIG_ID::SINE]),
	CreatePen(PS_SOLID, 3, cTrigColors[TRIG_ID::COSINE]),
	CreatePen(PS_SOLID, 3, cTrigColors[TRIG_ID::TANGENT]),
	CreatePen(PS_SOLID, 3, cTrigColors[TRIG_ID::COTANGENT])
};

// Zmienne globalne --------------------
STATIC STRUCT DrawParamPackage dpp = {};
STATIC INT cAnimSpeed = 2;
STATIC INT cAnimAccel = 0;
STATIC INT cxShift = 0;
STATIC PTP_POOL pPool = NULL;
STATIC PTP_WORK pWork = NULL;
STATIC PTP_WORK pAnimate = NULL;
STATIC PTP_CLEANUP_GROUP pCleanup = NULL;
STATIC HWND hSpeedometer = NULL;

// -------------------------------------

LRESULT CALLBACK TrigonometryWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	STATIC INT cxClient = 0;
	STATIC INT cyClient = 0;
	STATIC HANDLE hThread = 0;
	STATIC HMENU hMenu = GetMenu(hwnd);
	STATIC BOOL animRunning = FALSE;
	STATIC HWND hAccelerometer = NULL;

	switch (uMsg) {
	case WM_CREATE: {
		HRESULT hr = S_OK;
		
		// a do niego dorób dwa suwaki
		hSpeedometer = CreateWindowEx(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE, 200, 0, 200, 50, hwnd, NULL, GetInstanceModule(NULL), NULL);
		if (hSpeedometer == NULL) AWARIA(TEXT("nie uda³o siê utworzyæ suwaka"));
		SendMessage(hSpeedometer, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));
		//	SendMessage(hSpeedometer, TBM_SETPAGESIZE, 0, 1);
		
		hAccelerometer = CreateWindowEx(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE, 650, 0, 200, 50, hwnd, NULL, GetInstanceModule(NULL), NULL);
		if (hAccelerometer == NULL) AWARIA(TEXT("nie uda³o siê utworzyæ suwaka"));
		SendMessage(hAccelerometer, TBM_SETRANGE, TRUE, MAKELONG(-5, 5));
		//	SendMessage(hAccelerometer, TBM_SETPAGESIZE, 0, 1);
		hr = CreateRebar(hwnd, hSpeedometer, hAccelerometer);
		if (FAILED(hr)) AWARIA(TEXT("nie uda³o siê utworzyæ rebara"));
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
		GetClientRect(dpp.hwnd, &dpp.ClientRect);
		if (pWork == NULL) hr = PrepareThreadPool(2, pPool);
		if (FAILED(hr)) AWARIA(TEXT("nie uda³o siê utworzyæ puli w¹tków"));
		break;
	}
	case WM_HSCROLL: {
		HWND hParam = (HWND)lParam;
		if (hParam == hSpeedometer) {
			cAnimSpeed = SendMessage(hParam, TBM_GETPOS, 0, 0);
			break;
		}
		if (hParam == hAccelerometer) {
			cAnimAccel = SendMessage(hParam, TBM_GETPOS, 0, 0);
			break;
		}
		break;
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

		// SetForegroundWindow(GetParent(hwnd));
		return 0;
	}
	default: break;
	}
	return DefWindowProc(hwnd, uMsg, wParam, lParam);
}


INT_PTR TrigDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	// inicjalizacja wartoœci
	STATIC HWND hList = NULL;
	STATIC HWND hIndicator;
	LRESULT lDlg;

	HFONT hFont;
	STATIC HBITMAP hImage;
	STATIC DWORD dPos;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
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
		DWORD dPos = TrigFunctions[ComboBox_GetCurSel(hList)].precision;
		SendMessage(GetDlgItem(hwnd, IDC_TRIG_PRECISION),  TBM_SETPOS, TRUE, dPos);
		TCHAR tBuffer[3] = {};
		_itow_s(dPos, tBuffer, 10);
		SetWindowText(hIndicator, tBuffer);
		return TRUE;
	}
	case WM_CTLCOLORSTATIC:
	{
		DWORD dCtrlId = GetDlgCtrlID((HWND)lParam);
		if (dCtrlId == IDC_TRIG_COLOR_BOX) {
			COLORREF crColor = cTrigColors[ComboBox_GetCurSel(hList)];
			HBRUSH hbr = CreateSolidBrush(crColor);
			SetBkColor((HDC)wParam, crColor);
			return reinterpret_cast<INT_PTR>(hbr);
		}
		// kod do aplikowania zmian czcionki
		{
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
		}
	}
	case WM_HSCROLL: { // pasek
		dPos = static_cast<DWORD>(SendDlgItemMessage(hwnd, IDC_TRIG_PRECISION, TBM_GETPOS, 0, 0));
		TCHAR posStr[10];
		_itow_s(dPos, posStr, 10);
		posStr[9] = 0;
		SetWindowText(hIndicator, posStr);
		return TRUE;
	}
	case WM_COMMAND: {
		switch (LOWORD(wParam)) {
		case IDC_TRIG_COLOR: {
			STATIC COLORREF acrCustomColors[16];
			STATIC CHOOSECOLOR cc = {};
			cc.lStructSize = sizeof(CHOOSECOLOR);
			cc.hwndOwner = hwnd;
			cc.hInstance = (HWND)(HINSTANCE)GetModuleHandle(NULL);
			cc.rgbResult = (COLORREF)ComboBox_GetCurSel(hList);
			cc.lpCustColors = (LPDWORD)acrCustomColors;
			cc.Flags = CC_ANYCOLOR | CC_FULLOPEN | CC_RGBINIT;
			BOOL bColor = ChooseColor(&cc);
			if (bColor == 0) {
				// coœ siê popsu³o przy dialogu do wybierania koloru
				TCHAR bufor[20] = {};
				_itow_s((INT)CommDlgExtendedError(), bufor, 10);
				AWARIA(bufor);
			}
			// nie ma awarii, przetwórz kolor
			UpdateFunctionDrawingParams(hList, cc.rgbResult);
			// odœwie¿ okienko z podgl¹dem
			HWND hBox = GetDlgItem(hwnd, IDC_TRIG_COLOR_BOX);
			SendMessage(hwnd, WM_CTLCOLORSTATIC, (WPARAM)GetDC(hBox), (LPARAM)hBox);
			break;
		}


		case IDOK: {
			// TODO: zastosowaæ te wartoœci
			// odœwie¿ rysownictwo 
			RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			// zastosuj precyzjê
			DWORD dPrecision = SendMessage(GetDlgItem(hwnd, IDC_TRIG_PRECISION), TBM_GETPOS, 0, 0);
			if(!dPrecision) dPrecision = 1; // ma³o ambitne, ale zapobiega dzieleniu przez 0 dalej w kodzie
			TrigFunctions[ComboBox_GetCurSel(hList)].precision = dPrecision;
			EndDialog(hwnd, IDOK);

			break;
		}
		case IDCANCEL: {
			EndDialog(hwnd, IDCANCEL);
			break;
		}
		}
	default: return FALSE;
	}
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
	if (TrigFunctions[TRIG_ID::SINE].drawn) {
		LoadString(NULL, IDS_SINE, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	}
	if (TrigFunctions[TRIG_ID::COSINE].drawn) {
		LoadString(NULL, IDS_COSINE, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	}
	if (TrigFunctions[TRIG_ID::TANGENT].drawn) {
		LoadString(NULL, IDS_TANGENT, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	}
	if (TrigFunctions[TRIG_ID::COTANGENT].drawn) {
		LoadString(NULL, IDS_COTANGENT, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hwnd, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
	}
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

/**
 * Tworzy wykresy funkcji trygonometrycznych i je rysuje.
 * Funkcja przydzielona do basenu, wiêc parametry s¹ w globalnych zmiennych.
 *
 * \return
 */
DWORD WINAPI DrawTrig() {
	PAINTSTRUCT ps{};
	RECT paddedRect = { dpp.ClientRect.left, dpp.ClientRect.top + 100,  dpp.ClientRect.right, dpp.ClientRect.bottom - 100 };
	LONG cClientY = GetRectDimension(&dpp.ClientRect, DIMENSIONS::Y);
	// amortyzuj w przypadku ma³ego okna
	if (cClientY > 200 && cClientY < 300) paddedRect = { paddedRect.left, paddedRect.top - 300 + cClientY, paddedRect.right, paddedRect.bottom + 300 - cClientY };
	if (cClientY < 200) paddedRect = dpp.ClientRect; // za ma³o miejsca na marginesy jest
	STATIC BOOL bInterrupt = FALSE;
	std::vector<POINT> vPoints(dpp.ClientRect.right / NUM_CYCLES);

	HDC hdc = NULL;
	hdc = BeginPaint(dpp.hwnd, &ps);

	// wyczyœæ, bardzo wa¿ne po zmianie koloru 
	FillRect(hdc, &(dpp.ClientRect), (HBRUSH)GetStockObject(WHITE_BRUSH));
	// narysuj oœ
	MoveToEx(hdc, dpp.ClientRect.right, dpp.ClientRect.bottom / 2, NULL);
	LineTo(hdc, 0, dpp.ClientRect.bottom / 2);
	AUTO hpOldPen = SelectObject(hdc, GetStockObject(DC_PEN));	// zapisz stare pióro
	for (auto& aFunc : TrigFunctions) {
		std::vector<POINT>::iterator iBegin = vPoints.begin();
		std::vector<POINT>::iterator iEnd = vPoints.end();
		BOOL bNegOverride = FALSE;
		for (LONG iCycle = 0; iCycle <= NUM_CYCLES; iCycle++) {
			// MoveToEx(hdc, iCycle * cxClient / NUM_CYCLES, dpp->ClientRect.bottom / 2, NULL);
			if (!aFunc.drawn) continue;
			DWORD cPoints = PRECISION_TO_POINTS(aFunc.precision);
			STATIC DOUBLE dImprecision;
			STATIC INT iStep = 0;
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
					bNegOverride = FALSE;
					LDOUBLE ldYDraft = (i * abs(aFunc.to - aFunc.from) / cPoints);
					LDOUBLE ldSine = sin(ldYDraft);
					LDOUBLE ldCosine = cos(ldYDraft);

					switch (aFunc.type) {
					case SINE:
						//vPoints[i].y = static_cast<LONG>(i ? paddedRect.top - dpp.ClientRect.top + GetRectDimension(&paddedRect, DIMENSIONS::Y) * (1 - ldSine) / 2 :
						//paddedRect.top - dpp.ClientRect.top + GetRectDimension(&paddedRect, DIMENSIONS::Y) / 2); break;
						vPoints[i].y = static_cast<LONG>(paddedRect.top - dpp.ClientRect.top + GetRectDimension(&paddedRect, DIMENSIONS::Y) * (1 - ldSine) / 2); break;
					case COSINE: {
						if (!i) {
							MoveToEx(hdc, 0, paddedRect.top, NULL);
							vPoints[i].y = paddedRect.top;
							continue;
						}
						vPoints[i].y = static_cast<LONG>(paddedRect.top - dpp.ClientRect.top + GetRectDimension(&paddedRect, DIMENSIONS::Y) * (1 - ldCosine) / 2); break;
					}

							   // TODO: naprawiæ-------

					case COTANGENT: {
						if (!i) {
							vPoints[i].y = dpp.ClientRect.bottom; +10;

							MoveToEx(hdc, 0, dpp.ClientRect.bottom, NULL);
							continue;
						}
						vPoints[i].y = static_cast<LONG>(GetRectDimension(&dpp.ClientRect, DIMENSIONS::Y) * (1 + ldCosine / ldSine) / 2);
						break;
					}

					case TANGENT: {

						vPoints[i].y = static_cast<LONG>(GetRectDimension(&dpp.ClientRect, DIMENSIONS::Y) * (1 - ldCosine / ldSine) / 2); break;
					}


								// ---------
					}
				}
			}
			// rysuj!

			if (!bNegOverride) PolylineTo(hdc, vPoints.data(), cPoints);
			else {
				std::vector<POINT> vSubst(iBegin, iEnd);
				PolylineTo(hdc, vSubst.data(), cPoints);
				MoveToEx(hdc, vPoints[cPoints].x, dpp.ClientRect.bottom, NULL);
				bNegOverride = FALSE;
			}
		}


		// cofnij siê
		MoveToEx(hdc, 0, dpp.ClientRect.bottom / 2, NULL);
	}
	SelectObject(hdc, hpOldPen); // przywróæ stare pióro}
	EndPaint(dpp.hwnd, &ps);
	return 0;
}

/**
 * Animuje trygonometriê, bez parametrów.
 * Praca nadana w¹tkowi z puli, parametry s¹ w globalnych zmiennych
 * TODO: jakoœ wprowadziæ nadanie parametrów, kreator prac to oferuje
 *
 * \param nie
 * \return nie
 */
VOID AnimateTrig() {
	// te wszystkie 50 w kodzie wynikaj¹ z rêcznie wpisanego offsetu pseudo-rebara
	// todo: naprawiæ
	constexpr INT RebarOffset = 50;
	HDC hdc = GetDC(dpp.hwnd);
	DOPOKI_AUTORA_NIE_BEDZIE_STAC_NA_PIWO{
		STATIC INT cAccelAccum = 0;
		STATIC PAINTSTRUCT ps = {};
		HBITMAP hBm = NULL;
		if (cAnimSpeed > 0)
			hBm = CaptureScreenPart(hdc, dpp.ClientRect.right - cAnimSpeed - 1,
				RebarOffset, cAnimSpeed, GetRectDimension(&dpp.ClientRect, DIMENSIONS::Y) - RebarOffset);
		else if (cAnimSpeed < 0)
			hBm = CaptureScreenPart(hdc, 0,
				RebarOffset, abs(cAnimSpeed), GetRectDimension(&dpp.ClientRect, DIMENSIONS::Y) - RebarOffset);
		if (cAnimSpeed == 0 && cAnimAccel != 0) {
			// wybacz mi
			goto cAccelAccumProcessing;

		};
		ScrollWindowEx(dpp.hwnd, cAnimSpeed, 0, NULL, NULL, NULL, NULL, NULL);
		if (cAnimSpeed > 0)
			DrawScreenPart(hdc, 0, RebarOffset, hBm);
		else DrawScreenPart(hdc, dpp.ClientRect.right - abs(cAnimSpeed), RebarOffset, hBm);
		cxShift += cAnimSpeed;
		if (cAnimAccel != 0) {
			cAccelAccumProcessing:
			cAccelAccum += cAnimAccel;
			// wyt³um przyspieszenie 
			if (abs(cAccelAccum) > DampeningFactor) {
				cAccelAccum = 0;
				cAnimSpeed += cAnimAccel;
				SendMessage(hSpeedometer, TBM_SETPOS, TRUE, cAnimSpeed);
			}
		}
		Sleep(3);
	}
}

HRESULT CreateRebar(HWND hwnd, HWND hBand1, HWND hBand2) {
	LRESULT lResult = S_OK;
	HWND hRebar = CreateWindowEx(WS_EX_TOOLWINDOW,
		REBARCLASSNAME,
		NULL,
		WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS |
		WS_CLIPCHILDREN |
		CCS_NODIVIDER | RBS_BANDBORDERS, 0, 0, 0, 0, hwnd, NULL, GetInstanceModule(NULL), NULL);
	if (hRebar == NULL) AWARIA(TEXT("nie uda³o siê utworzyæ rebara"));
	// u¿yj go 
	REBARBANDINFO rbbi = { sizeof(REBARBANDINFO_V3_SIZE) };
	rbbi.fMask = RBBIM_STYLE | RBBIM_CHILD | RBBIM_CHILDSIZE | RBBIM_SIZE;
	rbbi.fStyle = RBBS_CHILDEDGE | RBBS_GRIPPERALWAYS;
	RECT trackbarRect = {};
	GetClientRect(hBand1, &trackbarRect);

	rbbi.lpText = (LPWSTR)TEXT("");
	rbbi.cyChild = 200;
	rbbi.cxMinChild = trackbarRect.right - trackbarRect.left;
	rbbi.cyMinChild = 200;
	rbbi.cx = 0;
	HWND hText = CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Prêdkoœæ animacji (-100 - 100)"), WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 200, 50, hwnd, NULL, GetInstanceModule(NULL), NULL);
	if (hText == NULL) AWARIA(TEXT("nie uda³o siê utworzyæ tekstu"));
	rbbi.hwndChild = hText;
	lResult = SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);

	rbbi.hwndChild = hBand1;
	rbbi.cx = 200;
	lResult = SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);

	if (lResult == 0) AWARIA(TEXT("nie uda³o siê wstawiæ suwaka"));
	hText = CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Przyspieszenie animacji (-5 - 5)"), WS_CHILD | WS_VISIBLE | SS_CENTER, 400, 0, 250, 50, hwnd, NULL, GetInstanceModule(NULL), NULL);

	rbbi.hwndChild = hBand2;


	lResult = SendMessage(hRebar, RB_INSERTBAND, (WPARAM)-1, (LPARAM)&rbbi);
	if (lResult == 0) AWARIA(TEXT("nie uda³o siê wstawiæ suwaka"));

	return S_OK;
}

/**
 * utwórz pulê w¹tków, które bêd¹ wykonywaæ zadania
 * nie bêdê musia³ ci¹gle u¿ywaæ kosztownego CreateThread()
 *
 * \param cThreads - ile w¹tków mocium panie?
 * \return zwyczajowy
 */
HRESULT PrepareThreadPool(UINT cThreads, PTP_POOL pPool) {
	pPool = CreateThreadpool(NULL);

	SetThreadpoolThreadMinimum(pPool, cThreads);
	SetThreadpoolThreadMaximum(pPool, cThreads);
	// zatrudnij sprz¹taczkê
	pCleanup = CreateThreadpoolCleanupGroup();
	TP_CALLBACK_ENVIRON tpcbe = {};
	InitializeThreadpoolEnvironment(&tpcbe);
	SetThreadpoolCallbackPool(&tpcbe, pPool);
	SetThreadpoolCallbackCleanupGroup(&tpcbe, pCleanup, NULL);
	pWork = CreateThreadpoolWork((PTP_WORK_CALLBACK)DrawTrig, nullptr, &tpcbe);
	// zabezpiecz siê na wypadek bezrobocia
	// szkoda, ¿e nie mogê tak poza C++
	if (pWork == NULL) {
#ifdef DEBUG
		AWARIA(TEXT("nie uda³o siê utworzyæ pracy"));
#endif 
		return E_FAIL;
}
	pAnimate = CreateThreadpoolWork((PTP_WORK_CALLBACK)AnimateTrig, nullptr, &tpcbe);
	if (pAnimate == NULL) {
#ifdef DEBUG
		AWARIA(TEXT("nie uda³o siê utworzyæ pracy"));
#endif
		return E_FAIL;
	}
	return S_OK;
}

VOID UpdateFunctionDrawingParams(HWND hList, COLORREF crColor) {
	cTrigColors[ComboBox_GetCurSel(hList)] = crColor;
	DeleteObject(hpColors[ComboBox_GetCurSel(hList)]);
	hpColors[ComboBox_GetCurSel(hList)] = CreatePen(PS_SOLID, 3, crColor);
	RedrawWindow(GetParent(hList), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	SubmitThreadpoolWork(pWork);
	return;
}

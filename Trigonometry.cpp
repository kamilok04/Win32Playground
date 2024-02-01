#include "Trigonometry.h"
// problemy:
// - zrobione? - funkcje o mniejszym zakresie nie powinny si� rysowa� w [0, cxClient], tylko w swoim zakresie
// - jak dobrze narysowa� tangensa/cotangensa?
// - zrobione  - to s� funkcje OKRESOWE, naprawd� da si� to zoptymalizowa� lepiej
// - zrobione  - zmiana koloru funkcji (jaka� paletka jak z painta)

#define NUM_CYCLES 4

// wst�pne parametry funkcji
STATIC STRUCT TrigData TrigFunctions[TRIG_ID::TRIG_ID_MAX] = {
	{SINE,		TRUE	, KPI(-1)	, KPI(1), 20, 3},
	{COSINE,	TRUE	, KPI(-1)	, KPI(1), 20, 3}, 
	{TANGENT,	FALSE	, KPI(-1)	, KPI(1), 20, 3},
	{COTANGENT, FALSE	, KPI(-1)	, KPI(1), 20, 3},
	{DEMO1,		FALSE	, 0			, KPI(2), 40, 3, 3},
	{DEMO2,		FALSE	, 0			, KPI(2), 40, 3, 4}
};

// wst�pne parametry rysowania
STATIC COLORREF cTrigColors[TRIG_ID::TRIG_ID_MAX] = {
	RED, GREEN, BLUE, PURPLE, YELLOW, BLACK
};

STATIC HPEN hpColors[TRIG_ID::TRIG_ID_MAX] = {
	CreatePen(PS_SOLID, TrigFunctions[TRIG_ID::SINE].thickness, cTrigColors[TRIG_ID::SINE]),
	CreatePen(PS_SOLID, TrigFunctions[TRIG_ID::COSINE].thickness, cTrigColors[TRIG_ID::COSINE]),
	CreatePen(PS_SOLID, TrigFunctions[TRIG_ID::TANGENT].thickness, cTrigColors[TRIG_ID::TANGENT]),
	CreatePen(PS_SOLID, TrigFunctions[TRIG_ID::COTANGENT].thickness, cTrigColors[TRIG_ID::COTANGENT]),
	CreatePen(PS_SOLID, TrigFunctions[TRIG_ID::DEMO1].thickness, cTrigColors[TRIG_ID::DEMO1]),
	CreatePen(PS_SOLID, TrigFunctions[TRIG_ID::DEMO2].thickness, cTrigColors[TRIG_ID::DEMO2])
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
STATIC TRIG_ID tiSelected[TRIG_ID::TRIG_ID_MAX] = {};

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
		
		// a do niego dor�b dwa suwaki
		hSpeedometer = CreateWindowEx(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE, 200, 0, 200, 50, hwnd, NULL, GetInstanceModule(NULL), NULL);
		if (hSpeedometer == NULL) AWARIA(TEXT("nie uda�o si� utworzy� suwaka"));
		SendMessage(hSpeedometer, TBM_SETRANGE, TRUE, MAKELONG(-100, 100));

		//	SendMessage(hSpeedometer, TBM_SETPAGESIZE, 0, 1);
		HWND hText = CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Pr�dko�� animacji (-100 - 100)"), WS_CHILD | WS_VISIBLE | SS_CENTER, 0, 0, 200, 50, hwnd, NULL, GetInstanceModule(NULL), NULL);
		if (hText == NULL) AWARIA(TEXT("nie uda�o si� utworzy� tekstu"));
		hText = CreateWindowEx(NULL, TEXT("STATIC"), TEXT("Przyspieszenie animacji (-5 - 5)"), WS_CHILD | WS_VISIBLE | SS_CENTER, 400, 0, 250, 50, hwnd, NULL, GetInstanceModule(NULL), NULL);
		hAccelerometer = CreateWindowEx(0, TRACKBAR_CLASS, NULL, WS_CHILD | WS_VISIBLE, 650, 0, 200, 50, hwnd, NULL, GetInstanceModule(NULL), NULL);
		if (hAccelerometer == NULL) AWARIA(TEXT("nie uda�o si� utworzy� suwaka"));
		SendMessage(hAccelerometer, TBM_SETRANGE, TRUE, MAKELONG(-5, 5));
		//	SendMessage(hAccelerometer, TBM_SETPAGESIZE, 0, 1);

		UINT uID = 0;
		for (auto& aFunc : TrigFunctions) {
			switch (aFunc.type) {
			case SINE: uID = IDM_SINUS1; break;
			case COSINE: uID = IDM_COSINUS1; break;
			case TANGENT: uID = IDM_TANGENS1; break;
			case COTANGENT: uID = IDM_COTANGENS1; break;
			case DEMO1: uID = IDM_DEMO_1; break;
			case DEMO2: uID = IDM_DEMO_2; break;
			}
			if (aFunc.drawn)
				CheckMenuItem(hMenu, uID, MF_CHECKED);
		}
		// ustaw wymiary wst�pne dla rysowania funkcji
		dpp.hwnd = hwnd;
		GetClientRect(dpp.hwnd, &dpp.ClientRect);
		
		// napompuj basenik
		if (pWork == NULL) hr = PrepareThreadPool(2, pPool);
		if (FAILED(hr)) AWARIA(TEXT("nie uda�o si� utworzy� puli w�tk�w"));

		break;
	}
	case WM_HSCROLL: {
		HWND hParam = (HWND)lParam;
		if (hParam == hSpeedometer) {
			cAnimSpeed = (INT)SendMessage(hParam, TBM_GETPOS, 0, 0);
			break;
		}
		if (hParam == hAccelerometer) {
			cAnimAccel = (INT)SendMessage(hParam, TBM_GETPOS, 0, 0);
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
		if (LOWORD(wParam) == IDM_RESET_TRIG) {
			RedrawWindow(hwnd, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			break;
		}
		// we� informacje o pozycji w menu i odpowiednio zareaguj 
		MENUITEMINFO mii = {};
		if (LOWORD(wParam) != IDM_TRIG_RANGE) {
			mii.cbSize = sizeof(MENUITEMINFO);
			// wa�ne: ustaw mask�! inaczej ca�a struktura si� wyzeruje i b�dzie przykro
			mii.fMask |= MIIM_STATE;
			GetMenuItemInfo(hMenu, LOWORD(wParam), FALSE, &mii);
			CheckMenuItem(hMenu, LOWORD(wParam), (mii.fState & MFS_CHECKED) ? MF_UNCHECKED : MF_CHECKED);
			ToggleTrigDraw(LOWORD(wParam), hwnd);
		}
		switch (LOWORD(wParam)) {
		case IDM_TRIG_RANGE: {
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
	// inicjalizacja warto�cit
	STATIC HWND hList = NULL;
	STATIC HWND hIndicator;
	LRESULT lDlg;
	STATIC BOOL bAdult = 0;

	HFONT hFont;
	STATIC HBITMAP hImage;
	STATIC DWORD dPos;
	switch (uMsg)
	{
	case WM_INITDIALOG:
	{
		hList = GetDlgItem(hwnd, IDC_TRIG_FUNCTION_SELECT);
		InitTrigList(hList, tiSelected);
		// bitmapa w okienku!
		// TODO: wklei� co� innego ni� piwo
		if (bAdult < 6) {
			if ((bAdult = MessageBox(NULL, L"Czy jeste� doros�y?", L"Pytanie", MB_YESNO | MB_ICONQUESTION)) == IDYES) {
				hImage = reinterpret_cast<HBITMAP>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_PIVO), IMAGE_BITMAP, 0, 0, NULL));
			}
			else {
				hImage = reinterpret_cast<HBITMAP>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDB_KOTEK), IMAGE_BITMAP, 0, 0, NULL));
			}
		}
		if (hImage == NULL) AWARIA(TEXT("piwko"));
		lDlg = SendDlgItemMessage(hwnd, IDC_PIVO, STM_SETIMAGE, IMAGE_BITMAP, (LPARAM)hImage);
		hIndicator = GetDlgItem(hwnd, IDC_TRIG_INDICATOR);
		// na�� efekty specjalne na czionk�
		/* CreateFont, zaskakuj�co, tworzy czcionk�
		* Przyjmuje:
		* - wysoko�� czcionki
		* - szeroko�� czcionki
		* - k�t nachylenia linijki
		* - k�t nachylenia znaku
		* - waga czcionki (FW_XXX)
		* - czy jest pochy�a
		* - czy jest podkre�lona
		* - czy jest przekre�lona
		* - kodowanie znak�w
		* - dok�adno�� (OUT_XXX)
		* - dok�adno�� przycinania (CLIP_XXX)
		* - jako�� wy�wietlania (DEFAULT_QUALITY, DEFAULT_PITCH, FF_DONTCARE)
		* - rodzaj czcionki (proporcjonalno��) i styl (FF_XXX)
		* - nazwa kroju czcionki (mo�e by� NULL)
		* Wszystkie parametry s� obowi�zkowe :)
		* 
		* Zwraca: HFONT albo NULL
		*/
		hFont = CreateFont(30, 70, 100, 100, FW_DONTCARE, TRUE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, NONANTIALIASED_QUALITY, FF_DONTCARE, TEXT("Consolas"));
		if (hFont == NULL) AWARIA(TEXT("hFont"));
		// zaaplikuj te efekty
		SendMessage(hIndicator, WM_SETFONT, reinterpret_cast<WPARAM>(hFont), FALSE);
		// wst�pna warto��
		DWORD dPos = TrigFunctions[ComboBox_GetCurSel(hList)].precision;
		SendMessage(GetDlgItem(hwnd, IDC_TRIG_PRECISION),  TBM_SETPOS, TRUE, dPos);
		TCHAR tBuffer[3] = {};
		_itow_s(dPos, tBuffer, 10);
		SetWindowText(hIndicator, tBuffer);
		// ustaw zakres suwaka z grubo�ci�
		SendMessage(GetDlgItem(hwnd, IDC_TRIG_THICKNESS), TBM_SETRANGE, TRUE, MAKELONG(1, 8));
		return TRUE;
	}
	case WM_CTLCOLORSTATIC:
	{
		DWORD dCtrlId = GetDlgCtrlID((HWND)lParam);
		if (dCtrlId == IDC_TRIG_COLOR_BOX) {
			TRIG_ID tiSel = tiSelected[ComboBox_GetCurSel(hList)];
			COLORREF crColor = cTrigColors[tiSel];
			HBRUSH hbr = CreateSolidBrush(crColor);
			SetBkColor((HDC)wParam,crColor);
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
		case IDC_TRIG_FUNCTION_SELECT: {
			switch (HIWORD(wParam)) {
			case CBN_SELCHANGE: {
				// od�wie� okienko z podgl�dem
				HWND hBox = GetDlgItem(hwnd, IDC_TRIG_COLOR_BOX);
				SendMessage(hwnd, WM_CTLCOLORSTATIC, (WPARAM)GetDC(hBox), (LPARAM)hBox);
				RedrawWindow(hBox, NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
				// zmie� suwak
				SendMessage(GetDlgItem(hwnd, IDC_TRIG_PRECISION), TBM_SETPOS, TRUE, CORRECTED_SELECTION.precision);
				TCHAR tBuffer[4]={};
				_itow_s(CORRECTED_SELECTION.precision, tBuffer, 10);
				SetWindowText(hIndicator, tBuffer);
				// zmie� grubo��
				SendMessage(GetDlgItem(hwnd, IDC_TRIG_THICKNESS), TBM_SETPOS, TRUE, CORRECTED_SELECTION.thickness);
				break;
			}
			}
			break;
		}
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
				// u�ytkownik m�g� anulowa�
				if (CommDlgExtendedError() == 0) break;
				// co� si� popsu�o przy dialogu do wybierania koloru
				TCHAR bufor[20] = {};
				_itow_s((INT)CommDlgExtendedError(), bufor, 10);
				AWARIA(bufor);
			}
			// nie ma awarii, przetw�rz kolor
			UpdateFunctionDrawingParams(hList, CORRECTED_SELECTION.precision, CORRECTED_SELECTION.thickness, cc.rgbResult);
			// od�wie� okienko z podgl�dem
			HWND hBox = GetDlgItem(hwnd, IDC_TRIG_COLOR_BOX);
			SendMessage(hwnd, WM_CTLCOLORSTATIC, (WPARAM)GetDC(hBox), (LPARAM)hBox);
			break;
		}


		case IDOK: {
			// TODO: zastosowa� te warto�ci
			// od�wie� rysownictwo 
			RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
			// zastosuj precyzj�
			DWORD dPrecision = (DWORD)SendMessage(GetDlgItem(hwnd, IDC_TRIG_PRECISION), TBM_GETPOS, 0, 0);
			if(!dPrecision) dPrecision = 1; // ma�o ambitne, ale zapobiega dzieleniu przez 0 dalej w kodzie
			// zastosuj grubo��
			DWORD dThickness = (DWORD)SendMessage(GetDlgItem(hwnd, IDC_TRIG_THICKNESS), TBM_GETPOS, 0, 0);
			UpdateFunctionDrawingParams(hList, dPrecision, dThickness, cTrigColors[tiSelected[ComboBox_GetCurSel(hList)]]);
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
 * Inicjalizuje list� rozwijan� funkcji trygonometrycznych.
 * \param hwnd - uchwyt do listy
 * \param tiSelected - tablica z wybranymi funkcjami
 * \return nic
 **/
STATIC VOID InitTrigList(HWND hList, TRIG_ID* tiSelected) {
	// wczytaj teskt do listy rozwijanej
	// domy�lnie za�aduj sinusa, bo tak
	UINT i = 0;
	TCHAR tcTmp[16];
	if (TrigFunctions[TRIG_ID::SINE].drawn) {
		LoadString(NULL, IDS_SINE, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hList, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
		tiSelected[i++] = TRIG_ID::SINE;
	}
	if (TrigFunctions[TRIG_ID::COSINE].drawn) {
		LoadString(NULL, IDS_COSINE, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hList, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
		tiSelected[i++] = TRIG_ID::COSINE;
	}
	if (TrigFunctions[TRIG_ID::TANGENT].drawn) {
		LoadString(NULL, IDS_TANGENT, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hList, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
		tiSelected[i++] = TRIG_ID::TANGENT;
	}
	if (TrigFunctions[TRIG_ID::COTANGENT].drawn) {
		LoadString(NULL, IDS_COTANGENT, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hList, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
		tiSelected[i++] = TRIG_ID::COTANGENT;
	}
	if (TrigFunctions[TRIG_ID::DEMO1].drawn) {
		LoadString(NULL, IDS_DEMO1, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hList, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
		tiSelected[i++] = TRIG_ID::DEMO1;
		
	}
	if (TrigFunctions[TRIG_ID::DEMO2].drawn) {
		LoadString(NULL, IDS_DEMO2, tcTmp, sizeof(tcTmp) / sizeof(TCHAR));
		SendMessage(hList, CB_ADDSTRING, NULL, reinterpret_cast<LPARAM>(tcTmp));
		tiSelected[i++] = TRIG_ID::DEMO2;
	}
	ComboBox_SetCurSel(hList, TRIG_ID::SINE);
	return;
}

/**
 * Prze��cza rysowanie funkcji po wybraniu odpowiedzniej opcji w menu.
 *
 * \param s�owo wID - identyfikator menu
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
	case IDM_DEMO_1:
		TrigFunctions[TRIG_ID::DEMO1].drawn = !TrigFunctions[TRIG_ID::DEMO1].drawn;
		break;
	case IDM_DEMO_2:
		TrigFunctions[TRIG_ID::DEMO2].drawn = !TrigFunctions[TRIG_ID::DEMO2].drawn;
		break;

	}
	// od�wie� okno
	// UWAGA: r�czne wysy�anie WM_PAINT jest odradzane przez dokumentacj�
	// prawdopodobnie powoduje samozap�on
	RECT rect = {};
	GetClientRect(hwnd, &rect);
	InvalidateRect(hwnd, &rect, TRUE);
	UpdateWindow(hwnd);
	return;
}

/**
 * Tworzy wykresy funkcji trygonometrycznych i je rysuje.
 * Funkcja przydzielona do basenu, wi�c parametry s� w globalnych zmiennych.
 *
 * \return
 */
DWORD WINAPI DrawTrig() {
	PAINTSTRUCT ps{};
	RECT paddedRect = { dpp.ClientRect.left, dpp.ClientRect.top + 100 + 50,  dpp.ClientRect.right, dpp.ClientRect.bottom - 100 };
	LONG cClientY = GetRectDimension(&dpp.ClientRect, DIMENSIONS::Y);
	// amortyzuj w przypadku ma�ego okna
	if (cClientY > 250 && cClientY < 350) paddedRect = { paddedRect.left, paddedRect.top - 350 + cClientY, paddedRect.right, paddedRect.bottom + 350 - cClientY };
	if (cClientY < 250) {
		paddedRect = dpp.ClientRect; // za ma�o miejsca na marginesy jest
		paddedRect.top += 50;
	}

	STATIC BOOL bInterrupt = FALSE;
	std::vector<POINT> vPoints(dpp.ClientRect.right / NUM_CYCLES);

	HDC hdc = NULL;
	hdc = BeginPaint(dpp.hwnd, &ps);

	// wyczy��, bardzo wa�ne po zmianie koloru 
	FillRect(hdc, &(dpp.ClientRect), (HBRUSH)GetStockObject(WHITE_BRUSH));
	// narysuj o�
	MoveToEx(hdc, dpp.ClientRect.right, paddedRect.top + GetRectDimension(&paddedRect, DIMENSIONS::Y) / 2, NULL);
	LineTo(hdc, 0, paddedRect.top + GetRectDimension(&paddedRect, DIMENSIONS::Y) / 2);
	AUTO hpOldPen = SelectObject(hdc, GetStockObject(DC_PEN));	// zapisz stare pi�ro
	for (auto& aFunc : TrigFunctions) {
		std::vector<POINT>::iterator iBegin = vPoints.begin();
		std::vector<POINT>::iterator iEnd = vPoints.end();
		BOOL bNegOverride = FALSE;
		for (LONG iCycle = 0; iCycle <= aFunc.intendedCycles; iCycle++) {
			// MoveToEx(hdc, iCycle * cxClient / NUM_CYCLES, dpp->ClientRect.bottom / 2, NULL);
			if (!aFunc.drawn) continue;
			DWORD cPoints = PRECISION_TO_POINTS(aFunc.precision);
			STATIC DOUBLE dImprecision;
			STATIC INT iStep = 0;
			SelectObject(hdc, hpColors[aFunc.type]);		// we� nowe pi�ro
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
					// nie dzia�aj�, jak powinny
					// problem polega na tym, �e obecna implementacja s�abo radzi sobie z dziurami w dziedzinie
					// oraz z przej�ciem +inf => -inf i odwrotnie w zakresie jednego Polyline 
					// potencjalne rozwi�zania: PolyPolyLine?
					case COTANGENT: {
						if (!i) {
							vPoints[i].y = dpp.ClientRect.bottom +10;

							MoveToEx(hdc, 0, dpp.ClientRect.bottom, NULL);
							continue;
						}
						vPoints[i].y = static_cast<LONG>(GetRectDimension(&dpp.ClientRect, DIMENSIONS::Y) * (1 + ldCosine / ldSine) / 2);
						break;
					}
					case TANGENT: {
						if (!i) {
							vPoints[i].y = dpp.ClientRect.top - 10;

							MoveToEx(hdc, 0, dpp.ClientRect.top, NULL);
							continue;
						}
						vPoints[i].y = static_cast<LONG>(GetRectDimension(&dpp.ClientRect, DIMENSIONS::Y) * (1 - ldCosine / ldSine) / 2); break;
					}	
					case DEMO1: {
						if (!i) {
							MoveToEx(hdc, 0, paddedRect.top, NULL);
							vPoints[i].y = paddedRect.top;
							continue;
						}
						vPoints[i].y = static_cast<LONG>(paddedRect.top - dpp.ClientRect.top + GetRectDimension(&paddedRect, DIMENSIONS::Y) * 
							((1 - (ldCosine + sin(2*ldYDraft) + cos(3*ldYDraft)) / 2) / 2)); break;
					}
					case DEMO2: {
						if (!i) {
							MoveToEx(hdc, 0, paddedRect.top, NULL);
							vPoints[i].y = paddedRect.top;
							continue;
						}
						vPoints[i].y = static_cast<LONG>(paddedRect.top - dpp.ClientRect.top + 
							GetRectDimension(&paddedRect, DIMENSIONS::Y) *
							((1 - (ldSine + cos(2*ldYDraft) - cos(3*ldYDraft) - cos(4*ldYDraft) -
								sin(7*ldYDraft) + sin(9*ldYDraft*ldYDraft)) / 2) / 4)); break;
					}
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


		// cofnij si�
		MoveToEx(hdc, 0, dpp.ClientRect.bottom / 2, NULL);
	}
	SelectObject(hdc, hpOldPen); // przywr�� stare pi�ro}
	EndPaint(dpp.hwnd, &ps);
	return 0;
}

/**
 * Animuje trygonometri�, bez parametr�w.
 * Praca nadana w�tkowi z puli, parametry s� w globalnych zmiennych
 * TODO: jako� wprowadzi� nadanie parametr�w, kreator prac to oferuje
 *
 * \param nie
 * \return nie
 */
VOID AnimateTrig() {
	constexpr INT RebarOffset = 50;

	while(1){
		HDC hdc = GetDC(dpp.hwnd);
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
			// wyt�um przyspieszenie 
			if (abs(cAccelAccum) > DampeningFactor) {
				cAccelAccum = 0;
				cAnimSpeed += cAnimAccel;
				SendMessage(hSpeedometer, TBM_SETPOS, TRUE, cAnimSpeed);
			}
		}
		ReleaseDC(dpp.hwnd, hdc);
		Sleep(3);
	}
}

/**
 * utw�rz pul� w�tk�w, kt�re b�d� wykonywa� zadania
 * nie b�d� musia� ci�gle u�ywa� kosztownego CreateThread()
 *
 * \param cThreads - ile w�tk�w mocium panie?
 * \return zwyczajowy
 */
HRESULT PrepareThreadPool(UINT cThreads, PTP_POOL pPool) {
	pPool = CreateThreadpool(NULL);

	SetThreadpoolThreadMinimum(pPool, cThreads);
	SetThreadpoolThreadMaximum(pPool, cThreads);
	// zatrudnij sprz�taczk�
	pCleanup = CreateThreadpoolCleanupGroup();
	TP_CALLBACK_ENVIRON tpcbe = {};
	InitializeThreadpoolEnvironment(&tpcbe);
	SetThreadpoolCallbackPool(&tpcbe, pPool);
	SetThreadpoolCallbackCleanupGroup(&tpcbe, pCleanup, NULL);
	pWork = CreateThreadpoolWork((PTP_WORK_CALLBACK)DrawTrig, nullptr, &tpcbe);
	// zabezpiecz si� na wypadek bezrobocia
	// szkoda, �e nie mog� tak poza C++
	if (pWork == NULL) {
#ifdef DEBUG
		AWARIA(TEXT("nie uda�o si� utworzy� pracy"));
#endif 
		return E_FAIL;
}
	pAnimate = CreateThreadpoolWork((PTP_WORK_CALLBACK)AnimateTrig, nullptr, &tpcbe);
	if (pAnimate == NULL) {
#ifdef DEBUG
		AWARIA(TEXT("nie uda�o si� utworzy� pracy"));
#endif
		return E_FAIL;
	}
	return S_OK;
}

VOID UpdateFunctionDrawingParams(HWND hList,DWORD dPrecision, DWORD dThickness, COLORREF crColor) {
	cTrigColors[tiSelected[ComboBox_GetCurSel(hList)]] = crColor;
	CORRECTED_SELECTION.precision = dPrecision;
	CORRECTED_SELECTION.thickness = dThickness;
	DeleteObject(hpColors[tiSelected[ComboBox_GetCurSel(hList)]]);
	hpColors[tiSelected[ComboBox_GetCurSel(hList)]] = CreatePen(PS_SOLID, dThickness, crColor);
	RedrawWindow(GetParent(hList), NULL, NULL, RDW_INVALIDATE | RDW_UPDATENOW);
	SubmitThreadpoolWork(pWork);
	return;
}

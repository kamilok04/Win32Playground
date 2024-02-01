#include "Tree.h"

LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {						// funkcja okna z drzewkiem
	static HWND ustawienia = NULL;
	PAINTSTRUCT ps{};
	RECT rect{};
	HDC hdc;
	switch (uMsg) {

	case WM_CREATE: {
#ifdef _WIN64
		CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TREE_SETTINGS), hwnd, TreeDialogProc); // dialog bez ramki
#else
		CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TREE_SETTINGS), hwnd, (DLGPROC)TreeDialogProc); //
#endif
		break;																						   // menu ustawieñ to dla systemu osobne okno
	}
	case WM_PAINT: {
		hdc = BeginPaint(hwnd, &ps);
		GetClientRect(hwnd, &rect);
		FillRect(hdc, &rect, reinterpret_cast<HBRUSH>(COLOR_MENU + 1));
		EndPaint(hwnd, &ps);
		break;
	}
	case WM_DESTROY:
		DestroyWindow(hwnd);
		break;
	default: return DefWindowProc(hwnd, uMsg, wParam, lParam);
	}
	return 0;
}

// Zmienne globalne: 

STATIC LVHITTESTINFO lvthi = {};
BOOL bUpdateTree = TRUE;
LEAF_VECTOR vTree;
WORK_MODE wmMode = WORK_MODE::PROBABILITY;
// ---------------------------

INT_PTR TreeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// TODO:
	// ZROBIONE	- edycja wartoœci w liœcie
	// ZROBIONE		- weryfikacja przysz³ych wejœæ 
	//		- jak lista siê zachowuje w ró¿nych DPI/rodzielczoœciach?
	//		- informuj o Ÿle znormalizowanym prawdopodobieñstwie
	LEAF_VECTOR vTree(DefaultTreeValues, DefaultTreeValues + cDefaultValues);
	switch (uMsg) {
	case WM_INITDIALOG: {
		// ustaw domyœlny tryb pracy 
		SendDlgItemMessage(hwnd, IDC_RADIO_PROB, BM_SETCHECK, BST_CHECKED, 1);
		// wczytaj listê
		HWND hList = GetDlgItem(hwnd, IDC_TREE_LEAVES);
		if (hList == NULL) AWARIA(TEXT("gdzie jest lista?"));
		// dodaj nag³ówki
		if (!AddListHeaders(hList)) AWARIA(TEXT("nag³ówki listy"));
		// za³aduj i wypisz domyœlne drzewo
		for (auto& q : vTree) { // iteracja po wektorze jest wygodna
			if (!InsertListItem(hList, q)) AWARIA(TEXT("³adowanie wartoœci domyœlnych"));
		}
		bUpdateTree = TRUE;
		return TRUE;
	}
	case WM_NOTIFY: {		// WM_NOTIFY jest wysy³ane, kiedy w oknie ListView (tym z list¹) zajdzie jakaœ zmiana

		switch (reinterpret_cast<LPNMHDR>(lParam)->code) { // w dokumentacji jest C-rzut
		case NM_DBLCLK: {
			LPNMITEMACTIVATE lpnitem = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
			// dokumentacja mówi, ¿e ta struktura zapewnia koordynaty pozycji w liœcie bez dodatkowych wywo³añ
			// ale tylko dla pierwszej kolumny
			UINT iItem = lpnitem->iItem;

			POINT point;
			GetCursorPos(&point);
			// zlikwiduj poprzednie okno, jeœli takie istnia³o
			STATIC HWND hTmpEdit = nullptr;
			if (hTmpEdit != nullptr) {
				TCHAR psBuffer[20];
				if (Edit_GetText(hTmpEdit, psBuffer, 20) != 0) {
					switch (lvthi.iSubItem) {
					case static_cast<INT>(COLUMNS::ID): {
						const TCHAR* psCompareString = COMPARE_STRING_INT;
						VerifyAndProcessInput<INT>(hTmpEdit, psBuffer, lvthi, vTree, 20, psCompareString);

						break;
					}
					case static_cast<INT>(COLUMNS::SYMBOL): {
						const TCHAR* psCompareString = COMPARE_STRING_STRING;
						VerifyAndProcessInput<PTCHAR>(hTmpEdit, psBuffer, lvthi, vTree, 20, psCompareString);
						break;
					}
					default: {
						const TCHAR* psCompareString = COMPARE_STRING_DOUBLE;
						VerifyAndProcessInput<DOUBLE>(hTmpEdit, psBuffer, lvthi, vTree, 20, psCompareString);
						break;
					}
					}

				}
				DestroyWindow(hTmpEdit);
			}
			// HitTest prosi o wspó³rzêdne wzglêdem lewego górnego rogu listy
			// konwersja
			ScreenToClient(GetDlgItem(hwnd, IDC_TREE_LEAVES), &point);
			lvthi.pt = point;
			if (ListView_SubItemHitTest(GetDlgItem(hwnd, IDC_TREE_LEAVES), &lvthi) == -1) break;
			// nie przerywam ca³ego programu, bo nie zawsze ca³e okno listy jest pe³ne pozycji
			// wtedy wyjœcie bez dzia³ania to po¿¹dany efekt
			iItem = lvthi.iItem;

			HWND hList = GetDlgItem(hwnd, IDC_TREE_LEAVES);
			if (hList == NULL) AWARIA(TEXT("Gdzie jest lista?"));
			TCHAR psBuffer[20] = {};
			ListView_GetItemText(GetDlgItem(hwnd, IDC_TREE_LEAVES),
				iItem,
				lvthi.iSubItem,
				psBuffer,
				sizeof(psBuffer) / sizeof(psBuffer[0]));
			psBuffer[19] = 0; // dla pewnoœci
			// wszystko w porz¹dku
			// zapisz wspó³rzêdne



			// zezwól na edycjê
			// edycja pierwszej kolumny jest wbudowana...
			//if (lvthi.iSubItem == 0)
			//	ListView_EditLabel(hList, iItem);

			// ...ale innych nie 
			// weŸ wymiary i pozycjê klikniêtego elementu, na nim powstanie okno
			// Uwaga! Jednostki dialogu, piksele logiczne i piksele fizyczne to trzy ró¿ne rzeczy
			// Trzeba obchodziæ siê z tym ostro¿nie, inaczej bêdzie Ÿle
			RECT rect = {};
			ListView_GetSubItemRect(hList, iItem, lvthi.iSubItem, LVIR_BOUNDS, &rect);
			POINT tl = { rect.left, rect.top };
			POINT br = { rect.right,rect.bottom };

			// ustaw wysokoœæ pola tekstowego na wysokoœæ systemowej czcionki
			// (która mo¿e mieæ inny rozmiar ni¿ ta wyœwietlana w liœcie)

			STATIC TEXTMETRIC tm = {};
			STATIC UINT cFontY = 0;
			if (tm.tmHeight == 0) {
				HDC hdc = GetDC(hList);
				if (GetTextMetrics(hdc, &tm) == 0) AWARIA(TEXT("gdzie czcionka?"));
				cFontY = tm.tmHeight + tm.tmExternalLeading;
				DeleteDC(hdc);
			}

			// utwórz pole tekstowe

			hTmpEdit = CreateWindowEx(
				WS_EX_TOPMOST | WS_EX_CONTROLPARENT,
				WC_EDIT,
				NULL,
				WS_CHILD | WS_VISIBLE | ES_LEFT,
				tl.x,
				tl.y,
				br.x - tl.x,
				cFontY,
				hList,
				NULL,
				NULL,
				NULL
			);
			if (hTmpEdit == NULL) AWARIA(TEXT("Podmianka okna"));

			// problem:
			// okno tekstowe wysy³a powiadomienia, a rodzic (lista) ich nie mo¿e przyj¹æ
			// utwórz hak, który przechwyca wiadomoœci i na nie odpowiada
			// w normalnym oknie robi to WNDPROC lub DLGPROC
			if (SetWindowsHookEx(WH_GETMESSAGE, AuxiliaryEditProc, NULL, GetCurrentThreadId()) == NULL) AWARIA(TEXT("funkcja wirtualna"));

			// niech pole zawiera tekst, jaki jest w polu pod nim
			if (Edit_SetText(hTmpEdit, psBuffer) == 0) AWARIA(TEXT("Tekst okna tymczasowego"));

			// niech tekst bêdzie na pocz¹tek zaznaczony
			Edit_SetSel(hTmpEdit, 0, -1);
			SetFocus(hTmpEdit);

			break;

		}
		default: return FALSE;
		}
		break;
	}

	case WM_COMMAND: {

		//if (HIWORD(wParam) == EN_KILLFOCUS) {
		//	if (DestroyWindow(reinterpret_cast<HWND>(lParam)) == FALSE) AWARIA(TEXT("Okno jest nieœmiertelne"));
		//	UpdateWindow(hwnd);

		//	break;
		//}
		HWND hList = GetDlgItem(hwnd, IDC_TREE_LEAVES);
		BOOL bListVisible = IsWindowVisible(hList);

		// podmieñ okna
		switch (LOWORD(wParam)) {

		case IDC_RADIO_PROB: {
			wmMode = WORK_MODE::PROBABILITY;
			if (!bListVisible) {
				ShowWindow(hList, SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, IDC_TREE_AUTO_LABEL), SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, IDC_TREE_AUTO_EDIT), SW_HIDE);
			}

			break;
		}
		case IDC_RADIO_AUTO: {
			wmMode = WORK_MODE::WORK_AUTO;
			if (bListVisible) {
				ShowWindow(hList, SW_HIDE);
				ShowWindow(GetDlgItem(hwnd, IDC_TREE_AUTO_LABEL), SW_SHOW);
				ShowWindow(GetDlgItem(hwnd, IDC_TREE_AUTO_EDIT), SW_SHOW);
			}

			break;
		}
		case IDOK: {
			// dialog ma zostaæ!
			// tu bêdzie wywo³ywane drzewo
			// pozb¹dŸ siê starego 
			RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
			switch (wmMode) {
			case WORK_MODE::PROBABILITY: {
					// sprawdŸ, czy prawdopodobieñstwa s¹ znormalizowane
					// jeœli nie, to wyœwietl ostrze¿enie
					// jeœli tak, to wyœwietl drzewo
				vTree = CreateTreeFromList(hList);
				bUpdateTree = TRUE;
				GenerateTree(vTree, GetParent(hwnd), static_cast<UINT>(SendDlgItemMessage(hwnd, IDC_TREE_LEAVES, LVM_GETITEMCOUNT, NULL, NULL)));
				break;
			}
			case WORK_MODE::WORK_AUTO: {
				TCHAR psBuffer[200] = {}; // 200? idk
				Edit_GetText(GetDlgItem(hwnd, IDC_TREE_AUTO_EDIT), psBuffer, 200);
				LEAF_VECTOR lvRaw = ProcessAutoText(psBuffer);
				if (lvRaw.size() == 0){
					return 0;
				}
				bUpdateTree = TRUE;
				GenerateTree(lvRaw, GetParent(hwnd), static_cast<UINT>(lvRaw.size()));

			}
			}
			return 0;
		}

		case IDCANCEL:
			// zniszcz rodzica
			SendMessage(GetParent(hwnd), WM_DESTROY, NULL, NULL);
			SetForegroundWindow(GetGrandParent(hwnd));
			return TRUE;
		}
		break;
	}
	default: return FALSE;
	}
	return TRUE;
}
BOOL AddListHeaders(HWND hList) {
	WCHAR psBuffer[50] = {};
	LVCOLUMN lvc = {};
	CONSTEXPR INT iCol = static_cast<CONST INT>(COLUMNS::COLUMNS_CNT);
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
	lvc.pszText = psBuffer;
	for (INT i = 0; i < iCol; i++) {
		switch (i) {
		case 0: lvc.cx = 40; break; // te wartoœci s¹ arbitralne
			// TODO: doczytaæ o zasadach projektowania aplikacji
		case 1: lvc.cx = 80; break;
		case 2: lvc.cx = 190; break;
		}
		// ta funkcja zak³ada, ¿e stringi odpowiedzialne za nag³ówki maj¹ ID po kolei od IDS_ORDER w górê
		// TODO: zrobiæ to lepiej
		if (LoadString(GetModuleHandle(NULL), IDS_ORDER + i, psBuffer, sizeof(psBuffer) / sizeof(psBuffer[0])) == NULL) AWARIA(TEXT("³adowanie stringów"));
		if (ListView_InsertColumn(hList, iCol, &lvc) == ERROR_UNHANDLED_ERROR) return FALSE;
	}
	return TRUE;
}

static BOOL InsertListItem(HWND hList, struct LEAF& LEAF) {
	TSTRING(10) lpszID = {};
	swprintf_s(lpszID.data(), 10, COMPARE_STRING_UINT, LEAF.ID);
	LVITEM lvI = {};
	lvI.pszText = lpszID.data();
	lvI.mask = LVIF_TEXT;
	lvI.iSubItem = static_cast<INT>(COLUMNS::ID);
	lvI.iItem = 0;
	// kod b³êdu w funkcjach ListView_* to zazwyczaj -1
	// -1 jest zdefiniowane jako ERROR_UNHANDLED_ERROR, wiêc u¿ywam tego
	// rzuca siê w oczy
	if (ListView_InsertItem(hList, &lvI) == ERROR_UNHANDLED_ERROR) return FALSE;
	lvI.iSubItem = static_cast<INT>(COLUMNS::SYMBOL);
	lvI.pszText = (LPWSTR)LEAF.tSymbol.c_str();
	if (ListView_SetItem(hList, &lvI) == ERROR_UNHANDLED_ERROR) return FALSE;
	lvI.iSubItem = static_cast<INT>(COLUMNS::VALUE);
	lvI.pszText = (LPWSTR)LEAF.tFPValue.c_str();
	if (ListView_SetItem(hList, &lvI) == ERROR_UNHANDLED_ERROR) return FALSE;
	return TRUE;
}

VOID GenerateTree(LEAF_VECTOR& vLeaves, HWND hwnd, UINT cLeaves) {

	STATIC LEAF lOptimizedTree;
	if (bUpdateTree) lOptimizedTree = PrepareTree(vLeaves);

	// TODO: przygotuj okno-dziecko, w nim bêdzie drzewo
	// okno-dziecko jest potrzebne w razie przewijania ekranu

	TREEPROPERTIES tp=  CreateTreeProperties(lOptimizedTree, cLeaves, hwnd);
	if (cLeaves == 1) {
		RECT rDlg = {};
		GetClientRect(GetFirstChild(hwnd), &rDlg);
		LONG cxDlg = rDlg.right - rDlg.left;
		LONG cyDlg = rDlg.bottom - rDlg.top;

		POINT pStart = { cxDlg / 2, cyDlg + 10};
		STATIC DRAWTEXTPARAMS dtParams = {};
		dtParams.cbSize = sizeof(DRAWTEXTPARAMS);
		dtParams.iLeftMargin = dtParams.iRightMargin = 10;
		DrawLeaf(hwnd, &pStart, vLeaves[0], 0, TRUE, dtParams, tp, NULL, FALSE);
	} else
		DrawTree(lOptimizedTree, hwnd, tp);

	bUpdateTree = FALSE;
	return;
}


LRESULT CALLBACK AuxiliaryEditProc(INT code, WPARAM wParam, LPARAM lParam) {

	if (code < 0) return CallNextHookEx(NULL, code, wParam, lParam);
	MSG msg = *reinterpret_cast<LPMSG>(lParam);
	if (msg.message == WM_KEYDOWN) {
		// za³o¿enie: pole tekstowe ma jedn¹ linijkê wysokoœci
		HWND target = GetFocus();
		switch (msg.wParam) {

		case VK_RETURN: {

			TCHAR psBuffer[20];
			// obiekt, który nie jest "Edit Control", nie obs³u¿y tej wiadomoœci
			// leniwy sposób, ¿eby upewniæ siê, ¿e na pewno dzia³amy na polu tekstowym
			if (SendMessage(target, EM_SETREADONLY, true, NULL)) {
				Edit_GetText(target, psBuffer, 20);
				switch (lvthi.iSubItem) {
				case static_cast<INT>(COLUMNS::ID): {
					const TCHAR* psCompareString = COMPARE_STRING_INT;
					if (VerifyAndProcessInput<INT>(target, psBuffer, lvthi, vTree, 20, psCompareString)) {

						bUpdateTree = TRUE;
					}
					break;
				}
				case static_cast<INT>(COLUMNS::SYMBOL): {
					const TCHAR* psCompareString = COMPARE_STRING_STRING;
					if (VerifyAndProcessInput<PTCHAR>(target, psBuffer, lvthi, vTree, 20, psCompareString)) {
						bUpdateTree = TRUE;
					}
					break;
				}
				default: {
					const TCHAR* psCompareString = COMPARE_STRING_DOUBLE;
					if (VerifyAndProcessInput<DOUBLE>(target, psBuffer, lvthi, vTree, 20, psCompareString)) {
						bUpdateTree = TRUE;
					}
					break;
				}

				}
				DestroyWindow(target);
			}
			break;
		}
		case VK_ESCAPE: {
			// ESCAPE wychodzi z okna edycji
			if (SendMessage(target, EM_SETREADONLY, true, NULL)) {
				DestroyWindow(target);
			}
		}
		}
	}
	return CallNextHookEx(NULL, code, wParam, lParam);
}

BOOL HandleConflicts(ConflictsArray& bConflicts, HWND hDlg) {
	UINT iConflict = 0;
	BOOL bDrawWarning = FALSE;
	std::vector<std::vector<TCHAR>> vlpszConflictInfo;
	for (auto b : bConflicts)
		if (b)
			bDrawWarning = TRUE;
	STATIC HICON hIcon = NULL;
	STATIC HWND hWarn = GetDlgItem(hDlg, IDC_WARN_PLACEHOLDER);
	if (bDrawWarning) {
		if (hIcon == NULL) {
			hIcon = static_cast<HICON>(LoadImage(GetModuleHandle(NULL), MAKEINTRESOURCE(IDI_WARN), IMAGE_ICON, LR_DEFAULTSIZE, LR_DEFAULTSIZE, LR_SHARED));
			SendDlgItemMessage(hDlg, IDC_WARN_PLACEHOLDER, STM_SETIMAGE, IMAGE_ICON, (LPARAM)hIcon);
		}
#ifdef DEBUG
	if (hIcon == NULL) AWARIA(TEXT("gdzie ikona?"));
#endif // DEBUG
		ShowWindow(hWarn, SW_NORMAL);
	}
	else {
		ShowWindow(hWarn, SW_HIDE);
	}
	bUpdateTree = TRUE;
	return !bDrawWarning;
}

// nareszcie!
/* Algorytm tworzenia optymalnego drzewa

wymagania wstêpne:
	+ u¿ytkownik poda³ prawdopodobieñstwa, upewnij siê,
	  ¿e s¹ znormalizowane (suma == 1) robi to szablon VerifyAndProcessInput w Drzewko.h
	  ZMIANA: algorytm pozwala na narysowanie Ÿle znormalizowanych drzew, u¿ytkownik jest informowany o problemie.

0.	jeœli w liœcie jest jeden element, drzewo jest gotowe
1.	upewnij siê, ¿e liœcie s¹ rosn¹co posortowane
		prawdopodobieñstwem/czêstoœci¹
1b. jeœli nie s¹, to je posortuj
2.	weŸ dwa liœcie o najmniejszym prawdopodobieñstwie
3.	utwórz nowy liœæ, który jako dzieci ma liœcie wziête w 2.
		jego prawdopodobieñstwo jest równe
		sumie prawdopodobieñstw dzieci
4.	usuñ liœcie wziête w 2. z listy
5.	w³ó¿ liœæ otrzymany w 3. do listy
	wróæ do 0.

*/

/**
 * Funkcja optymalizuj¹ca drzewo
 *
 * \param vLeaves - wektor do liœci w stanie surowym
 * \return
 */
LEAF PrepareTree(LEAF_VECTOR& vLeaves) {
	SIZE_T iLEAF = 0;
	UINT uID = 100;
	/*
	wektor ma tak du¿y rozmiar, bo zmiana rozmiaru przenosi go w inne miejsce
	i uniewa¿nia wszystkie dojœcia, a tego nie chcemy
	ten STATIC to kluczowa czêœæ kodu, bo
	dziêki niemu to po prostu Dzia³a™
	*/
	STATIC LEAF_VECTOR vDone(4 * sizeof(vLeaves));
	vDone.clear();
	while (vLeaves.size() > 1) { // 0
		// TODO: podmieniæ algorytm na w³asny
		// dobrym pomys³em bêdzie smoothsort (?), bo pracowaæ bêdzie siê na prawie-posortowanym wektorze

		std::sort(vLeaves.begin(), vLeaves.end(), [](const LEAF lA, const LEAF lB) {
			return lA.FPValue < lB.FPValue; // 1b, krok 1 pominiêty - zawsze sortujê
			});
		// ustalanie wartoœci nowego liœcia
		// krok 2/3
		DOUBLE dNewProb = vLeaves[0].FPValue + vLeaves[1].FPValue;
		TCHAR lpszNewTFP[FPLength] = {};

		// sformatuj nowy tekst
		// przy za ma³ym buforze zostanie œciêty
		swprintf_s(lpszNewTFP, FPLength, L"%.2F + %.2F", vLeaves[0].FPValue, vLeaves[1].FPValue);
		LEAF lBiggerLEAF = {};
		lBiggerLEAF.ID = uID;
		lBiggerLEAF.FPValue = dNewProb;
		vDone.emplace(vDone.begin() + iLEAF, vLeaves[0]);
		vLeaves.erase(vLeaves.begin());
		vDone.emplace(vDone.begin() + iLEAF + 1, vLeaves[0]);
		vLeaves.erase(vLeaves.begin());
		lBiggerLEAF.LeftChild = &vDone[iLEAF];
		lBiggerLEAF.RightChild = &vDone[iLEAF + 1];
		lBiggerLEAF.tFPValue = lpszNewTFP;

		vLeaves.push_back(lBiggerLEAF);

		iLEAF += 2;
		uID++;
		// co przebieg zabieram dwa elementy, ale dodajê jeden
		// nic nie zostanie pominiête

	}
	return vLeaves[0];
}

/**
 * Funkcja do rysowania ju¿ zoptymalizowanego drzewa.
 *
 * \param lTree - drzewo do narysowania
 * \param hwnd  - uchwyt do okna, w którym bêdzie rysowane; to powinien byæ rodzic dialogu
 * \return nie zwraca
 */

 /* Uk³ad wspó³rzêdnych Win32
 *	 +-------------------------> x
 *   | (0,0)
 *   |
 *   |
 *   |
 *	 v
 *   y
 */

 /* ========================
 *  WYCOFANE Z U¯YTKU
 *  ========================
 *  Algorytm do chodzenia po drzewie
 *	1. Zainicjalizuj tabelê stanów, ka¿dy liœæ ma stan w zakresie {0, 1, 2} - na pocz¹tku 0 dla wszystkich
 *	2. Jeœli liœæ jest w stanie 0, prze³¹cz go na 1, narysuj i przejdŸ do jego lewego dziecka
 *	- powtarzaj krok 2, dopóki lewe dziecko nie bêdzie mia³o wartoœci NULL
 *	- kiedy lewe dziecko ma wartoœæ NULL:
 *	3. Cofaj siê, dopóki nie znajdziesz liœcia w stanie 1
 *	3a. Jeœli rodzic liœcia w stanie 2 to NULL, koniec
 *	4. Prze³¹cz liœæ na stan 2, przejdŸ do jego prawego dziecka i narysuj
 *	- wróæ do kroku 2
 *   =======================
 */

 /* Algorytm do chodzenia po drzewie - nie u¿ywa ju¿ stanów
 *	1. Zainicjalizuj pusty stos rodziców
 *	2. Ustaw siê na najstarszym liœciu drzewa (tutaj lTree)
 *	3. Dopóki istnieje, rysuj lewe dziecko liœcia; ka¿de narysowane dziecko wrzuæ na stos (powtarzaj)
 *	4. Dopóki jesteœ prawym dzieckiem, cofnij siê; usuñ ka¿de dziecko, z którego siê cofasz, ze stosu (powtarzaj)
 *	   Na pocz¹tku kroku 4. ZAWSZE bêdzie istnia³ co najmniej jeden rodzic
 *	4a. Jeœli skoñczy³ siê stos, koniec
 *	5. Jesteœ lewym dzieckiem, wiêc narysuj jedno prawe dziecko, przejdŸ do niego i wrzuæ je na stos
 *	6. Wróæ do kroku 3.
 */
VOID DrawTree(LEAF& lTree, HWND hwnd, TREEPROPERTIES& tp) {

	HDC hdc = GetDC(hwnd);
	// ustaw statyczne w³aœciwoœci tekstu
	STATIC DRAWTEXTPARAMS dtParams = {};
	dtParams.cbSize = sizeof(DRAWTEXTPARAMS);
	dtParams.iLeftMargin = dtParams.iRightMargin = 10;
	SetBkMode(hdc, TRANSPARENT);

	RECT rDlg = {};
	GetClientRect(GetFirstChild(hwnd), &rDlg);
	LONG cxDlg = rDlg.right - rDlg.left;
	LONG cyDlg = rDlg.bottom - rDlg.top;

	POINT pStart = { static_cast<LONG>(tp.cxTreeWidth / 2 - tp.cxInnerLeafWidth / 2),
		static_cast<LONG>(cyDlg + 20) };
	RECT rBubbleRect = CreateRect(pStart, tp.cxInnerLeafWidth, tp.cyTotalLeafHeight);
	Ellipse(hdc, rBubbleRect.left, rBubbleRect.top, rBubbleRect.right, rBubbleRect.bottom);
	DrawTextEx(hdc, (LPWSTR)lTree.tFPValue.c_str(), (INT) lTree.tFPValue.length(), &rBubbleRect, DT_VCENTER | DT_SINGLELINE, &dtParams);
	POINT pTarget = CenterDown(rBubbleRect);
	MoveToEx(hdc, pTarget.x, pTarget.y, NULL);

	// 1
	std::vector<LEAF> lvParents;
	// 2
	LEAF lCurrentLeaf = lTree;
	UINT iNestLevel = 0;
	BOOL bAmALeftChild = FALSE;
	do {
		// 3
		while (lCurrentLeaf.LeftChild != nullptr) {
			iNestLevel++;
			DrawLeaf(hwnd, &pTarget, *lCurrentLeaf.LeftChild, iNestLevel, FALSE, dtParams, tp, hdc, TRUE);
			lvParents.push_back(lCurrentLeaf);
			lCurrentLeaf = *lCurrentLeaf.LeftChild;
		}
		// 4
		do {
			// cofnij siê
			// jestem na œrodku górnej krawêdzi m³odszego liœcia
			// muszê przenieœæ siê na œrodek dolnej krawêdzi starszego
			// (cofn¹æ siê o prostok¹t linii)

			bAmALeftChild = lvParents.back().LeftChild->ID == lCurrentLeaf.ID;
			if (bAmALeftChild)
				pTarget.x += (tp.cxTreeWidth >> (iNestLevel + 1));
			else
				pTarget.x -= (tp.cxTreeWidth >> (iNestLevel + 1));
			pTarget.y -= RECT_HEIGHT + tp.cyTotalLeafHeight;
			MoveToEx(hdc, pTarget.x, pTarget.y, NULL);
			iNestLevel--;
			lCurrentLeaf = lvParents.back();
			lvParents.pop_back();
		} while (!bAmALeftChild && lvParents.size() != 0);
		// 4a
		if (iNestLevel == 0 && !bAmALeftChild) break;
		// 5
		if (lCurrentLeaf.RightChild != nullptr) {
			iNestLevel++;
			DrawLeaf(hwnd, &pTarget, *lCurrentLeaf.RightChild, iNestLevel, TRUE, dtParams, tp, hdc, TRUE);
			lvParents.push_back(lCurrentLeaf);
			lCurrentLeaf = *lCurrentLeaf.RightChild;
		}
		// 6
	} while (1);

	ReleaseDC(hwnd, hdc);
	return;
}

RECT CreateRect(POINT pTopLeft, INT width, INT height) {
	RECT rRect = { pTopLeft.x, pTopLeft.y, pTopLeft.x + width, pTopLeft.y + height };
	return rRect;
}RECT CreateRect(PPOINT pTopLeft, INT width, INT height) {
	RECT rRect = { pTopLeft->x, pTopLeft->y, pTopLeft->x + width, pTopLeft->y + height };
	return rRect;
}

POINT CenterDown(RECT r) {
	POINT p = { r.left + (r.right - r.left) / 2, r.bottom };
	return p;
}
POINT CenterDown(LPRECT r) {
	POINT p = { r->left + (r->right - r->left) / 2, r->bottom };
	return p;
}


/** Funkcja rysuj¹ca liœæ
 *
 * \param hwnd		- uchwyt do okna, w którym bêdzie rysowany; to powinien byæ rodzic dialogu
 * \param pStart	- punkt startowy, z którego bêdzie rysowany liœæ (w zamyœle œrodek dolnej krawêdzi prostok¹ta dymka rodzica)
 * \param lChild	- liœæ do narysowania (ju¿ w formie dziecka)
 * \param iIncomingLevel - poziom, na którym znajduje siê liœæ, który ma zostaæ narysowany
 * \param isLeft	- lewe czy prawe dziecko?
 * \param dtParams	- parametry rysowania tekstu
 * \param TreeProps - w³aœciwoœci drzewa
 * \param hdc		- kontekst urz¹dzenia, opcjonalne
 * \return nie
 * \throw wo³aj¹cy powinien wywo³aæ ReleaseDC() po u¿yciu tej funkcji
 *		Funkcja przyjmuje œrodek dolnej krawêdzi starszego liœcia
 *		i zwraca œrodek dolnej krawêdzi m³odzego liœcia
 **/
VOID DrawLeaf(HWND hwnd, PPOINT pStart, LEAF& lChild, UINT iIncomingLevel, BOOL isRight, DRAWTEXTPARAMS& dtParams, TREEPROPERTIES& TreeProps, HDC hdc = NULL, BOOL bDrawLine = TRUE) {
	if (hdc == NULL) hdc = GetDC(hwnd);
	UINT cxLineWidth = TreeProps.cxTreeWidth >> (iIncomingLevel + 1);
	UINT cyLineHeight = RECT_HEIGHT; // mas³o maœlane
	BOOL bNarrow = TreeProps.bNarrow;
	BOOL bFinal = lstrlen(lChild.tSymbol.data()) != 0;
	BOOL bSymbol = (~bNarrow) & bFinal;
	RECT rect = {};
	if (isRight)
		pStart->x += cxLineWidth;
	else
		pStart->x -= cxLineWidth;
	pStart->y += cyLineHeight;
	if (bDrawLine)
		LineTo(hdc, pStart->x, pStart->y);

	// jestem na œroku górnej krawêdzi liœcia, chcê byæ w lewym górnym rogu

	
	if ((bNarrow & bFinal) && iIncomingLevel > 2) {
		pStart->x -= TreeProps.cxInnerLeafWidth / 2;
		 rect = CreateRect(pStart, (INT)(TreeProps.cxInnerLeafWidth / 1.4), TreeProps.cyTotalLeafHeight);
	}
	else {
		pStart->x -= TreeProps.cxInnerLeafWidth / 2;
		rect = CreateRect(pStart, TreeProps.cxInnerLeafWidth, TreeProps.cyTotalLeafHeight);
	}
	
	Rectangle(hdc, rect.left, rect.top, rect.right, rect.bottom);
	if (lstrlen(lChild.tSymbol.data()) != 0) {
		TSTRING(40) lpszBuffer = {};	
		std::wstring lpszTmp;
		if ((bSymbol || bNarrow & bFinal) && iIncomingLevel <= 5) {
			lpszTmp = lChild.tFPValue.substr(0, 5);
		}
		else lpszTmp = lChild.tFPValue.substr(0, 3);
		 _snwprintf_s(lpszBuffer.data(), 10, 40, TEXT("%s (%s)"), lpszTmp.c_str(), lChild.tSymbol.c_str());
		SetTextColor(hdc, RED);
		DrawTextEx(hdc, lpszBuffer.data(), 40, &rect, DT_NOCLIP | DT_VCENTER | DT_SINGLELINE, &dtParams);
		SetTextColor(hdc, BLACK);
	}
	else
		DrawTextEx(hdc, (LPWSTR)lChild.tFPValue.c_str(), (INT) lChild.tFPValue.length(), &rect, DT_VCENTER | DT_SINGLELINE, &dtParams);
	// ostatnia korekta: jestem w lewym górnym rogu liœcia, chcê byæ na œrodku dolnej krawêdzi
	pStart->x += TreeProps.cxInnerLeafWidth / 2;
	pStart->y += TreeProps.cyTotalLeafHeight;
	MoveToEx(hdc, pStart->x, pStart->y, NULL);
	// uwaga: zaleg³y release, rodzic powinien wywo³aæ ReleaseDC
	return;
}

TREEPROPERTIES CreateTreeProperties(LEAF& lTree, UINT cLeaves, HWND hwnd) {
	TEXTMETRIC tm = {};
	HDC hdc = GetDC(hwnd);
	GetTextMetrics(hdc, &tm);
	UINT cxChar = tm.tmAveCharWidth;
	UINT cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
	UINT cyChar = tm.tmHeight + tm.tmExternalLeading;
	BOOL bNarrowMode = FALSE;
	TREEPROPERTIES tp = {};
	tp.cLeaves = cLeaves;
	// wstêpnie oszacuj wymiary drzewa
	tp.cxInnerLeafWidth = FPLength * cxCaps / 2;
	tp.cNestLevels = ilog2<UINT>(cLeaves);
	if (tp.cNestLevels >= 3) bNarrowMode = TRUE;
	tp.cxTreeWidth = ToHigherTwoPower<UINT>(cLeaves) * tp.cxInnerLeafWidth;
	if (bNarrowMode) {
		tp.cxTreeWidth = (UINT)(tp.cxTreeWidth / 1.7);
	}
	tp.cxTotalLeafWidth = tp.cxTreeWidth >> (tp.cNestLevels + 2);
	tp.cyTotalLeafHeight = 2 * (cyChar + tm.tmExternalLeading); // BARDZO wstêpne za³o¿enie
	tp.bNarrow= bNarrowMode;
	ReleaseDC(hwnd, hdc);
	return tp;
}

BOOL __stdcall Ellipse(HDC hdc, RECT& rect) {
	return Ellipse(hdc, rect.left, rect.top, rect.right, rect.bottom);
};


LEAF_VECTOR CreateTreeFromList(HWND hList) {
	LEAF_VECTOR vLeaves;
	LEAF lLeaf = {};
	TCHAR psBuffer[60] = {};
	for (INT i = 0; i < ListView_GetItemCount(hList); i++) {
		ListView_GetItemText(hList, i, static_cast<INT>(COLUMNS::ID), psBuffer, 50);
		lLeaf.ID = _wtoi(psBuffer);
		ListView_GetItemText(hList, i, static_cast<INT>(COLUMNS::SYMBOL), psBuffer, 50);
		lLeaf.tSymbol = psBuffer;
		ListView_GetItemText(hList, i, static_cast<INT>(COLUMNS::VALUE), psBuffer, 50);
		lLeaf.tFPValue = psBuffer;
		lLeaf.FPValue = _wtof(psBuffer);
		vLeaves.push_back(lLeaf);
	}
	return vLeaves;
}


LEAF_VECTOR ProcessAutoText(PTCHAR psBuffer) {
	std::map<TCHAR, UINT> mLeaves;
	TCHAR* psCopy = psBuffer;
	psBuffer--;
	while (*++psBuffer) {
		if (mLeaves.find(*psBuffer) == mLeaves.end()) {
			mLeaves[*psBuffer] = 1;
		}
		else {
			mLeaves[*psBuffer]++;
		}
	}
	__int64 psLength = psBuffer - psCopy;
	if (psLength == 0) {
		MessageBox(NULL, TEXT("Nie podano tekstu!"), TEXT("B³¹d"), MB_OK | MB_ICONERROR);
		return LEAF_VECTOR();
	}
	INT iID = 50;
	LEAF_VECTOR vLeaves;
	for (auto& a : mLeaves) {
				LEAF lLeaf = {};
		lLeaf.ID = iID;
		lLeaf.tSymbol = a.first;
		lLeaf.FPValue = static_cast<DOUBLE>(a.second) / psLength;
		lLeaf.tFPValue = std::to_wstring(lLeaf.FPValue);
		vLeaves.push_back(lLeaf);
		iID++;
	}

	return vLeaves;
}

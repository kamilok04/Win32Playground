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
		break;																						   // menu ustawie� to dla systemu osobne okno
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
	// ZROBIONE	- edycja warto�ci w li�cie
	// ZROBIONE		- weryfikacja przysz�ych wej�� 
	//		- jak lista si� zachowuje w r�nych DPI/rodzielczo�ciach?
	//		- informuj o �le znormalizowanym prawdopodobie�stwie
	LEAF_VECTOR vTree(DefaultTreeValues, DefaultTreeValues + cDefaultValues);
	switch (uMsg) {
	case WM_INITDIALOG: {
		// ustaw domy�lny tryb pracy 
		SendDlgItemMessage(hwnd, IDC_RADIO_PROB, BM_SETCHECK, BST_CHECKED, 1);
		// wczytaj list�
		HWND hList = GetDlgItem(hwnd, IDC_TREE_LEAVES);
		if (hList == NULL) AWARIA(TEXT("gdzie jest lista?"));
		// dodaj nag��wki
		if (!AddListHeaders(hList)) AWARIA(TEXT("nag��wki listy"));
		// za�aduj i wypisz domy�lne drzewo
		for (auto& q : vTree) { // iteracja po wektorze jest wygodna
			if (!InsertListItem(hList, q)) AWARIA(TEXT("�adowanie warto�ci domy�lnych"));
		}
		bUpdateTree = TRUE;
		return TRUE;
	}
	case WM_NOTIFY: {		// WM_NOTIFY jest wysy�ane, kiedy w oknie ListView (tym z list�) zajdzie jaka� zmiana

		switch (reinterpret_cast<LPNMHDR>(lParam)->code) { // w dokumentacji jest C-rzut
		case NM_DBLCLK: {
			LPNMITEMACTIVATE lpnitem = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
			// dokumentacja m�wi, �e ta struktura zapewnia koordynaty pozycji w li�cie bez dodatkowych wywo�a�
			// ale tylko dla pierwszej kolumny
			UINT iItem = lpnitem->iItem;

			POINT point;
			GetCursorPos(&point);
			// zlikwiduj poprzednie okno, je�li takie istnia�o
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
			// HitTest prosi o wsp�rz�dne wzgl�dem lewego g�rnego rogu listy
			// konwersja
			ScreenToClient(GetDlgItem(hwnd, IDC_TREE_LEAVES), &point);
			lvthi.pt = point;
			if (ListView_SubItemHitTest(GetDlgItem(hwnd, IDC_TREE_LEAVES), &lvthi) == -1) break;
			// nie przerywam ca�ego programu, bo nie zawsze ca�e okno listy jest pe�ne pozycji
			// wtedy wyj�cie bez dzia�ania to po��dany efekt
			iItem = lvthi.iItem;

			HWND hList = GetDlgItem(hwnd, IDC_TREE_LEAVES);
			if (hList == NULL) AWARIA(TEXT("Gdzie jest lista?"));
			TCHAR psBuffer[20] = {};
			ListView_GetItemText(GetDlgItem(hwnd, IDC_TREE_LEAVES),
				iItem,
				lvthi.iSubItem,
				psBuffer,
				sizeof(psBuffer) / sizeof(psBuffer[0]));
			psBuffer[19] = 0; // dla pewno�ci
			// wszystko w porz�dku
			// zapisz wsp�rz�dne



			// zezw�l na edycj�
			// edycja pierwszej kolumny jest wbudowana...
			//if (lvthi.iSubItem == 0)
			//	ListView_EditLabel(hList, iItem);

			// ...ale innych nie 
			// we� wymiary i pozycj� klikni�tego elementu, na nim powstanie okno
			// Uwaga! Jednostki dialogu, piksele logiczne i piksele fizyczne to trzy r�ne rzeczy
			// Trzeba obchodzi� si� z tym ostro�nie, inaczej b�dzie �le
			RECT rect = {};
			ListView_GetSubItemRect(hList, iItem, lvthi.iSubItem, LVIR_BOUNDS, &rect);
			POINT tl = { rect.left, rect.top };
			POINT br = { rect.right,rect.bottom };

			// ustaw wysoko�� pola tekstowego na wysoko�� systemowej czcionki
			// (kt�ra mo�e mie� inny rozmiar ni� ta wy�wietlana w li�cie)

			STATIC TEXTMETRIC tm = {};
			STATIC UINT cFontY = 0;
			if (tm.tmHeight == 0) {
				HDC hdc = GetDC(hList);
				if (GetTextMetrics(hdc, &tm) == 0) AWARIA(TEXT("gdzie czcionka?"));
				cFontY = tm.tmHeight + tm.tmExternalLeading;
				DeleteDC(hdc);
			}

			// utw�rz pole tekstowe

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
			// okno tekstowe wysy�a powiadomienia, a rodzic (lista) ich nie mo�e przyj��
			// utw�rz hak, kt�ry przechwyca wiadomo�ci i na nie odpowiada
			// w normalnym oknie robi to WNDPROC lub DLGPROC
			if (SetWindowsHookEx(WH_GETMESSAGE, AuxiliaryEditProc, NULL, GetCurrentThreadId()) == NULL) AWARIA(TEXT("funkcja wirtualna"));

			// niech pole zawiera tekst, jaki jest w polu pod nim
			if (Edit_SetText(hTmpEdit, psBuffer) == 0) AWARIA(TEXT("Tekst okna tymczasowego"));

			// niech tekst b�dzie na pocz�tek zaznaczony
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
		//	if (DestroyWindow(reinterpret_cast<HWND>(lParam)) == FALSE) AWARIA(TEXT("Okno jest nie�miertelne"));
		//	UpdateWindow(hwnd);

		//	break;
		//}
		HWND hList = GetDlgItem(hwnd, IDC_TREE_LEAVES);
		BOOL bListVisible = IsWindowVisible(hList);

		// podmie� okna
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
			// dialog ma zosta�!
			// tu b�dzie wywo�ywane drzewo
			// pozb�d� si� starego 
			RedrawWindow(GetParent(hwnd), NULL, NULL, RDW_INVALIDATE | RDW_ERASE | RDW_UPDATENOW);
			switch (wmMode) {
			case WORK_MODE::PROBABILITY: {
					// sprawd�, czy prawdopodobie�stwa s� znormalizowane
					// je�li nie, to wy�wietl ostrze�enie
					// je�li tak, to wy�wietl drzewo
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
		case 0: lvc.cx = 40; break; // te warto�ci s� arbitralne
			// TODO: doczyta� o zasadach projektowania aplikacji
		case 1: lvc.cx = 80; break;
		case 2: lvc.cx = 190; break;
		}
		// ta funkcja zak�ada, �e stringi odpowiedzialne za nag��wki maj� ID po kolei od IDS_ORDER w g�r�
		// TODO: zrobi� to lepiej
		if (LoadString(GetModuleHandle(NULL), IDS_ORDER + i, psBuffer, sizeof(psBuffer) / sizeof(psBuffer[0])) == NULL) AWARIA(TEXT("�adowanie string�w"));
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
	// kod b��du w funkcjach ListView_* to zazwyczaj -1
	// -1 jest zdefiniowane jako ERROR_UNHANDLED_ERROR, wi�c u�ywam tego
	// rzuca si� w oczy
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

	// TODO: przygotuj okno-dziecko, w nim b�dzie drzewo
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
		// za�o�enie: pole tekstowe ma jedn� linijk� wysoko�ci
		HWND target = GetFocus();
		switch (msg.wParam) {

		case VK_RETURN: {

			TCHAR psBuffer[20];
			// obiekt, kt�ry nie jest "Edit Control", nie obs�u�y tej wiadomo�ci
			// leniwy spos�b, �eby upewni� si�, �e na pewno dzia�amy na polu tekstowym
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

wymagania wst�pne:
	+ u�ytkownik poda� prawdopodobie�stwa, upewnij si�,
	  �e s� znormalizowane (suma == 1) robi to szablon VerifyAndProcessInput w Drzewko.h
	  ZMIANA: algorytm pozwala na narysowanie �le znormalizowanych drzew, u�ytkownik jest informowany o problemie.

0.	je�li w li�cie jest jeden element, drzewo jest gotowe
1.	upewnij si�, �e li�cie s� rosn�co posortowane
		prawdopodobie�stwem/cz�sto�ci�
1b. je�li nie s�, to je posortuj
2.	we� dwa li�cie o najmniejszym prawdopodobie�stwie
3.	utw�rz nowy li��, kt�ry jako dzieci ma li�cie wzi�te w 2.
		jego prawdopodobie�stwo jest r�wne
		sumie prawdopodobie�stw dzieci
4.	usu� li�cie wzi�te w 2. z listy
5.	w�� li�� otrzymany w 3. do listy
	wr�� do 0.

*/

/**
 * Funkcja optymalizuj�ca drzewo
 *
 * \param vLeaves - wektor do li�ci w stanie surowym
 * \return
 */
LEAF PrepareTree(LEAF_VECTOR& vLeaves) {
	SIZE_T iLEAF = 0;
	UINT uID = 100;
	/*
	wektor ma tak du�y rozmiar, bo zmiana rozmiaru przenosi go w inne miejsce
	i uniewa�nia wszystkie doj�cia, a tego nie chcemy
	ten STATIC to kluczowa cz�� kodu, bo
	dzi�ki niemu to po prostu Dzia�a�
	*/
	STATIC LEAF_VECTOR vDone(4 * sizeof(vLeaves));
	vDone.clear();
	while (vLeaves.size() > 1) { // 0
		// TODO: podmieni� algorytm na w�asny
		// dobrym pomys�em b�dzie smoothsort (?), bo pracowa� b�dzie si� na prawie-posortowanym wektorze

		std::sort(vLeaves.begin(), vLeaves.end(), [](const LEAF lA, const LEAF lB) {
			return lA.FPValue < lB.FPValue; // 1b, krok 1 pomini�ty - zawsze sortuj�
			});
		// ustalanie warto�ci nowego li�cia
		// krok 2/3
		DOUBLE dNewProb = vLeaves[0].FPValue + vLeaves[1].FPValue;
		TCHAR lpszNewTFP[FPLength] = {};

		// sformatuj nowy tekst
		// przy za ma�ym buforze zostanie �ci�ty
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
		// co przebieg zabieram dwa elementy, ale dodaj� jeden
		// nic nie zostanie pomini�te

	}
	return vLeaves[0];
}

/**
 * Funkcja do rysowania ju� zoptymalizowanego drzewa.
 *
 * \param lTree - drzewo do narysowania
 * \param hwnd  - uchwyt do okna, w kt�rym b�dzie rysowane; to powinien by� rodzic dialogu
 * \return nie zwraca
 */

 /* Uk�ad wsp�rz�dnych Win32
 *	 +-------------------------> x
 *   | (0,0)
 *   |
 *   |
 *   |
 *	 v
 *   y
 */

 /* ========================
 *  WYCOFANE Z U�YTKU
 *  ========================
 *  Algorytm do chodzenia po drzewie
 *	1. Zainicjalizuj tabel� stan�w, ka�dy li�� ma stan w zakresie {0, 1, 2} - na pocz�tku 0 dla wszystkich
 *	2. Je�li li�� jest w stanie 0, prze��cz go na 1, narysuj i przejd� do jego lewego dziecka
 *	- powtarzaj krok 2, dop�ki lewe dziecko nie b�dzie mia�o warto�ci NULL
 *	- kiedy lewe dziecko ma warto�� NULL:
 *	3. Cofaj si�, dop�ki nie znajdziesz li�cia w stanie 1
 *	3a. Je�li rodzic li�cia w stanie 2 to NULL, koniec
 *	4. Prze��cz li�� na stan 2, przejd� do jego prawego dziecka i narysuj
 *	- wr�� do kroku 2
 *   =======================
 */

 /* Algorytm do chodzenia po drzewie - nie u�ywa ju� stan�w
 *	1. Zainicjalizuj pusty stos rodzic�w
 *	2. Ustaw si� na najstarszym li�ciu drzewa (tutaj lTree)
 *	3. Dop�ki istnieje, rysuj lewe dziecko li�cia; ka�de narysowane dziecko wrzu� na stos (powtarzaj)
 *	4. Dop�ki jeste� prawym dzieckiem, cofnij si�; usu� ka�de dziecko, z kt�rego si� cofasz, ze stosu (powtarzaj)
 *	   Na pocz�tku kroku 4. ZAWSZE b�dzie istnia� co najmniej jeden rodzic
 *	4a. Je�li sko�czy� si� stos, koniec
 *	5. Jeste� lewym dzieckiem, wi�c narysuj jedno prawe dziecko, przejd� do niego i wrzu� je na stos
 *	6. Wr�� do kroku 3.
 */
VOID DrawTree(LEAF& lTree, HWND hwnd, TREEPROPERTIES& tp) {

	HDC hdc = GetDC(hwnd);
	// ustaw statyczne w�a�ciwo�ci tekstu
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
			// cofnij si�
			// jestem na �rodku g�rnej kraw�dzi m�odszego li�cia
			// musz� przenie�� si� na �rodek dolnej kraw�dzi starszego
			// (cofn�� si� o prostok�t linii)

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


/** Funkcja rysuj�ca li��
 *
 * \param hwnd		- uchwyt do okna, w kt�rym b�dzie rysowany; to powinien by� rodzic dialogu
 * \param pStart	- punkt startowy, z kt�rego b�dzie rysowany li�� (w zamy�le �rodek dolnej kraw�dzi prostok�ta dymka rodzica)
 * \param lChild	- li�� do narysowania (ju� w formie dziecka)
 * \param iIncomingLevel - poziom, na kt�rym znajduje si� li��, kt�ry ma zosta� narysowany
 * \param isLeft	- lewe czy prawe dziecko?
 * \param dtParams	- parametry rysowania tekstu
 * \param TreeProps - w�a�ciwo�ci drzewa
 * \param hdc		- kontekst urz�dzenia, opcjonalne
 * \return nie
 * \throw wo�aj�cy powinien wywo�a� ReleaseDC() po u�yciu tej funkcji
 *		Funkcja przyjmuje �rodek dolnej kraw�dzi starszego li�cia
 *		i zwraca �rodek dolnej kraw�dzi m�odzego li�cia
 **/
VOID DrawLeaf(HWND hwnd, PPOINT pStart, LEAF& lChild, UINT iIncomingLevel, BOOL isRight, DRAWTEXTPARAMS& dtParams, TREEPROPERTIES& TreeProps, HDC hdc = NULL, BOOL bDrawLine = TRUE) {
	if (hdc == NULL) hdc = GetDC(hwnd);
	UINT cxLineWidth = TreeProps.cxTreeWidth >> (iIncomingLevel + 1);
	UINT cyLineHeight = RECT_HEIGHT; // mas�o ma�lane
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

	// jestem na �roku g�rnej kraw�dzi li�cia, chc� by� w lewym g�rnym rogu

	
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
	// ostatnia korekta: jestem w lewym g�rnym rogu li�cia, chc� by� na �rodku dolnej kraw�dzi
	pStart->x += TreeProps.cxInnerLeafWidth / 2;
	pStart->y += TreeProps.cyTotalLeafHeight;
	MoveToEx(hdc, pStart->x, pStart->y, NULL);
	// uwaga: zaleg�y release, rodzic powinien wywo�a� ReleaseDC
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
	// wst�pnie oszacuj wymiary drzewa
	tp.cxInnerLeafWidth = FPLength * cxCaps / 2;
	tp.cNestLevels = ilog2<UINT>(cLeaves);
	if (tp.cNestLevels >= 3) bNarrowMode = TRUE;
	tp.cxTreeWidth = ToHigherTwoPower<UINT>(cLeaves) * tp.cxInnerLeafWidth;
	if (bNarrowMode) {
		tp.cxTreeWidth = (UINT)(tp.cxTreeWidth / 1.7);
	}
	tp.cxTotalLeafWidth = tp.cxTreeWidth >> (tp.cNestLevels + 2);
	tp.cyTotalLeafHeight = 2 * (cyChar + tm.tmExternalLeading); // BARDZO wst�pne za�o�enie
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
		MessageBox(NULL, TEXT("Nie podano tekstu!"), TEXT("B��d"), MB_OK | MB_ICONERROR);
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

#include "Drzewko.h"

LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {						// funkcja okna z drzewkiem
	static HWND ustawienia = NULL;
	PAINTSTRUCT ps{};
	RECT rect{};
	HDC hdc;
	switch (uMsg) {

	case WM_CREATE: {
		CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TREE_SETTINGS), hwnd, TreeDialogProc); // dialog bez ramki
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

// ---------------------------

INT_PTR TreeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	// TODO:
	// - edycja wartoœci w liœcie
	// - weryfikacja przysz³ych wejœæ
	// - jak lista siê zachowuje w ró¿nych DPI/rodzielczoœciach?
	// - informuj o Ÿle znormalizowanym prawdopodobieñstwie

	WEKTOR_DO_LISCIA tree(DefaultTreeValues, DefaultTreeValues + cDefaultValues);
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
		for (auto& q : tree) { // iteracja po wektorze jest wygodna
			if (!InsertListItem(hList, q)) AWARIA(TEXT("³adowanie wartoœci domyœlnych"));
		}
		return TRUE;
	}
	case WM_NOTIFY: {		// WM_NOTIFY jest wysy³ane, kiedy w oknie ListView (tym z list¹) zajdzie jakaœ zmiana

		switch (reinterpret_cast<LPNMHDR>(lParam)->code) { // w dokumentacji jest C-rzut
		case NM_DBLCLK: {
			LPNMITEMACTIVATE lpnitem = reinterpret_cast<LPNMITEMACTIVATE>(lParam);
			// dokumentacja mówi, ¿e ta struktura zapewnia koordynaty pozycji w liœcie bez dodatkowych wywo³añ
			// ale tylko dla pierwszej kolumny
			UINT iItem = lpnitem->iItem;

			POINT punkt;
			GetCursorPos(&punkt);
			// zlikwiduj poprzednie okno, jeœli takie istnia³o
			STATIC HWND hTmpEdit = nullptr;
			if (hTmpEdit != nullptr) {
				TCHAR bufor[20];
				if (Edit_GetText(hTmpEdit, bufor, 20) != 0) {
					switch (lvthi.iSubItem) {
					case KOLUMNY::LPORZ: {
						const TCHAR* psCompareString = COMPARE_STRING_INT; 
						VerifyAndProcessInput<INT>(hTmpEdit, bufor, lvthi, 20, psCompareString);
						break;
					}
					case KOLUMNY::SYMBOL: {
						const TCHAR* psCompareString = COMPARE_STRING_STRING; 
						VerifyAndProcessInput<PTCHAR>(hTmpEdit, bufor, lvthi, 20, psCompareString);
						break; 
					}
					default: {
						const TCHAR* psCompareString = COMPARE_STRING_DOUBLE;
						VerifyAndProcessInput<DOUBLE>(hTmpEdit, bufor, lvthi, 20, psCompareString);
						break; 
					}
					}

				}
				DestroyWindow(hTmpEdit);
			}
			// HitTest prosi o wspó³rzêdne wzglêdem lewego górnego rogu listy
			// konwersja
			ScreenToClient(GetDlgItem(hwnd, IDC_TREE_LEAVES), &punkt);
			lvthi.pt = punkt;
			if (ListView_SubItemHitTest(GetDlgItem(hwnd, IDC_TREE_LEAVES), &lvthi) == -1) break;
			// nie przerywam ca³ego programu, bo nie zawsze ca³e okno listy jest pe³ne pozycji
			// wtedy wyjœcie bez dzia³ania to po¿¹dany efekt
			iItem = lvthi.iItem;

			HWND hList = GetDlgItem(hwnd, IDC_TREE_LEAVES);
			if (hList == NULL) AWARIA(TEXT("Gdzie jest lista?"));
			TCHAR bufor[20] = {};
			ListView_GetItemText(GetDlgItem(hwnd, IDC_TREE_LEAVES),
				iItem,
				lvthi.iSubItem,
				bufor,
				sizeof(bufor) / sizeof(bufor[0]));
			bufor[19] = 0; // dla pewnoœci
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
			STATIC UINT czcionkaY = 0;
			if (tm.tmHeight == 0) {
				HDC hdc = GetDC(hList);
				if (GetTextMetrics(hdc, &tm) == 0) AWARIA(TEXT("gdzie czcionka?"));
				czcionkaY = tm.tmHeight + tm.tmExternalLeading;
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
				czcionkaY,
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
			if (Edit_SetText(hTmpEdit, bufor) == 0) AWARIA(TEXT("Tekst okna tymczasowego"));

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
		switch (LOWORD(wParam)) {
		case IDOK:
			// dialog ma zostaæ!
			// tu bêdzie wywo³ywane drzewo
			GenerateTree(tree);
			return 0;

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
	WCHAR bufor[50];
	LVCOLUMN lvc;
	INT iCol = KOLUMNY::KOLUMNY_MAX;
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
	lvc.pszText = bufor;
	for (INT i = 0; i < iCol; i++) {
		switch (i) {
		case 0: lvc.cx = 40; break; // te wartoœci s¹ arbitralne
			// TODO: doczytaæ o zasadach projektowania aplikacji
		case 1: lvc.cx = 80; break;
		case 2: lvc.cx = 190; break;
		}
		// ta funkcja zak³ada, ¿e stringi odpowiedzialne za nag³ówki maj¹ ID po kolei od IDS_ORDER w górê
		// TODO: zrobiæ to lepiej
		if (LoadString(GetModuleHandle(NULL), IDS_ORDER + i, bufor, sizeof(bufor) / sizeof(bufor[0])) == NULL) AWARIA(TEXT("³adowanie stringów"));
		if (ListView_InsertColumn(hList, iCol, &lvc) == ERROR_UNHANDLED_ERROR) return FALSE;
	}
	return TRUE;
}

BOOL InsertListItem(HWND hList, struct Leaf& leaf) {
	LVITEM lvI = {};
	lvI.pszText = leaf.tID;
	lvI.mask = LVIF_TEXT;
	lvI.iSubItem = KOLUMNY::LPORZ;
	lvI.iItem = 0;
	// kod b³êdu w funkcjach ListView_* to zazwyczaj -1
	// -1 jest zdefiniowane jako ERROR_UNHANDLED_ERROR, wiêc u¿ywam tego
	// rzuca siê w oczy
	if (ListView_InsertItem(hList, &lvI) == ERROR_UNHANDLED_ERROR) return FALSE;
	lvI.iSubItem = KOLUMNY::SYMBOL;
	lvI.pszText = leaf.tSymbol;
	if (ListView_SetItem(hList, &lvI) == ERROR_UNHANDLED_ERROR) return FALSE;
	lvI.iSubItem = KOLUMNY::WARTOSC;
	lvI.pszText = leaf.tFPValue;
	if (ListView_SetItem(hList, &lvI) == ERROR_UNHANDLED_ERROR) return FALSE;
	return TRUE;
}

// nareszcie!
/* ALGORYTM RYSOWANIA DRZEWA

wymagania wstêpne:
	- u¿ytkownik poda³ prawdopodobieñstwa, upewnij siê,
	  ¿e s¹ znormalizowane (suma == 1)

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

VOID GenerateTree(WEKTOR_DO_LISCIA& vec) {

	// TODO
	// zaimplementowaæ algorytm opisany wy¿ej
	// i narysowaæ to drzewko

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

			TCHAR bufor[20];
			// obiekt, który nie jest "Edit Control", nie obs³u¿y tej wiadomoœci
			// leniwy sposób, ¿eby upewniæ siê, ¿e na pewno dzia³amy na polu tekstowym
			if (SendMessage(target, EM_SETREADONLY, true, NULL)) {
				Edit_GetText(target, bufor, 20);
				switch (lvthi.iSubItem) {
				case KOLUMNY::LPORZ: {
					const TCHAR* psCompareString = COMPARE_STRING_INT;
					VerifyAndProcessInput<INT>(target, bufor, lvthi, 20, psCompareString);
					break;
				}
				case KOLUMNY::SYMBOL: {
					const TCHAR* psCompareString = COMPARE_STRING_STRING;
					VerifyAndProcessInput<PTCHAR>(target, bufor, lvthi, 20, psCompareString);
					break;
				}
				default: {
					const TCHAR* psCompareString = COMPARE_STRING_DOUBLE;
					VerifyAndProcessInput<DOUBLE>(target, bufor, lvthi, 20, psCompareString);
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


#include "Drzewko.h"

LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {						// funkcja okna z drzewkiem
	static HWND ustawienia = NULL;
	PAINTSTRUCT ps{};
	RECT rect{};
	HDC hdc;
	switch (uMsg) {
	case WM_CREATE: {
		CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TREE_SETTINGS), hwnd, TreeDialogProc); // dialog bez ramki
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

INT_PTR TreeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) 
{
	// TODO:
	// - edycja warto�ci w li�cie
	// - weryfikacja przysz�ych wej��
	// - jak lista si� zachowuje w r�nych DPI/rodzielczo�ciach?
	// - informuj o �le znormalizowanym prawdopodobie�stwie

	WEKTOR_DO_LISCIA tree(DefaultTreeValues, DefaultTreeValues + cDefaultValues);
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
		for (auto &q : tree) { // iteracja po wektorze jest wygodna
			if (!InsertListItem(hList, q)) AWARIA(TEXT("�adowanie warto�ci domy�lnych"));
		}
		return TRUE;
	}
	case WM_NOTIFY: {		// WM_NOTIFY jest wysy�ane, kiedy w oknie ListView (tym z list�) zajdzie jaka� zmiana
		switch (reinterpret_cast<LPNMHDR>(lParam) -> code) { // w dokumentacji jest C-rzut
		case NM_DBLCLK: {
			LPNMITEMACTIVATE lpnitem = reinterpret_cast<LPNMITEMACTIVATE>(lParam); 
			// dokumentacja m�wi, �e ta struktura zapewnia koordynaty pozycji w li�cie bez dodatkowych wywo�a�
			// ale tylko dla pierwszej kolumny
			LVHITTESTINFO lvthi = {}; // b�dzie robiony hitscan
			UINT iItem = lpnitem->iItem;
			if (iItem == -1) {
				POINT punkt;
				// HitTest prosi o wsp�rz�dne wzgl�dem lewego g�rnego rogu listy
				// konwersja
				GetCursorPos(&punkt);
				ScreenToClient(GetDlgItem(hwnd, IDC_TREE_LEAVES), &punkt);
				lvthi.pt = punkt;
				if (ListView_SubItemHitTest(GetDlgItem(hwnd, IDC_TREE_LEAVES), &lvthi) == -1) break; 
				// nie przerywam ca�ego programu, bo nie zawsze ca�e okno listy jest pe�ne pozycji
				// wtedy wyj�cie bez dzia�ania to po��dany efekt
				iItem = lvthi.iItem;
			}
			TCHAR bufor[20] = {};
			ListView_GetItemText(GetDlgItem(hwnd, IDC_TREE_LEAVES),
				iItem,
				lvthi.iSubItem,
				bufor,
				sizeof(bufor) / sizeof(bufor[0]));
			bufor[19] = 0; // dla pewno�ci
			MessageBox(hwnd, bufor, L"CHUJ", MB_OK);
			break;
		}
		default: break;
		}
		break;
	}
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			// dialog ma zosta�!
			// tu b�dzie wywo�ywane drzewo
			GenerateTree(tree);
			return 0;

		case IDCANCEL:
			// zniszcz rodzica
			SendMessage(GetParent(hwnd), WM_DESTROY, NULL, NULL);
			return TRUE;
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
		case 0: lvc.cx = 40; break; // te warto�ci s� arbitralne
									// TODO: doczyta� o zasadach projektowania aplikacji
		case 1: lvc.cx = 80; break;
		case 2: lvc.cx = 190; break;
		}
		// ta funkcja zak�ada, �e stringi odpowiedzialne za nag��wki maj� ID po kolei od IDS_ORDER w g�r�
		// TODO: zrobi� to lepiej
		if (LoadString(GetModuleHandle(NULL), IDS_ORDER + i, bufor, sizeof(bufor) / sizeof(bufor[0])) == NULL) AWARIA(TEXT("�adowanie string�w"));
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
	// kod b��du w funkcjach ListView_* to zazwyczaj -1
	// -1 jest zdefiniowane jako ERROR_UNHANDLED_ERROR, wi�c u�ywam tego
	// rzuca si� w oczy
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

wymagania wst�pne:
	- u�ytkownik poda� prawdopodobie�stwa, upewnij si�, 
	  �e s� znormalizowane (suma == 1)

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

VOID GenerateTree(WEKTOR_DO_LISCIA& vec) {

	// TODO
	// zaimplementowa� algorytm opisany wy�ej
	// i narysowa� to drzewko

	return;
}
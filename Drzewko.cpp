#include "Drzewko.h"

BOOL AddListHeaders(HWND);
BOOL InsertListItems(HWND, INT);
BOOL PrepareItemText(HWND, LPARAM, STRUCT Lisc&, INT&);

LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND ustawienia = NULL;
	PAINTSTRUCT ps{};
	RECT rect{};
	HDC hdc;
	switch (uMsg) {
	case WM_CREATE: {
		CreateDialog(GetModuleHandle(NULL), MAKEINTRESOURCE(IDD_TREE_SETTINGS), hwnd, TreeDialogProc);
		break;
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

INT_PTR TreeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	STATIC HWND hList = nullptr;
	STATIC INT cId = 0;
	static struct Lisc currentLeaf = DefaultTreeValues[0];
	switch (uMsg) {
	case WM_INITDIALOG:
		hList = GetDlgItem(hwnd, IDC_TREE_LEAVES);
		if (hList == NULL) SEPPUKU(TEXT("gdzie jest lista?"));
		// dodaj nag³ówki
		if (!AddListHeaders(hList)) SEPPUKU(TEXT("nag³ówki listy"));
		// za³aduj i wypisz domyœlne drzewo
		if (!InsertListItems(hList, cDefaultValues)) SEPPUKU(TEXT("³adowanie wartoœci domyœlnych"));
		return TRUE;
	case WM_NOTIFY:
		currentLeaf = DefaultTreeValues[cId];
		PrepareItemText(hList, lParam, currentLeaf, cId);
	case WM_COMMAND:
		switch (LOWORD(wParam)) {
		case IDOK:
			// dialog ma zostaæ!
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
	INT iCol = 3; // l.p., symbol, czêstoœæ/prawdopodobieñstwo
	lvc.mask = LVCF_TEXT | LVCF_SUBITEM | LVCF_WIDTH;
	lvc.pszText = bufor;
	for (INT i = 0; i < iCol; i++) {
		switch (i) {
		case 0: lvc.cx = 40; break;
		case 1: lvc.cx = 80; break;
		case 2: lvc.cx = 190; break;
		}
		if (0 == LoadString(GetModuleHandle(NULL), IDS_ORDER + i, bufor, sizeof(bufor) / sizeof(bufor[0]))) SEPPUKU(TEXT("³adowanie stringów"));
		if (-1 == ListView_InsertColumn(hList, iCol, &lvc)) return FALSE;
	}
	return TRUE;
}

BOOL InsertListItems(HWND hList, INT cIle) {
	LVITEM lvI;

	lvI.pszText = LPSTR_TEXTCALLBACK;
	lvI.mask = LVIF_TEXT;
	lvI.iSubItem = 0;
	for (int index = 0; index < cIle; index++)
	{
		lvI.iItem = index;
		if (ListView_InsertItem(hList, &lvI) == -1) return FALSE;
	}
	return TRUE;
}

BOOL PrepareItemText(HWND hList, LPARAM lParam, STRUCT Lisc& leaf, INT& cId) {

	NMLVDISPINFO* plvdi;
	LPNMHDR lpnmhdr = reinterpret_cast<LPNMHDR>(lParam);
	switch (lpnmhdr->code)
	{
	case LVN_GETDISPINFO:

		plvdi = reinterpret_cast<NMLVDISPINFO*>(lParam);

		switch (plvdi->item.iSubItem)
		{
		case 0:
			plvdi->item.pszText = leaf.tID;
			break;
		case 1:
			plvdi->item.pszText = leaf.tSymbol;
			break;
		case 2:
			plvdi->item.pszText = leaf.tFPValue;
			cId++;
			if (cId >= cDefaultValues) cId = 0;
			break;

		default:
			return FALSE;
		}
		return TRUE;
		

	}
	return FALSE;
}
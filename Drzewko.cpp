#include "Drzewko.h"


LRESULT CALLBACK TreeWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
	static HWND ustawienia = NULL;
	PAINTSTRUCT ps{};
	RECT rect{};
	HDC hdc;
	switch (uMsg){
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

	switch (uMsg) {
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

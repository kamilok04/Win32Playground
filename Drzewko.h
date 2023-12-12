#include "main.h"


INT_PTR TreeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

struct Lisc {
	size_t prawdopodobienstwo;
	struct Lisc* leweDziecko = nullptr;
	struct Lisc* praweDziecko = nullptr;
};
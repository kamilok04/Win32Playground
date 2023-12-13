#include "main.h"

INT_PTR TrigDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void InitTrigList(HWND hwnd);

enum TRIG_ID { SINE, COSINE, TANGENT, COTANGENT };
STATIC CONSTEXPR INT HowManyTrigFunctions = sizeof(TRIG_ID);

STRUCT TrigData {
	BOOL drawn;
	DOUBLE from;
	DOUBLE to;
	INT precision;
};
#include "main.h"

INT_PTR TrigDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

void InitTrigList(HWND hwnd);

enum TRIG_ID { SINE, COSINE, TANGENT, COTANGENT };

struct TrigData {
	UINT TRIG_ID;
	double from;
	double to;
	int precision;
};
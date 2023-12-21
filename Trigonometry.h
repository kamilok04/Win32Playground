#include "main.h"

#define KPI(k) k*PI
#define MAX_POINTS 200
#define PRECISION_TO_POINTS(n) MAX_POINTS * n / 100 

enum TRIG_ID : INT { SINE, COSINE, TANGENT, COTANGENT, TRIG_ID_MAX };

// TODO: wiêcej danych!
STRUCT TrigData{
	BOOL drawn;
	DOUBLE from;
	DOUBLE to;
	INT precision;
};

static int num = 1000;
static constexpr double PI = 3.141592;

INT_PTR TrigDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
void InitTrigList(HWND hwnd);


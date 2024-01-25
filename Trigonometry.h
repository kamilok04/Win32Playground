#include "main.h"

#define KPI(k) k*PI // niezbyt wyrafinowane
// czy to ni¿ej jeszcze jest potrzbne?
#define MAX_POINTS 200
#define PRECISION_TO_POINTS(n) MAX_POINTS * n / 100 

enum TRIG_ID : INT { SINE, COSINE, TANGENT, COTANGENT, TRIG_ID_MAX };

// TODO: wiêcej danych!
STRUCT TrigData{
	TRIG_ID type;
	BOOL drawn;
	DOUBLE from;
	DOUBLE to;
	INT precision;
};

STRUCT DrawParamPackage{
	HWND hwnd;
	RECT ClientRect;
};

static int num = 1000;
static constexpr double PI = 3.141592;

INT_PTR TrigDialogProc(HWND, UINT, WPARAM, LPARAM);
void InitTrigList(HWND);
DWORD WINAPI DrawTrig();
VOID AnimateTrig();
STATIC VOID ToggleTrigDraw(WORD, HWND);


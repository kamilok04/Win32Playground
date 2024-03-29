#include "main.h"

#define KPI(k) k*PI // niezbyt wyrafinowane
// czy to ni�ej jeszcze jest potrzbne?
#define MAX_POINTS 200
#define PRECISION_TO_POINTS(n) MAX_POINTS * n / 100 
#define CORRECTED_SELECTION TrigFunctions[tiSelected[ComboBox_GetCurSel(hList)]]
#define ID_TRIG_VELOCITY 2136
#define ID_TRIG_ACCELERATION 2137

 enum TRIG_ID : INT { SINE, COSINE, TANGENT, COTANGENT, DEMO1, DEMO2, TRIG_ID_MAX };

// TODO: wi�cej danych!
STRUCT TrigData{
	TRIG_ID type;
	BOOL drawn;
	DOUBLE from;
	DOUBLE to;
	INT precision;
	INT thickness;
	INT intendedCycles = 4;
};

STRUCT DrawParamPackage{
	HWND hwnd;
	RECT ClientRect;
};

static int num = 1000;
static constexpr double PI = 3.14159265358979323846;
static constexpr int DampeningFactor = 50;

INT_PTR TrigDialogProc(HWND, UINT, WPARAM, LPARAM);
STATIC VOID InitTrigList(HWND, TRIG_ID*);
DWORD WINAPI DrawTrig();
VOID AnimateTrig();
STATIC VOID ToggleTrigDraw(WORD, HWND);
LONG ApplyRectCorrection(RECT*,RECT*, DIMENSIONS);
HRESULT CreateRebar(HWND, HWND, HWND);
HRESULT PrepareThreadPool(UINT, PTP_POOL pPool = CreateThreadpool(NULL));
VOID UpdateFunctionDrawingParams(HWND hList, DWORD dPrecision, DWORD dThickness, COLORREF crColor);

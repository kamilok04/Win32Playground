#include "main.h"


INT_PTR TreeDialogProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);

STRUCT Leaf{
	TCHAR tID[10];
	TCHAR tSymbol[20];
	TCHAR tFPValue[20];
	DOUBLE FPValue;
	STRUCT Leaf* LeftChild = nullptr;
	STRUCT Leaf* RightChild = nullptr;
};

STATIC STRUCT Leaf DefaultTreeValues[] = {
	{TEXT("9"), TEXT("i"), TEXT("0.04"), 0.04, nullptr, nullptr},
	{TEXT("8"), TEXT("h"), TEXT("0.06"), 0.06, nullptr, nullptr},
	{TEXT("7"), TEXT("g"), TEXT("0.08"), 0.08, nullptr, nullptr},
	{TEXT("6"), TEXT("f"), TEXT("0.1 "), 0.1 , nullptr, nullptr},
	{TEXT("5"), TEXT("e"), TEXT("0.1 "), 0.1 , nullptr, nullptr},
	{TEXT("4"), TEXT("d"), TEXT("0.12"), 0.12, nullptr, nullptr},
	{TEXT("3"), TEXT("c"), TEXT("0.15"), 0.15, nullptr, nullptr},
	{TEXT("2"), TEXT("b"), TEXT("0.15"), 0.15, nullptr, nullptr},
	{TEXT("1"), TEXT("a"), TEXT("0.2 "), 0.2 , nullptr, nullptr}
};

STATIC CONST INT cDefaultValues = sizeof(DefaultTreeValues) / sizeof(DefaultTreeValues[0]);

ENUM TRYB_PRACY : BOOL { PRAWDOPODOBIENSTWO, CZESTOSC, TRYB_PRACY_MAX};
// LPORZ mia³o byæ LP, ale ju¿ jest zdefiniowane w <windows.h> jako wskaŸnik
ENUM KOLUMNY : INT { LPORZ, SYMBOL, WARTOSC, KOLUMNY_MAX};


BOOL AddListHeaders(HWND);
BOOL InsertListItem(HWND, STRUCT Leaf&);
VOID GenerateTree(WEKTOR_DO_LISCIA&);
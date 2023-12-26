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

ENUM TRYB_PRACY : BOOL{ PRAWDOPODOBIENSTWO, CZESTOSC, TRYB_PRACY_MAX };
// LPORZ mia³o byæ LP, ale ju¿ jest zdefiniowane w <windows.h> jako wskaŸnik
ENUM KOLUMNY : INT{ LPORZ, SYMBOL, WARTOSC, KOLUMNY_MAX };


BOOL AddListHeaders(HWND);
BOOL InsertListItem(HWND, STRUCT Leaf&);
VOID GenerateTree(WEKTOR_DO_LISCIA&);
LRESULT CALLBACK AuxiliaryEditProc(INT, WPARAM, LPARAM);

// Weryfikuj podane dane
// przyjmuje: dane do weryfikacji, ich d³ugoœæ, ci¹g znaków jako klucz/katalizator

// ============================================================
// UWAGA: trzeba póŸniej delete[] adres zwracany przez funkcjê!
// ============================================================

// specjalizacja dla wszystkich typów zmiennoprzecinkowych


TEMPLATE <TYPENAME T>
STATIC PTCHAR VerifyInput(HWND hwnd, PTCHAR stringToValidate, UINT stringLength, CONST TCHAR* compareString) {
	T t = NULL;
	// dopasuj do ¿¹danego typu
	if (!std::is_same<T, PTCHAR>::value) {
		PTCHAR bufor = new TCHAR[stringLength]{};
		if (swscanf_s(stringToValidate, compareString, &t, stringLength) != 1) {
			delete[] bufor;
			return nullptr;
		}
		if (std::is_floating_point<T>::value) {
			// od C++20 istnieje bit_cast<T>(t), który robi dok³adnie to samo
			if (std::isnan(*(DOUBLE*)(VOID*)(&t)) || std::isinf(*(DOUBLE*)(VOID*)(&t))) { 
				delete[] bufor;
				return nullptr;
			}
		}
		if (_snwprintf_s(bufor, stringLength, _TRUNCATE, compareString, t) <= 0) {
			delete[] bufor;
			return nullptr;
		}
		return bufor;
	} else {
		if (lstrlen(stringToValidate) != 0) {
			return stringToValidate;
		}
		return nullptr;
	}
	
};

// Rozszerzona funkcja; weryfikuj i wstaw
// Uwaga: nale¿y podaæ HWND do pola tekstowego, nie do listy, w której jest!
TEMPLATE<TYPENAME CompareType>

BOOL VerifyAndProcessInput(HWND hEdit,
	PTCHAR stringToValidate,
	LVHITTESTINFO& lvthi,
	UINT stringLength = 10,
	CONST TCHAR* compareString = COMPARE_STRING_DEFAULT) {

	// weryfikuj
	PTCHAR cleanInput = VerifyInput<CompareType>(hEdit, stringToValidate, stringLength, compareString);
	if (cleanInput == nullptr)
		return FALSE;
	

	// zmieñ wartoœæ listy
	HWND hList = GetParent(hEdit);
	if (hList == NULL) return FALSE;
	ListView_SetItemText(hList, lvthi.iItem, lvthi.iSubItem, cleanInput);
	if(compareString != COMPARE_STRING_STRING)
		delete[] cleanInput; // nie lubimy wycieków

	// przetwórz now¹ listê
	// co nale¿y sprawdziæ:
	// kolumna 0: konflikt l.p.
	// kolumna 1: konflikt symboli
	// kolumna 2: Ÿle znormalizowane prawdopodobieñstwo/ujemne prawd.

	UINT cCols = ListView_GetItemCount(hList);

	// PPSz - bestseller 1941 roku
	std::vector <PTCHAR> vValues(cCols * stringLength);

	for (SIZE_T iCol = 0; iCol < cCols; iCol++) {
		ListView_GetItemText(hList, iCol, lvthi.iSubItem, vValues[iCol * stringLength], stringLength);
		iCol++;
	}
	switch (lvthi.iSubItem) {
	case KOLUMNY::LPORZ: {

		break;
	}
	case KOLUMNY::SYMBOL: {

		break;
	}
	default: {

	}
	}
	return TRUE;
}
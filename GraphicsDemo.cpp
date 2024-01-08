#include "GraphicsDemo.h"



LRESULT CALLBACK GraphicsDemoWndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam) {
	static int cxChar, cyChar, cxCaps, cxClient, cyClient, iMaxWidth;
	int i, x, y, iVertPos, iHorzPos, iPaintBeg, iPaintEnd;
	SCROLLINFO si = {};
	TCHAR szBuffer[10] = {};
	TEXTMETRIC tm;
	HDC hdc;
	PAINTSTRUCT ps;

	switch (message) {
	case WM_CREATE:
		hdc = GetDC(hwnd);
		GetTextMetrics(hdc, &tm);
		cxChar = tm.tmAveCharWidth;
		cxCaps = (tm.tmPitchAndFamily & 1 ? 3 : 2) * cxChar / 2;
		cyChar = tm.tmHeight + tm.tmExternalLeading;
		ReleaseDC(hwnd, hdc);
		iMaxWidth = 50 * cxChar + 22 * cxCaps;
		PlaySound(TEXT("hellowin.wav"), NULL, SND_FILENAME | SND_ASYNC);
		return 0;
	case WM_SIZE:
		// trzeba rozbiæ lParam
		// wysokie s³owo = wysokoœæ
		// niskie s³owo = szerokoœæ
		// ustaw pionowy pasek
		cxClient = LOWORD(lParam);
		cyClient = HIWORD(lParam);
		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = NUMLINES - 1;
		si.nPage = cyClient / cyChar;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

		// ustaw poziomy pasek

		si.cbSize = sizeof(si);
		si.fMask = SIF_RANGE | SIF_PAGE;
		si.nMin = 0;
		si.nMax = 2 + iMaxWidth / cxChar;
		si.nPage = cxClient / cxChar;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);


		return 0;
	case WM_VSCROLL:
		// dowiedz siê, co pasek pionowy robi
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVertPos = si.nPos;
		switch (LOWORD(wParam)) {
			// definicje s¹ w <winuser.h>
		case SB_TOP:			si.nPos = si.nMin; break;
		case SB_BOTTOM:			si.nPos = si.nMax; break;
		case SB_LINEUP:			--si.nPos;  break;
		case SB_LINEDOWN:		++si.nPos; break;
		case SB_PAGEUP:			si.nPos -= si.nPage; break;
		case SB_PAGEDOWN:		si.nPos += si.nPage; break;
		case SB_THUMBPOSITION:	si.nPos = si.nTrackPos; break;
		default: break;
		}

		// ustaw po³o¿enie i pobierz
		// jeœli s¹ ró¿ne, odœwie¿ okno

		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
		GetScrollInfo(hwnd, SB_VERT, &si);

		if (si.nPos != iVertPos) {
			ScrollWindow(hwnd, 0, cyChar * (iVertPos - si.nPos), NULL, NULL);
			UpdateWindow(hwnd);
		}

		return 0;

	case WM_HSCROLL:
		// dowiedz siê, co poziomy pasek robi
		si.cbSize = sizeof(si);
		si.fMask = SIF_ALL;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;

		switch (LOWORD(wParam)) {
			// definicje s¹ w <winuser.h>
		case SB_LINELEFT:		--si.nPos;  break;
		case SB_LINERIGHT:		++si.nPos; break;
		case SB_PAGELEFT:		si.nPos -= si.nPage; break;
		case SB_PAGERIGHT:		si.nPos += si.nPage; break;
		case SB_THUMBPOSITION:	si.nPos = si.nTrackPos; break;
		default: break;
		}
		// ewentualna korekta

		si.fMask = SIF_POS;
		SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
		GetScrollInfo(hwnd, SB_HORZ, &si);

		if (si.nPos != iHorzPos) {
			ScrollWindow(hwnd, cxChar * (iHorzPos - si.nPos), 0, NULL, NULL);
			UpdateWindow(hwnd);
		}

		return 0;


	case WM_PAINT:
		hdc = BeginPaint(hwnd, &ps);
		// pobierz paski i dostosuj siê do ich wskazañ
		si.cbSize = sizeof(si);
		si.fMask = SIF_POS;
		GetScrollInfo(hwnd, SB_VERT, &si);
		iVertPos = si.nPos;
		GetScrollInfo(hwnd, SB_HORZ, &si);
		iHorzPos = si.nPos;

		// czy ograniczyæ malowanie?
		iPaintBeg = max(0, iVertPos + ps.rcPaint.top / cyChar);
		iPaintEnd = min(NUMLINES - 1, iVertPos + ps.rcPaint.bottom / cyChar);
		for (i = iPaintBeg; i <= iPaintEnd; i++) {
			x = cxChar * (1 - iHorzPos);
			y = cyChar * (i - iVertPos);
			TextOut(hdc, x, y, sysmetrics[i].szLabel, lstrlen(sysmetrics[i].szLabel));
			TextOut(hdc, x + 22 * cxCaps, y, sysmetrics[i].szDesc, lstrlen(sysmetrics[i].szDesc));
			SetTextAlign(hdc, TA_RIGHT | TA_TOP);
			TextOut(hdc, x + 30 * cxCaps + 40 * cxChar, y, szBuffer, wsprintf(szBuffer, TEXT("%5d"), GetSystemMetrics(sysmetrics[i].iIndex)));
			SetTextAlign(hdc, TA_LEFT | TA_TOP);
		}
		EndPaint(hwnd, &ps);
		return 0;
	case WM_DESTROY:
		return 0;
	}
	return DefWindowProc(hwnd, message, wParam, lParam);

}



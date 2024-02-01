#include "DWrite.h"

/**
 * Utw�rz zasoby niezale�ne od urz�dzenia. Cz�onek klasy
 * 
 * \return zwyk�y
 */
HRESULT DWrite::CreateIndependentResources(){
	HRESULT hr = S_OK;
	// zr�b fabryk� wszystkich obiekt�w D2D
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	// zr�b fabryk� wszystkich obiekt�w DWrite
	if (SUCCEEDED(hr)) {
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	}
	if (SUCCEEDED(hr)) {
		// zr�b format
		hr = pDWriteFactory->CreateTextFormat(
			L"Comic Sans MS",
			NULL,
			DWRITE_FONT_WEIGHT_REGULAR,
			DWRITE_FONT_STYLE_NORMAL,
			DWRITE_FONT_STRETCH_NORMAL,
			30.0f,
			L"pl-pl",
			&pTextFormat
		);
	}
	//// sformatuj go wst�nie 
	//if (SUCCEEDED(hr)) {
	//	hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	//}
	//if (SUCCEEDED(hr)) {
	//	hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	//}
	return hr;
}

/**
 * Utw�rz zasoby uzale�nione od sprz�tu. Cz�onek klasy
 * 
 * \param hwnd - uchwyt do okna
 * \return zwyk�y
 */
HRESULT DWrite::CreateDependentResources(HWND hwnd) {
	HRESULT hr = S_OK;
	GetClientRect(hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	// je�li nie ma celu renderowania, to go zr�b
	// je�li jest, to u�yj istniej�cego
	if (!pRT) {
		hr = pD2DFactory->CreateHwndRenderTarget(
			D2D1::RenderTargetProperties(),
			D2D1::HwndRenderTargetProperties(hwnd, size),
			&pRT
		);
	}
	// nadaj tekstowi kolor
	if (SUCCEEDED(hr)) {
		hr = pRT->CreateSolidColorBrush(
			D2D1::ColorF(D2D1::ColorF::Violet),
			&pBrush
		);
	}
	InitializeDPIScale(hwnd);
	return hr;
}

VOID DWrite::DiscardRecources() {
	SafeRelease(&pRT);
	SafeRelease(&pBrush);

}

HRESULT DWrite::DrawText(LPCWSTR wszText, UINT32 cTextLength) {
	HRESULT hr = S_OK;
	// utw�rz prostok�t D2D
	// dokumentacja mia�a osobne warto�ci dla X i Y
	// je�li tekst b�dzie mia� intryguj�cy kszta�t, to tu jest problem
	D2D1_RECT_F layoutRect = D2D1::RectF(
		PixelsToDips(rc.left),
		PixelsToDips(rc.top),
		PixelsToDips(rc.right),
		PixelsToDips(rc.bottom)
	);
	pRT->DrawText(
		wszText,
		cTextLength,
		pTextFormat,
		layoutRect,
		pBrush
	);
	return hr;
}

HRESULT DWrite::DrawContent(HWND hwnd, LPCWSTR wszText, UINT32 cTextLength, BOOL bForceUpdate) {
	HRESULT hr = S_OK;
	// je�li nie ma fabryki, to j� zr�b
	if (!pDWriteFactory) {
		hr = CreateIndependentResources();
	}
	// je�li nie ma celu renderowania, to go zr�b
	if (SUCCEEDED(hr)) {
		if (!pRT) {
			hr = CreateDependentResources(hwnd);
		}
		if (bForceUpdate && pRT) {
			pRT->Resize(D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top));
			
		}
	}
	if (SUCCEEDED(hr)) {
		pRT->BeginDraw();
		pRT->SetTransform(D2D1::IdentityMatrix()); // nie obracamy tekstu
		pRT->Clear(D2D1::ColorF(D2D1::ColorF::White));
		hr = DrawText(wszText, cTextLength);
		if (SUCCEEDED(hr)) {
			hr = pRT->EndDraw();
		}
	}
	if (FAILED(hr))
	{
		DiscardRecources();
	}
	return hr;
}


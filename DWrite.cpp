#include "DWrite.h"

/**
 * Utwórz zasoby niezale¿ne od urz¹dzenia. Cz³onek klasy
 * 
 * \return zwyk³y
 */
HRESULT DWrite::CreateIndependentResources(){
	HRESULT hr = S_OK;
	// zrób fabrykê wszystkich obiektów D2D
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pD2DFactory);
	// zrób fabrykê wszystkich obiektów DWrite
	if (SUCCEEDED(hr)) {
		hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	}
	if (SUCCEEDED(hr)) {
		// zrób format
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
	//// sformatuj go wstênie 
	//if (SUCCEEDED(hr)) {
	//	hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	//}
	//if (SUCCEEDED(hr)) {
	//	hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	//}
	return hr;
}

/**
 * Utwórz zasoby uzale¿nione od sprzêtu. Cz³onek klasy
 * 
 * \param hwnd - uchwyt do okna
 * \return zwyk³y
 */
HRESULT DWrite::CreateDependentResources(HWND hwnd) {
	HRESULT hr = S_OK;
	GetClientRect(hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);
	// jeœli nie ma celu renderowania, to go zrób
	// jeœli jest, to u¿yj istniej¹cego
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
	// utwórz prostok¹t D2D
	// dokumentacja mia³a osobne wartoœci dla X i Y
	// jeœli tekst bêdzie mia³ intryguj¹cy kszta³t, to tu jest problem
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
	// jeœli nie ma fabryki, to j¹ zrób
	if (!pDWriteFactory) {
		hr = CreateIndependentResources();
	}
	// jeœli nie ma celu renderowania, to go zrób
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


#include "stdafx.h"
#include "Graphics.h"

// =========================================================
// ctor
// =========================================================
Graphics::Graphics()
{
	isInitialized = FALSE;
	pFactory = NULL;
	pRenderTarget = NULL;
	callbackCreateResources = NULL;
	pSolidBrush = NULL;
	pWriteFactory = NULL;
	pTextFormat = NULL;
}

// =========================================================
// dtor
// =========================================================
Graphics::~Graphics()
{
	SafeRelease(&pFactory);
	SafeRelease(&pRenderTarget);
	SafeRelease(&pSolidBrush);
	SafeRelease(&pWriteFactory);
	SafeRelease(&pTextFormat);
	isInitialized = FALSE;
}

// =========================================================
// Initializes this instance
// =========================================================
BOOL Graphics::Initialize(HWND hWnd)
{
	HRESULT hr = S_OK;

	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);
	if (!SUCCEEDED(hr))
	{
		OutputDebugStringA("Graphics::Initialize -> Cannot initialize D2D Factory.\n");
		return FALSE;
	}
	hTarget = hWnd;

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), (IUnknown**)&pWriteFactory);
	if (!SUCCEEDED(hr))
	{
		OutputDebugStringA("Graphics::Initialize -> Cannot initialize DWrite Factory.\n");
		SafeRelease(&pFactory);
		return FALSE;
	}

	hr = pWriteFactory->CreateTextFormat(L"Calibri", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 12.0f, L"en-us", &pTextFormat);
	if (!SUCCEEDED(hr))
	{
		OutputDebugStringA("Graphics::Initialize -> Cannot create default DWrite Text Format.\n");
		SafeRelease(&pFactory);
		SafeRelease(&pWriteFactory);
		return FALSE;
	}
	
	isInitialized = TRUE;
	OutputDebugStringA("Graphics::Initialize -> OK!\n");
	return TRUE;
}

// =========================================================
// Begins drawing process. Checks for render target availability first. 
// Only continue drawing if DRAWSTATUS_OK is returned.
// =========================================================
DRAWSTATUS Graphics::BeginDraw()
{
	if (!pRenderTarget)
	{
		BOOL statusOK = TRUE;
		if (InitRenderTarget() && callbackCreateResources) callbackCreateResources(*pRenderTarget, statusOK);

		if (!statusOK)
		{
			OutputDebugStringA("Graphics::BeginDraw() -> There was an error to recreate a device resource.\n");
			return DRAWSTATUS_RECREATE_ERROR;
		}

		if (!pRenderTarget)
		{
			OutputDebugStringA("Graphics::BeginDraw() -> Render target pointer is still invalid even after attempting to recreate resources.\n");
			return DRAWSTATUS_RT_NULL;
		}
	}

	// everything is fine, so begin drawing
	pRenderTarget->BeginDraw();
	return DRAWSTATUS_OK;
}

// =========================================================
// End of the drawing block. If DRAWSTATUS_INVOKE_DISCARD is returned,
// discard all resources for recreating later as device was possibly lost.
// =========================================================
DRAWSTATUS Graphics::EndDraw()
{
	// end the drawing
	HRESULT hr = pRenderTarget->EndDraw();
	if (hr == D2DERR_RECREATE_TARGET) return DRAWSTATUS_INVOKE_DISCARD;
	return DRAWSTATUS_OK;
}

// =========================================================
// Initializes the render target
// =========================================================
BOOL Graphics::InitRenderTarget()
{
	if (!isInitialized)
	{
		OutputDebugStringA("Graphics::InitRenderTarget() -> Attempted to init render target when uninitialized.\n");
		return FALSE;
	}

	if (pRenderTarget)
	{
		// release any previous render target if it has not done so
		SafeRelease(&pSolidBrush);
		SafeRelease(&pRenderTarget);
	}

	RECT rc;
	GetClientRect(hTarget, &rc);

	HRESULT hr = pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(D2D1_RENDER_TARGET_TYPE_DEFAULT, 
		D2D1::PixelFormat(DXGI_FORMAT_R8G8B8A8_UNORM, D2D1_ALPHA_MODE_PREMULTIPLIED)), 
		D2D1::HwndRenderTargetProperties(hTarget, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)), &pRenderTarget);

	if (!SUCCEEDED(hr))
	{
		OutputDebugStringA("Graphics::InitRenderTarget() -> Unable to init render target.\n");
		return FALSE;
	}

	hr = pRenderTarget->CreateSolidColorBrush(colorTemp, &pSolidBrush);
	if (!SUCCEEDED(hr))
	{
		OutputDebugStringA("Graphics::InitRenderTarget() -> Unable to init brush.\n");
		return FALSE;
	}

	return TRUE;
}

// =========================================================
// Resizes the render target
// =========================================================
void Graphics::ResizeRenderTarget(UINT width, UINT height)
{
	if (pRenderTarget) pRenderTarget->Resize(D2D1::SizeU(width, height));
}

// =========================================================
// Sets the internal colour variable
// =========================================================
void Graphics::SetColor(float r, float g, float b)
{
	colorTemp = D2D1::ColorF(r, g, b);

	if (pSolidBrush) pSolidBrush->SetColor(colorTemp);
}

void Graphics::SetColor(D2D1::ColorF::Enum enumColor)
{
	colorTemp = D2D1::ColorF(enumColor);
	if (pSolidBrush) pSolidBrush->SetColor(colorTemp);
}

// =========================================================
// Sets the internal alpha variable
// =========================================================
void Graphics::SetAlpha(float a)
{
	colorTemp.a = a;
	if (pSolidBrush) pSolidBrush->SetOpacity(colorTemp.a);
}

// =========================================================
// Clears render target for drawing
// =========================================================
void Graphics::Clear()
{
	pRenderTarget->Clear(colorTemp);
}

// =========================================================
// Draws the given text on screen
// =========================================================
void Graphics::DrawString(LPCWSTR text, float x, float y, float w, float h, IDWriteTextFormat *pFormat)
{
	if (!isInitialized || !pRenderTarget) return;
	if (!pFormat) pFormat = pTextFormat;
	
	rectTemp.left = x;
	rectTemp.top = y;
	rectTemp.right = x + w;
	rectTemp.bottom = y + h;
	pRenderTarget->DrawTextW(text, wcslen(text), pFormat, rectTemp, pSolidBrush);

}

// =========================================================
// Draws the specified rectangle
// =========================================================
void Graphics::DrawRectangle(float x, float y, float w, float h, float strokeWidth, BOOL filled)
{
	if (!isInitialized || !pRenderTarget) return;
	rectTemp.left = x;
	rectTemp.top = y;
	rectTemp.right = x + w;
	rectTemp.bottom = y + h;
	if (!filled)
		pRenderTarget->DrawRectangle(rectTemp, pSolidBrush, strokeWidth);
	else
		pRenderTarget->FillRectangle(rectTemp, pSolidBrush);
}

// =========================================================
// Draws the specified sprite on screen
// =========================================================
void Graphics::DrawSprite(D2D1_RECT_F &src, D2D1_RECT_F &dst, ID2D1Bitmap *pBitmap)
{
	if (!isInitialized || !pRenderTarget) return;
	pRenderTarget->DrawBitmap(pBitmap, dst, colorTemp.a, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, src);
}

// =========================================================
// Create a text format to be used in draw string
// =========================================================
BOOL Graphics::CreateTextFormat(LPCWSTR fontFamily, FLOAT fontSize, IDWriteTextFormat **pFormat, DWRITE_FONT_WEIGHT fontWeight,
	DWRITE_FONT_STYLE fontStyle, DWRITE_FONT_STRETCH fontStretch)
{
	if (!isInitialized || !pWriteFactory) return FALSE;

	pWriteFactory->CreateTextFormat(fontFamily, NULL, fontWeight, fontStyle, fontStretch, fontSize, L"en-us", pFormat);

	return TRUE;
}
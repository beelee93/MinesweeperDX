#pragma once
#include <dwrite.h>
#include <d2d1.h>
#include "Loader.h"

typedef unsigned int				DRAWSTATUS;
#define DRAWSTATUS_OK				0x00
#define DRAWSTATUS_RT_NULL			0x01
#define DRAWSTATUS_INVOKE_DISCARD	0x02
#define DRAWSTATUS_RECREATE_ERROR	0x03

//Graphics engine wrapper
class Graphics
{
	friend class Loader;
public:
	void					(*callbackCreateResources)(const ID2D1HwndRenderTarget&, BOOL&);

	Graphics();
	~Graphics();
	BOOL					Initialize(HWND);
	DRAWSTATUS				BeginDraw();
	DRAWSTATUS				EndDraw();
	void					ResizeRenderTarget(UINT, UINT);

	void					SetColor(float r, float g, float b);
	void					SetColor(D2D1::ColorF::Enum enumColor);
	void					SetAlpha(float a=1.0f);
	void					Clear();
	BOOL					Graphics::CreateTextFormat(LPCWSTR fontFamily, FLOAT fontSize, IDWriteTextFormat **pFormat, DWRITE_FONT_WEIGHT fontWeight = DWRITE_FONT_WEIGHT_NORMAL,
								DWRITE_FONT_STYLE fontStyle = DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH fontStretch = DWRITE_FONT_STRETCH_NORMAL);
	void					DrawRectangle(float x, float y, float w, float h, float strokeWidth = 1.0f, BOOL filled=0);
	void					DrawString(LPCWSTR text, float x, float y, float w, float h, IDWriteTextFormat *pFormat = NULL);
	void					DrawSprite(D2D1_RECT_F &src, D2D1_RECT_F &dst, ID2D1Bitmap *pBitmap);

private:
	BOOL					isInitialized;
	ID2D1Factory			*pFactory;
	ID2D1SolidColorBrush	*pSolidBrush;
	ID2D1HwndRenderTarget	*pRenderTarget;
	IDWriteFactory			*pWriteFactory;
	IDWriteTextFormat		*pTextFormat;
	HWND					hTarget;

	BOOL					InitRenderTarget();
	D2D1_COLOR_F			colorTemp;
	D2D1_RECT_F				rectTemp;

};


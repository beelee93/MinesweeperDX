#pragma once
#include <wincodec.h>
#include "Graphics.h"

// Handles the loading of assets
class Loader
{
public:
	Loader();
	~Loader();
	BOOL		LoadBitmapFromFile(LPCWSTR filename, const Graphics* graphicsWrapper, ID2D1Bitmap** ppBitmap);
	BOOL		LoadBitmapFromResource(LPCWSTR resourceId, LPCWSTR resourceType, const Graphics* graphicsWrapper, ID2D1Bitmap** ppBitmap);
	BOOL		Initialize();

private:
	BOOL				isInitialized;
	IWICImagingFactory	*pFactory;
};


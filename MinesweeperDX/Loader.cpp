#include "stdafx.h"
#include "Loader.h"

#define CHECK(x) if(!SUCCEEDED(hr)) goto x

// =========================================================
// ctor
// =========================================================
Loader::Loader()
{
	isInitialized = 0;
	pFactory = NULL;
}

// =========================================================
// dtor
// =========================================================
Loader::~Loader()
{
	SafeRelease(&pFactory);
}

// =========================================================
// Initializes this instance
// =========================================================
BOOL Loader::Initialize()
{
	HRESULT hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pFactory));
	if (SUCCEEDED(hr))
	{
		isInitialized = TRUE;
		OutputDebugStringA("Loader::Initialize -> OK!\n");
		return TRUE;
	}
	else
	{
		OutputDebugStringA("Cannot initialize WIC Factory.\n");
		return FALSE;
	}
}

// =========================================================
// Load the specified bitmap from file into D2D bitmap
// =========================================================
BOOL Loader::LoadBitmapFromFile(LPCWSTR filename, const Graphics* graphicsWrapper, ID2D1Bitmap** ppBitmap)
{
	if (!isInitialized)
		return FALSE;

	HRESULT hr;

	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICFormatConverter *pConverter = NULL;

	hr = pFactory->CreateDecoderFromFilename(filename, NULL, GENERIC_READ, WICDecodeMetadataCacheOnDemand, &pDecoder);
	CHECK(failedDecoder);

	hr = pDecoder->GetFrame(0, &pSource);
	CHECK(failedGetFrame);

	hr = pFactory->CreateFormatConverter(&pConverter);
	CHECK(failedConverter);

	hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeMedianCut);
	CHECK(failedConversion);

	hr = graphicsWrapper->pRenderTarget->CreateBitmapFromWicBitmap(pSource, ppBitmap);
	CHECK(failedBitmap);

	return TRUE;

failedBitmap:
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to create bitmap.\n");
failedConversion:
	pConverter->Release();
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to initialise format converter.\n");
failedConverter:
	pSource->Release();
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to create format converter.\n");
failedGetFrame:
	pDecoder->Release();
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to get frame 0 from source bitmap.\n");
failedDecoder:
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to create bitmap decoder.\n");
	return FALSE;
}

// =========================================================
// Load the specified bitmap from file into D2D bitmap
// =========================================================
BOOL Loader::LoadBitmapFromResource(LPCWSTR resourceId, LPCWSTR resourceType, const Graphics* graphicsWrapper, ID2D1Bitmap** ppBitmap)
{
	if (!isInitialized)
		return FALSE;

	HRESULT hr;

	IWICStream *pStream = NULL;
	IWICBitmapDecoder *pDecoder = NULL;
	IWICBitmapFrameDecode *pSource = NULL;
	IWICFormatConverter *pConverter = NULL;

	HRSRC imageResHandle = NULL;
	HGLOBAL imageResDataHandle = NULL;
	void *pImageFile = NULL;
	DWORD imageFileSize = 0;

	HMODULE module = GetModuleHandle(0);

	// Locate the resource
	imageResHandle = FindResource(module, resourceId, resourceType);
	if (!imageResHandle)
	{
		OutputDebugStringA("Loader::LoadBitmapFromResource -> Failed to locate resource.\n");
		return FALSE;
	}

	imageResDataHandle = LoadResource(module, imageResHandle);
	if (!imageResDataHandle)
	{
		OutputDebugStringA("Loader::LoadBitmapFromResource -> Failed to load resource.\n");
		return FALSE;
	}

	// lock resource to get pointer to data
	pImageFile = LockResource(imageResDataHandle);
	if (!pImageFile)
	{
		OutputDebugStringA("Loader::LoadBitmapFromResource -> Failed to lock resource for processing.\n");
		return FALSE;
	}

	// get the size of the image
	imageFileSize = SizeofResource(module, imageResHandle);
	if (!imageFileSize)
	{
		OutputDebugStringA("Loader::LoadBitmapFromResource -> Failed to obtain size of image resource.\n");
		return FALSE;
	}

	// now that we have everything, we create the stream
	hr = pFactory->CreateStream(&pStream);
	CHECK(failedCreateStream);

	// initialise the stream
	hr = pStream->InitializeFromMemory((byte*)pImageFile, imageFileSize);
	CHECK(failedStreamInit);

	hr = pFactory->CreateDecoderFromStream(pStream, NULL, WICDecodeMetadataCacheOnDemand, &pDecoder);
	CHECK(failedDecoder);

	hr = pDecoder->GetFrame(0, &pSource);
	CHECK(failedGetFrame);

	hr = pFactory->CreateFormatConverter(&pConverter);
	CHECK(failedConverter);

	hr = pConverter->Initialize(pSource, GUID_WICPixelFormat32bppPBGRA, WICBitmapDitherTypeNone, NULL, 0.0, WICBitmapPaletteTypeMedianCut);
	CHECK(failedConversion);

	hr = graphicsWrapper->pRenderTarget->CreateBitmapFromWicBitmap(pConverter, ppBitmap);
	CHECK(failedBitmap);

	return TRUE;
	
failedBitmap:
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to create bitmap.\n");
failedConversion:
	pConverter->Release();
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to initialise format converter.\n");
failedConverter:
	pSource->Release();
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to create format converter.\n");
failedGetFrame:
	pDecoder->Release();
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to get frame 0 from source bitmap.\n");
failedDecoder:
	pStream->Release();
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to create bitmap decoder.\n");
	return FALSE;
failedStreamInit:
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to initialize WIC stream.\n");
	return FALSE;
failedCreateStream:
	OutputDebugStringA("Loader::LoadBitmapFromFile -> Failed to create WIC stream.\n");
	return FALSE;
}

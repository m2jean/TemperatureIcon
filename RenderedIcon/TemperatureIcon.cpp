#include "TemperatureIcon.h"

#include <strsafe.h>
#include <stdlib.h>
#include <d2d1.h>
#include <d2d1helper.h>
#include <PathCch.h>
#include <MagickCore/MagickCore.h>
#include <MagickCore/utility.h>
#include <MagickCore/constitute.h>
#include <wincodec.h>
#include <dwrite.h>

const UINT TemperatureIcon::g_bitmapSizes[] = { 256, 128, 64, 32, 16 };

TemperatureIcon::TemperatureIcon() {
	HRESULT hr;

	const D2D1_FACTORY_OPTIONS factoryOption{ D2D1_DEBUG_LEVEL_WARNING };
	hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, __uuidof(ID2D1Factory), &factoryOption, (void**)&m_dx2Factory);

	if (!IsMagickCoreInstantiated())
	{
		m_magickInstantiatedByThis = true;

		WCHAR wpath[MAX_PATH];
		hr = GetModuleFileNameW(NULL, wpath, MAX_PATH);
		hr = PathCchRemoveFileSpec(wpath, MAX_PATH);

		CHAR path[MAX_PATH];
		size_t pathCount;
		hr = wcstombs_s(&pathCount, path, wpath, MAX_PATH);

		MagickCoreGenesis(path, MagickFalse);
	}
}

TemperatureIcon::~TemperatureIcon() {
	for (int i = 0; i < g_numBitmapSizes; ++i)
		m_pBitmaps[i]->Release();

	if (m_magickInstantiatedByThis && IsMagickCoreInstantiated())
		MagickCoreTerminus();

	m_dx2Factory->Release();
}


void TemperatureIcon::WriteWICBitmap(IWICImagingFactory* pWicFactory, IWICBitmap* pBitmap, WCHAR* filename) {
	HRESULT hr;

	IWICStream* pStream;
	hr = pWicFactory->CreateStream(&pStream);

	hr = pStream->InitializeFromFilename(filename, GENERIC_WRITE);

	IWICBitmapEncoder* pEncoder = NULL;
	hr = pWicFactory->CreateEncoder(GUID_ContainerFormatBmp, NULL, &pEncoder);

	hr = pEncoder->Initialize(pStream, WICBitmapEncoderNoCache);

	IWICBitmapFrameEncode* pBitmapFrame = NULL;
	IPropertyBag2* pPropertybag = NULL;
	hr = pEncoder->CreateNewFrame(&pBitmapFrame, &pPropertybag);

	hr = pBitmapFrame->Initialize(pPropertybag);
	hr = pBitmapFrame->SetSize(256, 256);
	GUID pixelFormat = GUID_WICPixelFormat32bppBGRA;
	hr = pBitmapFrame->SetPixelFormat(&pixelFormat);

	WICRect rect; rect.X = rect.Y = 0; rect.Height = rect.Width = 256;
	hr = pBitmapFrame->WriteSource(pBitmap, &rect);

	hr = pBitmapFrame->Commit();
	hr = pEncoder->Commit();

	pPropertybag->Release();
	pBitmapFrame->Release();
	pEncoder->Release();
	pStream->Release();
}

void TemperatureIcon::DrawTemperature(IDWriteFactory* pDwFactory, IWICBitmap* pBitmap, FLOAT fontSize, UINT temperature, WCHAR unit)
{
	HRESULT hr;

	WCHAR text[8];
	_ultow_s(temperature, text, 10);
	size_t tempEnd = 0;
	StringCchLengthW(text, ARRAYSIZE(text), &tempEnd);
	text[tempEnd] = unit;
	text[tempEnd + 1] = NULL;

	UINT size;
	pBitmap->GetSize(&size, &size);

	ID2D1RenderTarget* pTarget;
	D2D1_RENDER_TARGET_PROPERTIES properties = D2D1::RenderTargetProperties();
	properties.dpiX = properties.dpiY = 96;
	hr = m_dx2Factory->CreateWicBitmapRenderTarget(pBitmap, properties, &pTarget);

	IDWriteTextFormat* pTextFormat;
	hr = pDwFactory->CreateTextFormat(L"Calibri", NULL, DWRITE_FONT_WEIGHT_REGULAR, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, fontSize, L"en-us", &pTextFormat);
	hr = pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	hr = pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
	hr = pTextFormat->SetWordWrapping(DWRITE_WORD_WRAPPING_NO_WRAP);

	IDWriteTextLayout* pTextLayout;
	hr = pDwFactory->CreateTextLayout(text, tempEnd + 1, pTextFormat, size, size, &pTextLayout);
	pTextLayout->SetFontSize(fontSize / 2, DWRITE_TEXT_RANGE{ (UINT)tempEnd, 1 });

	ID2D1SolidColorBrush* pBrush;
	pTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);

	pTarget->BeginDraw();
	pTarget->Clear(D2D1::ColorF(1.f, 1.f, 1.f, 0.f));
	pTarget->DrawTextLayout(D2D1::Point2F(0, 0), pTextLayout, pBrush);
	pTarget->EndDraw();

	pBrush->Release();
	pTextLayout->Release();
	pTextFormat->Release();
	pTarget->Release();
}

Image* TemperatureIcon::ConvertToMagickImage(IWICBitmap* pBitmap)
{
	HRESULT hr;
	UINT size;
	pBitmap->GetSize(&size, &size);

	IWICBitmapSource* pBitmapOut;
	hr = WICConvertBitmapSource(GUID_WICPixelFormat32bppBGRA, pBitmap, &pBitmapOut);

	int cntBytes = size * size * 4;
	byte* pixels = new byte[cntBytes];
	WICRect rcBitmap; rcBitmap.X = rcBitmap.Y = 0; rcBitmap.Width = rcBitmap.Height = size;

	hr = pBitmapOut->CopyPixels(&rcBitmap, size * 4, cntBytes, pixels);

	ExceptionInfo* exception = AcquireExceptionInfo();
	char pixelFormat[] = "BGRA";
	Image* pImage = ConstituteImage(size, size, pixelFormat, CharPixel, pixels, exception);

	DestroyExceptionInfo(exception);
	delete[] pixels;
	pBitmapOut->Release();

	return pImage;
}


void TemperatureIcon::CreateTemperatureIcon()
{
	HRESULT hr;

	IWICImagingFactory* pWicFactory = NULL;
	hr = CoCreateInstance(CLSID_WICImagingFactory, NULL, CLSCTX_INPROC_SERVER, IID_PPV_ARGS(&pWicFactory));

	IDWriteFactory* pDwFactory;
	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, _uuidof(IDWriteFactory), (IUnknown**)&pDwFactory);

	Image* icos = NewImageList();
	for (int i = 0; i < g_numBitmapSizes; ++i)
	{
		UINT size = g_bitmapSizes[i];
		FLOAT fontSize = size * 0.8f;

		IWICBitmap* pBitmap;
		pWicFactory->CreateBitmap(size, size, GUID_WICPixelFormat32bppPBGRA, WICBitmapCacheOnDemand, &pBitmap);
		m_pBitmaps[i] = pBitmap;

		DrawTemperature(pDwFactory, pBitmap, fontSize, 12U, L'F');

		Image* pImage = ConvertToMagickImage(pBitmap);
		AppendImageToList(&icos, pImage);
	}

	ExceptionInfo* exception = AcquireExceptionInfo();

	CHAR icoName[MAX_PATH];
	wcstombs_s(NULL, icoName, L"icos.ico", MAX_PATH);

	ImageInfo* imageInfo = CloneImageInfo(NULL);
	MagickBooleanType result = WriteImages(imageInfo, icos, icoName, exception);

	DestroyExceptionInfo(exception);
	DestroyImageInfo(imageInfo);
	DestroyImageList(icos);

	pDwFactory->Release();
	pWicFactory->Release();
}

void TemperatureIcon::DrawIcons(HWND hWnd) {
	RECT rc;
	GetClientRect(hWnd, &rc);
	ID2D1HwndRenderTarget* pHwndRenderTarget;
	m_dx2Factory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top)), &pHwndRenderTarget);

	ID2D1SolidColorBrush* pBrush;
	pHwndRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::Black), &pBrush);

	pHwndRenderTarget->BeginDraw();
	pHwndRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Gray));

	FLOAT x = 0, y = 0;
	for (IWICBitmap* pBitmap : m_pBitmaps)
	{
		UINT size;
		pBitmap->GetSize(&size, &size);

		ID2D1Bitmap* pD2D1Bitmap;
		pHwndRenderTarget->CreateBitmapFromWicBitmap(pBitmap, &pD2D1Bitmap);
		D2D1_RECT_F rect = D2D1::RectF(x, y, x + size, y + size);
		pHwndRenderTarget->FillRectangle(rect, pBrush);
		pHwndRenderTarget->DrawBitmap(pD2D1Bitmap, rect, 1.f, D2D1_BITMAP_INTERPOLATION_MODE_LINEAR, D2D1::RectF(0, 0, size, size));

		pD2D1Bitmap->Release();

		x += size;
	}

	pHwndRenderTarget->EndDraw();

	pBrush->Release();
	pHwndRenderTarget->Release();
}

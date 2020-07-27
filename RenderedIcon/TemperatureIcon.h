#pragma once

#include <wincodec.h>
#include <d2d1.h>
#include <dwrite.h>
#include <MagickCore/MagickCore.h>

class TemperatureIcon
{
private:
    static const UINT g_numBitmapSizes = 5;
    static const UINT g_bitmapSizes[g_numBitmapSizes];

public:
    TemperatureIcon();
    ~TemperatureIcon();

    void CreateTemperatureIcon();
    void DrawIcons(HWND hWnd);

private:
    bool m_magickInstantiatedByThis = false;
    IWICBitmap* m_pBitmaps[g_numBitmapSizes];
    ID2D1Factory* m_dx2Factory;

    void WriteWICBitmap(IWICImagingFactory* pWicFactory, IWICBitmap* pBitmap, WCHAR* filename);
    void DrawTemperature(IDWriteFactory* pDwFactory, IWICBitmap* pBitmap, FLOAT fontSize, UINT temperature, WCHAR unit);
    Image* ConvertToMagickImage(IWICBitmap* pBitmap);
};

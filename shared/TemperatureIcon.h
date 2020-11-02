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

    /// <summary>
    /// Create and output an ICO file with temperature icons of various sizes
    /// </summary>
    void CreateTemperatureIconResource(INT temperature, WCHAR unit, WCHAR *outputPath);

    /// <summary>
    /// If called after CreateTemperatureIconResource, draw the icons of different sizes to the given window
    /// </summary>
    void DrawIcons(HWND hWnd);

    /// <summary>
    /// Create a square temperature icon with given size
    /// </summary>
    /// <param name="size">The size of width and height in pixel</param>
    HICON CreateTemperatureIcon(INT temperature, WCHAR unit, UINT size);

private:
    bool m_magickInstantiatedByThis = false;
    IWICBitmap* m_pBitmaps[g_numBitmapSizes];
    ID2D1Factory* m_dx2Factory;

    void WriteWICBitmap(IWICImagingFactory* pWicFactory, IWICBitmap* pBitmap, WCHAR* filename);
    void DrawTemperature(IDWriteFactory* pDwFactory, IWICBitmap* pBitmap, FLOAT fontSize, INT temperature, WCHAR unit);
    Image* ConvertToMagickImage(IWICBitmap* pBitmap);
};

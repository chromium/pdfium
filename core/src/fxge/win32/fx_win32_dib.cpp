// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#if _FX_OS_ == _FX_WIN32_DESKTOP_ ||  _FX_OS_ == _FX_WIN64_
#include <windows.h>
#include "../../../include/fxge/fx_ge_win32.h"
#include "win32_int.h"
CFX_ByteString CFX_WindowsDIB::GetBitmapInfo(const CFX_DIBitmap* pBitmap)
{
    CFX_ByteString result;
    int len = sizeof (BITMAPINFOHEADER);
    if (pBitmap->GetBPP() == 1 || pBitmap->GetBPP() == 8) {
        len += sizeof (DWORD) * (int)(1 << pBitmap->GetBPP());
    }
    BITMAPINFOHEADER* pbmih = (BITMAPINFOHEADER*)result.GetBuffer(len);
    FXSYS_memset32(pbmih, 0, sizeof (BITMAPINFOHEADER));
    pbmih->biSize = sizeof(BITMAPINFOHEADER);
    pbmih->biBitCount = pBitmap->GetBPP();
    pbmih->biCompression = BI_RGB;
    pbmih->biHeight = -(int)pBitmap->GetHeight();
    pbmih->biPlanes = 1;
    pbmih->biWidth = pBitmap->GetWidth();
    if (pBitmap->GetBPP() == 8) {
        FX_DWORD* pPalette = (FX_DWORD*)(pbmih + 1);
        if (pBitmap->GetPalette() == NULL) {
            for (int i = 0; i < 256; i ++) {
                pPalette[i] = i * 0x010101;
            }
        } else {
            for (int i = 0; i < 256; i ++) {
                pPalette[i] = pBitmap->GetPalette()[i];
            }
        }
    }
    if (pBitmap->GetBPP() == 1) {
        FX_DWORD* pPalette = (FX_DWORD*)(pbmih + 1);
        if (pBitmap->GetPalette() == NULL) {
            pPalette[0] = 0;
            pPalette[1] = 0xffffff;
        } else {
            pPalette[0] = pBitmap->GetPalette()[0];
            pPalette[1] = pBitmap->GetPalette()[1];
        }
    }
    result.ReleaseBuffer(len);
    return result;
}
CFX_DIBitmap* _FX_WindowsDIB_LoadFromBuf(BITMAPINFO* pbmi, LPVOID pData, FX_BOOL bAlpha)
{
    int width = pbmi->bmiHeader.biWidth;
    int height = pbmi->bmiHeader.biHeight;
    BOOL bBottomUp = TRUE;
    if (height < 0) {
        height = -height;
        bBottomUp = FALSE;
    }
    int pitch = (width * pbmi->bmiHeader.biBitCount + 31) / 32 * 4;
    CFX_DIBitmap* pBitmap = FX_NEW CFX_DIBitmap;
    if (!pBitmap) {
        return NULL;
    }
    FXDIB_Format format = bAlpha ? (FXDIB_Format)(pbmi->bmiHeader.biBitCount + 0x200) : (FXDIB_Format)pbmi->bmiHeader.biBitCount;
    FX_BOOL ret = pBitmap->Create(width, height, format);
    if (!ret) {
        delete pBitmap;
        return NULL;
    }
    FXSYS_memcpy32(pBitmap->GetBuffer(), pData, pitch * height);
    if (bBottomUp) {
        FX_LPBYTE temp_buf = FX_Alloc(FX_BYTE, pitch);
        if (!temp_buf) {
            if (pBitmap) {
                delete pBitmap;
            }
            return NULL;
        }
        int top = 0, bottom = height - 1;
        while (top < bottom) {
            FXSYS_memcpy32(temp_buf, pBitmap->GetBuffer() + top * pitch, pitch);
            FXSYS_memcpy32(pBitmap->GetBuffer() + top * pitch, pBitmap->GetBuffer() + bottom * pitch, pitch);
            FXSYS_memcpy32(pBitmap->GetBuffer() + bottom * pitch, temp_buf, pitch);
            top ++;
            bottom --;
        }
        FX_Free(temp_buf);
        temp_buf = NULL;
    }
    if (pbmi->bmiHeader.biBitCount == 1) {
        for (int i = 0; i < 2; i ++) {
            pBitmap->SetPaletteEntry(i, ((FX_DWORD*)pbmi->bmiColors)[i] | 0xff000000);
        }
    } else if (pbmi->bmiHeader.biBitCount == 8) {
        for (int i = 0; i < 256; i ++) {
            pBitmap->SetPaletteEntry(i, ((FX_DWORD*)pbmi->bmiColors)[i] | 0xff000000);
        }
    }
    return pBitmap;
}
CFX_DIBitmap* CFX_WindowsDIB::LoadFromBuf(BITMAPINFO* pbmi, LPVOID pData)
{
    return _FX_WindowsDIB_LoadFromBuf(pbmi, pData, FALSE);
}
HBITMAP	CFX_WindowsDIB::GetDDBitmap(const CFX_DIBitmap* pBitmap, HDC hDC)
{
    CFX_ByteString info = GetBitmapInfo(pBitmap);
    HBITMAP hBitmap = NULL;
    hBitmap = CreateDIBitmap(hDC, (BITMAPINFOHEADER*)info.c_str(), CBM_INIT,
        pBitmap->GetBuffer(), (BITMAPINFO*)info.c_str(), DIB_RGB_COLORS);
    return hBitmap;
}
void GetBitmapSize(HBITMAP hBitmap, int& w, int& h)
{
    BITMAP bmp;
    GetObject(hBitmap, sizeof bmp, &bmp);
    w = bmp.bmWidth;
    h = bmp.bmHeight;
}
CFX_DIBitmap* CFX_WindowsDIB::LoadFromFile(FX_LPCWSTR filename)
{
    CWin32Platform* pPlatform = (CWin32Platform*)CFX_GEModule::Get()->GetPlatformData();
    if (pPlatform->m_GdiplusExt.IsAvailable()) {
        WINDIB_Open_Args_ args;
        args.flags = WINDIB_OPEN_PATHNAME;
        args.path_name = filename;
        return pPlatform->m_GdiplusExt.LoadDIBitmap(args);
    }
    HBITMAP hBitmap = (HBITMAP)LoadImageW(NULL, (wchar_t*)filename, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBitmap == NULL) {
        return NULL;
    }
    HDC hDC = CreateCompatibleDC(NULL);
    int width, height;
    GetBitmapSize(hBitmap, width, height);
    CFX_DIBitmap* pDIBitmap = FX_NEW CFX_DIBitmap;
    if (!pDIBitmap) {
        DeleteDC(hDC);
        return NULL;
    }
    if (!pDIBitmap->Create(width, height, FXDIB_Rgb)) {
        delete pDIBitmap;
        DeleteDC(hDC);
        return NULL;
    }
    CFX_ByteString info = GetBitmapInfo(pDIBitmap);
    int ret = GetDIBits(hDC, hBitmap, 0, height, pDIBitmap->GetBuffer(), (BITMAPINFO*)info.c_str(), DIB_RGB_COLORS);
    if (!ret) {
        if (pDIBitmap) {
            delete pDIBitmap;
        }
        pDIBitmap = NULL;
    }
    DeleteDC(hDC);
    return pDIBitmap;
}
CFX_DIBitmap* CFX_WindowsDIB::LoadDIBitmap(WINDIB_Open_Args_ args)
{
    CWin32Platform* pPlatform = (CWin32Platform*)CFX_GEModule::Get()->GetPlatformData();
    if (pPlatform->m_GdiplusExt.IsAvailable()) {
        return pPlatform->m_GdiplusExt.LoadDIBitmap(args);
    } else if (args.flags == WINDIB_OPEN_MEMORY) {
        return NULL;
    }
    HBITMAP hBitmap = (HBITMAP)LoadImageW(NULL, (wchar_t*)args.path_name, IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    if (hBitmap == NULL) {
        return NULL;
    }
    HDC hDC = CreateCompatibleDC(NULL);
    int width, height;
    GetBitmapSize(hBitmap, width, height);
    CFX_DIBitmap* pDIBitmap = FX_NEW CFX_DIBitmap;
    if (!pDIBitmap) {
        DeleteDC(hDC);
        return NULL;
    }
    if (!pDIBitmap->Create(width, height, FXDIB_Rgb)) {
        delete pDIBitmap;
        DeleteDC(hDC);
        return NULL;
    }
    CFX_ByteString info = GetBitmapInfo(pDIBitmap);
    int ret = GetDIBits(hDC, hBitmap, 0, height, pDIBitmap->GetBuffer(), (BITMAPINFO*)info.c_str(), DIB_RGB_COLORS);
    if (!ret) {
        if (pDIBitmap) {
            delete pDIBitmap;
        }
        pDIBitmap = NULL;
    }
    DeleteDC(hDC);
    return pDIBitmap;
}
CFX_DIBitmap* CFX_WindowsDIB::LoadFromDDB(HDC hDC, HBITMAP hBitmap, FX_DWORD* pPalette, FX_DWORD palsize)
{
    FX_BOOL bCreatedDC = hDC == NULL;
    if (hDC == NULL) {
        hDC = CreateCompatibleDC(NULL);
    }
    BITMAPINFOHEADER bmih;
    FXSYS_memset32(&bmih, 0, sizeof bmih);
    bmih.biSize = sizeof bmih;
    GetDIBits(hDC, hBitmap, 0, 0, NULL, (BITMAPINFO*)&bmih, DIB_RGB_COLORS);
    int width = bmih.biWidth;
    int height = abs(bmih.biHeight);
    bmih.biHeight = -height;
    bmih.biCompression = BI_RGB;
    CFX_DIBitmap* pDIBitmap = FX_NEW CFX_DIBitmap;
    if (!pDIBitmap) {
        return NULL;
    }
    int ret = 0;
    if (bmih.biBitCount == 1 || bmih.biBitCount == 8) {
        int size = sizeof (BITMAPINFOHEADER) + 8;
        if (bmih.biBitCount == 8) {
            size += sizeof (FX_DWORD) * 254;
        }
        BITMAPINFO* pbmih = (BITMAPINFO*)FX_Alloc(FX_BYTE, size);
        if (!pbmih) {
            delete pDIBitmap;
            if (bCreatedDC) {
                DeleteDC(hDC);
            }
            return NULL;
        }
        pbmih->bmiHeader.biSize = sizeof(BITMAPINFOHEADER);
        pbmih->bmiHeader.biBitCount = bmih.biBitCount;
        pbmih->bmiHeader.biCompression = BI_RGB;
        pbmih->bmiHeader.biHeight = -height;
        pbmih->bmiHeader.biPlanes = 1;
        pbmih->bmiHeader.biWidth = bmih.biWidth;
        if (!pDIBitmap->Create(bmih.biWidth, height, bmih.biBitCount == 1 ? FXDIB_1bppRgb : FXDIB_8bppRgb)) {
            delete pDIBitmap;
            FX_Free(pbmih);
            if (bCreatedDC) {
                DeleteDC(hDC);
            }
            return NULL;
        }
        ret = GetDIBits(hDC, hBitmap, 0, height, pDIBitmap->GetBuffer(), (BITMAPINFO*)pbmih, DIB_RGB_COLORS);
        FX_Free(pbmih);
        pbmih = NULL;
        pDIBitmap->CopyPalette(pPalette, palsize);
    } else {
        if (bmih.biBitCount <= 24) {
            bmih.biBitCount = 24;
        } else {
            bmih.biBitCount = 32;
        }
        if (!pDIBitmap->Create(bmih.biWidth, height, bmih.biBitCount == 24 ? FXDIB_Rgb : FXDIB_Rgb32)) {
            delete pDIBitmap;
            if (bCreatedDC) {
                DeleteDC(hDC);
            }
            return NULL;
        }
        ret = GetDIBits(hDC, hBitmap, 0, height, pDIBitmap->GetBuffer(), (BITMAPINFO*)&bmih, DIB_RGB_COLORS);
        if (ret != 0 && bmih.biBitCount == 32) {
            int pitch = pDIBitmap->GetPitch();
            for (int row = 0; row < height; row ++) {
                FX_BYTE* dest_scan = (FX_BYTE*)(pDIBitmap->GetBuffer() + row * pitch);
                for (int col = 0; col < width; col++) {
                    dest_scan[3] = 255;
                    dest_scan += 4;
                }
            }
        }
    }
    if (ret == 0) {
        if (pDIBitmap) {
            delete pDIBitmap;
        }
        pDIBitmap = NULL;
    }
    if (bCreatedDC) {
        DeleteDC(hDC);
    }
    return pDIBitmap;
}
CFX_WindowsDIB::CFX_WindowsDIB(HDC hDC, int width, int height)
{
    Create(width, height, FXDIB_Rgb, (FX_LPBYTE)1);
    BITMAPINFOHEADER bmih;
    FXSYS_memset32(&bmih, 0, sizeof bmih);
    bmih.biSize = sizeof bmih;
    bmih.biBitCount = 24;
    bmih.biHeight = -height;
    bmih.biPlanes = 1;
    bmih.biWidth = width;
    m_hBitmap = CreateDIBSection(hDC, (BITMAPINFO*)&bmih, DIB_RGB_COLORS, (LPVOID*)&m_pBuffer, NULL, 0);
    m_hMemDC = CreateCompatibleDC(hDC);
    m_hOldBitmap = (HBITMAP)SelectObject(m_hMemDC, m_hBitmap);
}
CFX_WindowsDIB::~CFX_WindowsDIB()
{
    SelectObject(m_hMemDC, m_hOldBitmap);
    DeleteDC(m_hMemDC);
    DeleteObject(m_hBitmap);
}
void CFX_WindowsDIB::LoadFromDevice(HDC hDC, int left, int top)
{
    ::BitBlt(m_hMemDC, 0, 0, m_Width, m_Height, hDC, left, top, SRCCOPY);
}
void CFX_WindowsDIB::SetToDevice(HDC hDC, int left, int top)
{
    ::BitBlt(hDC, left, top, m_Width, m_Height, m_hMemDC, 0, 0, SRCCOPY);
}
#endif

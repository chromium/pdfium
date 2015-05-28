// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXGE_FX_GE_WIN32_H_
#define CORE_INCLUDE_FXGE_FX_GE_WIN32_H_

#ifdef _WIN32
#ifndef _WINDOWS_
#include <windows.h>
#endif
#define WINDIB_OPEN_MEMORY    0x1
#define WINDIB_OPEN_PATHNAME  0x2
typedef struct  WINDIB_Open_Args_ {

    int         flags;

    const FX_BYTE*  memory_base;

    size_t    memory_size;

    FX_LPCWSTR  path_name;
} WINDIB_Open_Args_;
class CFX_WindowsDIB : public CFX_DIBitmap
{
public:

    static CFX_ByteString	GetBitmapInfo(const CFX_DIBitmap* pBitmap);

    static CFX_DIBitmap* LoadFromBuf(BITMAPINFO* pbmi, void* pData);

    static HBITMAP		GetDDBitmap(const CFX_DIBitmap* pBitmap, HDC hDC);

    static CFX_DIBitmap* LoadFromDDB(HDC hDC, HBITMAP hBitmap, FX_DWORD* pPalette = NULL, FX_DWORD size = 256);

    static CFX_DIBitmap* LoadFromFile(FX_LPCWSTR filename);

    static CFX_DIBitmap* LoadFromFile(FX_LPCSTR filename)
    {
        return LoadFromFile(CFX_WideString::FromLocal(filename).c_str());
    }

    static CFX_DIBitmap* LoadDIBitmap(WINDIB_Open_Args_ args);

    CFX_WindowsDIB(HDC hDC, int width, int height);

    ~CFX_WindowsDIB();

    HDC					GetDC() const
    {
        return m_hMemDC;
    }

    HBITMAP				GetWindowsBitmap() const
    {
        return m_hBitmap;
    }

    void				LoadFromDevice(HDC hDC, int left, int top);

    void				SetToDevice(HDC hDC, int left, int top);
protected:

    HDC					m_hMemDC;

    HBITMAP				m_hBitmap;

    HBITMAP				m_hOldBitmap;
};
class CFX_WindowsDevice : public CFX_RenderDevice
{
public:
    static IFX_RenderDeviceDriver*	CreateDriver(HDC hDC, FX_BOOL bCmykOutput = FALSE);

    CFX_WindowsDevice(HDC hDC, FX_BOOL bCmykOutput = FALSE, FX_BOOL bForcePSOutput = FALSE, int psLevel = 2);

    HDC		GetDC() const;

    FX_BOOL m_bForcePSOutput;

    static int m_psLevel;
};
class CFX_WinBitmapDevice : public CFX_RenderDevice
{
public:

    CFX_WinBitmapDevice(int width, int height, FXDIB_Format format);

    ~CFX_WinBitmapDevice();

    HDC		GetDC()
    {
        return m_hDC;
    }
protected:

    HBITMAP	m_hBitmap;

    HBITMAP m_hOldBitmap;

    HDC		m_hDC;
};
#endif

#endif  // CORE_INCLUDE_FXGE_FX_GE_WIN32_H_

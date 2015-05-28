// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
#ifndef CORE_SRC_FXGE_WIN32_WIN32_INT_H_
#define CORE_SRC_FXGE_WIN32_WIN32_INT_H_

struct  WINDIB_Open_Args_;
class CGdiplusExt
{
public:
    CGdiplusExt();
    ~CGdiplusExt();
    void			Load();
    FX_BOOL			IsAvailable()
    {
        return m_hModule != NULL;
    }
    FX_BOOL			StretchBitMask(HDC hDC, BOOL bMonoDevice, const CFX_DIBitmap* pBitmap, int dest_left, int dest_top,
                                   int dest_width, int dest_height, FX_DWORD argb, const FX_RECT* pClipRect, int flags);
    FX_BOOL			StretchDIBits(HDC hDC, const CFX_DIBitmap* pBitmap, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, int flags);
    FX_BOOL			DrawPath(HDC hDC, const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_argb,
                             FX_DWORD stroke_argb,
                             int fill_mode
                      );

    void*			LoadMemFont(FX_BYTE* pData, FX_DWORD size);
    void			DeleteMemFont(void* pFontCollection);
    FX_BOOL         GdipCreateFromImage(void* bitmap, void** graphics);
    void            GdipDeleteGraphics(void* graphics);
    void            GdipSetTextRenderingHint(void* graphics, int mode);
    void            GdipSetPageUnit(void* graphics, FX_DWORD unit);
    void            GdipSetWorldTransform(void* graphics, void* pMatrix);
    FX_BOOL         GdipDrawDriverString(void *graphics,  unsigned short *text, int length, void *font, void* brush, void *positions, int flags, const void *matrix);
    void            GdipCreateBrush(FX_DWORD fill_argb, void** pBrush);
    void            GdipDeleteBrush(void* pBrush);
    void            GdipCreateMatrix(FX_FLOAT a, FX_FLOAT b, FX_FLOAT c, FX_FLOAT d, FX_FLOAT e, FX_FLOAT f, void** matrix);
    void            GdipDeleteMatrix(void* matrix);
    FX_BOOL         GdipCreateFontFamilyFromName(FX_LPCWSTR name, void* pFontCollection, void**pFamily);
    void            GdipDeleteFontFamily(void* pFamily);
    FX_BOOL         GdipCreateFontFromFamily(void* pFamily, FX_FLOAT font_size, int fontstyle, int flag, void** pFont);
    void*           GdipCreateFontFromCollection(void* pFontCollection, FX_FLOAT font_size, int fontstyle);
    void            GdipDeleteFont(void* pFont);
    FX_BOOL         GdipCreateBitmap(CFX_DIBitmap* pBitmap, void**bitmap);
    void            GdipDisposeImage(void* bitmap);
    void            GdipGetFontSize(void *pFont, FX_FLOAT *size);
    void*           GdiAddFontMemResourceEx(void *pFontdata, FX_DWORD size, void* pdv, FX_DWORD* num_face);
    FX_BOOL         GdiRemoveFontMemResourceEx(void* handle);
    void*			m_Functions[100];
    void*           m_pGdiAddFontMemResourceEx;
    void*           m_pGdiRemoveFontMemResourseEx;
    CFX_DIBitmap*	LoadDIBitmap(WINDIB_Open_Args_ args);
protected:
    HMODULE			m_hModule;
    HMODULE         m_GdiModule;
};
#include "dwrite_int.h"
class CWin32Platform 
{
public:
    FX_BOOL			m_bHalfTone;
    CGdiplusExt		m_GdiplusExt;
    CDWriteExt      m_DWriteExt;
};
class CGdiDeviceDriver : public IFX_RenderDeviceDriver
{
protected:
    virtual int		GetDeviceCaps(int caps_id);
    virtual void	SaveState()
    {
        SaveDC(m_hDC);
    }
    virtual void	RestoreState(FX_BOOL bKeepSaved = FALSE)
    {
        RestoreDC(m_hDC, -1);
        if (bKeepSaved) {
            SaveDC(m_hDC);
        }
    }
    virtual FX_BOOL	SetClip_PathFill(const CFX_PathData* pPathData,
                                     const CFX_AffineMatrix* pObject2Device,
                                     int fill_mode
                                    );
    virtual FX_BOOL	SetClip_PathStroke(const CFX_PathData* pPathData,
                                       const CFX_AffineMatrix* pObject2Device,
                                       const CFX_GraphStateData* pGraphState
                                      );
    virtual FX_BOOL	DrawPath(const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_color,
                             FX_DWORD stroke_color,
                             int fill_mode,
                             int alpha_flag,
                             void* pIccTransform,
                             int	blend_type
                            );
    virtual FX_BOOL FillRect(const FX_RECT* pRect,
                             FX_DWORD fill_color,
                             int alpha_flag, void* pIccTransform, int blend_type);
    virtual FX_BOOL	DrawCosmeticLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_DWORD color,
                                     int alpha_flag, void* pIccTransform, int blend_type);
    virtual FX_LPVOID GetClipRgn() ;
    virtual FX_BOOL SetClipRgn(FX_LPVOID pRgn) ;
    virtual FX_BOOL GetClipBox(FX_RECT* pRect);
    virtual FX_BOOL DeleteDeviceRgn(FX_LPVOID pRgn);
    virtual void	DrawLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2);
    virtual void*	GetPlatformSurface()
    {
        return (void*)m_hDC;
    }
    FX_BOOL			GDI_SetDIBits(const CFX_DIBitmap* pBitmap, const FX_RECT* pSrcRect, int left, int top,
                                  void* pIccTransform);
    FX_BOOL			GDI_StretchDIBits(const CFX_DIBitmap* pBitmap, int dest_left, int dest_top,
                                      int dest_width, int dest_height, FX_DWORD flags,
                                      void* pIccTransform);
    FX_BOOL			GDI_StretchBitMask(const CFX_DIBitmap* pBitmap, int dest_left, int dest_top,
                                       int dest_width, int dest_height, FX_DWORD bitmap_color, FX_DWORD flags,
                                       int alpha_flag, void* pIccTransform);
    HDC				m_hDC;
    int				m_Width, m_Height, m_nBitsPerPixel;
    int				m_DeviceClass, m_RenderCaps;
    CGdiDeviceDriver(HDC hDC, int device_class);
    ~CGdiDeviceDriver() {}
};
class CGdiDisplayDriver : public CGdiDeviceDriver
{
public:
    CGdiDisplayDriver(HDC hDC);
protected:
    virtual FX_BOOL GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform = NULL, FX_BOOL bDEdge = FALSE);
    virtual FX_BOOL SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                              int alpha_flag, void* pIccTransform);
    virtual FX_BOOL StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                  int alpha_flag, void* pIccTransform, int blend_type);
    virtual FX_BOOL	StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, FX_LPVOID& handle,
                                int alpha_flag, void* pIccTransform, int blend_type)
    {
        return FALSE;
    }
    FX_BOOL			UseFoxitStretchEngine(const CFX_DIBSource* pSource, FX_DWORD color, int dest_left, int dest_top,
                                          int dest_width, int dest_height, const FX_RECT* pClipRect, int render_flags,
                                          int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);
};
class CGdiPrinterDriver : public CGdiDeviceDriver
{
public:
    CGdiPrinterDriver(HDC hDC);
protected:
    virtual int		GetDeviceCaps(int caps_id);
    virtual FX_BOOL SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                              int alpha_flag, void* pIccTransform);
    virtual FX_BOOL StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                  int alpha_flag, void* pIccTransform, int blend_type);
    virtual FX_BOOL	StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, FX_LPVOID& handle,
                                int alpha_flag, void* pIccTransform, int blend_type);
    int				m_HorzSize, m_VertSize;
    FX_BOOL			m_bSupportROP;
};
class CPSOutput : public IFX_PSOutput
{
public:
    CPSOutput(HDC hDC);
    virtual ~CPSOutput();
    virtual void			Release()
    {
        delete this;
    }
    void Init();
    virtual void	OutputPS(FX_LPCSTR string, int len);
    HDC				m_hDC;
    FX_LPSTR        m_pBuf;
};
class CPSPrinterDriver : public IFX_RenderDeviceDriver
{
public:
    CPSPrinterDriver();
    FX_BOOL			Init(HDC hDC, int ps_level, FX_BOOL bCmykOutput);
    ~CPSPrinterDriver();
protected:
    virtual FX_BOOL IsPSPrintDriver()
    {
        return TRUE;
    }
    virtual int		GetDeviceCaps(int caps_id);
    virtual FX_BOOL	StartRendering();
    virtual void	EndRendering();
    virtual void	SaveState();
    virtual void	RestoreState(FX_BOOL bKeepSaved = FALSE);
    virtual FX_BOOL	SetClip_PathFill(const CFX_PathData* pPathData,
                                     const CFX_AffineMatrix* pObject2Device,
                                     int fill_mode
                                    );
    virtual FX_BOOL	SetClip_PathStroke(const CFX_PathData* pPathData,
                                       const CFX_AffineMatrix* pObject2Device,
                                       const CFX_GraphStateData* pGraphState
                                      );
    virtual FX_BOOL	DrawPath(const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_color,
                             FX_DWORD stroke_color,
                             int fill_mode,
                             int alpha_flag,
                             void* pIccTransform,
                             int blend_type
                            );
    virtual FX_BOOL GetClipBox(FX_RECT* pRect);
    virtual FX_BOOL SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                              int alpha_flag, void* pIccTransform);
    virtual FX_BOOL StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                  int alpha_flag, void* pIccTransform, int blend_type);
    virtual FX_BOOL	StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, FX_LPVOID& handle,
                                int alpha_flag, void* pIccTransform, int blend_type);
    virtual FX_BOOL DrawDeviceText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
                                   CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device, FX_FLOAT font_size, FX_DWORD color,
                                   int alpha_flag, void* pIccTransform);
    virtual void*	GetPlatformSurface()
    {
        return (void*)m_hDC;
    }
    HDC				m_hDC;
    FX_BOOL			m_bCmykOutput;
    int				m_Width, m_Height, m_nBitsPerPixel;
    int				m_HorzSize, m_VertSize;
    CPSOutput*		m_pPSOutput;
    CFX_PSRenderer	m_PSRenderer;
};
void _Color2Argb(FX_ARGB& argb, FX_DWORD color, int alpha_flag, void* pIccTransform);

#endif  // CORE_SRC_FXGE_WIN32_WIN32_INT_H_

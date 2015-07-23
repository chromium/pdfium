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
    bool			IsAvailable()
    {
        return m_hModule != NULL;
    }
    bool			StretchBitMask(HDC hDC, BOOL bMonoDevice, const CFX_DIBitmap* pBitmap, int dest_left, int dest_top,
                                   int dest_width, int dest_height, FX_DWORD argb, const FX_RECT* pClipRect, int flags);
    bool			StretchDIBits(HDC hDC, const CFX_DIBitmap* pBitmap, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, int flags);
    bool			DrawPath(HDC hDC, const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_argb,
                             FX_DWORD stroke_argb,
                             int fill_mode
                      );

    void*			LoadMemFont(uint8_t* pData, FX_DWORD size);
    void			DeleteMemFont(void* pFontCollection);
    bool         GdipCreateFromImage(void* bitmap, void** graphics);
    void            GdipDeleteGraphics(void* graphics);
    void            GdipSetTextRenderingHint(void* graphics, int mode);
    void            GdipSetPageUnit(void* graphics, FX_DWORD unit);
    void            GdipSetWorldTransform(void* graphics, void* pMatrix);
    bool         GdipDrawDriverString(void *graphics,  unsigned short *text, int length, void *font, void* brush, void *positions, int flags, const void *matrix);
    void            GdipCreateBrush(FX_DWORD fill_argb, void** pBrush);
    void            GdipDeleteBrush(void* pBrush);
    void            GdipCreateMatrix(FX_FLOAT a, FX_FLOAT b, FX_FLOAT c, FX_FLOAT d, FX_FLOAT e, FX_FLOAT f, void** matrix);
    void            GdipDeleteMatrix(void* matrix);
    bool         GdipCreateFontFamilyFromName(const FX_WCHAR* name, void* pFontCollection, void**pFamily);
    void            GdipDeleteFontFamily(void* pFamily);
    bool         GdipCreateFontFromFamily(void* pFamily, FX_FLOAT font_size, int fontstyle, int flag, void** pFont);
    void*           GdipCreateFontFromCollection(void* pFontCollection, FX_FLOAT font_size, int fontstyle);
    void            GdipDeleteFont(void* pFont);
    bool         GdipCreateBitmap(CFX_DIBitmap* pBitmap, void**bitmap);
    void            GdipDisposeImage(void* bitmap);
    void            GdipGetFontSize(void *pFont, FX_FLOAT *size);
    void*           GdiAddFontMemResourceEx(void *pFontdata, FX_DWORD size, void* pdv, FX_DWORD* num_face);
    bool         GdiRemoveFontMemResourceEx(void* handle);
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
    bool			m_bHalfTone;
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
    virtual void	RestoreState(bool bKeepSaved = false)
    {
        RestoreDC(m_hDC, -1);
        if (bKeepSaved) {
            SaveDC(m_hDC);
        }
    }
    virtual bool	SetClip_PathFill(const CFX_PathData* pPathData,
                                     const CFX_AffineMatrix* pObject2Device,
                                     int fill_mode
                                    );
    virtual bool	SetClip_PathStroke(const CFX_PathData* pPathData,
                                       const CFX_AffineMatrix* pObject2Device,
                                       const CFX_GraphStateData* pGraphState
                                      );
    virtual bool	DrawPath(const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_color,
                             FX_DWORD stroke_color,
                             int fill_mode,
                             int alpha_flag,
                             void* pIccTransform,
                             int	blend_type
                            );
    virtual bool FillRect(const FX_RECT* pRect,
                             FX_DWORD fill_color,
                             int alpha_flag, void* pIccTransform, int blend_type);
    virtual bool	DrawCosmeticLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_DWORD color,
                                     int alpha_flag, void* pIccTransform, int blend_type);
    virtual void* GetClipRgn() ;
    virtual bool SetClipRgn(void* pRgn) ;
    virtual bool GetClipBox(FX_RECT* pRect);
    virtual bool DeleteDeviceRgn(void* pRgn);
    virtual void	DrawLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2);
    virtual void*	GetPlatformSurface()
    {
        return (void*)m_hDC;
    }
    bool			GDI_SetDIBits(const CFX_DIBitmap* pBitmap, const FX_RECT* pSrcRect, int left, int top,
                                  void* pIccTransform);
    bool			GDI_StretchDIBits(const CFX_DIBitmap* pBitmap, int dest_left, int dest_top,
                                      int dest_width, int dest_height, FX_DWORD flags,
                                      void* pIccTransform);
    bool			GDI_StretchBitMask(const CFX_DIBitmap* pBitmap, int dest_left, int dest_top,
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
    virtual bool GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform = NULL, bool bDEdge = false);
    virtual bool SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                              int alpha_flag, void* pIccTransform);
    virtual bool StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                  int alpha_flag, void* pIccTransform, int blend_type);
    virtual bool	StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, void*& handle,
                                int alpha_flag, void* pIccTransform, int blend_type)
    {
        return false;
    }
    bool			UseFoxitStretchEngine(const CFX_DIBSource* pSource, FX_DWORD color, int dest_left, int dest_top,
                                          int dest_width, int dest_height, const FX_RECT* pClipRect, int render_flags,
                                          int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);
};
class CGdiPrinterDriver : public CGdiDeviceDriver
{
public:
    CGdiPrinterDriver(HDC hDC);
protected:
    virtual int		GetDeviceCaps(int caps_id);
    virtual bool SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                              int alpha_flag, void* pIccTransform);
    virtual bool StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                  int alpha_flag, void* pIccTransform, int blend_type);
    virtual bool	StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, void*& handle,
                                int alpha_flag, void* pIccTransform, int blend_type);
    int				m_HorzSize, m_VertSize;
    bool			m_bSupportROP;
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
    virtual void	OutputPS(const FX_CHAR* string, int len);
    HDC				m_hDC;
    FX_CHAR*        m_pBuf;
};
class CPSPrinterDriver : public IFX_RenderDeviceDriver
{
public:
    CPSPrinterDriver();
    bool			Init(HDC hDC, int ps_level, bool bCmykOutput);
    ~CPSPrinterDriver();
protected:
    virtual bool IsPSPrintDriver()
    {
        return true;
    }
    virtual int		GetDeviceCaps(int caps_id);
    virtual bool	StartRendering();
    virtual void	EndRendering();
    virtual void	SaveState();
    virtual void	RestoreState(bool bKeepSaved = false);
    virtual bool	SetClip_PathFill(const CFX_PathData* pPathData,
                                     const CFX_AffineMatrix* pObject2Device,
                                     int fill_mode
                                    );
    virtual bool	SetClip_PathStroke(const CFX_PathData* pPathData,
                                       const CFX_AffineMatrix* pObject2Device,
                                       const CFX_GraphStateData* pGraphState
                                      );
    virtual bool	DrawPath(const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_color,
                             FX_DWORD stroke_color,
                             int fill_mode,
                             int alpha_flag,
                             void* pIccTransform,
                             int blend_type
                            );
    virtual bool GetClipBox(FX_RECT* pRect);
    virtual bool SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                              int alpha_flag, void* pIccTransform);
    virtual bool StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                  int alpha_flag, void* pIccTransform, int blend_type);
    virtual bool	StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD render_flags, void*& handle,
                                int alpha_flag, void* pIccTransform, int blend_type);
    virtual bool DrawDeviceText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
                                   CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device, FX_FLOAT font_size, FX_DWORD color,
                                   int alpha_flag, void* pIccTransform);
    virtual void*	GetPlatformSurface()
    {
        return (void*)m_hDC;
    }
    HDC				m_hDC;
    bool			m_bCmykOutput;
    int				m_Width, m_Height, m_nBitsPerPixel;
    int				m_HorzSize, m_VertSize;
    CPSOutput*		m_pPSOutput;
    CFX_PSRenderer	m_PSRenderer;
};
void _Color2Argb(FX_ARGB& argb, FX_DWORD color, int alpha_flag, void* pIccTransform);

#endif  // CORE_SRC_FXGE_WIN32_WIN32_INT_H_

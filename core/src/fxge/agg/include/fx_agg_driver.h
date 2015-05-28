// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FX_AGG_DRIVER_H_
#define FX_AGG_DRIVER_H_

#include "fxfx_agg_path_storage.h"
#include "fxfx_agg_rasterizer_scanline_aa.h"

class CAgg_PathData 
{
public:
    CAgg_PathData() {}
    ~CAgg_PathData() {}
    FX_NAMESPACE_DECLARE(agg, path_storage)	m_PathData;
    void			BuildPath(const CFX_PathData* pPathData, const CFX_AffineMatrix* pObject2Device);
};
class CFX_AggDeviceDriver : public IFX_RenderDeviceDriver
{
public:
    CFX_AggDeviceDriver(CFX_DIBitmap* pBitmap, int dither_bits, FX_BOOL bRgbByteOrder, CFX_DIBitmap* pOriDevice, FX_BOOL bGroupKnockout);
    virtual ~CFX_AggDeviceDriver();
    void				InitPlatform();
    void				DestroyPlatform();


    virtual int			GetDeviceCaps(int caps_id);


    virtual void		SaveState();
    virtual void		RestoreState(FX_BOOL bKeepSaved);


    virtual FX_BOOL		SetClip_PathFill(const CFX_PathData* pPathData,
                                         const CFX_AffineMatrix* pObject2Device,
                                         int fill_mode
                                     );


    virtual FX_BOOL		SetClip_PathStroke(const CFX_PathData* pPathData,
                                           const CFX_AffineMatrix* pObject2Device,
                                           const CFX_GraphStateData* pGraphState
                                       );


    virtual FX_BOOL		DrawPath(const CFX_PathData* pPathData,
                                 const CFX_AffineMatrix* pObject2Device,
                                 const CFX_GraphStateData* pGraphState,
                                 FX_DWORD fill_color,
                                 FX_DWORD stroke_color,
                                 int fill_mode,
                                 int alpha_flag,
                                 void* pIccTransform,
                                 int blend_type
                             );

    virtual FX_BOOL		SetPixel(int x, int y, FX_DWORD color,
                                 int alpha_flag, void* pIccTransform);

    virtual FX_BOOL		FillRect(const FX_RECT* pRect,
                                 FX_DWORD fill_color, int alpha_flag, void* pIccTransform, int blend_type);


    virtual FX_BOOL		DrawCosmeticLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_DWORD color,
                                         int alpha_flag, void* pIccTransform, int blend_type)
    {
        return FALSE;
    }

    virtual FX_BOOL		GetClipBox(FX_RECT* pRect);


    virtual FX_BOOL		GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform = NULL, FX_BOOL bDEdge = FALSE);
    virtual CFX_DIBitmap*   GetBackDrop()
    {
        return m_pOriDevice;
    }

    virtual FX_BOOL		SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                                  int alpha_flag, void* pIccTransform);
    virtual FX_BOOL		StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                      int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                      int alpha_flag, void* pIccTransform, int blend_type);

    virtual FX_BOOL		StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                    const CFX_AffineMatrix* pMatrix, FX_DWORD flags, FX_LPVOID& handle,
                                    int alpha_flag, void* pIccTransform, int blend_type);
    virtual FX_BOOL		ContinueDIBits(FX_LPVOID handle, IFX_Pause* pPause);
    virtual void		CancelDIBits(FX_LPVOID handle);

    virtual FX_BOOL     DrawDeviceText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
                                       CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device, FX_FLOAT font_size, FX_DWORD color,
                                       int alpha_flag, void* pIccTransform);
    virtual FX_BOOL		RenderRasterizer(FX_NAMESPACE_DECLARE(agg, rasterizer_scanline_aa)& rasterizer, FX_DWORD color, FX_BOOL bFullCover, FX_BOOL bGroupKnockout,
                                         int alpha_flag, void* pIccTransform);

    void				SetClipMask(FX_NAMESPACE_DECLARE(agg, rasterizer_scanline_aa)& rasterizer);

    virtual	FX_LPBYTE	GetBuffer() const
    {
        return m_pBitmap->GetBuffer();
    }
    virtual int			GetDriverType()
    {
        return 1;
    }

    CFX_DIBitmap*		m_pBitmap;
    CFX_ClipRgn*		m_pClipRgn;
    CFX_PtrArray		m_StateStack;
    void*				m_pPlatformGraphics;
    void*				m_pPlatformBitmap;
    void*				m_pDwRenderTartget;
    int					m_FillFlags;
    int					m_DitherBits;
    FX_BOOL				m_bRgbByteOrder;
    CFX_DIBitmap*       m_pOriDevice;
    FX_BOOL             m_bGroupKnockout;
};

#endif  // FX_AGG_DRIVER_H_

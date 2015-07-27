// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FX_AGG_DRIVER_H_
#define FX_AGG_DRIVER_H_

#include "../../../../../third_party/agg23/agg_clip_liang_barsky.h"
#include "../../../../../third_party/agg23/agg_path_storage.h"
#include "../../../../../third_party/agg23/agg_rasterizer_scanline_aa.h"

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
    CFX_AggDeviceDriver(CFX_DIBitmap* pBitmap, int dither_bits, bool bRgbByteOrder, CFX_DIBitmap* pOriDevice, bool bGroupKnockout);
    virtual ~CFX_AggDeviceDriver();
    void				InitPlatform();
    void				DestroyPlatform();


    virtual int			GetDeviceCaps(int caps_id);


    virtual void		SaveState();
    virtual void		RestoreState(bool bKeepSaved);


    virtual bool		SetClip_PathFill(const CFX_PathData* pPathData,
                                         const CFX_AffineMatrix* pObject2Device,
                                         int fill_mode
                                     );


    virtual bool		SetClip_PathStroke(const CFX_PathData* pPathData,
                                           const CFX_AffineMatrix* pObject2Device,
                                           const CFX_GraphStateData* pGraphState
                                       );


    virtual bool		DrawPath(const CFX_PathData* pPathData,
                                 const CFX_AffineMatrix* pObject2Device,
                                 const CFX_GraphStateData* pGraphState,
                                 FX_DWORD fill_color,
                                 FX_DWORD stroke_color,
                                 int fill_mode,
                                 int alpha_flag,
                                 void* pIccTransform,
                                 int blend_type
                             );

    virtual bool		SetPixel(int x, int y, FX_DWORD color,
                                 int alpha_flag, void* pIccTransform);

    virtual bool		FillRect(const FX_RECT* pRect,
                                 FX_DWORD fill_color, int alpha_flag, void* pIccTransform, int blend_type);


    virtual bool		DrawCosmeticLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_DWORD color,
                                         int alpha_flag, void* pIccTransform, int blend_type)
    {
        return false;
    }

    virtual bool		GetClipBox(FX_RECT* pRect);


    virtual bool		GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform = NULL, bool bDEdge = false);
    virtual CFX_DIBitmap*   GetBackDrop()
    {
        return m_pOriDevice;
    }

    virtual bool		SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect, int left, int top, int blend_type,
                                  int alpha_flag, void* pIccTransform);
    virtual bool		StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                      int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                      int alpha_flag, void* pIccTransform, int blend_type);

    virtual bool		StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                    const CFX_AffineMatrix* pMatrix, FX_DWORD flags, void*& handle,
                                    int alpha_flag, void* pIccTransform, int blend_type);
    virtual bool		ContinueDIBits(void* handle, IFX_Pause* pPause);
    virtual void		CancelDIBits(void* handle);

    virtual bool     DrawDeviceText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
                                       CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device, FX_FLOAT font_size, FX_DWORD color,
                                       int alpha_flag, void* pIccTransform);
    virtual bool		RenderRasterizer(FX_NAMESPACE_DECLARE(agg, rasterizer_scanline_aa)& rasterizer, FX_DWORD color, bool bFullCover, bool bGroupKnockout,
                                         int alpha_flag, void* pIccTransform);

    void				SetClipMask(FX_NAMESPACE_DECLARE(agg, rasterizer_scanline_aa)& rasterizer);

    virtual	uint8_t*	GetBuffer() const
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
    bool				m_bRgbByteOrder;
    CFX_DIBitmap*       m_pOriDevice;
    bool             m_bGroupKnockout;
};

#endif  // FX_AGG_DRIVER_H_

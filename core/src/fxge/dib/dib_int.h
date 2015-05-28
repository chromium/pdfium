// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_DIB_DIB_INT_H_
#define CORE_SRC_FXGE_DIB_DIB_INT_H_

class CPDF_FixedMatrix 
{
public:
    CPDF_FixedMatrix(const CFX_AffineMatrix& src, int bits)
    {
        base = 1 << bits;
        a = FXSYS_round(src.a * base);
        b = FXSYS_round(src.b * base);
        c = FXSYS_round(src.c * base);
        d = FXSYS_round(src.d * base);
        e = FXSYS_round(src.e * base);
        f = FXSYS_round(src.f * base);
    }
    inline void	Transform(int x, int y, int& x1, int& y1)
    {
        x1 = (a * x + c * y + e + base / 2) / base;
        y1 = (b * x + d * y + f + base / 2) / base;
    }
    int		a, b, c, d, e, f;
    int		base;
};
#define FPDF_HUGE_IMAGE_SIZE	60000000
struct PixelWeight {
    int		m_SrcStart;
    int		m_SrcEnd;
    int		m_Weights[1];
};
class CWeightTable 
{
public:
    CWeightTable()
    {
        m_pWeightTables = NULL;
    }
    ~CWeightTable()
    {
        if(m_pWeightTables) {
            FX_Free(m_pWeightTables);
        }
        m_pWeightTables = NULL;
    }
    void			Calc(int dest_len, int dest_min, int dest_max, int src_len, int src_min, int src_max, int flags);
    PixelWeight*	GetPixelWeight(int pixel)
    {
        return (PixelWeight*)(m_pWeightTables + (pixel - m_DestMin) * m_ItemSize);
    }
    int				m_DestMin, m_ItemSize;
    FX_LPBYTE		m_pWeightTables;
};
class CStretchEngine 
{
public:
    CStretchEngine(IFX_ScanlineComposer* pDestBitmap, FXDIB_Format dest_format,
                   int dest_width, int dest_height, const FX_RECT& clip_rect,
                   const CFX_DIBSource* pSrcBitmap, int flags);
    ~CStretchEngine();
    FX_BOOL		Continue(IFX_Pause* pPause);
public:
    FXDIB_Format m_DestFormat;
    int		m_DestBpp, m_SrcBpp, m_bHasAlpha;
    IFX_ScanlineComposer*	m_pDestBitmap;
    int		m_DestWidth, m_DestHeight;
    FX_RECT	m_DestClip;
    FX_LPBYTE	m_pDestScanline;
    FX_LPBYTE   m_pDestMaskScanline;
    FX_RECT	m_SrcClip;
    const CFX_DIBSource*	m_pSource;
    FX_DWORD*	m_pSrcPalette;
    int		m_SrcWidth, m_SrcHeight;
    int		m_SrcPitch, m_InterPitch;
    int m_ExtraMaskPitch;
    unsigned char*	m_pInterBuf;
    unsigned char*	m_pExtraAlphaBuf;
    int		m_TransMethod;
    int 	m_Flags;
    CWeightTable	m_WeightTable;
    int		m_CurRow;
    FX_BOOL	StartStretchHorz();
    FX_BOOL	ContinueStretchHorz(IFX_Pause* pPause);
    void	StretchVert();
    int		m_State;
};

#endif  // CORE_SRC_FXGE_DIB_DIB_INT_H_

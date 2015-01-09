// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDF_DIB_H_
#define _FPDF_DIB_H_
#ifndef _FXCRT_EXTENSION_
#include "../fxcrt/fx_ext.h"
#endif
enum FXDIB_Format {
    FXDIB_Invalid = 0,
    FXDIB_1bppMask = 0x101,
    FXDIB_1bppRgb = 0x001,
    FXDIB_1bppCmyk = 0x401,
    FXDIB_8bppMask = 0x108,
    FXDIB_8bppRgb = 0x008,
    FXDIB_8bppRgba = 0x208,
    FXDIB_8bppCmyk = 0x408,
    FXDIB_8bppCmyka = 0x608,
    FXDIB_Rgb = 0x018,
    FXDIB_Rgba = 0x218,
    FXDIB_Rgb32 = 0x020,
    FXDIB_Argb = 0x220,
    FXDIB_Cmyk = 0x420,
    FXDIB_Cmyka = 0x620,
};
enum FXDIB_Channel {
    FXDIB_Red = 1,
    FXDIB_Green,
    FXDIB_Blue,
    FXDIB_Cyan,
    FXDIB_Magenta,
    FXDIB_Yellow,
    FXDIB_Black,
    FXDIB_Alpha
};
#define FXDIB_DOWNSAMPLE		0x04
#define FXDIB_INTERPOL			0x20
#define FXDIB_BICUBIC_INTERPOL  0x80
#define FXDIB_NOSMOOTH			0x100
#define FXDIB_PALETTE_LOC		0x01
#define FXDIB_PALETTE_WIN		0x02
#define FXDIB_PALETTE_MAC		0x04
#define FXDIB_BLEND_NORMAL			0
#define FXDIB_BLEND_MULTIPLY		1
#define FXDIB_BLEND_SCREEN			2
#define FXDIB_BLEND_OVERLAY			3
#define FXDIB_BLEND_DARKEN			4
#define FXDIB_BLEND_LIGHTEN			5

#define FXDIB_BLEND_COLORDODGE		6
#define FXDIB_BLEND_COLORBURN		7
#define FXDIB_BLEND_HARDLIGHT		8
#define FXDIB_BLEND_SOFTLIGHT		9
#define FXDIB_BLEND_DIFFERENCE		10
#define FXDIB_BLEND_EXCLUSION		11
#define FXDIB_BLEND_NONSEPARABLE	21
#define FXDIB_BLEND_HUE				21
#define FXDIB_BLEND_SATURATION		22
#define FXDIB_BLEND_COLOR			23
#define FXDIB_BLEND_LUMINOSITY		24
#define FXDIB_BLEND_UNSUPPORTED		-1
typedef FX_DWORD	FX_ARGB;
typedef FX_DWORD	FX_COLORREF;
typedef FX_DWORD	FX_CMYK;
class CFX_ClipRgn;
class CFX_DIBSource;
class CFX_DIBitmap;
#define FXSYS_RGB(r, g, b)  ((r) | ((g) << 8) | ((b) << 16))
#define FXSYS_GetRValue(rgb) ((rgb) & 0xff)
#define FXSYS_GetGValue(rgb) (((rgb) >> 8) & 0xff)
#define FXSYS_GetBValue(rgb) (((rgb) >> 16) & 0xff)
#define FX_CCOLOR(val) (255-(val))
#define FXSYS_CMYK(c, m, y, k) (((c) << 24) | ((m) << 16) | ((y) << 8) | (k))
#define FXSYS_GetCValue(cmyk) ((FX_BYTE)((cmyk) >> 24) & 0xff)
#define FXSYS_GetMValue(cmyk) ((FX_BYTE)((cmyk) >> 16) & 0xff)
#define FXSYS_GetYValue(cmyk) ((FX_BYTE)((cmyk) >> 8) & 0xff)
#define FXSYS_GetKValue(cmyk) ((FX_BYTE)(cmyk) & 0xff)
void CmykDecode(FX_CMYK cmyk, int& c, int& m, int& y, int& k);
inline FX_CMYK CmykEncode(int c, int m, int y, int k)
{
    return (c << 24) | (m << 16) | (y << 8) | k;
}
void ArgbDecode(FX_ARGB argb, int& a, int& r, int&g, int& b);
void ArgbDecode(FX_ARGB argb, int& a, FX_COLORREF& rgb);
inline FX_ARGB ArgbEncode(int a, int r, int g, int b)
{
    return (a << 24) | (r << 16) | (g << 8) | b;
}
FX_ARGB ArgbEncode(int a, FX_COLORREF rgb);
#define FXARGB_A(argb) ((FX_BYTE)((argb) >> 24))
#define FXARGB_R(argb) ((FX_BYTE)((argb) >> 16))
#define FXARGB_G(argb) ((FX_BYTE)((argb) >> 8))
#define FXARGB_B(argb) ((FX_BYTE)(argb))
#define FXARGB_MAKE(a,r,g,b) (((FX_DWORD)(a) << 24) | ((r) << 16) | ((g) << 8) | (b))
#define FXARGB_MUL_ALPHA(argb, alpha) (((((argb) >> 24) * (alpha) / 255) << 24) | ((argb) & 0xffffff))
#define FXRGB2GRAY(r,g,b) (((b) * 11 + (g) * 59 + (r) * 30) / 100)
#define FXCMYK2GRAY(c,m,y,k) (((255-(c)) * (255-(k)) * 30 + (255-(m)) * (255-(k)) * 59 + (255-(y)) * (255-(k)) * 11) / 25500)
#define FXDIB_ALPHA_MERGE(backdrop, source, source_alpha) (((backdrop) * (255-(source_alpha)) + (source)*(source_alpha))/255)
#define FXDIB_ALPHA_UNION(dest, src) ((dest) + (src) - (dest)*(src)/255)
#define FXCMYK_GETDIB(p) ((((FX_LPBYTE)(p))[0] << 24 | (((FX_LPBYTE)(p))[1] << 16) | (((FX_LPBYTE)(p))[2] << 8) | ((FX_LPBYTE)(p))[3]))
#define FXCMYK_SETDIB(p, cmyk)  ((FX_LPBYTE)(p))[0] = (FX_BYTE)((cmyk) >> 24), \
        ((FX_LPBYTE)(p))[1] = (FX_BYTE)((cmyk) >> 16), \
                              ((FX_LPBYTE)(p))[2] = (FX_BYTE)((cmyk) >> 8), \
                                      ((FX_LPBYTE)(p))[3] = (FX_BYTE)(cmyk))
#define FXARGB_GETDIB(p) ((((FX_LPBYTE)(p))[0]) | (((FX_LPBYTE)(p))[1] << 8) | (((FX_LPBYTE)(p))[2] << 16) | (((FX_LPBYTE)(p))[3] << 24))
#define FXARGB_SETDIB(p, argb) ((FX_LPBYTE)(p))[0] = (FX_BYTE)(argb), \
        ((FX_LPBYTE)(p))[1] = (FX_BYTE)((argb) >> 8), \
                              ((FX_LPBYTE)(p))[2] = (FX_BYTE)((argb) >> 16), \
                                      ((FX_LPBYTE)(p))[3] = (FX_BYTE)((argb) >> 24)
#define FXARGB_COPY(dest, src) *(FX_LPBYTE)(dest) = *(FX_LPBYTE)(src), \
        *((FX_LPBYTE)(dest)+1) = *((FX_LPBYTE)(src)+1), \
                                 *((FX_LPBYTE)(dest)+2) = *((FX_LPBYTE)(src)+2), \
                                         *((FX_LPBYTE)(dest)+3) = *((FX_LPBYTE)(src)+3)
#define FXCMYK_COPY(dest, src)  *(FX_LPBYTE)(dest) = *(FX_LPBYTE)(src), \
        *((FX_LPBYTE)(dest)+1) = *((FX_LPBYTE)(src)+1), \
                                 *((FX_LPBYTE)(dest)+2) = *((FX_LPBYTE)(src)+2), \
                                         *((FX_LPBYTE)(dest)+3) = *((FX_LPBYTE)(src)+3)
#define FXARGB_SETRGBORDERDIB(p, argb) ((FX_LPBYTE)(p))[3] = (FX_BYTE)(argb>>24), \
        ((FX_LPBYTE)(p))[0] = (FX_BYTE)((argb) >> 16), \
                              ((FX_LPBYTE)(p))[1] = (FX_BYTE)((argb) >> 8), \
                                      ((FX_LPBYTE)(p))[2] = (FX_BYTE)(argb)
#define FXARGB_GETRGBORDERDIB(p) (((FX_LPBYTE)(p))[2]) | (((FX_LPBYTE)(p))[1] << 8) | (((FX_LPBYTE)(p))[0] << 16) | (((FX_LPBYTE)(p))[3] << 24)
#define FXARGB_RGBORDERCOPY(dest, src) *((FX_LPBYTE)(dest)+3) = *((FX_LPBYTE)(src)+3), \
        *(FX_LPBYTE)(dest) = *((FX_LPBYTE)(src)+2), \
                             *((FX_LPBYTE)(dest)+1) = *((FX_LPBYTE)(src)+1), \
                                     *((FX_LPBYTE)(dest)+2) = *((FX_LPBYTE)(src))
#define FXARGB_TODIB(argb) (argb)
#define FXCMYK_TODIB(cmyk) ((FX_BYTE)((cmyk) >> 24) | ((FX_BYTE)((cmyk) >> 16)) << 8 | ((FX_BYTE)((cmyk) >> 8)) << 16 | ((FX_BYTE)(cmyk) << 24))
#define FXARGB_TOBGRORDERDIB(argb) ((FX_BYTE)(argb>>16) | ((FX_BYTE)(argb>>8)) << 8 | ((FX_BYTE)(argb)) << 16 | ((FX_BYTE)(argb>>24) << 24))
#define FXGETFLAG_COLORTYPE(flag)			(FX_BYTE)((flag)>>8)
#define FXGETFLAG_ALPHA_FILL(flag)			(FX_BYTE)(flag)
#define FXGETFLAG_ALPHA_STROKE(flag)		(FX_BYTE)((flag)>>16)
#define FXSETFLAG_COLORTYPE(flag, val)		flag = (((val)<<8)|(flag&0xffff00ff))
#define FXSETFLAG_ALPHA_FILL(flag, val)		flag = ((val)|(flag&0xffffff00))
#define FXSETFLAG_ALPHA_STROKE(flag, val)	flag = (((val)<<16)|(flag&0xff00ffff))
class CFX_DIBSource : public CFX_Object
{
public:

    virtual			~CFX_DIBSource();



    int				GetWidth() const
    {
        return m_Width;
    }

    int				GetHeight() const
    {
        return m_Height;
    }

    FXDIB_Format	GetFormat() const
    {
        return (FXDIB_Format)(m_AlphaFlag * 0x100 + m_bpp);
    }

    FX_DWORD		GetPitch() const
    {
        return m_Pitch;
    }

    FX_DWORD*		GetPalette() const
    {
        return m_pPalette;
    }



    virtual	FX_LPBYTE	GetBuffer() const
    {
        return NULL;
    }

    virtual FX_LPCBYTE	GetScanline(int line) const = 0;

    virtual FX_BOOL		SkipToScanline(int line, IFX_Pause* pPause) const
    {
        return FALSE;
    }

    virtual void		DownSampleScanline(int line, FX_LPBYTE dest_scan, int dest_bpp,
                                           int dest_width, FX_BOOL bFlipX, int clip_left, int clip_width) const = 0;

    virtual void		SetDownSampleSize(int width, int height) const {}

    int				GetBPP() const
    {
        return m_bpp;
    }

    FX_BOOL			IsAlphaMask() const
    {
        return m_AlphaFlag == 1;
    }

    FX_BOOL			HasAlpha() const
    {
        return m_AlphaFlag & 2 ? TRUE : FALSE;
    }

    FX_BOOL			IsOpaqueImage() const
    {
        return !(m_AlphaFlag & 3);
    }

    FX_BOOL			IsCmykImage() const
    {
        return m_AlphaFlag & 4 ? TRUE : FALSE;
    }



    int				GetPaletteSize() const
    {
        return IsAlphaMask() ? 0 : (m_bpp == 1 ? 2 : (m_bpp == 8 ? 256 : 0));
    }

    FX_DWORD		GetPaletteEntry(int index) const;

    void			SetPaletteEntry(int index, FX_DWORD color);
    FX_DWORD		GetPaletteArgb(int index) const
    {
        return GetPaletteEntry(index);
    }
    void			SetPaletteArgb(int index, FX_DWORD color)
    {
        SetPaletteEntry(index, color);
    }

    void			CopyPalette(const FX_DWORD* pSrcPal, FX_DWORD size = 256);


    CFX_DIBitmap*	Clone(const FX_RECT* pClip = NULL) const;

    CFX_DIBitmap*	CloneConvert(FXDIB_Format format, const FX_RECT* pClip = NULL, void* pIccTransform = NULL) const;

    CFX_DIBitmap*	StretchTo(int dest_width, int dest_height, FX_DWORD flags = 0, const FX_RECT* pClip = NULL) const;


    CFX_DIBitmap*	TransformTo(const CFX_AffineMatrix* pMatrix, int& left, int &top,
                                FX_DWORD flags = 0, const FX_RECT* pClip = NULL) const;

    CFX_DIBitmap*	GetAlphaMask(const FX_RECT* pClip = NULL) const;

    FX_BOOL			CopyAlphaMask(const CFX_DIBSource* pAlphaMask, const FX_RECT* pClip = NULL);

    CFX_DIBitmap*	SwapXY(FX_BOOL bXFlip, FX_BOOL bYFlip, const FX_RECT* pClip = NULL) const;

    CFX_DIBitmap*	FlipImage(FX_BOOL bXFlip, FX_BOOL bYFlip) const;

    void			GetOverlapRect(int& dest_left, int& dest_top, int& width, int& height, int src_width,
                                   int src_height, int& src_left, int& src_top, const CFX_ClipRgn* pClipRgn);

    CFX_DIBitmap*	m_pAlphaMask;
protected:

    CFX_DIBSource();

    int				m_Width;

    int				m_Height;

    int				m_bpp;

    FX_DWORD		m_AlphaFlag;

    FX_DWORD		m_Pitch;

    FX_DWORD*		m_pPalette;

    void			BuildPalette();

    FX_BOOL			BuildAlphaMask();

    int				FindPalette(FX_DWORD color) const;

    void			GetPalette(FX_DWORD* pal, int alpha) const;
};
class CFX_DIBitmap : public CFX_DIBSource
{
public:

    virtual ~CFX_DIBitmap();

    CFX_DIBitmap();

    CFX_DIBitmap(const CFX_DIBitmap& src);

    FX_BOOL			Create(int width, int height, FXDIB_Format format, FX_LPBYTE pBuffer = NULL, int pitch = 0);

    FX_BOOL			Copy(const CFX_DIBSource* pSrc);

    virtual	FX_LPBYTE	GetBuffer() const
    {
        return m_pBuffer;
    }

    virtual FX_LPCBYTE	GetScanline(int line) const
    {
        return m_pBuffer ? m_pBuffer + line * m_Pitch : NULL;
    }

    virtual void	DownSampleScanline(int line, FX_LPBYTE dest_scan, int dest_bpp,
                                       int dest_width, FX_BOOL bFlipX, int clip_left, int clip_width) const;

    void			TakeOver(CFX_DIBitmap* pSrcBitmap);

    FX_BOOL			ConvertFormat(FXDIB_Format format, void* pIccTransform = NULL);

    void			Clear(FX_DWORD color);

    FX_DWORD		GetPixel(int x, int y) const;

    void			SetPixel(int x, int y, FX_DWORD color);

    FX_BOOL			LoadChannel(FXDIB_Channel destChannel, const CFX_DIBSource* pSrcBitmap, FXDIB_Channel srcChannel);

    FX_BOOL			LoadChannel(FXDIB_Channel destChannel, int value);

    FX_BOOL			MultiplyAlpha(int alpha);

    FX_BOOL			MultiplyAlpha(const CFX_DIBSource* pAlphaMask);

    FX_BOOL			TransferBitmap(int dest_left, int dest_top, int width, int height,
                                   const CFX_DIBSource* pSrcBitmap, int src_left, int src_top, void* pIccTransform = NULL);

    FX_BOOL			CompositeBitmap(int dest_left, int dest_top, int width, int height,
                                    const CFX_DIBSource* pSrcBitmap, int src_left, int src_top,
                                    int blend_type = FXDIB_BLEND_NORMAL, const CFX_ClipRgn* pClipRgn = NULL, FX_BOOL bRgbByteOrder = FALSE, void* pIccTransform = NULL);

    FX_BOOL			TransferMask(int dest_left, int dest_top, int width, int height,
                                 const CFX_DIBSource* pMask, FX_DWORD color, int src_left, int src_top, int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			CompositeMask(int dest_left, int dest_top, int width, int height,
                                  const CFX_DIBSource* pMask, FX_DWORD color, int src_left, int src_top,
                                  int blend_type = FXDIB_BLEND_NORMAL, const CFX_ClipRgn* pClipRgn = NULL, FX_BOOL bRgbByteOrder = FALSE, int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			CompositeRect(int dest_left, int dest_top, int width, int height, FX_DWORD color, int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			ConvertColorScale(FX_DWORD forecolor, FX_DWORD backcolor);

    FX_BOOL			DitherFS(const FX_DWORD* pPalette, int pal_size, const FX_RECT* pRect = NULL);
protected:

    FX_LPBYTE		m_pBuffer;

    FX_BOOL			m_bExtBuf;

    FX_BOOL			GetGrayData(void* pIccTransform = NULL);
};
class CFX_DIBExtractor : public CFX_Object
{
public:

    CFX_DIBExtractor(const CFX_DIBSource* pSrc);

    ~CFX_DIBExtractor();

    operator CFX_DIBitmap*()
    {
        return m_pBitmap;
    }
private:

    CFX_DIBitmap*			m_pBitmap;
};
typedef CFX_CountRef<CFX_DIBitmap> CFX_DIBitmapRef;
class CFX_FilteredDIB : public CFX_DIBSource
{
public:

    CFX_FilteredDIB();

    ~CFX_FilteredDIB();

    void					LoadSrc(const CFX_DIBSource* pSrc, FX_BOOL bAutoDropSrc = FALSE);

    virtual FXDIB_Format	GetDestFormat() = 0;

    virtual FX_DWORD*		GetDestPalette() = 0;


    virtual void			TranslateScanline(FX_LPBYTE dest_buf, FX_LPCBYTE src_buf) const = 0;

    virtual void			TranslateDownSamples(FX_LPBYTE dest_buf, FX_LPCBYTE src_buf, int pixels, int Bpp) const = 0;
protected:
    virtual FX_LPCBYTE		GetScanline(int line) const;
    virtual void			DownSampleScanline(int line, FX_LPBYTE dest_scan, int dest_bpp,
            int dest_width, FX_BOOL bFlipX, int clip_left, int clip_width) const;

    const CFX_DIBSource*	m_pSrc;

    FX_BOOL					m_bAutoDropSrc;

    FX_LPBYTE				m_pScanline;
};
class IFX_ScanlineComposer
{
public:
    virtual ~IFX_ScanlineComposer() { }

    virtual	void		ComposeScanline(int line, FX_LPCBYTE scanline, FX_LPCBYTE scan_extra_alpha = NULL) = 0;

    virtual FX_BOOL		SetInfo(int width, int height, FXDIB_Format src_format, FX_DWORD* pSrcPalette) = 0;
};
class CFX_ScanlineCompositor : public CFX_Object
{
public:
    CFX_ScanlineCompositor();

    ~CFX_ScanlineCompositor();

    FX_BOOL				Init(FXDIB_Format dest_format, FXDIB_Format src_format, FX_INT32 width, FX_DWORD* pSrcPalette,
                             FX_DWORD mask_color, int blend_type, FX_BOOL bClip, FX_BOOL bRgbByteOrder = FALSE, int alpha_flag = 0, void* pIccTransform = NULL);


    void				CompositeRgbBitmapLine(FX_LPBYTE dest_scan, FX_LPCBYTE src_scan, int width, FX_LPCBYTE clip_scan,
            FX_LPCBYTE src_extra_alpha = NULL, FX_LPBYTE dst_extra_alpha = NULL);


    void				CompositePalBitmapLine(FX_LPBYTE dest_scan, FX_LPCBYTE src_scan, int src_left, int width, FX_LPCBYTE clip_scan,
            FX_LPCBYTE src_extra_alpha = NULL, FX_LPBYTE dst_extra_alpha = NULL);


    void				CompositeByteMaskLine(FX_LPBYTE dest_scan, FX_LPCBYTE src_scan, int width, FX_LPCBYTE clip_scan,
            FX_LPBYTE dst_extra_alpha = NULL);


    void				CompositeBitMaskLine(FX_LPBYTE dest_scan, FX_LPCBYTE src_scan, int src_left, int width, FX_LPCBYTE clip_scan,
            FX_LPBYTE dst_extra_alpha = NULL);
protected:
    int					m_Transparency;
    FXDIB_Format		m_SrcFormat,
                        m_DestFormat;
    FX_DWORD*			m_pSrcPalette;

    int					m_MaskAlpha,
                        m_MaskRed,
                        m_MaskGreen,
                        m_MaskBlue,
                        m_MaskBlack;
    int					m_BlendType;
    void*				m_pIccTransform;
    FX_LPBYTE			m_pCacheScanline;
    int					m_CacheSize;
    FX_BOOL             m_bRgbByteOrder;
};
class CFX_BitmapComposer : public IFX_ScanlineComposer, public CFX_Object
{
public:

    CFX_BitmapComposer();

    ~CFX_BitmapComposer();


    void				Compose(CFX_DIBitmap* pDest, const CFX_ClipRgn* pClipRgn, int bitmap_alpha,
                                FX_DWORD mask_color, FX_RECT& dest_rect, FX_BOOL bVertical,
                                FX_BOOL bFlipX, FX_BOOL bFlipY, FX_BOOL bRgbByteOrder = FALSE,
                                int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);

    virtual FX_BOOL		SetInfo(int width, int height, FXDIB_Format src_format, FX_DWORD* pSrcPalette);


    virtual	void		ComposeScanline(int line, FX_LPCBYTE scanline, FX_LPCBYTE scan_extra_alpha);
protected:

    void				DoCompose(FX_LPBYTE dest_scan, FX_LPCBYTE src_scan, int dest_width, FX_LPCBYTE clip_scan,
                                  FX_LPCBYTE src_extra_alpha = NULL, FX_LPBYTE dst_extra_alpha = NULL);
    CFX_DIBitmap*		m_pBitmap;
    const CFX_ClipRgn*	m_pClipRgn;
    FXDIB_Format		m_SrcFormat;
    int					m_DestLeft, m_DestTop, m_DestWidth, m_DestHeight, m_BitmapAlpha;
    FX_DWORD			m_MaskColor;
    const CFX_DIBitmap*	m_pClipMask;
    CFX_ScanlineCompositor	m_Compositor;
    FX_BOOL				m_bVertical, m_bFlipX, m_bFlipY;
    int					m_AlphaFlag;
    void*				m_pIccTransform;
    FX_BOOL             m_bRgbByteOrder;
    int					m_BlendType;
    void				ComposeScanlineV(int line, FX_LPCBYTE scanline, FX_LPCBYTE scan_extra_alpha = NULL);
    FX_LPBYTE			m_pScanlineV, m_pClipScanV, m_pAddClipScan, m_pScanlineAlphaV;
};
class CFX_BitmapStorer : public IFX_ScanlineComposer, public CFX_Object
{
public:

    CFX_BitmapStorer();

    ~CFX_BitmapStorer();

    virtual	void		ComposeScanline(int line, FX_LPCBYTE scanline, FX_LPCBYTE scan_extra_alpha);

    virtual FX_BOOL		SetInfo(int width, int height, FXDIB_Format src_format, FX_DWORD* pSrcPalette);

    CFX_DIBitmap*		GetBitmap()
    {
        return m_pBitmap;
    }

    CFX_DIBitmap*		Detach();

    void				Replace(CFX_DIBitmap* pBitmap);
private:
    CFX_DIBitmap*		m_pBitmap;
};
class CStretchEngine;
class CFX_ImageStretcher : public CFX_Object
{
public:

    CFX_ImageStretcher();

    ~CFX_ImageStretcher();

    FX_INT32		Start(IFX_ScanlineComposer* pDest, const CFX_DIBSource* pBitmap,
                          int dest_width, int dest_height, const FX_RECT& bitmap_rect, FX_DWORD flags);


    FX_INT32		Continue(IFX_Pause* pPause);
    IFX_ScanlineComposer*	m_pDest;
    const CFX_DIBSource*	m_pSource;
    CStretchEngine*		m_pStretchEngine;
    FX_DWORD		m_Flags;
    FX_BOOL			m_bFlipX,
                    m_bFlipY;
    int				m_DestWidth,
                    m_DestHeight;
    FX_RECT			m_ClipRect;
    int				m_LineIndex;
    int				m_DestBPP;
    FX_LPBYTE		m_pScanline;
    FX_LPBYTE       m_pMaskScanline;
    FXDIB_Format	m_DestFormat;
    FX_INT32		m_Status;

    FX_INT32		StartQuickStretch();

    FX_INT32		StartStretch();

    FX_INT32		ContinueQuickStretch(IFX_Pause* pPause);

    FX_INT32		ContinueStretch(IFX_Pause* pPause);
};
class CFX_ImageTransformer : public CFX_Object
{
public:

    CFX_ImageTransformer();

    ~CFX_ImageTransformer();

    FX_INT32	Start(const CFX_DIBSource* pSrc, const CFX_AffineMatrix* pMatrix, int flags, const FX_RECT* pClip);


    FX_INT32	Continue(IFX_Pause* pPause);
    CFX_AffineMatrix* m_pMatrix;
    FX_RECT		m_StretchClip;
    int			m_ResultLeft, m_ResultTop, m_ResultWidth, m_ResultHeight;
    CFX_AffineMatrix	m_dest2stretch;
    CFX_ImageStretcher	m_Stretcher;
    CFX_BitmapStorer	m_Storer;
    FX_DWORD	m_Flags;
    int			m_Status;
};
class CFX_ImageRenderer : public CFX_Object
{
public:

    CFX_ImageRenderer();

    ~CFX_ImageRenderer();

    FX_INT32			Start(CFX_DIBitmap* pDevice, const CFX_ClipRgn* pClipRgn,
                              const CFX_DIBSource* pSource, int bitmap_alpha,
                              FX_DWORD mask_color, const CFX_AffineMatrix* pMatrix, FX_DWORD dib_flags,
                              FX_BOOL bRgbByteOrder = FALSE, int alpha_flag = 0, void* pIccTransform = NULL,
                              int blend_type = FXDIB_BLEND_NORMAL);

    FX_INT32			Continue(IFX_Pause* pPause);
protected:
    CFX_DIBitmap*		m_pDevice;
    const CFX_ClipRgn*	m_pClipRgn;
    int					m_BitmapAlpha;
    FX_DWORD			m_MaskColor;
    CFX_AffineMatrix	m_Matrix;
    CFX_ImageTransformer*	m_pTransformer;
    CFX_ImageStretcher	m_Stretcher;
    CFX_BitmapComposer	m_Composer;
    int					m_Status;
    int					m_DestLeft, m_DestTop;
    FX_RECT				m_ClipBox;
    FX_DWORD			m_Flags;
    int					m_AlphaFlag;
    void*				m_pIccTransform;
    FX_BOOL				m_bRgbByteOrder;
    int					m_BlendType;
};
#endif

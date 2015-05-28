// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FXGE_FX_GE_H_
#define CORE_INCLUDE_FXGE_FX_GE_H_

#include "fx_dib.h"
#include "fx_font.h"

class CFX_ClipRgn;
class CFX_PathData;
class CFX_GraphStateData;
class CFX_Font;
class CFX_FontMgr;
class CFX_FontCache;
class CFX_FaceCache;
class CFX_RenderDevice;
class IFX_RenderDeviceDriver;
class CCodec_ModuleMgr;
class IFXG_PaintModuleMgr;

class CFX_GEModule 
{
public:

    static void				Create();

    static void				Use(CFX_GEModule* pMgr);

    static CFX_GEModule*	Get();

    static void				Destroy();
public:

    CFX_FontCache*			GetFontCache();
    CFX_FontMgr*			GetFontMgr()
    {
        return m_pFontMgr;
    }
    void					SetTextGamma(FX_FLOAT gammaValue);
    FX_LPCBYTE				GetTextGammaTable();
    void					SetExtFontMapper(IFX_FontMapper* pFontMapper);

    void					SetCodecModule(CCodec_ModuleMgr* pCodecModule)
    {
        m_pCodecModule = pCodecModule;
    }
    CCodec_ModuleMgr*		GetCodecModule()
    {
        return m_pCodecModule;
    }
    FXFT_Library			m_FTLibrary;
    void*					GetPlatformData()
    {
        return m_pPlatformData;
    }
protected:

    CFX_GEModule();

    ~CFX_GEModule();
    void					InitPlatform();
    void					DestroyPlatform();
private:
    FX_BYTE					m_GammaValue[256];
    CFX_FontCache*			m_pFontCache;
    CFX_FontMgr*			m_pFontMgr;
    CCodec_ModuleMgr*		m_pCodecModule;
    void*					m_pPlatformData;
};
typedef struct {

    FX_FLOAT			m_PointX;

    FX_FLOAT			m_PointY;

    int					m_Flag;
} FX_PATHPOINT;
#define FXPT_CLOSEFIGURE		0x01
#define FXPT_LINETO				0x02
#define FXPT_BEZIERTO			0x04
#define FXPT_MOVETO				0x06
#define FXPT_TYPE				0x06
#define FXFILL_ALTERNATE		1
#define FXFILL_WINDING			2
class CFX_ClipRgn 
{
public:

    CFX_ClipRgn(int device_width, int device_height);

    CFX_ClipRgn(const FX_RECT& rect);

    CFX_ClipRgn(const CFX_ClipRgn& src);

    ~CFX_ClipRgn();

    typedef enum {
        RectI,
        MaskF
    } ClipType;

    void			Reset(const FX_RECT& rect);

    ClipType		GetType() const
    {
        return m_Type;
    }

    const FX_RECT&	GetBox() const
    {
        return m_Box;
    }

    CFX_DIBitmapRef	GetMask() const
    {
        return m_Mask;
    }

    void			IntersectRect(const FX_RECT& rect);

    void			IntersectMaskF(int left, int top, CFX_DIBitmapRef Mask);
protected:

    ClipType		m_Type;

    FX_RECT			m_Box;

    CFX_DIBitmapRef	m_Mask;

    void			IntersectMaskRect(FX_RECT rect, FX_RECT mask_box, CFX_DIBitmapRef Mask);
};
extern const FX_BYTE g_GammaRamp[256];
extern const FX_BYTE g_GammaInverse[256];
#define FX_GAMMA(value)			(value)
#define FX_GAMMA_INVERSE(value)	(value)
inline FX_ARGB ArgbGamma(FX_ARGB argb)
{
    return argb;
}
inline FX_ARGB ArgbGammaInverse(FX_ARGB argb)
{
    return argb;
}
class CFX_PathData 
{
public:

    CFX_PathData();

    CFX_PathData(const CFX_PathData& src);

    ~CFX_PathData();




    int					GetPointCount() const
    {
        return m_PointCount;
    }

    int					GetFlag(int index) const
    {
        return m_pPoints[index].m_Flag;
    }

    FX_FLOAT			GetPointX(int index) const
    {
        return m_pPoints[index].m_PointX;
    }

    FX_FLOAT			GetPointY(int index) const
    {
        return m_pPoints[index].m_PointY;
    }



    FX_PATHPOINT*		GetPoints() const
    {
        return m_pPoints;
    }

    void SetPointCount(int nPoints);
    void AllocPointCount(int nPoints);
    void AddPointCount(int addPoints);

    CFX_FloatRect		GetBoundingBox() const;

    CFX_FloatRect		GetBoundingBox(FX_FLOAT line_width, FX_FLOAT miter_limit) const;

    void				Transform(const CFX_AffineMatrix* pMatrix);

    FX_BOOL				IsRect() const;

    FX_BOOL				GetZeroAreaPath(CFX_PathData& NewPath, CFX_AffineMatrix* pMatrix, FX_BOOL&bThin, FX_BOOL bAdjust) const;

    FX_BOOL				IsRect(const CFX_AffineMatrix* pMatrix, CFX_FloatRect* rect) const;

    void Append(const CFX_PathData* pSrc, const CFX_AffineMatrix* pMatrix);
    void AppendRect(FX_FLOAT left, FX_FLOAT bottom, FX_FLOAT right, FX_FLOAT top);

    void				SetPoint(int index, FX_FLOAT x, FX_FLOAT y, int flag);

    void				TrimPoints(int nPoints);

    void Copy(const CFX_PathData &src);
protected:
    friend class		CPDF_Path;

    int					m_PointCount;

    FX_PATHPOINT*		m_pPoints;

    int					m_AllocCount;
};
class CFX_GraphStateData 
{
public:

    CFX_GraphStateData();

    CFX_GraphStateData(const CFX_GraphStateData& src);

    ~CFX_GraphStateData();

    void				Copy(const CFX_GraphStateData& src);

    void				SetDashCount(int count);



    typedef enum {
        LineCapButt = 0,
        LineCapRound = 1,
        LineCapSquare = 2
    } LineCap;
    LineCap				m_LineCap;
    int					m_DashCount;
    FX_FLOAT*		m_DashArray;
    FX_FLOAT			m_DashPhase;

    typedef enum {
        LineJoinMiter = 0,
        LineJoinRound = 1,
        LineJoinBevel = 2,
    } LineJoin;
    LineJoin			m_LineJoin;
    FX_FLOAT			m_MiterLimit;
    FX_FLOAT			m_LineWidth;

};
#define FXDC_DEVICE_CLASS			1
#define FXDC_PIXEL_WIDTH			2
#define FXDC_PIXEL_HEIGHT			3
#define FXDC_BITS_PIXEL				4
#define FXDC_HORZ_SIZE				5
#define FXDC_VERT_SIZE				6
#define FXDC_RENDER_CAPS			7
#define FXDC_DITHER_BITS			8
#define FXDC_DISPLAY				1
#define FXDC_PRINTER				2
#define FXRC_GET_BITS				0x01
#define FXRC_BIT_MASK				0x02
#define FXRC_ALPHA_MASK				0x04
#define FXRC_ALPHA_PATH				0x10
#define FXRC_ALPHA_IMAGE			0x20
#define FXRC_ALPHA_OUTPUT			0x40
#define FXRC_BLEND_MODE				0x80
#define FXRC_SOFT_CLIP				0x100
#define FXRC_CMYK_OUTPUT			0x200
#define FXRC_BITMASK_OUTPUT         0x400
#define FXRC_BYTEMASK_OUTPUT        0x800
#define FXRENDER_IMAGE_LOSSY		0x1000
#define FXFILL_ALTERNATE		1
#define FXFILL_WINDING			2
#define FXFILL_FULLCOVER		4
#define FXFILL_RECT_AA			8
#define FX_FILL_STROKE			16
#define FX_STROKE_ADJUST		32
#define FX_STROKE_TEXT_MODE		64
#define FX_FILL_TEXT_MODE		128
#define FX_ZEROAREA_FILL		256
#define FXFILL_NOPATHSMOOTH		512
#define FXTEXT_CLEARTYPE			0x01
#define FXTEXT_BGR_STRIPE			0x02
#define FXTEXT_PRINTGRAPHICTEXT		0x04
#define FXTEXT_NO_NATIVETEXT		0x08
#define FXTEXT_PRINTIMAGETEXT		0x10
#define FXTEXT_NOSMOOTH				0x20
typedef struct {
    FX_DWORD			m_GlyphIndex;
    FX_FLOAT			m_OriginX, m_OriginY;
    int					m_FontCharWidth;
    FX_BOOL				m_bGlyphAdjust;
    FX_FLOAT			m_AdjustMatrix[4];
    FX_DWORD			m_ExtGID;
    FX_BOOL				m_bFontStyle;
} FXTEXT_CHARPOS;
class CFX_RenderDevice 
{
public:
    CFX_RenderDevice();

    virtual ~CFX_RenderDevice();

    void			SetDeviceDriver(IFX_RenderDeviceDriver* pDriver);

    IFX_RenderDeviceDriver*	GetDeviceDriver() const
    {
        return m_pDeviceDriver;
    }

    FX_BOOL			StartRendering();

    void			EndRendering();



    void			SaveState();

    void			RestoreState(FX_BOOL bKeepSaved = FALSE);




    int				GetWidth() const
    {
        return m_Width;
    }

    int				GetHeight() const
    {
        return m_Height;
    }

    int				GetDeviceClass() const
    {
        return m_DeviceClass;
    }

    int				GetBPP() const
    {
        return m_bpp;
    }

    int				GetRenderCaps() const
    {
        return m_RenderCaps;
    }

    int				GetDeviceCaps(int id) const;

    CFX_Matrix		GetCTM() const;


    CFX_DIBitmap*	GetBitmap() const
    {
        return m_pBitmap;
    }
    void			SetBitmap(CFX_DIBitmap* pBitmap)
    {
        m_pBitmap = pBitmap;
    }

    FX_BOOL			CreateCompatibleBitmap(CFX_DIBitmap* pDIB, int width, int height) const;

    const FX_RECT&	GetClipBox() const
    {
        return m_ClipBox;
    }

    FX_BOOL			SetClip_PathFill(const CFX_PathData* pPathData,
                                     const CFX_AffineMatrix* pObject2Device,
                                     int fill_mode
                              );

    FX_BOOL			SetClip_Rect(const FX_RECT* pRect);

    FX_BOOL			SetClip_PathStroke(const CFX_PathData* pPathData,
                                       const CFX_AffineMatrix* pObject2Device,
                                       const CFX_GraphStateData* pGraphState
                                );

    FX_BOOL			DrawPath(const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_color,
                             FX_DWORD stroke_color,
                             int fill_mode,
                             int alpha_flag = 0,
                             void* pIccTransform = NULL,
                             int blend_type = FXDIB_BLEND_NORMAL
                      );

    FX_BOOL			SetPixel(int x, int y, FX_DWORD color,
                             int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			FillRect(const FX_RECT* pRect, FX_DWORD color,
                             int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);

    FX_BOOL			DrawCosmeticLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_DWORD color,
                                     int fill_mode = 0, int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);

    FX_BOOL			GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform = NULL);

    CFX_DIBitmap*   GetBackDrop();

    FX_BOOL			SetDIBits(const CFX_DIBSource* pBitmap, int left, int top, int blend_type = FXDIB_BLEND_NORMAL,
                              void* pIccTransform = NULL);

    FX_BOOL			StretchDIBits(const CFX_DIBSource* pBitmap, int left, int top, int dest_width, int dest_height,
                                  FX_DWORD flags = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);

    FX_BOOL			SetBitMask(const CFX_DIBSource* pBitmap, int left, int top, FX_DWORD color,
                               int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			StretchBitMask(const CFX_DIBSource* pBitmap, int left, int top, int dest_width, int dest_height,
                                   FX_DWORD color, FX_DWORD flags = 0, int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD flags, FX_LPVOID& handle,
                                int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);

    FX_BOOL			ContinueDIBits(FX_LPVOID handle, IFX_Pause* pPause);

    void			CancelDIBits(FX_LPVOID handle);

    FX_BOOL			DrawNormalText(int nChars, const FXTEXT_CHARPOS* pCharPos,
                                   CFX_Font* pFont, CFX_FontCache* pCache,
                                   FX_FLOAT font_size, const CFX_AffineMatrix* pText2Device,
                                   FX_DWORD fill_color, FX_DWORD text_flags,
                                   int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			DrawTextPath(int nChars, const FXTEXT_CHARPOS* pCharPos,
                                 CFX_Font* pFont, CFX_FontCache* pCache,
                                 FX_FLOAT font_size, const CFX_AffineMatrix* pText2User,
                                 const CFX_AffineMatrix* pUser2Device, const CFX_GraphStateData* pGraphState,
                                 FX_DWORD fill_color, FX_DWORD stroke_color, CFX_PathData* pClippingPath, int nFlag = 0,
                                 int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL);
    virtual void Begin() {}
    virtual void End() {}
private:

    CFX_DIBitmap*	m_pBitmap;



    int				m_Width;

    int				m_Height;

    int				m_bpp;

    int				m_RenderCaps;

    int				m_DeviceClass;

    FX_RECT			m_ClipBox;

protected:

    IFX_RenderDeviceDriver*	m_pDeviceDriver;
private:

    void			InitDeviceInfo();

    void			UpdateClipBox();
};
class CFX_FxgeDevice : public CFX_RenderDevice
{
public:

    CFX_FxgeDevice();

    ~CFX_FxgeDevice();

    FX_BOOL			Attach(CFX_DIBitmap* pBitmap, int dither_bits = 0, FX_BOOL bRgbByteOrder = FALSE, CFX_DIBitmap* pOriDevice = NULL, FX_BOOL bGroupKnockout = FALSE);

    FX_BOOL			Create(int width, int height, FXDIB_Format format, int dither_bits = 0, CFX_DIBitmap* pOriDevice = NULL);
protected:

    FX_BOOL			m_bOwnedBitmap;
};
class CFX_SkiaDevice : public CFX_RenderDevice
{
public:

    CFX_SkiaDevice();

    ~CFX_SkiaDevice();

    FX_BOOL			Attach(CFX_DIBitmap* pBitmap, int dither_bits = 0, FX_BOOL bRgbByteOrder = FALSE, CFX_DIBitmap* pOriDevice = NULL, FX_BOOL bGroupKnockout = FALSE);

    FX_BOOL			Create(int width, int height, FXDIB_Format format, int dither_bits = 0, CFX_DIBitmap* pOriDevice = NULL);
protected:

    FX_BOOL			m_bOwnedBitmap;
};
class IFX_RenderDeviceDriver 
{
public:

    static IFX_RenderDeviceDriver*		CreateFxgeDriver(CFX_DIBitmap* pBitmap, FX_BOOL bRgbByteOrder = FALSE,
            CFX_DIBitmap* pOriDevice = NULL, FX_BOOL bGroupKnockout = FALSE);

    virtual ~IFX_RenderDeviceDriver() {}
    virtual void Begin() { }
    virtual void End() { }

    virtual int		GetDeviceCaps(int caps_id) = 0;

    virtual CFX_Matrix	GetCTM() const
    {
        return CFX_Matrix();
    }

    virtual FX_BOOL IsPSPrintDriver()
    {
        return FALSE;
    }

    virtual FX_BOOL	StartRendering()
    {
        return TRUE;
    }

    virtual void	EndRendering() {}




    virtual void	SaveState() = 0;

    virtual void	RestoreState(FX_BOOL bKeepSaved = FALSE) = 0;


    virtual FX_BOOL	SetClip_PathFill(const CFX_PathData* pPathData,
                                     const CFX_AffineMatrix* pObject2Device,
                                     int fill_mode
                                    ) = 0;

    virtual FX_BOOL	SetClip_PathStroke(const CFX_PathData* pPathData,
                                       const CFX_AffineMatrix* pObject2Device,
                                       const CFX_GraphStateData* pGraphState
                                      )
    {
        return FALSE;
    }

    virtual FX_BOOL	DrawPath(const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_color,
                             FX_DWORD stroke_color,
                             int fill_mode,
                             int alpha_flag = 0,
                             void* pIccTransform = NULL,
                             int blend_type = FXDIB_BLEND_NORMAL
                            ) = 0;

    virtual FX_BOOL	SetPixel(int x, int y, FX_DWORD color,
                             int alpha_flag = 0, void* pIccTransform = NULL)
    {
        return FALSE;
    }

    virtual FX_BOOL FillRect(const FX_RECT* pRect, FX_DWORD fill_color,
                             int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL)
    {
        return FALSE;
    }

    virtual FX_BOOL	DrawCosmeticLine(FX_FLOAT x1, FX_FLOAT y1, FX_FLOAT x2, FX_FLOAT y2, FX_DWORD color,
                                     int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL)
    {
        return FALSE;
    }

    virtual FX_BOOL GetClipBox(FX_RECT* pRect) = 0;

    virtual FX_BOOL	GetDIBits(CFX_DIBitmap* pBitmap, int left, int top, void* pIccTransform = NULL, FX_BOOL bDEdge = FALSE)
    {
        return FALSE;
    }
    virtual CFX_DIBitmap*   GetBackDrop()
    {
        return NULL;
    }

    virtual FX_BOOL	SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, const FX_RECT* pSrcRect,
                              int dest_left, int dest_top, int blend_type,
                              int alpha_flag = 0, void* pIccTransform = NULL) = 0;

    virtual FX_BOOL	StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, const FX_RECT* pClipRect, FX_DWORD flags,
                                  int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL) = 0;

    virtual FX_BOOL	StartDIBits(const CFX_DIBSource* pBitmap, int bitmap_alpha, FX_DWORD color,
                                const CFX_AffineMatrix* pMatrix, FX_DWORD flags, FX_LPVOID& handle,
                                int alpha_flag = 0, void* pIccTransform = NULL, int blend_type = FXDIB_BLEND_NORMAL) = 0;

    virtual FX_BOOL	ContinueDIBits(FX_LPVOID handle, IFX_Pause* pPause)
    {
        return FALSE;
    }

    virtual void	CancelDIBits(FX_LPVOID handle) {}

    virtual FX_BOOL DrawDeviceText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont,
                                   CFX_FontCache* pCache, const CFX_AffineMatrix* pObject2Device, FX_FLOAT font_size, FX_DWORD color,
                                   int alpha_flag = 0, void* pIccTransform = NULL)
    {
        return FALSE;
    }

    virtual void*	GetPlatformSurface()
    {
        return NULL;
    }

    virtual int		GetDriverType()
    {
        return 0;
    }

    virtual void    ClearDriver() {}
};
class IFX_PSOutput
{
public:
    virtual void  Release() = 0;
    virtual void	OutputPS(FX_LPCSTR string, int len) = 0;

protected:
    ~IFX_PSOutput() { }
};
class CPSFont;
class CFX_PSRenderer 
{
public:

    CFX_PSRenderer();

    ~CFX_PSRenderer();

    void			Init(IFX_PSOutput* pOutput, int ps_level, int width, int height, FX_BOOL bCmykOutput);
    FX_BOOL			StartRendering();
    void			EndRendering();

    void			SaveState();

    void			RestoreState(FX_BOOL bKeepSaved = FALSE);

    void			SetClip_PathFill(const CFX_PathData* pPathData,
                                     const CFX_AffineMatrix* pObject2Device,
                                     int fill_mode
                           );

    void			SetClip_PathStroke(const CFX_PathData* pPathData,
                                       const CFX_AffineMatrix* pObject2Device,
                                       const CFX_GraphStateData* pGraphState
                             );

    FX_RECT			GetClipBox()
    {
        return m_ClipBox;
    }

    FX_BOOL			DrawPath(const CFX_PathData* pPathData,
                             const CFX_AffineMatrix* pObject2Device,
                             const CFX_GraphStateData* pGraphState,
                             FX_DWORD fill_color,
                             FX_DWORD stroke_color,
                             int fill_mode,
                             int alpha_flag = 0,
                             void* pIccTransform = NULL
                      );

    FX_BOOL			SetDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                              int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			StretchDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color, int dest_left, int dest_top,
                                  int dest_width, int dest_height, FX_DWORD flags,
                                  int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			DrawDIBits(const CFX_DIBSource* pBitmap, FX_DWORD color,
                               const CFX_AffineMatrix* pMatrix, FX_DWORD flags,
                               int alpha_flag = 0, void* pIccTransform = NULL);

    FX_BOOL			DrawText(int nChars, const FXTEXT_CHARPOS* pCharPos, CFX_Font* pFont, CFX_FontCache* pCache,
                             const CFX_AffineMatrix* pObject2Device, FX_FLOAT font_size, FX_DWORD color,
                             int alpha_flag = 0, void* pIccTransform = NULL);
private:

    IFX_PSOutput*	m_pOutput;

    int				m_PSLevel;

    CFX_GraphStateData	m_CurGraphState;

    FX_BOOL			m_bGraphStateSet;

    FX_BOOL			m_bCmykOutput;

    FX_BOOL			m_bColorSet;

    FX_DWORD		m_LastColor;

    FX_RECT			m_ClipBox;

    CFX_ArrayTemplate<CPSFont*>	m_PSFontList;

    CFX_ArrayTemplate<FX_RECT>	m_ClipBoxStack;
    FX_BOOL			m_bInited;

    void			OutputPath(const CFX_PathData* pPathData, const CFX_AffineMatrix* pObject2Device);

    void			SetGraphState(const CFX_GraphStateData* pGraphState);

    void			SetColor(FX_DWORD color, int alpha_flag, void* pIccTransform);

    void			FindPSFontGlyph(CFX_FaceCache* pFaceCache, CFX_Font* pFont, const FXTEXT_CHARPOS& charpos, int& ps_fontnum, int &ps_glyphindex);

    void			WritePSBinary(FX_LPCBYTE data, int len);
};

#endif  // CORE_INCLUDE_FXGE_FX_GE_H_

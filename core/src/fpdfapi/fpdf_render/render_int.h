// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDF_PAGEOBJ_H_
#include "../../../include/fpdfapi/fpdf_pageobj.h"
#endif
class CPDF_QuickStretcher;
#define TYPE3_MAX_BLUES		16
class CPDF_Type3Glyphs : public CFX_Object
{
public:
    CPDF_Type3Glyphs()
    {
        m_GlyphMap.InitHashTable(253);
        m_TopBlueCount = m_BottomBlueCount = 0;
    }
    ~CPDF_Type3Glyphs();
    CFX_MapPtrToPtr			m_GlyphMap;
    void					AdjustBlue(FX_FLOAT top, FX_FLOAT bottom, int& top_line, int& bottom_line);

    int						m_TopBlue[TYPE3_MAX_BLUES], m_BottomBlue[TYPE3_MAX_BLUES];
    int						m_TopBlueCount, m_BottomBlueCount;
};
class CFX_GlyphBitmap;
class CPDF_Type3Cache : public CFX_Object
{
public:
    CPDF_Type3Cache(CPDF_Type3Font* pFont)
    {
        m_pFont = pFont;
    }
    ~CPDF_Type3Cache();
    CFX_GlyphBitmap*		LoadGlyph(FX_DWORD charcode, const CFX_AffineMatrix* pMatrix, FX_FLOAT retinaScaleX = 1.0f, FX_FLOAT retinaScaleY = 1.0f);
protected:
    CFX_GlyphBitmap*		RenderGlyph(CPDF_Type3Glyphs* pSize, FX_DWORD charcode, const CFX_AffineMatrix* pMatrix, FX_FLOAT retinaScaleX = 1.0f, FX_FLOAT retinaScaleY = 1.0f);
    CPDF_Type3Font*			m_pFont;
    CFX_MapByteStringToPtr	m_SizeMap;
};
class CPDF_TransferFunc : public CFX_Object
{
public:
    CPDF_Document*	m_pPDFDoc;
    FX_BYTE			m_Samples[256 * 3];
    FX_BOOL			m_bIdentity;

    CFX_DIBSource*	TranslateImage(const CFX_DIBSource* pSrc, FX_BOOL bAutoDropSrc);
    FX_COLORREF		TranslateColor(FX_COLORREF src);
};
typedef CFX_MapPtrTemplate<CPDF_Font*, CPDF_CountedObject<CPDF_Type3Cache*>*> CPDF_Type3CacheMap;
typedef CFX_MapPtrTemplate<CPDF_Object*, CPDF_CountedObject<CPDF_TransferFunc*>*> CPDF_TransferFuncMap;
class CPDF_DocRenderData : public CFX_Object
{
public:
    CPDF_DocRenderData(CPDF_Document* pPDFDoc = NULL);
    ~CPDF_DocRenderData();
    FX_BOOL				Initialize();
    CPDF_Type3Cache*	GetCachedType3(CPDF_Type3Font* pFont);
    CPDF_TransferFunc*	GetTransferFunc(CPDF_Object* pObj);
    CFX_FontCache*		GetFontCache()
    {
        return m_pFontCache;
    }
    void				Clear(FX_BOOL bRelease = FALSE);
    void				ReleaseCachedType3(CPDF_Type3Font* pFont);
    void				ReleaseTransferFunc(CPDF_Object* pObj);
private:
    CPDF_Document*		m_pPDFDoc;
    CFX_FontCache*		m_pFontCache;
    CPDF_Type3CacheMap	m_Type3FaceMap;
    CPDF_TransferFuncMap	m_TransferFuncMap;
};
struct _PDF_RenderItem {
public:
    CPDF_PageObjects*			m_pObjectList;
    CFX_AffineMatrix			m_Matrix;
};
typedef CFX_ArrayTemplate<_PDF_RenderItem>	CPDF_RenderLayer;
class IPDF_ObjectRenderer : public CFX_Object
{
public:
    static IPDF_ObjectRenderer* Create(int type);
    virtual ~IPDF_ObjectRenderer() {}
    virtual FX_BOOL Start(CPDF_RenderStatus* pRenderStatus, const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bStdCS, int blendType = FXDIB_BLEND_NORMAL) = 0;
    virtual FX_BOOL Continue(IFX_Pause* pPause) = 0;
    FX_BOOL		m_Result;
};
class CPDF_RenderStatus : public CFX_Object
{
public:
    CPDF_RenderStatus();
    ~CPDF_RenderStatus();
    FX_BOOL			Initialize(class CPDF_RenderContext* pContext, CFX_RenderDevice* pDevice, const CFX_AffineMatrix* pDeviceMatrix,
                               const CPDF_PageObject* pStopObj, const CPDF_RenderStatus* pParentStatus,
                               const CPDF_GraphicStates* pInitialStates, const CPDF_RenderOptions* pOptions,
                               int transparency, FX_BOOL bDropObjects, CPDF_Dictionary* pFormResource = NULL,
                               FX_BOOL bStdCS = FALSE,	CPDF_Type3Char* pType3Char = NULL, FX_ARGB fill_color = 0,
                               FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE);
    void			RenderObjectList(const CPDF_PageObjects* pObjs, const CFX_AffineMatrix* pObj2Device);
    void			RenderSingleObject(const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device);
    FX_BOOL			ContinueSingleObject(const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device, IFX_Pause* pPause);
    CPDF_RenderOptions	m_Options;
    CPDF_Dictionary*    m_pFormResource;
    CPDF_Dictionary*    m_pPageResource;
    CFX_PtrArray		m_Type3FontCache;
    CPDF_RenderContext* GetContext()
    {
        return m_pContext;
    }
protected:
    friend class	CPDF_ImageRenderer;
    friend class	CPDF_RenderContext;
    void			ProcessClipPath(CPDF_ClipPath ClipPath, const CFX_AffineMatrix* pObj2Device);
    void			DrawClipPath(CPDF_ClipPath ClipPath, const CFX_AffineMatrix* pObj2Device);
    FX_BOOL			ProcessTransparency(const CPDF_PageObject* PageObj, const CFX_AffineMatrix* pObj2Device);
    void			ProcessObjectNoClip(const CPDF_PageObject* PageObj, const CFX_AffineMatrix* pObj2Device);
    void			DrawObjWithBackground(const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device);
    FX_BOOL         DrawObjWithBlend(const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device);
    FX_BOOL			ProcessPath(CPDF_PathObject* pPathObj, const CFX_AffineMatrix* pObj2Device);
    void			ProcessPathPattern(CPDF_PathObject* pPathObj, const CFX_AffineMatrix* pObj2Device, int& filltype, FX_BOOL& bStroke);
    void			DrawPathWithPattern(CPDF_PathObject* pPathObj, const CFX_AffineMatrix* pObj2Device, CPDF_Color* pColor, FX_BOOL bStroke);
    void			DrawTilingPattern(CPDF_TilingPattern* pPattern, CPDF_PageObject* pPageObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bStroke);
    void			DrawShadingPattern(CPDF_ShadingPattern* pPattern, CPDF_PageObject* pPageObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bStroke);
    FX_BOOL			SelectClipPath(CPDF_PathObject* pPathObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bStroke);
    FX_BOOL			ProcessImage(CPDF_ImageObject* pImageObj, const CFX_AffineMatrix* pObj2Device);
    FX_BOOL			OutputBitmapAlpha(CPDF_ImageObject* pImageObj, const CFX_AffineMatrix* pImage2Device);
    FX_BOOL			OutputImage(CPDF_ImageObject* pImageObj, const CFX_AffineMatrix* pImage2Device);
    FX_BOOL			OutputDIBSource(const CFX_DIBSource* pOutputBitmap, FX_ARGB fill_argb, int bitmap_alpha,
                                    const CFX_AffineMatrix* pImage2Device, CPDF_ImageCache* pImageCache, FX_DWORD flags);
    void			CompositeDIBitmap(CFX_DIBitmap* pDIBitmap, int left, int top, FX_ARGB mask_argb,
                                      int bitmap_alpha, int blend_mode, int bIsolated);
    FX_BOOL			ProcessInlines(CPDF_InlineImages* pInlines, const CFX_AffineMatrix* pObj2Device);
    FX_BOOL			ProcessShading(CPDF_ShadingObject* pShadingObj, const CFX_AffineMatrix* pObj2Device);
    void			DrawShading(CPDF_ShadingPattern* pPattern, CFX_AffineMatrix* pMatrix, FX_RECT& clip_rect,
                                int alpha, FX_BOOL bAlphaMode);
    FX_BOOL			ProcessType3Text(const CPDF_TextObject* textobj, const CFX_AffineMatrix* pObj2Device);
    FX_BOOL			ProcessText(const CPDF_TextObject* textobj, const CFX_AffineMatrix* pObj2Device, CFX_PathData* pClippingPath);
    void			DrawTextPathWithPattern(const CPDF_TextObject* textobj, const CFX_AffineMatrix* pObj2Device,
                                            CPDF_Font* pFont, FX_FLOAT font_size,
                                            const CFX_AffineMatrix* pTextMatrix, FX_BOOL bFill, FX_BOOL bStroke);
    FX_BOOL			ProcessForm(CPDF_FormObject* pFormObj, const CFX_AffineMatrix* pObj2Device);
    CFX_DIBitmap*	GetBackdrop(const CPDF_PageObject* pObj, const FX_RECT& rect, int& left, int& top,
                                FX_BOOL bBackAlphaRequired);
    CFX_DIBitmap*	LoadSMask(CPDF_Dictionary* pSMaskDict, FX_RECT* pClipRect, const CFX_AffineMatrix* pMatrix);
    void			Init(CPDF_RenderContext* pParent);
    static class CPDF_Type3Cache*	GetCachedType3(CPDF_Type3Font* pFont);
    static CPDF_GraphicStates* CloneObjStates(const CPDF_GraphicStates* pPathObj, FX_BOOL bStroke);
    CPDF_TransferFunc*	GetTransferFunc(CPDF_Object* pObject) const;
    FX_ARGB			GetFillArgb(const CPDF_PageObject* pObj, FX_BOOL bType3 = FALSE) const;
    FX_ARGB			GetStrokeArgb(const CPDF_PageObject* pObj) const;
    CPDF_RenderContext*		m_pContext;
    FX_BOOL					m_bStopped;
    void			DitherObjectArea(const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device);
    FX_BOOL			GetObjectClippedRect(const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bLogical, FX_RECT &rect) const;
    void			GetScaledMatrix(CFX_Matrix &matrix) const;

protected:
    static const int kRenderMaxRecursionDepth = 64;
    static int s_CurrentRecursionDepth;

    CFX_RenderDevice*		m_pDevice;
    CFX_AffineMatrix		m_DeviceMatrix;
    CPDF_ClipPath			m_LastClipPath;
    const CPDF_PageObject*	m_pCurObj;
    const CPDF_PageObject*	m_pStopObj;
    CPDF_GraphicStates		m_InitialStates;
    int						m_HalftoneLimit;
    IPDF_ObjectRenderer*	m_pObjectRenderer;
    FX_BOOL					m_bPrint;
    int						m_Transparency;
    int						m_DitherBits;
    FX_BOOL					m_bDropObjects;
    FX_BOOL					m_bStdCS;
    FX_DWORD                m_GroupFamily;
    FX_BOOL                 m_bLoadMask;
    CPDF_Type3Char *        m_pType3Char;
    FX_ARGB					m_T3FillColor;
    int                     m_curBlend;
};
class CPDF_ImageLoader : public CFX_Object
{
public:
    CPDF_ImageLoader()
    {
        m_pBitmap = NULL;
        m_pMask = NULL;
        m_MatteColor = 0;
        m_bCached = FALSE;
        m_nDownsampleWidth = 0;
        m_nDownsampleHeight = 0;
    }

    FX_BOOL					Load(const CPDF_ImageObject* pImage, CPDF_PageRenderCache* pCache, FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE, CPDF_RenderStatus* pRenderStatus = NULL);

    FX_BOOL					StartLoadImage(const CPDF_ImageObject* pImage, CPDF_PageRenderCache* pCache, FX_LPVOID& LoadHandle, FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE, CPDF_RenderStatus* pRenderStatus = NULL, FX_INT32 nDownsampleWidth = 0, FX_INT32 nDownsampleHeight = 0);
    FX_BOOL					Continue(FX_LPVOID LoadHandle, IFX_Pause* pPause);
    ~CPDF_ImageLoader();
    CFX_DIBSource*			m_pBitmap;
    CFX_DIBSource*			m_pMask;
    FX_DWORD				m_MatteColor;
    FX_BOOL					m_bCached;
protected:
    FX_INT32                m_nDownsampleWidth;
    FX_INT32                m_nDownsampleHeight;
};
class CPDF_ProgressiveImageLoaderHandle : public CFX_Object
{
public:
    CPDF_ProgressiveImageLoaderHandle();
    ~CPDF_ProgressiveImageLoaderHandle();

    FX_BOOL			Start(CPDF_ImageLoader* pImageLoader, const CPDF_ImageObject* pImage, CPDF_PageRenderCache* pCache, FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE, CPDF_RenderStatus* pRenderStatus = NULL, FX_INT32 nDownsampleWidth = 0, FX_INT32 nDownsampleHeight = 0);
    FX_BOOL			Continue(IFX_Pause* pPause);
protected:
    CPDF_ImageLoader*	m_pImageLoader;
    CPDF_PageRenderCache* m_pCache;
    CPDF_ImageObject* m_pImage;
    FX_INT32            m_nDownsampleWidth;
    FX_INT32            m_nDownsampleHeight;
};
class CFX_ImageTransformer;
class CPDF_ImageRenderer : public IPDF_ObjectRenderer
{
public:
    CPDF_ImageRenderer();
    ~CPDF_ImageRenderer();
    FX_BOOL		Start(CPDF_RenderStatus* pStatus, const CPDF_PageObject* pObj, const CFX_AffineMatrix* pObj2Device, FX_BOOL bStdCS, int blendType = FXDIB_BLEND_NORMAL);
    FX_BOOL		Start(CPDF_RenderStatus* pStatus, const CFX_DIBSource* pDIBSource, FX_ARGB bitmap_argb,
                      int bitmap_alpha, const CFX_AffineMatrix* pImage2Device, FX_DWORD flags, FX_BOOL bStdCS, int blendType = FXDIB_BLEND_NORMAL);
    FX_BOOL		Continue(IFX_Pause* pPause);
protected:
    CPDF_RenderStatus*	m_pRenderStatus;
    CPDF_ImageObject*	m_pImageObject;
    int					m_Status;
    const CFX_AffineMatrix* m_pObj2Device;
    CFX_AffineMatrix	m_ImageMatrix;
    CPDF_ImageLoader	m_Loader;
    const CFX_DIBSource*		m_pDIBSource;
    CFX_DIBitmap*		m_pClone;
    int					m_BitmapAlpha;
    FX_BOOL				m_bPatternColor;
    CPDF_Pattern*		m_pPattern;
    FX_ARGB				m_FillArgb;
    FX_DWORD			m_Flags;
    CPDF_QuickStretcher*	m_pQuickStretcher;
    CFX_ImageTransformer*	m_pTransformer;
    CPDF_ImageRenderer*	m_pRenderer2;
    FX_LPVOID			m_DeviceHandle;
    FX_LPVOID           m_LoadHandle;
    FX_BOOL				m_bStdCS;
    int					m_BlendType;
    FX_BOOL				StartBitmapAlpha();
    FX_BOOL				StartDIBSource();
    FX_BOOL				StartRenderDIBSource();
    FX_BOOL				StartLoadDIBSource();
    FX_BOOL				DrawMaskedImage();
    FX_BOOL				DrawPatternImage(const CFX_Matrix* pObj2Device);
};
class CPDF_ScaledRenderBuffer : public CFX_Object
{
public:
    CPDF_ScaledRenderBuffer();
    ~CPDF_ScaledRenderBuffer();
    FX_BOOL				Initialize(CPDF_RenderContext* pContext, CFX_RenderDevice* pDevice, FX_RECT* pRect,
                                   const CPDF_PageObject* pObj, const CPDF_RenderOptions *pOptions = NULL, int max_dpi = 0);
    CFX_RenderDevice*	GetDevice()
    {
        return m_pBitmapDevice ? m_pBitmapDevice : m_pDevice;
    }
    CFX_AffineMatrix*	GetMatrix()
    {
        return &m_Matrix;
    }
    void				OutputToDevice();
private:
    CFX_RenderDevice*	m_pDevice;
    CPDF_RenderContext*	m_pContext;
    FX_RECT				m_Rect;
    const CPDF_PageObject* m_pObject;
    CFX_FxgeDevice*	m_pBitmapDevice;
    CFX_AffineMatrix	m_Matrix;
};
class ICodec_ScanlineDecoder;
class CPDF_QuickStretcher : public CFX_Object
{
public:
    CPDF_QuickStretcher();
    ~CPDF_QuickStretcher();
    FX_BOOL		Start(CPDF_ImageObject* pImageObj, CFX_AffineMatrix* pImage2Device, const FX_RECT* pClipBox);
    FX_BOOL		Continue(IFX_Pause* pPause);
    CFX_DIBitmap*	m_pBitmap;
    int			m_ResultLeft, m_ResultTop, m_ClipLeft, m_ClipTop;
    int			m_DestWidth, m_DestHeight, m_ResultWidth, m_ResultHeight;
    int			m_Bpp, m_SrcWidth, m_SrcHeight;
    FX_BOOL		m_bFlipX, m_bFlipY;
    CPDF_ColorSpace*	m_pCS;
    ICodec_ScanlineDecoder*	m_pDecoder;
    CPDF_StreamAcc m_StreamAcc;
    int			m_LineIndex;
};
class CPDF_DeviceBuffer : public CFX_Object
{
public:
    CPDF_DeviceBuffer();
    ~CPDF_DeviceBuffer();
    FX_BOOL				Initialize(CPDF_RenderContext* pContext, CFX_RenderDevice* pDevice, FX_RECT* pRect,
                                   const CPDF_PageObject* pObj, int max_dpi = 0);
    void				OutputToDevice();
    CFX_DIBitmap*		GetBitmap() const
    {
        return m_pBitmap;
    }
    const CFX_AffineMatrix*	GetMatrix() const
    {
        return &m_Matrix;
    }
private:
    CFX_RenderDevice*	m_pDevice;
    CPDF_RenderContext*	m_pContext;
    FX_RECT				m_Rect;
    const CPDF_PageObject* m_pObject;
    CFX_DIBitmap*		m_pBitmap;
    CFX_AffineMatrix	m_Matrix;
};
class CPDF_ImageCache : public CFX_Object
{
public:
    CPDF_ImageCache(CPDF_Document* pDoc, CPDF_Stream* pStream);
    ~CPDF_ImageCache();
    void				ClearImageData();
    void				Reset(const CFX_DIBitmap* pBitmap);
    FX_BOOL				GetCachedBitmap(CFX_DIBSource*& pBitmap, CFX_DIBSource*& pMask, FX_DWORD& MatteColor, CPDF_Dictionary* pPageResources,
                                        FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE,
                                        CPDF_RenderStatus* pRenderStatus = NULL, FX_INT32 downsampleWidth = 0, FX_INT32 downsampleHeight = 0);
    FX_DWORD			EstimateSize() const
    {
        return m_dwCacheSize;
    }
    FX_DWORD			GetTimeCount() const
    {
        return m_dwTimeCount;
    }
    CPDF_Stream*		GetStream() const
    {
        return m_pStream;
    }
    void				SetTimeCount(FX_DWORD dwTimeCount)
    {
        m_dwTimeCount = dwTimeCount;
    }
    int					m_dwTimeCount;
public:
    int					StartGetCachedBitmap(CPDF_Dictionary* pFormResources, CPDF_Dictionary* pPageResources,
            FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0,
            FX_BOOL bLoadMask = FALSE, CPDF_RenderStatus* pRenderStatus = NULL, FX_INT32 downsampleWidth = 0, FX_INT32 downsampleHeight = 0);
    int					Continue(IFX_Pause* pPause);
    int 				ContinueGetCachedBitmap();
    CFX_DIBSource*		DetachBitmap();
    CFX_DIBSource*		DetachMask();
    CFX_DIBSource*		m_pCurBitmap;
    CFX_DIBSource*		m_pCurMask;
    FX_DWORD			m_MatteColor;
    CPDF_RenderStatus*  m_pRenderStatus;
protected:
    CPDF_Document*		m_pDocument;
    CPDF_Stream*		m_pStream;
    CFX_DIBSource*		m_pCachedBitmap;
    CFX_DIBSource*		m_pCachedMask;
    FX_DWORD			m_dwCacheSize;
    void	CalcSize();
};
typedef struct {
    FX_FLOAT			m_DecodeMin;
    FX_FLOAT			m_DecodeStep;
    int					m_ColorKeyMin;
    int					m_ColorKeyMax;
} DIB_COMP_DATA;
class CPDF_DIBSource : public CFX_DIBSource
{
public:
    CPDF_DIBSource();
    virtual ~CPDF_DIBSource();
    FX_BOOL				Load(CPDF_Document* pDoc, const CPDF_Stream* pStream,
                             CPDF_DIBSource** ppMask, FX_DWORD* pMatteColor,
                             CPDF_Dictionary* pFormResources, CPDF_Dictionary* pPageResources,
                             FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE);
    virtual FX_BOOL		SkipToScanline(int line, IFX_Pause* pPause) const;
    virtual	FX_LPBYTE	GetBuffer() const;
    virtual FX_LPCBYTE	GetScanline(int line) const;
    virtual void		DownSampleScanline(int line, FX_LPBYTE dest_scan, int dest_bpp,
                                           int dest_width, FX_BOOL bFlipX, int clip_left, int clip_width) const;
    virtual void		SetDownSampleSize(int dest_width, int dest_height) const;
    CFX_DIBitmap*		GetBitmap() const;
    void				ReleaseBitmap(CFX_DIBitmap*) const;
    void				ClearImageData();
public:
    int					StartLoadDIBSource(CPDF_Document* pDoc, const CPDF_Stream* pStream, FX_BOOL bHasMask,
                                           CPDF_Dictionary* pFormResources, CPDF_Dictionary* pPageResources,
                                           FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE);
    int					ContinueLoadDIBSource(IFX_Pause* pPause);
    int					StratLoadMask();
    int					StartLoadMaskDIB();
    int					ContinueLoadMaskDIB(IFX_Pause* pPause);
    int					ContinueToLoadMask();
    CPDF_DIBSource*		DetachMask();
    CPDF_DIBSource*		m_pMask;
    FX_DWORD			m_MatteColor;
    FX_LPVOID			m_pJbig2Context;
    CPDF_StreamAcc*		m_pGlobalStream;
    FX_BOOL				m_bStdCS;
    int					m_Status;
    CPDF_Object*		m_pMaskStream;
    FX_BOOL				m_bHasMask;
protected:
    FX_BOOL				LoadColorInfo(CPDF_Dictionary* pFormResources, CPDF_Dictionary* pPageResources);
    DIB_COMP_DATA*      GetDecodeAndMaskArray(FX_BOOL& bDefaultDecode, FX_BOOL& bColorKey);
    CPDF_DIBSource*		LoadMask(FX_DWORD& MatteColor);
    CPDF_DIBSource*		LoadMaskDIB(CPDF_Stream* pMask);
    void				LoadJpxBitmap();
    void				LoadPalette();
    int					CreateDecoder();
    void				TranslateScanline24bpp(FX_LPBYTE dest_scan, FX_LPCBYTE src_scan) const;
    void                ValidateDictParam();
    CPDF_Document*		m_pDocument;
    const CPDF_Stream*	m_pStream;
    CPDF_StreamAcc*		m_pStreamAcc;
    const CPDF_Dictionary*	m_pDict;
    CPDF_ColorSpace*	m_pColorSpace;
    FX_DWORD			m_Family;
    FX_DWORD			m_bpc;
    FX_DWORD			m_bpc_orig;
    FX_DWORD			m_nComponents;
    FX_DWORD			m_GroupFamily;
    FX_BOOL				m_bLoadMask;
    FX_BOOL				m_bDefaultDecode;
    FX_BOOL				m_bImageMask;
    FX_BOOL				m_bDoBpcCheck;
    FX_BOOL				m_bColorKey;
    DIB_COMP_DATA*		m_pCompData;
    FX_LPBYTE			m_pLineBuf;
    FX_LPBYTE			m_pMaskedLine;
    CFX_DIBitmap*		m_pCachedBitmap;
    ICodec_ScanlineDecoder*	m_pDecoder;
};
#define FPDF_HUGE_IMAGE_SIZE	60000000
class CPDF_DIBTransferFunc : public CFX_FilteredDIB
{
public:
    CPDF_DIBTransferFunc(const CPDF_TransferFunc* pTransferFunc);
    virtual FXDIB_Format	GetDestFormat();
    virtual FX_ARGB*		GetDestPalette()
    {
        return NULL;
    }
    virtual void			TranslateScanline(FX_LPBYTE dest_buf, FX_LPCBYTE src_buf) const;
    virtual void			TranslateDownSamples(FX_LPBYTE dest_buf, FX_LPCBYTE src_buf, int pixels, int Bpp) const;
    FX_LPCBYTE				m_RampR;
    FX_LPCBYTE				m_RampG;
    FX_LPCBYTE				m_RampB;
};
struct _CPDF_UniqueKeyGen {
    void		Generate(int count, ...);
    FX_CHAR		m_Key[128];
    int			m_KeyLen;
};

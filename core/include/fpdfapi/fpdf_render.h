// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDF_RENDER_
#define _FPDF_RENDER_
#ifndef _FPDF_PAGE_
#include "fpdf_page.h"
#endif
#ifndef _FX_GE_H_
#include "../fxge/fx_ge.h"
#endif
class CPDF_RenderContext;
class CPDF_RenderOptions;
class CPDF_ImageCache;
class IPDF_OCContext;
class CPDF_QuickStretcher;
class CFX_PathData;
class CFX_GraphStateData;
class CFX_RenderDevice;
class CPDF_TextObject;
class CPDF_PathObject;
class CPDF_ImageObject;
class CPDF_ShadingObject;
class CPDF_FormObject;
class IPDF_OCContext
{
public:

    virtual ~IPDF_OCContext() {}

    virtual FX_BOOL	CheckOCGVisible(const CPDF_Dictionary* pOCG) = 0;

    FX_BOOL CheckObjectVisible(const CPDF_PageObject* pObj);
};
#define RENDER_COLOR_NORMAL		0
#define RENDER_COLOR_GRAY		1
#define RENDER_COLOR_TWOCOLOR	2
#define RENDER_COLOR_ALPHA		3
#define RENDER_CLEARTYPE			0x00000001
#define RENDER_PRINTGRAPHICTEXT		0x00000002
#define RENDER_FORCE_DOWNSAMPLE		0x00000004
#define RENDER_PRINTPREVIEW			0x00000008
#define RENDER_BGR_STRIPE			0x00000010
#define RENDER_NO_NATIVETEXT		0x00000020
#define RENDER_FORCE_HALFTONE		0x00000040
#define RENDER_RECT_AA				0x00000080
#define RENDER_FILL_FULLCOVER		0x00000100
#define RENDER_PRINTIMAGETEXT       0x00000200
#define RENDER_OVERPRINT            0x00000400
#define RENDER_THINLINE             0x00000800
#define RENDER_NOTEXTSMOOTH			0x10000000
#define RENDER_NOPATHSMOOTH			0x20000000
#define RENDER_NOIMAGESMOOTH		0x40000000
#define RENDER_LIMITEDIMAGECACHE	0x80000000
class CPDF_RenderOptions : public CFX_Object
{
public:

    CPDF_RenderOptions();

    int				m_ColorMode;

    FX_COLORREF		m_BackColor;

    FX_COLORREF		m_ForeColor;

    FX_DWORD		m_Flags;

    int				m_Interpolation;

    FX_DWORD		m_AddFlags;

    IPDF_OCContext*	m_pOCContext;

    FX_DWORD		m_dwLimitCacheSize;

    int				m_HalftoneLimit;

    FX_ARGB			TranslateColor(FX_ARGB argb) const;
};
class CPDF_RenderContext : public CFX_Object
{
public:

    CPDF_RenderContext();

    void			Create(CPDF_Page* pPage, FX_BOOL bFirstLayer = TRUE);

    void			Create(CPDF_Document* pDoc = NULL, CPDF_PageRenderCache* pPageCache = NULL,
                           CPDF_Dictionary* pPageResources = NULL, FX_BOOL bFirstLayer = TRUE);

    ~CPDF_RenderContext();

    void			Clear();

    void			AppendObjectList(CPDF_PageObjects* pObjs, const CFX_AffineMatrix* pObject2Device);

    void			SetBackground(class IPDF_BackgroundDraw* pBackground);

    void			Render(CFX_RenderDevice* pDevice, const CPDF_RenderOptions* pOptions = NULL,
                           const CFX_AffineMatrix* pFinalMatrix = NULL);

    void			DrawObjectList(CFX_RenderDevice* pDevice, CPDF_PageObjects* pObjs,
                                   const CFX_AffineMatrix* pObject2Device, const CPDF_RenderOptions* pOptions);

    void			GetBackground(CFX_DIBitmap* pBuffer, const CPDF_PageObject* pObj,
                                  const CPDF_RenderOptions* pOptions, CFX_AffineMatrix* pFinalMatrix);

    CPDF_PageRenderCache*	GetPageCache() const
    {
        return m_pPageCache;
    }



    CPDF_Document*			m_pDocument;

    CPDF_Dictionary*		m_pPageResources;

    CPDF_PageRenderCache*	m_pPageCache;

protected:

    CFX_ArrayTemplate<struct _PDF_RenderItem>	m_ContentList;

    IPDF_BackgroundDraw*	m_pBackgroundDraw;

    FX_BOOL					m_bFirstLayer;

    void			Render(CFX_RenderDevice* pDevice, const CPDF_PageObject* pStopObj,
                           const CPDF_RenderOptions* pOptions, const CFX_AffineMatrix* pFinalMatrix);
    friend class CPDF_RenderStatus;
    friend class CPDF_ProgressiveRenderer;
};
class IPDF_BackgroundDraw
{
public:
    virtual ~IPDF_BackgroundDraw() { }
    virtual	void	OnDrawBackground(
        CFX_RenderDevice* pBitmapDevice,
        const CFX_AffineMatrix* pOriginal2Bitmap
    ) = 0;
};
class CPDF_ProgressiveRenderer : public CFX_Object
{
public:

    CPDF_ProgressiveRenderer();

    ~CPDF_ProgressiveRenderer();

    typedef enum {
        Ready,
        ToBeContinued,
        Done,
        Failed
    } RenderStatus;

    RenderStatus		GetStatus()
    {
        return m_Status;
    }



    void				Start(CPDF_RenderContext* pContext, CFX_RenderDevice* pDevice,
                              const CPDF_RenderOptions* pOptions, class IFX_Pause* pPause, FX_BOOL bDropObjects = FALSE);

    void				Continue(class IFX_Pause* pPause);


    int					EstimateProgress();

    void				Clear();
protected:

    RenderStatus		m_Status;

    CPDF_RenderContext*	m_pContext;

    CFX_RenderDevice*	m_pDevice;

    const CPDF_RenderOptions*	m_pOptions;

    FX_BOOL				m_bDropObjects;

    class CPDF_RenderStatus*	m_pRenderer;

    CFX_FloatRect		m_ClipRect;

    FX_DWORD			m_LayerIndex;

    FX_DWORD			m_ObjectIndex;

    FX_POSITION			m_ObjectPos;

    FX_POSITION			m_PrevLastPos;

    void				RenderStep();
};
class CPDF_TextRenderer : public CFX_Object
{
public:

    static void		DrawTextString(CFX_RenderDevice* pDevice, int left, int top,
                                   CPDF_Font* pFont,
                                   int height,
                                   const CFX_ByteString& str,
                                   FX_ARGB argb);

    static void		DrawTextString(CFX_RenderDevice* pDevice, FX_FLOAT origin_x, FX_FLOAT origin_y,
                                   CPDF_Font* pFont,
                                   FX_FLOAT font_size,
                                   const CFX_AffineMatrix* matrix,
                                   const CFX_ByteString& str,
                                   FX_ARGB fill_argb,
                                   FX_ARGB stroke_argb = 0,
                                   const CFX_GraphStateData* pGraphState = NULL,
                                   const CPDF_RenderOptions* pOptions = NULL
                               );

    static FX_BOOL	DrawTextPath(CFX_RenderDevice* pDevice, int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos,
                                 CPDF_Font* pFont, FX_FLOAT font_size,
                                 const CFX_AffineMatrix* pText2User, const CFX_AffineMatrix* pUser2Device,
                                 const CFX_GraphStateData* pGraphState,
                                 FX_ARGB fill_argb, FX_ARGB stroke_argb, CFX_PathData* pClippingPath, int nFlag = 0);

    static FX_BOOL	DrawNormalText(CFX_RenderDevice* pDevice, int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos,
                                   CPDF_Font* pFont, FX_FLOAT font_size, const CFX_AffineMatrix* pText2Device,
                                   FX_ARGB fill_argb, const CPDF_RenderOptions* pOptions);

    static FX_BOOL	DrawType3Text(CFX_RenderDevice* pDevice, int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos,
                                  CPDF_Font* pFont, FX_FLOAT font_size, const CFX_AffineMatrix* pText2Device,
                                  FX_ARGB fill_argb);
};
class IPDF_PageImageCache
{
public:

    static IPDF_PageImageCache* Create();

    virtual ~IPDF_PageImageCache() {}

    virtual void		OutputPage(CFX_RenderDevice* pDevice, CPDF_Page* pPage,
                                   int pos_x, int pos_y, int size_x, int size_y, int rotate) = 0;

    virtual void		SetCacheLimit(FX_DWORD limit) = 0;
};
class CPDF_PageRenderCache : public CFX_Object
{
public:
    CPDF_PageRenderCache(CPDF_Page* pPage)
    {
        m_pPage = pPage;
        m_nTimeCount = 0;
        m_nCacheSize = 0;
        m_pCurImageCache = NULL;
        m_bCurFindCache = FALSE;
        m_pCurImageCaches = NULL;
    }
    ~CPDF_PageRenderCache()
    {
        ClearAll();
    }
    void				ClearAll();
    void				ClearImageData();

    FX_DWORD			EstimateSize();
    void				CacheOptimization(FX_INT32 dwLimitCacheSize);
    FX_DWORD			GetCachedSize(CPDF_Stream* pStream) const;
    FX_DWORD			GetTimeCount() const
    {
        return m_nTimeCount;
    }
    void				SetTimeCount(FX_DWORD dwTimeCount)
    {
        m_nTimeCount = dwTimeCount;
    }

    void				GetCachedBitmap(CPDF_Stream* pStream, CFX_DIBSource*& pBitmap, CFX_DIBSource*& pMask, FX_DWORD& MatteColor,
                                        FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE,
                                        CPDF_RenderStatus* pRenderStatus = NULL, FX_INT32 downsampleWidth = 0, FX_INT32 downsampleHeight = 0);

    void				ResetBitmap(CPDF_Stream* pStream, const CFX_DIBitmap* pBitmap);
    void				ClearImageCache(CPDF_Stream* pStream);
    CPDF_Page*			GetPage()
    {
        return m_pPage;
    }
    CFX_MapPtrToPtr		m_ImageCaches;
public:
    FX_BOOL				StartGetCachedBitmap(CPDF_Stream* pStream, FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0,
            FX_BOOL bLoadMask = FALSE, CPDF_RenderStatus* pRenderStatus = NULL,
            FX_INT32 downsampleWidth = 0, FX_INT32 downsampleHeight = 0);

    FX_BOOL				Continue(IFX_Pause* pPause);
    CPDF_ImageCache*	m_pCurImageCache;
    CFX_PtrArray*       m_pCurImageCaches;
protected:
    friend class		CPDF_Page;
    CPDF_Page*			m_pPage;

    FX_DWORD			m_nTimeCount;
    FX_DWORD			m_nCacheSize;
    FX_BOOL				m_bCurFindCache;
};
class CPDF_RenderConfig : public CFX_Object
{
public:
    CPDF_RenderConfig();
    ~CPDF_RenderConfig();
    int					m_HalftoneLimit;
    int					m_RenderStepLimit;
};
#endif

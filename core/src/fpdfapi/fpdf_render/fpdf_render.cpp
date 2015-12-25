// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "render_int.h"

#include "core/include/fpdfapi/fpdf_module.h"
#include "core/include/fpdfapi/fpdf_render.h"
#include "core/include/fxge/fx_ge.h"
#include "core/src/fpdfapi/fpdf_page/pageint.h"

CPDF_DocRenderData::CPDF_DocRenderData(CPDF_Document* pPDFDoc)
    : m_pPDFDoc(pPDFDoc), m_pFontCache(new CFX_FontCache) {}

CPDF_DocRenderData::~CPDF_DocRenderData() {
  Clear(TRUE);
}

void CPDF_DocRenderData::Clear(FX_BOOL bRelease) {
  for (auto it = m_Type3FaceMap.begin(); it != m_Type3FaceMap.end();) {
    auto curr_it = it++;
    CPDF_CountedObject<CPDF_Type3Cache>* cache = curr_it->second;
    if (bRelease || cache->use_count() < 2) {
      delete cache->get();
      delete cache;
      m_Type3FaceMap.erase(curr_it);
    }
  }

  for (auto it = m_TransferFuncMap.begin(); it != m_TransferFuncMap.end();) {
    auto curr_it = it++;
    CPDF_CountedObject<CPDF_TransferFunc>* value = curr_it->second;
    if (bRelease || value->use_count() < 2) {
      delete value->get();
      delete value;
      m_TransferFuncMap.erase(curr_it);
    }
  }

  if (m_pFontCache) {
    if (bRelease) {
      delete m_pFontCache;
      m_pFontCache = NULL;
    } else {
      m_pFontCache->FreeCache(FALSE);
    }
  }
}

CPDF_Type3Cache* CPDF_DocRenderData::GetCachedType3(CPDF_Type3Font* pFont) {
  CPDF_CountedObject<CPDF_Type3Cache>* pCache;
  auto it = m_Type3FaceMap.find(pFont);
  if (it == m_Type3FaceMap.end()) {
    CPDF_Type3Cache* pType3 = new CPDF_Type3Cache(pFont);
    pCache = new CPDF_CountedObject<CPDF_Type3Cache>(pType3);
    m_Type3FaceMap[pFont] = pCache;
  } else {
    pCache = it->second;
  }
  return pCache->AddRef();
}

void CPDF_DocRenderData::ReleaseCachedType3(CPDF_Type3Font* pFont) {
  auto it = m_Type3FaceMap.find(pFont);
  if (it != m_Type3FaceMap.end())
    it->second->RemoveRef();
}

class CPDF_RenderModule : public IPDF_RenderModule {
 public:
  CPDF_RenderModule() {}

 private:
  ~CPDF_RenderModule() override {}

  CPDF_DocRenderData* CreateDocData(CPDF_Document* pDoc) override;
  void DestroyDocData(CPDF_DocRenderData* p) override;
  void ClearDocData(CPDF_DocRenderData* p) override;

  CPDF_DocRenderData* GetRenderData() override { return &m_RenderData; }

  CPDF_PageRenderCache* CreatePageCache(CPDF_Page* pPage) override {
    return new CPDF_PageRenderCache(pPage);
  }

  void DestroyPageCache(CPDF_PageRenderCache* pCache) override;

  CPDF_RenderConfig* GetConfig() override { return &m_RenderConfig; }

  CPDF_DocRenderData m_RenderData;
  CPDF_RenderConfig m_RenderConfig;
};

CPDF_DocRenderData* CPDF_RenderModule::CreateDocData(CPDF_Document* pDoc) {
  return new CPDF_DocRenderData(pDoc);
}
void CPDF_RenderModule::DestroyDocData(CPDF_DocRenderData* pDocData) {
  delete pDocData;
}
void CPDF_RenderModule::ClearDocData(CPDF_DocRenderData* p) {
  if (p) {
    p->Clear(FALSE);
  }
}
void CPDF_RenderModule::DestroyPageCache(CPDF_PageRenderCache* pCache) {
  delete pCache;
}

void CPDF_ModuleMgr::InitRenderModule() {
  m_pRenderModule.reset(new CPDF_RenderModule);
}

CPDF_RenderOptions::CPDF_RenderOptions()
    : m_ColorMode(RENDER_COLOR_NORMAL),
      m_Flags(RENDER_CLEARTYPE),
      m_Interpolation(0),
      m_AddFlags(0),
      m_pOCContext(NULL),
      m_dwLimitCacheSize(1024 * 1024 * 100),
      m_HalftoneLimit(-1) {}
FX_ARGB CPDF_RenderOptions::TranslateColor(FX_ARGB argb) const {
  if (m_ColorMode == RENDER_COLOR_NORMAL) {
    return argb;
  }
  if (m_ColorMode == RENDER_COLOR_ALPHA) {
    return argb;
  }
  int a, r, g, b;
  ArgbDecode(argb, a, r, g, b);
  int gray = FXRGB2GRAY(r, g, b);
  if (m_ColorMode == RENDER_COLOR_TWOCOLOR) {
    int color = (r - gray) * (r - gray) + (g - gray) * (g - gray) +
                (b - gray) * (b - gray);
    if (gray < 35 && color < 20) {
      return ArgbEncode(a, m_ForeColor);
    }
    if (gray > 221 && color < 20) {
      return ArgbEncode(a, m_BackColor);
    }
    return argb;
  }
  int fr = FXSYS_GetRValue(m_ForeColor);
  int fg = FXSYS_GetGValue(m_ForeColor);
  int fb = FXSYS_GetBValue(m_ForeColor);
  int br = FXSYS_GetRValue(m_BackColor);
  int bg = FXSYS_GetGValue(m_BackColor);
  int bb = FXSYS_GetBValue(m_BackColor);
  r = (br - fr) * gray / 255 + fr;
  g = (bg - fg) * gray / 255 + fg;
  b = (bb - fb) * gray / 255 + fb;
  return ArgbEncode(a, r, g, b);
}

// static
int CPDF_RenderStatus::s_CurrentRecursionDepth = 0;

CPDF_RenderStatus::CPDF_RenderStatus()
    : m_pFormResource(nullptr),
      m_pPageResource(nullptr),
      m_pContext(nullptr),
      m_bStopped(FALSE),
      m_pDevice(nullptr),
      m_pCurObj(nullptr),
      m_pStopObj(nullptr),
      m_HalftoneLimit(0),
      m_bPrint(FALSE),
      m_Transparency(0),
      m_DitherBits(0),
      m_bDropObjects(FALSE),
      m_bStdCS(FALSE),
      m_GroupFamily(0),
      m_bLoadMask(FALSE),
      m_pType3Char(nullptr),
      m_T3FillColor(0),
      m_curBlend(FXDIB_BLEND_NORMAL) {}

CPDF_RenderStatus::~CPDF_RenderStatus() {
}

FX_BOOL CPDF_RenderStatus::Initialize(CPDF_RenderContext* pContext,
                                      CFX_RenderDevice* pDevice,
                                      const CFX_Matrix* pDeviceMatrix,
                                      const CPDF_PageObject* pStopObj,
                                      const CPDF_RenderStatus* pParentState,
                                      const CPDF_GraphicStates* pInitialStates,
                                      const CPDF_RenderOptions* pOptions,
                                      int transparency,
                                      FX_BOOL bDropObjects,
                                      CPDF_Dictionary* pFormResource,
                                      FX_BOOL bStdCS,
                                      CPDF_Type3Char* pType3Char,
                                      FX_ARGB fill_color,
                                      FX_DWORD GroupFamily,
                                      FX_BOOL bLoadMask) {
  m_pContext = pContext;
  m_pDevice = pDevice;
  m_DitherBits = pDevice->GetDeviceCaps(FXDC_DITHER_BITS);
  m_bPrint = m_pDevice->GetDeviceClass() != FXDC_DISPLAY;
  if (pDeviceMatrix) {
    m_DeviceMatrix = *pDeviceMatrix;
  }
  m_pStopObj = pStopObj;
  if (pOptions) {
    m_Options = *pOptions;
  }
  m_bDropObjects = bDropObjects;
  m_bStdCS = bStdCS;
  m_T3FillColor = fill_color;
  m_pType3Char = pType3Char;
  m_GroupFamily = GroupFamily;
  m_bLoadMask = bLoadMask;
  m_pFormResource = pFormResource;
  m_pPageResource = m_pContext->m_pPageResources;
  if (pInitialStates && !m_pType3Char) {
    m_InitialStates.CopyStates(*pInitialStates);
    if (pParentState) {
      CPDF_ColorStateData* pColorData =
          (CPDF_ColorStateData*)(const CPDF_ColorStateData*)
              m_InitialStates.m_ColorState;
      CPDF_ColorStateData* pParentData =
          (CPDF_ColorStateData*)(const CPDF_ColorStateData*)
              pParentState->m_InitialStates.m_ColorState;
      if (!pColorData || pColorData->m_FillColor.IsNull()) {
        CPDF_ColorStateData* pData = m_InitialStates.m_ColorState.GetModify();
        pData->m_FillRGB = pParentData->m_FillRGB;
        pData->m_FillColor.Copy(&pParentData->m_FillColor);
      }
      if (!pColorData || pColorData->m_StrokeColor.IsNull()) {
        CPDF_ColorStateData* pData = m_InitialStates.m_ColorState.GetModify();
        pData->m_StrokeRGB = pParentData->m_FillRGB;
        pData->m_StrokeColor.Copy(&pParentData->m_StrokeColor);
      }
    }
  } else {
    m_InitialStates.DefaultStates();
  }
  m_pObjectRenderer.reset();
  m_Transparency = transparency;
  return TRUE;
}
void CPDF_RenderStatus::RenderObjectList(const CPDF_PageObjects* pObjs,
                                         const CFX_Matrix* pObj2Device) {
  CFX_FloatRect clip_rect = m_pDevice->GetClipBox();
  CFX_Matrix device2object;
  device2object.SetReverse(*pObj2Device);
  device2object.TransformRect(clip_rect);
  int index = 0;
  FX_POSITION pos = pObjs->GetFirstObjectPosition();
  while (pos) {
    index++;
    CPDF_PageObject* pCurObj = pObjs->GetNextObject(pos);
    if (pCurObj == m_pStopObj) {
      m_bStopped = TRUE;
      return;
    }
    if (!pCurObj) {
      continue;
    }
    if (!pCurObj || pCurObj->m_Left > clip_rect.right ||
        pCurObj->m_Right < clip_rect.left ||
        pCurObj->m_Bottom > clip_rect.top ||
        pCurObj->m_Top < clip_rect.bottom) {
      continue;
    }
    RenderSingleObject(pCurObj, pObj2Device);
    if (m_bStopped) {
      return;
    }
  }
}
void CPDF_RenderStatus::RenderSingleObject(const CPDF_PageObject* pObj,
                                           const CFX_Matrix* pObj2Device) {
  CFX_AutoRestorer<int> restorer(&s_CurrentRecursionDepth);
  if (++s_CurrentRecursionDepth > kRenderMaxRecursionDepth) {
    return;
  }
  m_pCurObj = pObj;
  if (m_Options.m_pOCContext && pObj->m_ContentMark.NotNull()) {
    if (!m_Options.m_pOCContext->CheckObjectVisible(pObj)) {
      return;
    }
  }
  ProcessClipPath(pObj->m_ClipPath, pObj2Device);
  if (ProcessTransparency(pObj, pObj2Device)) {
    return;
  }
  ProcessObjectNoClip(pObj, pObj2Device);
}

FX_BOOL CPDF_RenderStatus::ContinueSingleObject(const CPDF_PageObject* pObj,
                                                const CFX_Matrix* pObj2Device,
                                                IFX_Pause* pPause) {
  if (m_pObjectRenderer) {
    if (m_pObjectRenderer->Continue(pPause))
      return TRUE;

    if (!m_pObjectRenderer->m_Result)
      DrawObjWithBackground(pObj, pObj2Device);
    m_pObjectRenderer.reset();
    return FALSE;
  }

  m_pCurObj = pObj;
  if (m_Options.m_pOCContext && pObj->m_ContentMark.NotNull() &&
      !m_Options.m_pOCContext->CheckObjectVisible(pObj)) {
    return FALSE;
  }

  ProcessClipPath(pObj->m_ClipPath, pObj2Device);
  if (ProcessTransparency(pObj, pObj2Device))
    return FALSE;

  if (pObj->m_Type == PDFPAGE_IMAGE) {
    m_pObjectRenderer.reset(IPDF_ObjectRenderer::Create(pObj->m_Type));
    if (!m_pObjectRenderer->Start(this, pObj, pObj2Device, FALSE)) {
      if (!m_pObjectRenderer->m_Result)
        DrawObjWithBackground(pObj, pObj2Device);
      m_pObjectRenderer.reset();
      return FALSE;
    }
    return ContinueSingleObject(pObj, pObj2Device, pPause);
  }

  ProcessObjectNoClip(pObj, pObj2Device);
  return FALSE;
}

IPDF_ObjectRenderer* IPDF_ObjectRenderer::Create(int type) {
  if (type != PDFPAGE_IMAGE) {
    return NULL;
  }
  return new CPDF_ImageRenderer;
}
FX_BOOL CPDF_RenderStatus::GetObjectClippedRect(const CPDF_PageObject* pObj,
                                                const CFX_Matrix* pObj2Device,
                                                FX_BOOL bLogical,
                                                FX_RECT& rect) const {
  rect = pObj->GetBBox(pObj2Device);
  FX_RECT rtClip = m_pDevice->GetClipBox();
  if (!bLogical) {
    CFX_Matrix dCTM = m_pDevice->GetCTM();
    FX_FLOAT a = FXSYS_fabs(dCTM.a);
    FX_FLOAT d = FXSYS_fabs(dCTM.d);
    if (a != 1.0f || d != 1.0f) {
      rect.right = rect.left + (int32_t)FXSYS_ceil((FX_FLOAT)rect.Width() * a);
      rect.bottom = rect.top + (int32_t)FXSYS_ceil((FX_FLOAT)rect.Height() * d);
      rtClip.right =
          rtClip.left + (int32_t)FXSYS_ceil((FX_FLOAT)rtClip.Width() * a);
      rtClip.bottom =
          rtClip.top + (int32_t)FXSYS_ceil((FX_FLOAT)rtClip.Height() * d);
    }
  }
  rect.Intersect(rtClip);
  return rect.IsEmpty();
}
void CPDF_RenderStatus::DitherObjectArea(const CPDF_PageObject* pObj,
                                         const CFX_Matrix* pObj2Device) {
  CFX_DIBitmap* pBitmap = m_pDevice->GetBitmap();
  if (!pBitmap) {
    return;
  }
  FX_RECT rect;
  if (GetObjectClippedRect(pObj, pObj2Device, FALSE, rect)) {
    return;
  }
  if (m_DitherBits == 2) {
    static FX_ARGB pal[4] = {0, 85, 170, 255};
    pBitmap->DitherFS(pal, 4, &rect);
  } else if (m_DitherBits == 3) {
    static FX_ARGB pal[8] = {0, 36, 73, 109, 146, 182, 219, 255};
    pBitmap->DitherFS(pal, 8, &rect);
  } else if (m_DitherBits == 4) {
    static FX_ARGB pal[16] = {0,   17,  34,  51,  68,  85,  102, 119,
                              136, 153, 170, 187, 204, 221, 238, 255};
    pBitmap->DitherFS(pal, 16, &rect);
  }
}
void CPDF_RenderStatus::ProcessObjectNoClip(const CPDF_PageObject* pObj,
                                            const CFX_Matrix* pObj2Device) {
  FX_BOOL bRet = FALSE;
  switch (pObj->m_Type) {
    case PDFPAGE_TEXT:
      bRet = ProcessText((CPDF_TextObject*)pObj, pObj2Device, NULL);
      break;
    case PDFPAGE_PATH:
      bRet = ProcessPath((CPDF_PathObject*)pObj, pObj2Device);
      break;
    case PDFPAGE_IMAGE:
      bRet = ProcessImage((CPDF_ImageObject*)pObj, pObj2Device);
      break;
    case PDFPAGE_SHADING:
      bRet = ProcessShading((CPDF_ShadingObject*)pObj, pObj2Device);
      break;
    case PDFPAGE_FORM:
      bRet = ProcessForm((CPDF_FormObject*)pObj, pObj2Device);
      break;
  }
  if (!bRet) {
    DrawObjWithBackground(pObj, pObj2Device);
  }
}
FX_BOOL CPDF_RenderStatus::DrawObjWithBlend(const CPDF_PageObject* pObj,
                                            const CFX_Matrix* pObj2Device) {
  FX_BOOL bRet = FALSE;
  switch (pObj->m_Type) {
    case PDFPAGE_PATH:
      bRet = ProcessPath((CPDF_PathObject*)pObj, pObj2Device);
      break;
    case PDFPAGE_IMAGE:
      bRet = ProcessImage((CPDF_ImageObject*)pObj, pObj2Device);
      break;
    case PDFPAGE_FORM:
      bRet = ProcessForm((CPDF_FormObject*)pObj, pObj2Device);
      break;
  }
  return bRet;
}
void CPDF_RenderStatus::GetScaledMatrix(CFX_Matrix& matrix) const {
  CFX_Matrix dCTM = m_pDevice->GetCTM();
  matrix.a *= FXSYS_fabs(dCTM.a);
  matrix.d *= FXSYS_fabs(dCTM.d);
}
void CPDF_RenderStatus::DrawObjWithBackground(const CPDF_PageObject* pObj,
                                              const CFX_Matrix* pObj2Device) {
  FX_RECT rect;
  if (GetObjectClippedRect(pObj, pObj2Device, FALSE, rect)) {
    return;
  }
  int res = 300;
  if (pObj->m_Type == PDFPAGE_IMAGE &&
      m_pDevice->GetDeviceCaps(FXDC_DEVICE_CLASS) == FXDC_PRINTER) {
    res = 0;
  }
  CPDF_ScaledRenderBuffer buffer;
  if (!buffer.Initialize(m_pContext, m_pDevice, rect, pObj, &m_Options, res)) {
    return;
  }
  CFX_Matrix matrix = *pObj2Device;
  matrix.Concat(*buffer.GetMatrix());
  GetScaledMatrix(matrix);
  CPDF_Dictionary* pFormResource = NULL;
  if (pObj->m_Type == PDFPAGE_FORM) {
    CPDF_FormObject* pFormObj = (CPDF_FormObject*)pObj;
    if (pFormObj->m_pForm && pFormObj->m_pForm->m_pFormDict) {
      pFormResource = pFormObj->m_pForm->m_pFormDict->GetDict("Resources");
    }
  }
  CPDF_RenderStatus status;
  status.Initialize(m_pContext, buffer.GetDevice(), buffer.GetMatrix(), NULL,
                    NULL, NULL, &m_Options, m_Transparency, m_bDropObjects,
                    pFormResource);
  status.RenderSingleObject(pObj, &matrix);
  buffer.OutputToDevice();
}
FX_BOOL CPDF_RenderStatus::ProcessForm(CPDF_FormObject* pFormObj,
                                       const CFX_Matrix* pObj2Device) {
  CPDF_Dictionary* pOC = pFormObj->m_pForm->m_pFormDict->GetDict("OC");
  if (pOC && m_Options.m_pOCContext &&
      !m_Options.m_pOCContext->CheckOCGVisible(pOC)) {
    return TRUE;
  }
  CFX_Matrix matrix = pFormObj->m_FormMatrix;
  matrix.Concat(*pObj2Device);
  CPDF_Dictionary* pResources = NULL;
  if (pFormObj->m_pForm && pFormObj->m_pForm->m_pFormDict) {
    pResources = pFormObj->m_pForm->m_pFormDict->GetDict("Resources");
  }
  CPDF_RenderStatus status;
  status.Initialize(m_pContext, m_pDevice, NULL, m_pStopObj, this, pFormObj,
                    &m_Options, m_Transparency, m_bDropObjects, pResources,
                    FALSE);
  status.m_curBlend = m_curBlend;
  m_pDevice->SaveState();
  status.RenderObjectList(pFormObj->m_pForm, &matrix);
  m_bStopped = status.m_bStopped;
  m_pDevice->RestoreState();
  return TRUE;
}
FX_BOOL IsAvailableMatrix(const CFX_Matrix& matrix) {
  if (matrix.a == 0 || matrix.d == 0) {
    return matrix.b != 0 && matrix.c != 0;
  }
  if (matrix.b == 0 || matrix.c == 0) {
    return matrix.a != 0 && matrix.d != 0;
  }
  return TRUE;
}
FX_BOOL CPDF_RenderStatus::ProcessPath(CPDF_PathObject* pPathObj,
                                       const CFX_Matrix* pObj2Device) {
  int FillType = pPathObj->m_FillType;
  FX_BOOL bStroke = pPathObj->m_bStroke;
  ProcessPathPattern(pPathObj, pObj2Device, FillType, bStroke);
  if (FillType == 0 && !bStroke) {
    return TRUE;
  }
  FX_DWORD fill_argb = 0;
  if (FillType) {
    fill_argb = GetFillArgb(pPathObj);
  }
  FX_DWORD stroke_argb = 0;
  if (bStroke) {
    stroke_argb = GetStrokeArgb(pPathObj);
  }
  CFX_Matrix path_matrix = pPathObj->m_Matrix;
  path_matrix.Concat(*pObj2Device);
  if (!IsAvailableMatrix(path_matrix)) {
    return TRUE;
  }
  if (FillType && (m_Options.m_Flags & RENDER_RECT_AA)) {
    FillType |= FXFILL_RECT_AA;
  }
  if (m_Options.m_Flags & RENDER_FILL_FULLCOVER) {
    FillType |= FXFILL_FULLCOVER;
  }
  if (m_Options.m_Flags & RENDER_NOPATHSMOOTH) {
    FillType |= FXFILL_NOPATHSMOOTH;
  }
  if (bStroke) {
    FillType |= FX_FILL_STROKE;
  }
  const CPDF_GeneralStateData* pGeneralData =
      ((CPDF_PageObject*)pPathObj)->m_GeneralState;
  if (pGeneralData && pGeneralData->m_StrokeAdjust) {
    FillType |= FX_STROKE_ADJUST;
  }
  if (m_pType3Char) {
    FillType |= FX_FILL_TEXT_MODE;
  }
  CFX_GraphStateData graphState(*pPathObj->m_GraphState);
  if (m_Options.m_Flags & RENDER_THINLINE) {
    graphState.m_LineWidth = 0;
  }
  return m_pDevice->DrawPath(pPathObj->m_Path, &path_matrix, &graphState,
                             fill_argb, stroke_argb, FillType, 0, NULL,
                             m_curBlend);
}
CPDF_TransferFunc* CPDF_RenderStatus::GetTransferFunc(CPDF_Object* pObj) const {
  ASSERT(pObj);
  CPDF_DocRenderData* pDocCache = m_pContext->m_pDocument->GetRenderData();
  return pDocCache ? pDocCache->GetTransferFunc(pObj) : nullptr;
}
FX_ARGB CPDF_RenderStatus::GetFillArgb(const CPDF_PageObject* pObj,
                                       FX_BOOL bType3) const {
  CPDF_ColorStateData* pColorData =
      (CPDF_ColorStateData*)(const CPDF_ColorStateData*)pObj->m_ColorState;
  if (m_pType3Char && !bType3 &&
      (!m_pType3Char->m_bColored ||
       (m_pType3Char->m_bColored &&
        (!pColorData || pColorData->m_FillColor.IsNull())))) {
    return m_T3FillColor;
  }
  if (!pColorData || pColorData->m_FillColor.IsNull()) {
    pColorData = (CPDF_ColorStateData*)(const CPDF_ColorStateData*)
                     m_InitialStates.m_ColorState;
  }
  FX_COLORREF rgb = pColorData->m_FillRGB;
  if (rgb == (FX_DWORD)-1) {
    return 0;
  }
  const CPDF_GeneralStateData* pGeneralData = pObj->m_GeneralState;
  int alpha;
  if (pGeneralData) {
    alpha = (int32_t)(pGeneralData->m_FillAlpha * 255);
    if (pGeneralData->m_pTR) {
      if (!pGeneralData->m_pTransferFunc) {
        ((CPDF_GeneralStateData*)pGeneralData)->m_pTransferFunc =
            GetTransferFunc(pGeneralData->m_pTR);
      }
      if (pGeneralData->m_pTransferFunc) {
        rgb = pGeneralData->m_pTransferFunc->TranslateColor(rgb);
      }
    }
  } else {
    alpha = 255;
  }
  return m_Options.TranslateColor(ArgbEncode(alpha, rgb));
}
FX_ARGB CPDF_RenderStatus::GetStrokeArgb(const CPDF_PageObject* pObj) const {
  CPDF_ColorStateData* pColorData =
      (CPDF_ColorStateData*)(const CPDF_ColorStateData*)pObj->m_ColorState;
  if (m_pType3Char && (!m_pType3Char->m_bColored ||
                       (m_pType3Char->m_bColored &&
                        (!pColorData || pColorData->m_StrokeColor.IsNull())))) {
    return m_T3FillColor;
  }
  if (!pColorData || pColorData->m_StrokeColor.IsNull()) {
    pColorData = (CPDF_ColorStateData*)(const CPDF_ColorStateData*)
                     m_InitialStates.m_ColorState;
  }
  FX_COLORREF rgb = pColorData->m_StrokeRGB;
  if (rgb == (FX_DWORD)-1) {
    return 0;
  }
  const CPDF_GeneralStateData* pGeneralData = pObj->m_GeneralState;
  int alpha;
  if (pGeneralData) {
    alpha = (int32_t)(pGeneralData->m_StrokeAlpha * 255);
    if (pGeneralData->m_pTR) {
      if (!pGeneralData->m_pTransferFunc) {
        ((CPDF_GeneralStateData*)pGeneralData)->m_pTransferFunc =
            GetTransferFunc(pGeneralData->m_pTR);
      }
      if (pGeneralData->m_pTransferFunc) {
        rgb = pGeneralData->m_pTransferFunc->TranslateColor(rgb);
      }
    }
  } else {
    alpha = 255;
  }
  return m_Options.TranslateColor(ArgbEncode(alpha, rgb));
}
void CPDF_RenderStatus::ProcessClipPath(CPDF_ClipPath ClipPath,
                                        const CFX_Matrix* pObj2Device) {
  if (ClipPath.IsNull()) {
    if (!m_LastClipPath.IsNull()) {
      m_pDevice->RestoreState(TRUE);
      m_LastClipPath.SetNull();
    }
    return;
  }
  if (m_LastClipPath == ClipPath)
    return;

  m_LastClipPath = ClipPath;
  m_pDevice->RestoreState(TRUE);
  int nClipPath = ClipPath.GetPathCount();
  for (int i = 0; i < nClipPath; ++i) {
    const CFX_PathData* pPathData = ClipPath.GetPath(i);
    if (!pPathData)
      continue;

    if (pPathData->GetPointCount() == 0) {
      CFX_PathData EmptyPath;
      EmptyPath.AppendRect(-1, -1, 0, 0);
      int fill_mode = FXFILL_WINDING;
      m_pDevice->SetClip_PathFill(&EmptyPath, nullptr, fill_mode);
    } else {
      int ClipType = ClipPath.GetClipType(i);
      m_pDevice->SetClip_PathFill(pPathData, pObj2Device, ClipType);
    }
  }
  int textcount = ClipPath.GetTextCount();
  if (textcount == 0)
    return;

  if (m_pDevice->GetDeviceClass() == FXDC_DISPLAY &&
      !(m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SOFT_CLIP)) {
    return;
  }

  std::unique_ptr<CFX_PathData> pTextClippingPath;
  for (int i = 0; i < textcount; ++i) {
    CPDF_TextObject* pText = ClipPath.GetText(i);
    if (pText) {
      if (!pTextClippingPath)
        pTextClippingPath.reset(new CFX_PathData);
      ProcessText(pText, pObj2Device, pTextClippingPath.get());
      continue;
    }

    if (!pTextClippingPath)
      continue;

    int fill_mode = FXFILL_WINDING;
    if (m_Options.m_Flags & RENDER_NOTEXTSMOOTH)
      fill_mode |= FXFILL_NOPATHSMOOTH;
    m_pDevice->SetClip_PathFill(pTextClippingPath.get(), nullptr, fill_mode);
    pTextClippingPath.reset();
  }
}

void CPDF_RenderStatus::DrawClipPath(CPDF_ClipPath ClipPath,
                                     const CFX_Matrix* pObj2Device) {
  if (ClipPath.IsNull()) {
    return;
  }
  int fill_mode = 0;
  if (m_Options.m_Flags & RENDER_NOPATHSMOOTH) {
    fill_mode |= FXFILL_NOPATHSMOOTH;
  }
  int nClipPath = ClipPath.GetPathCount();
  int i;
  for (i = 0; i < nClipPath; i++) {
    const CFX_PathData* pPathData = ClipPath.GetPath(i);
    if (!pPathData) {
      continue;
    }
    CFX_GraphStateData stroke_state;
    if (m_Options.m_Flags & RENDER_THINLINE) {
      stroke_state.m_LineWidth = 0;
    }
    m_pDevice->DrawPath(pPathData, pObj2Device, &stroke_state, 0, 0xffff0000,
                        fill_mode);
  }
}
FX_BOOL CPDF_RenderStatus::SelectClipPath(CPDF_PathObject* pPathObj,
                                          const CFX_Matrix* pObj2Device,
                                          FX_BOOL bStroke) {
  CFX_Matrix path_matrix = pPathObj->m_Matrix;
  path_matrix.Concat(*pObj2Device);
  if (bStroke) {
    CFX_GraphStateData graphState(*pPathObj->m_GraphState);
    if (m_Options.m_Flags & RENDER_THINLINE) {
      graphState.m_LineWidth = 0;
    }
    return m_pDevice->SetClip_PathStroke(pPathObj->m_Path, &path_matrix,
                                         &graphState);
  }
  int fill_mode = pPathObj->m_FillType;
  if (m_Options.m_Flags & RENDER_NOPATHSMOOTH) {
    fill_mode |= FXFILL_NOPATHSMOOTH;
  }
  return m_pDevice->SetClip_PathFill(pPathObj->m_Path, &path_matrix, fill_mode);
}
FX_BOOL CPDF_RenderStatus::ProcessTransparency(const CPDF_PageObject* pPageObj,
                                               const CFX_Matrix* pObj2Device) {
  const CPDF_GeneralStateData* pGeneralState = pPageObj->m_GeneralState;
  int blend_type =
      pGeneralState ? pGeneralState->m_BlendType : FXDIB_BLEND_NORMAL;
  if (blend_type == FXDIB_BLEND_UNSUPPORTED) {
    return TRUE;
  }
  CPDF_Dictionary* pSMaskDict =
      pGeneralState ? ToDictionary(pGeneralState->m_pSoftMask) : NULL;
  if (pSMaskDict) {
    if (pPageObj->m_Type == PDFPAGE_IMAGE &&
        ((CPDF_ImageObject*)pPageObj)->m_pImage->GetDict()->KeyExist("SMask")) {
      pSMaskDict = NULL;
    }
  }
  CPDF_Dictionary* pFormResource = NULL;
  FX_FLOAT group_alpha = 1.0f;
  int Transparency = m_Transparency;
  FX_BOOL bGroupTransparent = FALSE;
  if (pPageObj->m_Type == PDFPAGE_FORM) {
    CPDF_FormObject* pFormObj = (CPDF_FormObject*)pPageObj;
    const CPDF_GeneralStateData* pStateData =
        pFormObj->m_GeneralState.GetObject();
    if (pStateData) {
      group_alpha = pStateData->m_FillAlpha;
    }
    Transparency = pFormObj->m_pForm->m_Transparency;
    bGroupTransparent = !!(Transparency & PDFTRANS_ISOLATED);
    if (pFormObj->m_pForm->m_pFormDict) {
      pFormResource = pFormObj->m_pForm->m_pFormDict->GetDict("Resources");
    }
  }
  FX_BOOL bTextClip = FALSE;
  if (pPageObj->m_ClipPath.NotNull() && pPageObj->m_ClipPath.GetTextCount() &&
      m_pDevice->GetDeviceClass() == FXDC_DISPLAY &&
      !(m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SOFT_CLIP)) {
    bTextClip = TRUE;
  }
  if ((m_Options.m_Flags & RENDER_OVERPRINT) &&
      pPageObj->m_Type == PDFPAGE_IMAGE && pGeneralState &&
      pGeneralState->m_FillOP && pGeneralState->m_StrokeOP) {
    CPDF_Document* pDocument = NULL;
    CPDF_Page* pPage = NULL;
    if (m_pContext->m_pPageCache) {
      pPage = m_pContext->m_pPageCache->GetPage();
      pDocument = pPage->m_pDocument;
    } else {
      pDocument = ((CPDF_ImageObject*)pPageObj)->m_pImage->GetDocument();
    }
    CPDF_Dictionary* pPageResources = pPage ? pPage->m_pPageResources : NULL;
    CPDF_Object* pCSObj = ((CPDF_ImageObject*)pPageObj)
                              ->m_pImage->GetStream()
                              ->GetDict()
                              ->GetElementValue("ColorSpace");
    CPDF_ColorSpace* pColorSpace =
        pDocument->LoadColorSpace(pCSObj, pPageResources);
    if (pColorSpace) {
      int format = pColorSpace->GetFamily();
      if (format == PDFCS_DEVICECMYK || format == PDFCS_SEPARATION ||
          format == PDFCS_DEVICEN) {
        blend_type = FXDIB_BLEND_DARKEN;
      }
      pDocument->GetPageData()->ReleaseColorSpace(pCSObj);
    }
  }
  if (!pSMaskDict && group_alpha == 1.0f && blend_type == FXDIB_BLEND_NORMAL &&
      !bTextClip && !bGroupTransparent) {
    return FALSE;
  }
  FX_BOOL isolated = Transparency & PDFTRANS_ISOLATED;
  if (m_bPrint) {
    FX_BOOL bRet = FALSE;
    int rendCaps = m_pDevice->GetRenderCaps();
    if (!((Transparency & PDFTRANS_ISOLATED) || pSMaskDict || bTextClip) &&
        (rendCaps & FXRC_BLEND_MODE)) {
      int oldBlend = m_curBlend;
      m_curBlend = blend_type;
      bRet = DrawObjWithBlend(pPageObj, pObj2Device);
      m_curBlend = oldBlend;
    }
    if (!bRet) {
      DrawObjWithBackground(pPageObj, pObj2Device);
    }
    return TRUE;
  }
  FX_RECT rect = pPageObj->GetBBox(pObj2Device);
  rect.Intersect(m_pDevice->GetClipBox());
  if (rect.IsEmpty()) {
    return TRUE;
  }
  CFX_Matrix deviceCTM = m_pDevice->GetCTM();
  FX_FLOAT scaleX = FXSYS_fabs(deviceCTM.a);
  FX_FLOAT scaleY = FXSYS_fabs(deviceCTM.d);
  int width = FXSYS_round((FX_FLOAT)rect.Width() * scaleX);
  int height = FXSYS_round((FX_FLOAT)rect.Height() * scaleY);
  CFX_FxgeDevice bitmap_device;
  std::unique_ptr<CFX_DIBitmap> oriDevice;
  if (!isolated && (m_pDevice->GetRenderCaps() & FXRC_GET_BITS)) {
    oriDevice.reset(new CFX_DIBitmap);
    if (!m_pDevice->CreateCompatibleBitmap(oriDevice.get(), width, height))
      return TRUE;

    m_pDevice->GetDIBits(oriDevice.get(), rect.left, rect.top);
  }
  if (!bitmap_device.Create(width, height, FXDIB_Argb, 0, oriDevice.get()))
    return TRUE;

  CFX_DIBitmap* bitmap = bitmap_device.GetBitmap();
  bitmap->Clear(0);
  CFX_Matrix new_matrix = *pObj2Device;
  new_matrix.TranslateI(-rect.left, -rect.top);
  new_matrix.Scale(scaleX, scaleY);
  std::unique_ptr<CFX_DIBitmap> pTextMask;
  if (bTextClip) {
    pTextMask.reset(new CFX_DIBitmap);
    if (!pTextMask->Create(width, height, FXDIB_8bppMask))
      return TRUE;

    pTextMask->Clear(0);
    CFX_FxgeDevice text_device;
    text_device.Attach(pTextMask.get());
    for (FX_DWORD i = 0; i < pPageObj->m_ClipPath.GetTextCount(); i++) {
      CPDF_TextObject* textobj = pPageObj->m_ClipPath.GetText(i);
      if (!textobj) {
        break;
      }
      CFX_Matrix text_matrix;
      textobj->GetTextMatrix(&text_matrix);
      CPDF_TextRenderer::DrawTextPath(
          &text_device, textobj->m_nChars, textobj->m_pCharCodes,
          textobj->m_pCharPos, textobj->m_TextState.GetFont(),
          textobj->m_TextState.GetFontSize(), &text_matrix, &new_matrix,
          textobj->m_GraphState, (FX_ARGB)-1, 0, NULL);
    }
  }
  CPDF_RenderStatus bitmap_render;
  bitmap_render.Initialize(m_pContext, &bitmap_device, NULL, m_pStopObj, NULL,
                           NULL, &m_Options, 0, m_bDropObjects, pFormResource,
                           TRUE);
  bitmap_render.ProcessObjectNoClip(pPageObj, &new_matrix);
  m_bStopped = bitmap_render.m_bStopped;
  if (pSMaskDict) {
    CFX_Matrix smask_matrix;
    FXSYS_memcpy(&smask_matrix, pGeneralState->m_SMaskMatrix,
                 sizeof smask_matrix);
    smask_matrix.Concat(*pObj2Device);
    std::unique_ptr<CFX_DIBSource> pSMaskSource(
        LoadSMask(pSMaskDict, &rect, &smask_matrix));
    if (pSMaskSource)
      bitmap->MultiplyAlpha(pSMaskSource.get());
  }
  if (pTextMask) {
    bitmap->MultiplyAlpha(pTextMask.get());
    pTextMask.reset();
  }
  if (Transparency & PDFTRANS_GROUP && group_alpha != 1.0f) {
    bitmap->MultiplyAlpha((int32_t)(group_alpha * 255));
  }
  Transparency = m_Transparency;
  if (pPageObj->m_Type == PDFPAGE_FORM) {
    Transparency |= PDFTRANS_GROUP;
  }
  CompositeDIBitmap(bitmap, rect.left, rect.top, 0, 255, blend_type,
                    Transparency);
  return TRUE;
}

CFX_DIBitmap* CPDF_RenderStatus::GetBackdrop(const CPDF_PageObject* pObj,
                                             const FX_RECT& rect,
                                             int& left,
                                             int& top,
                                             FX_BOOL bBackAlphaRequired) {
  FX_RECT bbox = rect;
  bbox.Intersect(m_pDevice->GetClipBox());
  left = bbox.left;
  top = bbox.top;
  CFX_Matrix deviceCTM = m_pDevice->GetCTM();
  FX_FLOAT scaleX = FXSYS_fabs(deviceCTM.a);
  FX_FLOAT scaleY = FXSYS_fabs(deviceCTM.d);
  int width = FXSYS_round(bbox.Width() * scaleX);
  int height = FXSYS_round(bbox.Height() * scaleY);
  std::unique_ptr<CFX_DIBitmap> pBackdrop(new CFX_DIBitmap);
  if (bBackAlphaRequired && !m_bDropObjects)
    pBackdrop->Create(width, height, FXDIB_Argb);
  else
    m_pDevice->CreateCompatibleBitmap(pBackdrop.get(), width, height);

  if (!pBackdrop->GetBuffer())
    return nullptr;

  FX_BOOL bNeedDraw;
  if (pBackdrop->HasAlpha())
    bNeedDraw = !(m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT);
  else
    bNeedDraw = !(m_pDevice->GetRenderCaps() & FXRC_GET_BITS);

  if (!bNeedDraw) {
    m_pDevice->GetDIBits(pBackdrop.get(), left, top);
    return pBackdrop.release();
  }

  CFX_Matrix FinalMatrix = m_DeviceMatrix;
  FinalMatrix.TranslateI(-left, -top);
  FinalMatrix.Scale(scaleX, scaleY);
  pBackdrop->Clear(pBackdrop->HasAlpha() ? 0 : 0xffffffff);
  CFX_FxgeDevice device;
  device.Attach(pBackdrop.get());
  m_pContext->Render(&device, pObj, &m_Options, &FinalMatrix);
  return pBackdrop.release();
}

void CPDF_RenderContext::GetBackground(CFX_DIBitmap* pBuffer,
                                       const CPDF_PageObject* pObj,
                                       const CPDF_RenderOptions* pOptions,
                                       CFX_Matrix* pFinalMatrix) {
  CFX_FxgeDevice device;
  device.Attach(pBuffer);

  FX_RECT rect(0, 0, device.GetWidth(), device.GetHeight());
  device.FillRect(&rect, 0xffffffff);
  Render(&device, pObj, pOptions, pFinalMatrix);
}
CPDF_GraphicStates* CPDF_RenderStatus::CloneObjStates(
    const CPDF_GraphicStates* pSrcStates,
    FX_BOOL bStroke) {
  if (!pSrcStates) {
    return NULL;
  }
  CPDF_GraphicStates* pStates = new CPDF_GraphicStates;
  pStates->CopyStates(*pSrcStates);
  CPDF_Color* pObjColor = bStroke ? pSrcStates->m_ColorState.GetStrokeColor()
                                  : pSrcStates->m_ColorState.GetFillColor();
  if (!pObjColor->IsNull()) {
    CPDF_ColorStateData* pColorData = pStates->m_ColorState.GetModify();
    pColorData->m_FillRGB =
        bStroke ? pSrcStates->m_ColorState.GetObject()->m_StrokeRGB
                : pSrcStates->m_ColorState.GetObject()->m_FillRGB;
    pColorData->m_StrokeRGB = pColorData->m_FillRGB;
  }
  return pStates;
}
CPDF_RenderContext::CPDF_RenderContext(CPDF_Page* pPage)
    : m_pDocument(pPage->m_pDocument),
      m_pPageResources(pPage->m_pPageResources),
      m_pPageCache(pPage->GetRenderCache()),
      m_bFirstLayer(TRUE) {}
CPDF_RenderContext::CPDF_RenderContext(CPDF_Document* pDoc,
                                       CPDF_PageRenderCache* pPageCache)
    : m_pDocument(pDoc),
      m_pPageResources(nullptr),
      m_pPageCache(pPageCache),
      m_bFirstLayer(TRUE) {}
CPDF_RenderContext::~CPDF_RenderContext() {}
void CPDF_RenderContext::AppendObjectList(CPDF_PageObjects* pObjs,
                                          const CFX_Matrix* pObject2Device) {
  _PDF_RenderItem* pItem = m_ContentList.AddSpace();
  pItem->m_pObjectList = pObjs;
  if (pObject2Device) {
    pItem->m_Matrix = *pObject2Device;
  } else {
    pItem->m_Matrix.SetIdentity();
  }
}
void CPDF_RenderContext::Render(CFX_RenderDevice* pDevice,
                                const CPDF_RenderOptions* pOptions,
                                const CFX_Matrix* pLastMatrix) {
  Render(pDevice, NULL, pOptions, pLastMatrix);
}
void CPDF_RenderContext::Render(CFX_RenderDevice* pDevice,
                                const CPDF_PageObject* pStopObj,
                                const CPDF_RenderOptions* pOptions,
                                const CFX_Matrix* pLastMatrix) {
  int count = m_ContentList.GetSize();
  for (int j = 0; j < count; j++) {
    pDevice->SaveState();
    _PDF_RenderItem* pItem = m_ContentList.GetDataPtr(j);
    if (pLastMatrix) {
      CFX_Matrix FinalMatrix = pItem->m_Matrix;
      FinalMatrix.Concat(*pLastMatrix);
      CPDF_RenderStatus status;
      status.Initialize(this, pDevice, pLastMatrix, pStopObj, NULL, NULL,
                        pOptions, pItem->m_pObjectList->m_Transparency, FALSE,
                        NULL);
      status.RenderObjectList(pItem->m_pObjectList, &FinalMatrix);
      if (status.m_Options.m_Flags & RENDER_LIMITEDIMAGECACHE) {
        m_pPageCache->CacheOptimization(status.m_Options.m_dwLimitCacheSize);
      }
      if (status.m_bStopped) {
        pDevice->RestoreState();
        break;
      }
    } else {
      CPDF_RenderStatus status;
      status.Initialize(this, pDevice, NULL, pStopObj, NULL, NULL, pOptions,
                        pItem->m_pObjectList->m_Transparency, FALSE, NULL);
      status.RenderObjectList(pItem->m_pObjectList, &pItem->m_Matrix);
      if (status.m_Options.m_Flags & RENDER_LIMITEDIMAGECACHE) {
        m_pPageCache->CacheOptimization(status.m_Options.m_dwLimitCacheSize);
      }
      if (status.m_bStopped) {
        pDevice->RestoreState();
        break;
      }
    }
    pDevice->RestoreState();
  }
}
void CPDF_RenderContext::DrawObjectList(CFX_RenderDevice* pDevice,
                                        CPDF_PageObjects* pObjs,
                                        const CFX_Matrix* pObject2Device,
                                        const CPDF_RenderOptions* pOptions) {
  AppendObjectList(pObjs, pObject2Device);
  Render(pDevice, pOptions);
}

CPDF_ProgressiveRenderer::CPDF_ProgressiveRenderer(
    CPDF_RenderContext* pContext,
    CFX_RenderDevice* pDevice,
    const CPDF_RenderOptions* pOptions)
    : m_Status(Ready),
      m_pContext(pContext),
      m_pDevice(pDevice),
      m_pOptions(pOptions),
      m_LayerIndex(0),
      m_ObjectIndex(0),
      m_ObjectPos(nullptr),
      m_PrevLastPos(nullptr) {
}

CPDF_ProgressiveRenderer::~CPDF_ProgressiveRenderer() {
  if (m_pRenderStatus)
    m_pDevice->RestoreState();
}

void CPDF_ProgressiveRenderer::Start(IFX_Pause* pPause) {
  if (!m_pContext || !m_pDevice || m_Status != Ready) {
    m_Status = Failed;
    return;
  }
  m_Status = ToBeContinued;
  Continue(pPause);
}

#define RENDER_STEP_LIMIT 100
void CPDF_ProgressiveRenderer::Continue(IFX_Pause* pPause) {
  if (m_Status != ToBeContinued) {
    return;
  }
  FX_DWORD nLayers = m_pContext->m_ContentList.GetSize();
  for (; m_LayerIndex < nLayers; m_LayerIndex++) {
    _PDF_RenderItem* pItem = m_pContext->m_ContentList.GetDataPtr(m_LayerIndex);
    FX_POSITION LastPos = pItem->m_pObjectList->GetLastObjectPosition();
    if (!m_ObjectPos) {
      if (LastPos == m_PrevLastPos) {
        if (!pItem->m_pObjectList->IsParsed()) {
          pItem->m_pObjectList->ContinueParse(pPause);
          if (!pItem->m_pObjectList->IsParsed()) {
            return;
          }
          LastPos = pItem->m_pObjectList->GetLastObjectPosition();
        }
      }
      if (LastPos == m_PrevLastPos) {
        if (m_pRenderStatus) {
          m_pRenderStatus.reset();
          m_pDevice->RestoreState();
          m_ObjectPos = NULL;
          m_PrevLastPos = NULL;
        }
        continue;
      }
      if (m_PrevLastPos) {
        m_ObjectPos = m_PrevLastPos;
        pItem->m_pObjectList->GetNextObject(m_ObjectPos);
      } else {
        m_ObjectPos = pItem->m_pObjectList->GetFirstObjectPosition();
      }
      m_PrevLastPos = LastPos;
    }
    if (!m_pRenderStatus) {
      m_ObjectPos = pItem->m_pObjectList->GetFirstObjectPosition();
      m_ObjectIndex = 0;
      m_pRenderStatus.reset(new CPDF_RenderStatus());
      m_pRenderStatus->Initialize(
          m_pContext, m_pDevice, NULL, NULL, NULL, NULL, m_pOptions,
          pItem->m_pObjectList->m_Transparency, FALSE, NULL);
      m_pDevice->SaveState();
      m_ClipRect = m_pDevice->GetClipBox();
      CFX_Matrix device2object;
      device2object.SetReverse(pItem->m_Matrix);
      device2object.TransformRect(m_ClipRect);
    }
    int objs_to_go = CPDF_ModuleMgr::Get()
                         ->GetRenderModule()
                         ->GetConfig()
                         ->m_RenderStepLimit;
    while (m_ObjectPos) {
      CPDF_PageObject* pCurObj = pItem->m_pObjectList->GetObjectAt(m_ObjectPos);
      if (pCurObj && pCurObj->m_Left <= m_ClipRect.right &&
          pCurObj->m_Right >= m_ClipRect.left &&
          pCurObj->m_Bottom <= m_ClipRect.top &&
          pCurObj->m_Top >= m_ClipRect.bottom) {
        if (m_pRenderStatus->ContinueSingleObject(pCurObj, &pItem->m_Matrix,
                                                  pPause)) {
          return;
        }
        if (pCurObj->m_Type == PDFPAGE_IMAGE &&
            m_pRenderStatus->m_Options.m_Flags & RENDER_LIMITEDIMAGECACHE) {
          m_pContext->GetPageCache()->CacheOptimization(
              m_pRenderStatus->m_Options.m_dwLimitCacheSize);
        }
        if (pCurObj->m_Type == PDFPAGE_FORM ||
            pCurObj->m_Type == PDFPAGE_SHADING) {
          objs_to_go = 0;
        } else {
          objs_to_go--;
        }
      }
      m_ObjectIndex++;
      pItem->m_pObjectList->GetNextObject(m_ObjectPos);
      if (objs_to_go == 0) {
        if (pPause && pPause->NeedToPauseNow()) {
          return;
        }
        objs_to_go = CPDF_ModuleMgr::Get()
                         ->GetRenderModule()
                         ->GetConfig()
                         ->m_RenderStepLimit;
      }
    }
    if (!pItem->m_pObjectList->IsParsed()) {
      return;
    }
    m_pRenderStatus.reset();
    m_pDevice->RestoreState();
    m_ObjectPos = NULL;
    m_PrevLastPos = NULL;
    if (pPause && pPause->NeedToPauseNow()) {
      m_LayerIndex++;
      return;
    }
  }
  m_Status = Done;
}
int CPDF_ProgressiveRenderer::EstimateProgress() {
  if (!m_pContext) {
    return 0;
  }
  FX_DWORD nLayers = m_pContext->m_ContentList.GetSize();
  int nTotal = 0, nRendered = 0;
  for (FX_DWORD layer = 0; layer < nLayers; layer++) {
    _PDF_RenderItem* pItem = m_pContext->m_ContentList.GetDataPtr(layer);
    int nObjs = pItem->m_pObjectList->CountObjects();
    if (layer == m_LayerIndex) {
      nRendered += m_ObjectIndex;
    } else if (layer < m_LayerIndex) {
      nRendered += nObjs;
    }
    nTotal += nObjs;
  }
  if (nTotal == 0) {
    return 0;
  }
  return 100 * nRendered / nTotal;
}
CPDF_TransferFunc* CPDF_DocRenderData::GetTransferFunc(CPDF_Object* pObj) {
  if (!pObj)
    return nullptr;

  auto it = m_TransferFuncMap.find(pObj);
  if (it != m_TransferFuncMap.end()) {
    CPDF_CountedObject<CPDF_TransferFunc>* pTransferCounter = it->second;
    return pTransferCounter->AddRef();
  }

  std::unique_ptr<CPDF_Function> pFuncs[3];
  FX_BOOL bUniTransfer = TRUE;
  FX_BOOL bIdentity = TRUE;
  if (CPDF_Array* pArray = pObj->AsArray()) {
    bUniTransfer = FALSE;
    if (pArray->GetCount() < 3)
      return nullptr;

    for (FX_DWORD i = 0; i < 3; ++i) {
      pFuncs[2 - i].reset(CPDF_Function::Load(pArray->GetElementValue(i)));
      if (!pFuncs[2 - i])
        return nullptr;
    }
  } else {
    pFuncs[0].reset(CPDF_Function::Load(pObj));
    if (!pFuncs[0])
      return nullptr;
  }
  CPDF_TransferFunc* pTransfer = new CPDF_TransferFunc(m_pPDFDoc);
  CPDF_CountedObject<CPDF_TransferFunc>* pTransferCounter =
      new CPDF_CountedObject<CPDF_TransferFunc>(pTransfer);
  m_TransferFuncMap[pObj] = pTransferCounter;
  static const int kMaxOutputs = 16;
  FX_FLOAT output[kMaxOutputs];
  FXSYS_memset(output, 0, sizeof(output));
  FX_FLOAT input;
  int noutput;
  for (int v = 0; v < 256; ++v) {
    input = (FX_FLOAT)v / 255.0f;
    if (bUniTransfer) {
      if (pFuncs[0] && pFuncs[0]->CountOutputs() <= kMaxOutputs)
        pFuncs[0]->Call(&input, 1, output, noutput);
      int o = FXSYS_round(output[0] * 255);
      if (o != v)
        bIdentity = FALSE;
      for (int i = 0; i < 3; ++i) {
        pTransfer->m_Samples[i * 256 + v] = o;
      }
    } else {
      for (int i = 0; i < 3; ++i) {
        if (pFuncs[i] && pFuncs[i]->CountOutputs() <= kMaxOutputs) {
          pFuncs[i]->Call(&input, 1, output, noutput);
          int o = FXSYS_round(output[0] * 255);
          if (o != v)
            bIdentity = FALSE;
          pTransfer->m_Samples[i * 256 + v] = o;
        } else {
          pTransfer->m_Samples[i * 256 + v] = v;
        }
      }
    }
  }

  pTransfer->m_bIdentity = bIdentity;
  return pTransferCounter->AddRef();
}

void CPDF_DocRenderData::ReleaseTransferFunc(CPDF_Object* pObj) {
  auto it = m_TransferFuncMap.find(pObj);
  if (it != m_TransferFuncMap.end())
    it->second->RemoveRef();
}
CPDF_RenderConfig::CPDF_RenderConfig() {
  m_HalftoneLimit = 0;
  m_RenderStepLimit = 100;
}
CPDF_RenderConfig::~CPDF_RenderConfig() {}

CPDF_DeviceBuffer::CPDF_DeviceBuffer()
    : m_pDevice(nullptr), m_pContext(nullptr), m_pObject(nullptr) {}

CPDF_DeviceBuffer::~CPDF_DeviceBuffer() {
}

FX_BOOL CPDF_DeviceBuffer::Initialize(CPDF_RenderContext* pContext,
                                      CFX_RenderDevice* pDevice,
                                      FX_RECT* pRect,
                                      const CPDF_PageObject* pObj,
                                      int max_dpi) {
  m_pDevice = pDevice;
  m_pContext = pContext;
  m_Rect = *pRect;
  m_pObject = pObj;
  m_Matrix.TranslateI(-pRect->left, -pRect->top);
#if _FXM_PLATFORM_ != _FXM_PLATFORM_APPLE_
  int horz_size = pDevice->GetDeviceCaps(FXDC_HORZ_SIZE);
  int vert_size = pDevice->GetDeviceCaps(FXDC_VERT_SIZE);
  if (horz_size && vert_size && max_dpi) {
    int dpih =
        pDevice->GetDeviceCaps(FXDC_PIXEL_WIDTH) * 254 / (horz_size * 10);
    int dpiv =
        pDevice->GetDeviceCaps(FXDC_PIXEL_HEIGHT) * 254 / (vert_size * 10);
    if (dpih > max_dpi) {
      m_Matrix.Scale((FX_FLOAT)(max_dpi) / dpih, 1.0f);
    }
    if (dpiv > max_dpi) {
      m_Matrix.Scale(1.0f, (FX_FLOAT)(max_dpi) / (FX_FLOAT)dpiv);
    }
  }
#endif
  CFX_Matrix ctm = m_pDevice->GetCTM();
  FX_FLOAT fScaleX = FXSYS_fabs(ctm.a);
  FX_FLOAT fScaleY = FXSYS_fabs(ctm.d);
  m_Matrix.Concat(fScaleX, 0, 0, fScaleY, 0, 0);
  CFX_FloatRect rect(*pRect);
  m_Matrix.TransformRect(rect);
  FX_RECT bitmap_rect = rect.GetOutterRect();
  m_pBitmap.reset(new CFX_DIBitmap);
  m_pBitmap->Create(bitmap_rect.Width(), bitmap_rect.Height(), FXDIB_Argb);
  return TRUE;
}
void CPDF_DeviceBuffer::OutputToDevice() {
  if (m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_GET_BITS) {
    if (m_Matrix.a == 1.0f && m_Matrix.d == 1.0f) {
      m_pDevice->SetDIBits(m_pBitmap.get(), m_Rect.left, m_Rect.top);
    } else {
      m_pDevice->StretchDIBits(m_pBitmap.get(), m_Rect.left, m_Rect.top,
                               m_Rect.Width(), m_Rect.Height());
    }
  } else {
    CFX_DIBitmap buffer;
    m_pDevice->CreateCompatibleBitmap(&buffer, m_pBitmap->GetWidth(),
                                      m_pBitmap->GetHeight());
    m_pContext->GetBackground(&buffer, m_pObject, NULL, &m_Matrix);
    buffer.CompositeBitmap(0, 0, buffer.GetWidth(), buffer.GetHeight(),
                           m_pBitmap.get(), 0, 0);
    m_pDevice->StretchDIBits(&buffer, m_Rect.left, m_Rect.top, m_Rect.Width(),
                             m_Rect.Height());
  }
}

CPDF_ScaledRenderBuffer::CPDF_ScaledRenderBuffer() {}

CPDF_ScaledRenderBuffer::~CPDF_ScaledRenderBuffer() {}

#define _FPDFAPI_IMAGESIZE_LIMIT_ (30 * 1024 * 1024)
FX_BOOL CPDF_ScaledRenderBuffer::Initialize(CPDF_RenderContext* pContext,
                                            CFX_RenderDevice* pDevice,
                                            const FX_RECT& pRect,
                                            const CPDF_PageObject* pObj,
                                            const CPDF_RenderOptions* pOptions,
                                            int max_dpi) {
  m_pDevice = pDevice;
  if (m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_GET_BITS) {
    return TRUE;
  }
  m_pContext = pContext;
  m_Rect = pRect;
  m_pObject = pObj;
  m_Matrix.TranslateI(-pRect.left, -pRect.top);
  int horz_size = pDevice->GetDeviceCaps(FXDC_HORZ_SIZE);
  int vert_size = pDevice->GetDeviceCaps(FXDC_VERT_SIZE);
  if (horz_size && vert_size && max_dpi) {
    int dpih =
        pDevice->GetDeviceCaps(FXDC_PIXEL_WIDTH) * 254 / (horz_size * 10);
    int dpiv =
        pDevice->GetDeviceCaps(FXDC_PIXEL_HEIGHT) * 254 / (vert_size * 10);
    if (dpih > max_dpi) {
      m_Matrix.Scale((FX_FLOAT)(max_dpi) / dpih, 1.0f);
    }
    if (dpiv > max_dpi) {
      m_Matrix.Scale(1.0f, (FX_FLOAT)(max_dpi) / (FX_FLOAT)dpiv);
    }
  }
  m_pBitmapDevice.reset(new CFX_FxgeDevice);
  FXDIB_Format dibFormat = FXDIB_Rgb;
  int32_t bpp = 24;
  if (m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_ALPHA_OUTPUT) {
    dibFormat = FXDIB_Argb;
    bpp = 32;
  }
  CFX_FloatRect rect;
  int32_t iWidth, iHeight, iPitch;
  while (1) {
    rect = pRect;
    m_Matrix.TransformRect(rect);
    FX_RECT bitmap_rect = rect.GetOutterRect();
    iWidth = bitmap_rect.Width();
    iHeight = bitmap_rect.Height();
    iPitch = (iWidth * bpp + 31) / 32 * 4;
    if (iWidth * iHeight < 1) {
      return FALSE;
    }
    if (iPitch * iHeight <= _FPDFAPI_IMAGESIZE_LIMIT_ &&
        m_pBitmapDevice->Create(iWidth, iHeight, dibFormat)) {
      break;
    }
    m_Matrix.Scale(0.5f, 0.5f);
  }
  m_pContext->GetBackground(m_pBitmapDevice->GetBitmap(), m_pObject, pOptions,
                            &m_Matrix);
  return TRUE;
}
void CPDF_ScaledRenderBuffer::OutputToDevice() {
  if (m_pBitmapDevice) {
    m_pDevice->StretchDIBits(m_pBitmapDevice->GetBitmap(), m_Rect.left,
                             m_Rect.top, m_Rect.Width(), m_Rect.Height());
  }
}
FX_BOOL IPDF_OCContext::CheckObjectVisible(const CPDF_PageObject* pObj) {
  const CPDF_ContentMarkData* pData = pObj->m_ContentMark;
  int nItems = pData->CountItems();
  for (int i = 0; i < nItems; i++) {
    CPDF_ContentMarkItem& item = pData->GetItem(i);
    if (item.GetName() == "OC" &&
        item.GetParamType() == CPDF_ContentMarkItem::PropertiesDict) {
      CPDF_Dictionary* pOCG =
          ToDictionary(static_cast<CPDF_Object*>(item.GetParam()));
      if (!CheckOCGVisible(pOCG)) {
        return FALSE;
      }
    }
  }
  return TRUE;
}

// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_renderstatus.h"

#include <algorithm>
#include <cmath>
#include <limits>
#include <memory>
#include <numeric>
#include <set>
#include <utility>
#include <vector>

#include "build/build_config.h"
#include "constants/transparency.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/font/cpdf_type3char.h"
#include "core/fpdfapi/font/cpdf_type3font.h"
#include "core/fpdfapi/page/cpdf_docpagedata.h"
#include "core/fpdfapi/page/cpdf_form.h"
#include "core/fpdfapi/page/cpdf_formobject.h"
#include "core/fpdfapi/page/cpdf_function.h"
#include "core/fpdfapi/page/cpdf_graphicstates.h"
#include "core/fpdfapi/page/cpdf_image.h"
#include "core/fpdfapi/page/cpdf_imageobject.h"
#include "core/fpdfapi/page/cpdf_occontext.h"
#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fpdfapi/page/cpdf_pageobject.h"
#include "core/fpdfapi/page/cpdf_pathobject.h"
#include "core/fpdfapi/page/cpdf_shadingobject.h"
#include "core/fpdfapi/page/cpdf_shadingpattern.h"
#include "core/fpdfapi/page/cpdf_textobject.h"
#include "core/fpdfapi/page/cpdf_tilingpattern.h"
#include "core/fpdfapi/page/cpdf_transferfunc.h"
#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/fpdf_parser_utility.h"
#include "core/fpdfapi/render/cpdf_charposlist.h"
#include "core/fpdfapi/render/cpdf_docrenderdata.h"
#include "core/fpdfapi/render/cpdf_imagerenderer.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfapi/render/cpdf_rendershading.h"
#include "core/fpdfapi/render/cpdf_scaledrenderbuffer.h"
#include "core/fpdfapi/render/cpdf_textrenderer.h"
#include "core/fpdfapi/render/cpdf_type3cache.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_glyphbitmap.h"
#include "core/fxge/cfx_pathdata.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/renderdevicedriver_iface.h"
#include "core/fxge/text_char_pos.h"
#include "core/fxge/text_glyph_pos.h"
#include "third_party/base/compiler_specific.h"
#include "third_party/base/logging.h"
#include "third_party/base/numerics/safe_math.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"

#ifdef _SKIA_SUPPORT_
#include "core/fxge/skia/fx_skia_device.h"
#endif

namespace {

constexpr int kRenderMaxRecursionDepth = 64;
int g_CurrentRecursionDepth = 0;

RetainPtr<CFX_DIBitmap> DrawPatternBitmap(
    CPDF_Document* pDoc,
    CPDF_PageRenderCache* pCache,
    CPDF_TilingPattern* pPattern,
    CPDF_Form* pPatternForm,
    const CFX_Matrix& mtObject2Device,
    int width,
    int height,
    const CPDF_RenderOptions::Options& draw_options) {
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pBitmap->Create(width, height,
                       pPattern->colored() ? FXDIB_Argb : FXDIB_8bppMask)) {
    return nullptr;
  }
  CFX_DefaultRenderDevice bitmap_device;
  bitmap_device.Attach(pBitmap, false, nullptr, false);
  pBitmap->Clear(0);
  CFX_FloatRect cell_bbox =
      pPattern->pattern_to_form().TransformRect(pPattern->bbox());
  cell_bbox = mtObject2Device.TransformRect(cell_bbox);
  CFX_FloatRect bitmap_rect(0.0f, 0.0f, width, height);
  CFX_Matrix mtAdjust;
  mtAdjust.MatchRect(bitmap_rect, cell_bbox);

  CFX_Matrix mtPattern2Bitmap = mtObject2Device * mtAdjust;
  CPDF_RenderOptions options;
  if (!pPattern->colored())
    options.SetColorMode(CPDF_RenderOptions::kAlpha);

  options.GetOptions() = draw_options;
  options.GetOptions().bForceHalftone = true;

  CPDF_RenderContext context(pDoc, nullptr, pCache);
  context.AppendLayer(pPatternForm, &mtPattern2Bitmap);
  context.Render(&bitmap_device, &options, nullptr);
#if defined _SKIA_SUPPORT_PATHS_
  bitmap_device.Flush(true);
  pBitmap->UnPreMultiply();
#endif
  return pBitmap;
}

bool IsAvailableMatrix(const CFX_Matrix& matrix) {
  if (matrix.a == 0 || matrix.d == 0)
    return matrix.b != 0 && matrix.c != 0;

  if (matrix.b == 0 || matrix.c == 0)
    return matrix.a != 0 && matrix.d != 0;

  return true;
}

bool MissingFillColor(const CPDF_ColorState* pColorState) {
  return !pColorState->HasRef() || pColorState->GetFillColor()->IsNull();
}

bool MissingStrokeColor(const CPDF_ColorState* pColorState) {
  return !pColorState->HasRef() || pColorState->GetStrokeColor()->IsNull();
}

bool Type3CharMissingFillColor(const CPDF_Type3Char* pChar,
                               const CPDF_ColorState* pColorState) {
  return pChar && (!pChar->colored() || MissingFillColor(pColorState));
}

bool Type3CharMissingStrokeColor(const CPDF_Type3Char* pChar,
                                 const CPDF_ColorState* pColorState) {
  return pChar && (!pChar->colored() || MissingStrokeColor(pColorState));
}

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
class ScopedSkiaDeviceFlush {
 public:
  explicit ScopedSkiaDeviceFlush(CFX_RenderDevice* pDevice)
      : m_pDevice(pDevice) {}

  ScopedSkiaDeviceFlush(const ScopedSkiaDeviceFlush&) = delete;
  ScopedSkiaDeviceFlush& operator=(const ScopedSkiaDeviceFlush&) = delete;

  ~ScopedSkiaDeviceFlush() { m_pDevice->Flush(/*release=*/false); }

 private:
  CFX_RenderDevice* const m_pDevice;
};
#endif

}  // namespace

CPDF_RenderStatus::CPDF_RenderStatus(CPDF_RenderContext* pContext,
                                     CFX_RenderDevice* pDevice)
    : m_pContext(pContext), m_pDevice(pDevice) {}

CPDF_RenderStatus::~CPDF_RenderStatus() {}

void CPDF_RenderStatus::Initialize(const CPDF_RenderStatus* pParentStatus,
                                   const CPDF_GraphicStates* pInitialStates) {
  m_bPrint = m_pDevice->GetDeviceType() != DeviceType::kDisplay;
  m_pPageResource.Reset(m_pContext->GetPageResources());
  if (pInitialStates && !m_pType3Char) {
    m_InitialStates.CopyStates(*pInitialStates);
    if (pParentStatus) {
      if (!m_InitialStates.m_ColorState.HasFillColor()) {
        m_InitialStates.m_ColorState.SetFillColorRef(
            pParentStatus->m_InitialStates.m_ColorState.GetFillColorRef());
        *m_InitialStates.m_ColorState.GetMutableFillColor() =
            *pParentStatus->m_InitialStates.m_ColorState.GetFillColor();
      }
      if (!m_InitialStates.m_ColorState.HasStrokeColor()) {
        m_InitialStates.m_ColorState.SetStrokeColorRef(
            pParentStatus->m_InitialStates.m_ColorState.GetFillColorRef());
        *m_InitialStates.m_ColorState.GetMutableStrokeColor() =
            *pParentStatus->m_InitialStates.m_ColorState.GetStrokeColor();
      }
    }
  } else {
    m_InitialStates.DefaultStates();
  }
}

void CPDF_RenderStatus::RenderObjectList(
    const CPDF_PageObjectHolder* pObjectHolder,
    const CFX_Matrix& mtObj2Device) {
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
  CFX_FloatRect clip_rect = mtObj2Device.GetInverse().TransformRect(
      CFX_FloatRect(m_pDevice->GetClipBox()));
  for (const auto& pCurObj : *pObjectHolder) {
    if (pCurObj.get() == m_pStopObj) {
      m_bStopped = true;
      return;
    }
    if (!pCurObj)
      continue;

    if (pCurObj->GetRect().left > clip_rect.right ||
        pCurObj->GetRect().right < clip_rect.left ||
        pCurObj->GetRect().bottom > clip_rect.top ||
        pCurObj->GetRect().top < clip_rect.bottom) {
      continue;
    }
    RenderSingleObject(pCurObj.get(), mtObj2Device);
    if (m_bStopped)
      return;
  }
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
}

void CPDF_RenderStatus::RenderSingleObject(CPDF_PageObject* pObj,
                                           const CFX_Matrix& mtObj2Device) {
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
  AutoRestorer<int> restorer(&g_CurrentRecursionDepth);
  if (++g_CurrentRecursionDepth > kRenderMaxRecursionDepth) {
    return;
  }
  m_pCurObj = pObj;
  if (m_Options.GetOCContext() &&
      !m_Options.GetOCContext()->CheckObjectVisible(pObj)) {
    return;
  }
  ProcessClipPath(pObj->m_ClipPath, mtObj2Device);
  if (ProcessTransparency(pObj, mtObj2Device)) {
    return;
  }
  ProcessObjectNoClip(pObj, mtObj2Device);
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
}

bool CPDF_RenderStatus::ContinueSingleObject(CPDF_PageObject* pObj,
                                             const CFX_Matrix& mtObj2Device,
                                             PauseIndicatorIface* pPause) {
  if (m_pImageRenderer) {
    if (m_pImageRenderer->Continue(pPause))
      return true;

    if (!m_pImageRenderer->GetResult())
      DrawObjWithBackground(pObj, mtObj2Device);
    m_pImageRenderer.reset();
    return false;
  }

  m_pCurObj = pObj;
  if (m_Options.GetOCContext() &&
      !m_Options.GetOCContext()->CheckObjectVisible(pObj)) {
    return false;
  }

  ProcessClipPath(pObj->m_ClipPath, mtObj2Device);
  if (ProcessTransparency(pObj, mtObj2Device))
    return false;

  if (!pObj->IsImage()) {
    ProcessObjectNoClip(pObj, mtObj2Device);
    return false;
  }

  m_pImageRenderer = pdfium::MakeUnique<CPDF_ImageRenderer>();
  if (!m_pImageRenderer->Start(this, pObj->AsImage(), mtObj2Device, false,
                               BlendMode::kNormal)) {
    if (!m_pImageRenderer->GetResult())
      DrawObjWithBackground(pObj, mtObj2Device);
    m_pImageRenderer.reset();
    return false;
  }
  return ContinueSingleObject(pObj, mtObj2Device, pPause);
}

FX_RECT CPDF_RenderStatus::GetObjectClippedRect(
    const CPDF_PageObject* pObj,
    const CFX_Matrix& mtObj2Device) const {
  FX_RECT rect = pObj->GetTransformedBBox(mtObj2Device);
  rect.Intersect(m_pDevice->GetClipBox());
  return rect;
}

void CPDF_RenderStatus::ProcessObjectNoClip(CPDF_PageObject* pObj,
                                            const CFX_Matrix& mtObj2Device) {
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
  bool bRet = false;
  switch (pObj->GetType()) {
    case CPDF_PageObject::TEXT:
      bRet = ProcessText(pObj->AsText(), mtObj2Device, nullptr);
      break;
    case CPDF_PageObject::PATH:
      bRet = ProcessPath(pObj->AsPath(), mtObj2Device);
      break;
    case CPDF_PageObject::IMAGE:
      bRet = ProcessImage(pObj->AsImage(), mtObj2Device);
      break;
    case CPDF_PageObject::SHADING:
      ProcessShading(pObj->AsShading(), mtObj2Device);
      return;
    case CPDF_PageObject::FORM:
      bRet = ProcessForm(pObj->AsForm(), mtObj2Device);
      break;
  }
  if (!bRet)
    DrawObjWithBackground(pObj, mtObj2Device);
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
}

bool CPDF_RenderStatus::DrawObjWithBlend(CPDF_PageObject* pObj,
                                         const CFX_Matrix& mtObj2Device) {
  switch (pObj->GetType()) {
    case CPDF_PageObject::PATH:
      return ProcessPath(pObj->AsPath(), mtObj2Device);
    case CPDF_PageObject::IMAGE:
      return ProcessImage(pObj->AsImage(), mtObj2Device);
    case CPDF_PageObject::FORM:
      return ProcessForm(pObj->AsForm(), mtObj2Device);
    default:
      return false;
  }
}

void CPDF_RenderStatus::DrawObjWithBackground(CPDF_PageObject* pObj,
                                              const CFX_Matrix& mtObj2Device) {
  FX_RECT rect = GetObjectClippedRect(pObj, mtObj2Device);
  if (rect.IsEmpty())
    return;

  int res = 300;
  if (pObj->IsImage() && m_pDevice->GetDeviceType() == DeviceType::kPrinter)
    res = 0;

  CPDF_ScaledRenderBuffer buffer;
  if (!buffer.Initialize(m_pContext.Get(), m_pDevice, rect, pObj, &m_Options,
                         res)) {
    return;
  }
  CFX_Matrix matrix = mtObj2Device * buffer.GetMatrix();
  const CPDF_Dictionary* pFormResource = nullptr;
  const CPDF_FormObject* pFormObj = pObj->AsForm();
  if (pFormObj)
    pFormResource = pFormObj->form()->GetDict()->GetDictFor("Resources");
  CPDF_RenderStatus status(m_pContext.Get(), buffer.GetDevice());
  status.SetOptions(m_Options);
  status.SetDeviceMatrix(buffer.GetMatrix());
  status.SetTransparency(m_Transparency);
  status.SetDropObjects(m_bDropObjects);
  status.SetFormResource(pFormResource);
  status.Initialize(nullptr, nullptr);
  status.RenderSingleObject(pObj, matrix);
  buffer.OutputToDevice();
}

bool CPDF_RenderStatus::ProcessForm(const CPDF_FormObject* pFormObj,
                                    const CFX_Matrix& mtObj2Device) {
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
  const CPDF_Dictionary* pOC = pFormObj->form()->GetDict()->GetDictFor("OC");
  if (pOC && m_Options.GetOCContext() &&
      !m_Options.GetOCContext()->CheckOCGVisible(pOC)) {
    return true;
  }
  CFX_Matrix matrix = pFormObj->form_matrix() * mtObj2Device;
  const CPDF_Dictionary* pResources =
      pFormObj->form()->GetDict()->GetDictFor("Resources");
  CPDF_RenderStatus status(m_pContext.Get(), m_pDevice);
  status.SetOptions(m_Options);
  status.SetStopObject(m_pStopObj.Get());
  status.SetTransparency(m_Transparency);
  status.SetDropObjects(m_bDropObjects);
  status.SetFormResource(pResources);
  status.Initialize(this, pFormObj);
  status.m_curBlend = m_curBlend;
  {
    CFX_RenderDevice::StateRestorer restorer(m_pDevice);
    status.RenderObjectList(pFormObj->form(), matrix);
    m_bStopped = status.m_bStopped;
  }
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
  return true;
}

bool CPDF_RenderStatus::ProcessPath(CPDF_PathObject* pPathObj,
                                    const CFX_Matrix& mtObj2Device) {
  int FillType = pPathObj->filltype();
  bool bStroke = pPathObj->stroke();
  ProcessPathPattern(pPathObj, mtObj2Device, &FillType, &bStroke);
  if (FillType == 0 && !bStroke)
    return true;

  uint32_t fill_argb = FillType ? GetFillArgb(pPathObj) : 0;
  uint32_t stroke_argb = bStroke ? GetStrokeArgb(pPathObj) : 0;
  CFX_Matrix path_matrix = pPathObj->matrix() * mtObj2Device;
  if (!IsAvailableMatrix(path_matrix))
    return true;

  if (FillType && m_Options.GetOptions().bRectAA)
    FillType |= FXFILL_RECT_AA;
  if (m_Options.GetOptions().bFillFullcover)
    FillType |= FXFILL_FULLCOVER;
  if (m_Options.GetOptions().bNoPathSmooth)
    FillType |= FXFILL_NOPATHSMOOTH;
  if (bStroke)
    FillType |= FX_FILL_STROKE;

  const CPDF_PageObject* pPageObj =
      static_cast<const CPDF_PageObject*>(pPathObj);
  if (pPageObj->m_GeneralState.GetStrokeAdjust())
    FillType |= FX_STROKE_ADJUST;
  if (m_pType3Char)
    FillType |= FX_FILL_TEXT_MODE;

  CFX_GraphState graphState = pPathObj->m_GraphState;
  if (m_Options.GetOptions().bThinLine)
    graphState.SetLineWidth(0);
  return m_pDevice->DrawPathWithBlend(
      pPathObj->path().GetObject(), &path_matrix, graphState.GetObject(),
      fill_argb, stroke_argb, FillType, m_curBlend);
}

RetainPtr<CPDF_TransferFunc> CPDF_RenderStatus::GetTransferFunc(
    const CPDF_Object* pObj) const {
  ASSERT(pObj);
  auto* pDocCache = CPDF_DocRenderData::FromDocument(m_pContext->GetDocument());
  return pDocCache ? pDocCache->GetTransferFunc(pObj) : nullptr;
}

FX_ARGB CPDF_RenderStatus::GetFillArgbInternal(CPDF_PageObject* pObj,
                                               bool bType3) const {
  const CPDF_ColorState* pColorState = &pObj->m_ColorState;
  if (!bType3 && Type3CharMissingFillColor(m_pType3Char.Get(), pColorState))
    return m_T3FillColor;

  if (MissingFillColor(pColorState))
    pColorState = &m_InitialStates.m_ColorState;

  FX_COLORREF colorref = pColorState->GetFillColorRef();
  if (colorref == 0xFFFFFFFF)
    return 0;

  int32_t alpha =
      static_cast<int32_t>((pObj->m_GeneralState.GetFillAlpha() * 255));
  if (pObj->m_GeneralState.GetTR()) {
    if (!pObj->m_GeneralState.GetTransferFunc()) {
      pObj->m_GeneralState.SetTransferFunc(
          GetTransferFunc(pObj->m_GeneralState.GetTR()));
    }
    if (pObj->m_GeneralState.GetTransferFunc()) {
      colorref =
          pObj->m_GeneralState.GetTransferFunc()->TranslateColor(colorref);
    }
  }
  return m_Options.TranslateColor(AlphaAndColorRefToArgb(alpha, colorref));
}

FX_ARGB CPDF_RenderStatus::GetStrokeArgb(CPDF_PageObject* pObj) const {
  const CPDF_ColorState* pColorState = &pObj->m_ColorState;
  if (Type3CharMissingStrokeColor(m_pType3Char.Get(), pColorState))
    return m_T3FillColor;

  if (MissingStrokeColor(pColorState))
    pColorState = &m_InitialStates.m_ColorState;

  FX_COLORREF colorref = pColorState->GetStrokeColorRef();
  if (colorref == 0xFFFFFFFF)
    return 0;

  int32_t alpha = static_cast<int32_t>(pObj->m_GeneralState.GetStrokeAlpha() *
                                       255);  // not rounded.
  if (pObj->m_GeneralState.GetTR()) {
    if (!pObj->m_GeneralState.GetTransferFunc()) {
      pObj->m_GeneralState.SetTransferFunc(
          GetTransferFunc(pObj->m_GeneralState.GetTR()));
    }
    if (pObj->m_GeneralState.GetTransferFunc()) {
      colorref =
          pObj->m_GeneralState.GetTransferFunc()->TranslateColor(colorref);
    }
  }
  return m_Options.TranslateColor(AlphaAndColorRefToArgb(alpha, colorref));
}

void CPDF_RenderStatus::ProcessClipPath(const CPDF_ClipPath& ClipPath,
                                        const CFX_Matrix& mtObj2Device) {
  if (!ClipPath.HasRef()) {
    if (m_LastClipPath.HasRef()) {
      m_pDevice->RestoreState(true);
      m_LastClipPath.SetNull();
    }
    return;
  }
  if (m_LastClipPath == ClipPath)
    return;

  m_LastClipPath = ClipPath;
  m_pDevice->RestoreState(true);
  for (size_t i = 0; i < ClipPath.GetPathCount(); ++i) {
    const CFX_PathData* pPathData = ClipPath.GetPath(i).GetObject();
    if (!pPathData)
      continue;

    if (pPathData->GetPoints().empty()) {
      CFX_PathData EmptyPath;
      EmptyPath.AppendRect(-1, -1, 0, 0);
      m_pDevice->SetClip_PathFill(&EmptyPath, nullptr, FXFILL_WINDING);
    } else {
      m_pDevice->SetClip_PathFill(pPathData, &mtObj2Device,
                                  ClipPath.GetClipType(i));
    }
  }

  if (ClipPath.GetTextCount() == 0)
    return;

  if (m_pDevice->GetDeviceType() == DeviceType::kDisplay &&
      !(m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SOFT_CLIP)) {
    return;
  }

  std::unique_ptr<CFX_PathData> pTextClippingPath;
  for (size_t i = 0; i < ClipPath.GetTextCount(); ++i) {
    CPDF_TextObject* pText = ClipPath.GetText(i);
    if (pText) {
      if (!pTextClippingPath)
        pTextClippingPath = pdfium::MakeUnique<CFX_PathData>();
      ProcessText(pText, mtObj2Device, pTextClippingPath.get());
      continue;
    }

    if (!pTextClippingPath)
      continue;

    int fill_mode = FXFILL_WINDING;
    if (m_Options.GetOptions().bNoTextSmooth)
      fill_mode |= FXFILL_NOPATHSMOOTH;
    m_pDevice->SetClip_PathFill(pTextClippingPath.get(), nullptr, fill_mode);
    pTextClippingPath.reset();
  }
}

bool CPDF_RenderStatus::ClipPattern(const CPDF_PageObject* pPageObj,
                                    const CFX_Matrix& mtObj2Device,
                                    bool bStroke) {
  if (pPageObj->IsPath())
    return SelectClipPath(pPageObj->AsPath(), mtObj2Device, bStroke);
  if (pPageObj->IsImage()) {
    m_pDevice->SetClip_Rect(pPageObj->GetTransformedBBox(mtObj2Device));
    return true;
  }
  return false;
}

bool CPDF_RenderStatus::SelectClipPath(const CPDF_PathObject* pPathObj,
                                       const CFX_Matrix& mtObj2Device,
                                       bool bStroke) {
  CFX_Matrix path_matrix = pPathObj->matrix() * mtObj2Device;
  if (bStroke) {
    CFX_GraphState graphState = pPathObj->m_GraphState;
    if (m_Options.GetOptions().bThinLine)
      graphState.SetLineWidth(0);
    return m_pDevice->SetClip_PathStroke(pPathObj->path().GetObject(),
                                         &path_matrix, graphState.GetObject());
  }
  int fill_mode = pPathObj->filltype();
  if (m_Options.GetOptions().bNoPathSmooth) {
    fill_mode |= FXFILL_NOPATHSMOOTH;
  }
  return m_pDevice->SetClip_PathFill(pPathObj->path().GetObject(), &path_matrix,
                                     fill_mode);
}

bool CPDF_RenderStatus::ProcessTransparency(CPDF_PageObject* pPageObj,
                                            const CFX_Matrix& mtObj2Device) {
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
  BlendMode blend_type = pPageObj->m_GeneralState.GetBlendType();
  CPDF_Dictionary* pSMaskDict =
      ToDictionary(pPageObj->m_GeneralState.GetSoftMask());
  if (pSMaskDict) {
    if (pPageObj->IsImage() &&
        pPageObj->AsImage()->GetImage()->GetDict()->KeyExist("SMask")) {
      pSMaskDict = nullptr;
    }
  }
  const CPDF_Dictionary* pFormResource = nullptr;
  float group_alpha = 1.0f;
  CPDF_Transparency transparency = m_Transparency;
  bool bGroupTransparent = false;
  const CPDF_FormObject* pFormObj = pPageObj->AsForm();
  if (pFormObj) {
    group_alpha = pFormObj->m_GeneralState.GetFillAlpha();
    transparency = pFormObj->form()->GetTransparency();
    bGroupTransparent = transparency.IsIsolated();
    pFormResource = pFormObj->form()->GetDict()->GetDictFor("Resources");
  }
  bool bTextClip =
      (pPageObj->m_ClipPath.HasRef() &&
       pPageObj->m_ClipPath.GetTextCount() > 0 &&
       m_pDevice->GetDeviceType() == DeviceType::kDisplay &&
       !(m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SOFT_CLIP));
  if (m_Options.GetOptions().bOverprint && pPageObj->IsImage() &&
      pPageObj->m_GeneralState.GetFillOP() &&
      pPageObj->m_GeneralState.GetStrokeOP()) {
    CPDF_Document* pDocument = nullptr;
    CPDF_Page* pPage = nullptr;
    if (m_pContext->GetPageCache()) {
      pPage = m_pContext->GetPageCache()->GetPage();
      pDocument = pPage->GetDocument();
    } else {
      pDocument = pPageObj->AsImage()->GetImage()->GetDocument();
    }
    const CPDF_Dictionary* pPageResources =
        pPage ? pPage->m_pPageResources.Get() : nullptr;
    auto* pImageStream = pPageObj->AsImage()->GetImage()->GetStream();
    const CPDF_Object* pCSObj =
        pImageStream->GetDict()->GetDirectObjectFor("ColorSpace");
    RetainPtr<CPDF_ColorSpace> pColorSpace =
        CPDF_DocPageData::FromDocument(pDocument)->GetColorSpace(
            pCSObj, pPageResources);
    if (pColorSpace) {
      int format = pColorSpace->GetFamily();
      if (format == PDFCS_DEVICECMYK || format == PDFCS_SEPARATION ||
          format == PDFCS_DEVICEN) {
        blend_type = BlendMode::kDarken;
      }
    }
  }
  if (!pSMaskDict && group_alpha == 1.0f && blend_type == BlendMode::kNormal &&
      !bTextClip && !bGroupTransparent) {
    return false;
  }
  if (m_bPrint) {
    bool bRet = false;
    int rendCaps = m_pDevice->GetRenderCaps();
    if (!(transparency.IsIsolated() || pSMaskDict || bTextClip) &&
        (rendCaps & FXRC_BLEND_MODE)) {
      BlendMode oldBlend = m_curBlend;
      m_curBlend = blend_type;
      bRet = DrawObjWithBlend(pPageObj, mtObj2Device);
      m_curBlend = oldBlend;
    }
    if (!bRet) {
      DrawObjWithBackground(pPageObj, mtObj2Device);
    }
    return true;
  }
  FX_RECT rect = pPageObj->GetTransformedBBox(mtObj2Device);
  rect.Intersect(m_pDevice->GetClipBox());
  if (rect.IsEmpty())
    return true;

  int width = rect.Width();
  int height = rect.Height();
  CFX_DefaultRenderDevice bitmap_device;
  RetainPtr<CFX_DIBitmap> backdrop;
  if (!transparency.IsIsolated() &&
      (m_pDevice->GetRenderCaps() & FXRC_GET_BITS)) {
    backdrop = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!m_pDevice->CreateCompatibleBitmap(backdrop, width, height))
      return true;
    m_pDevice->GetDIBits(backdrop, rect.left, rect.top);
  }
  if (!bitmap_device.Create(width, height, FXDIB_Argb, backdrop))
    return true;

  RetainPtr<CFX_DIBitmap> bitmap = bitmap_device.GetBitmap();
  bitmap->Clear(0);

  CFX_Matrix new_matrix = mtObj2Device;
  new_matrix.Translate(-rect.left, -rect.top);

  RetainPtr<CFX_DIBitmap> pTextMask;
  if (bTextClip) {
    pTextMask = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!pTextMask->Create(width, height, FXDIB_8bppMask))
      return true;

    pTextMask->Clear(0);
    CFX_DefaultRenderDevice text_device;
    text_device.Attach(pTextMask, false, nullptr, false);
    for (size_t i = 0; i < pPageObj->m_ClipPath.GetTextCount(); ++i) {
      CPDF_TextObject* textobj = pPageObj->m_ClipPath.GetText(i);
      if (!textobj)
        break;

      // TODO(thestig): Should we check the return value here?
      CPDF_TextRenderer::DrawTextPath(
          &text_device, textobj->GetCharCodes(), textobj->GetCharPositions(),
          textobj->m_TextState.GetFont().Get(),
          textobj->m_TextState.GetFontSize(), textobj->GetTextMatrix(),
          &new_matrix, textobj->m_GraphState.GetObject(), 0xffffffff, 0,
          nullptr, 0);
    }
  }
  CPDF_RenderStatus bitmap_render(m_pContext.Get(), &bitmap_device);
  bitmap_render.SetOptions(m_Options);
  bitmap_render.SetStopObject(m_pStopObj.Get());
  bitmap_render.SetStdCS(true);
  bitmap_render.SetDropObjects(m_bDropObjects);
  bitmap_render.SetFormResource(pFormResource);
  bitmap_render.Initialize(nullptr, nullptr);
  bitmap_render.ProcessObjectNoClip(pPageObj, new_matrix);
#if defined _SKIA_SUPPORT_PATHS_
  bitmap_device.Flush(true);
  bitmap->UnPreMultiply();
#endif
  m_bStopped = bitmap_render.m_bStopped;
  if (pSMaskDict) {
    CFX_Matrix smask_matrix =
        *pPageObj->m_GeneralState.GetSMaskMatrix() * mtObj2Device;
    RetainPtr<CFX_DIBBase> pSMaskSource =
        LoadSMask(pSMaskDict, &rect, &smask_matrix);
    if (pSMaskSource)
      bitmap->MultiplyAlpha(pSMaskSource);
  }
  if (pTextMask) {
    bitmap->MultiplyAlpha(pTextMask);
    pTextMask.Reset();
  }
  int32_t blitAlpha = 255;
  if (group_alpha != 1.0f && transparency.IsGroup()) {
    blitAlpha = (int32_t)(group_alpha * 255);
#ifndef _SKIA_SUPPORT_
    bitmap->MultiplyAlpha(blitAlpha);
    blitAlpha = 255;
#endif
  }
  transparency = m_Transparency;
  if (pPageObj->IsForm()) {
    transparency.SetGroup();
  }
  CompositeDIBitmap(bitmap, rect.left, rect.top, 0, blitAlpha, blend_type,
                    transparency);
#if defined _SKIA_SUPPORT_
  DebugVerifyDeviceIsPreMultiplied();
#endif
  return true;
}

RetainPtr<CFX_DIBitmap> CPDF_RenderStatus::GetBackdrop(
    const CPDF_PageObject* pObj,
    const FX_RECT& rect,
    bool bBackAlphaRequired,
    int* left,
    int* top) {
  FX_RECT bbox = rect;
  bbox.Intersect(m_pDevice->GetClipBox());
  *left = bbox.left;
  *top = bbox.top;
  int width = bbox.Width();
  int height = bbox.Height();
  auto pBackdrop = pdfium::MakeRetain<CFX_DIBitmap>();
  if (bBackAlphaRequired && !m_bDropObjects)
    pBackdrop->Create(width, height, FXDIB_Argb);
  else
    m_pDevice->CreateCompatibleBitmap(pBackdrop, width, height);

  if (!pBackdrop->GetBuffer())
    return nullptr;

  bool bNeedDraw;
  if (pBackdrop->HasAlpha())
    bNeedDraw = !(m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT);
  else
    bNeedDraw = !(m_pDevice->GetRenderCaps() & FXRC_GET_BITS);

  if (!bNeedDraw) {
    m_pDevice->GetDIBits(pBackdrop, *left, *top);
    return pBackdrop;
  }
  CFX_Matrix FinalMatrix = m_DeviceMatrix;
  FinalMatrix.Translate(-*left, -*top);
  pBackdrop->Clear(pBackdrop->HasAlpha() ? 0 : 0xffffffff);

  CFX_DefaultRenderDevice device;
  device.Attach(pBackdrop, false, nullptr, false);
  m_pContext->Render(&device, pObj, &m_Options, &FinalMatrix);
  return pBackdrop;
}

std::unique_ptr<CPDF_GraphicStates> CPDF_RenderStatus::CloneObjStates(
    const CPDF_GraphicStates* pSrcStates,
    bool bStroke) {
  if (!pSrcStates)
    return nullptr;

  auto pStates = pdfium::MakeUnique<CPDF_GraphicStates>();
  pStates->CopyStates(*pSrcStates);
  const CPDF_Color* pObjColor = bStroke
                                    ? pSrcStates->m_ColorState.GetStrokeColor()
                                    : pSrcStates->m_ColorState.GetFillColor();
  if (!pObjColor->IsNull()) {
    pStates->m_ColorState.SetFillColorRef(
        bStroke ? pSrcStates->m_ColorState.GetStrokeColorRef()
                : pSrcStates->m_ColorState.GetFillColorRef());
    pStates->m_ColorState.SetStrokeColorRef(
        pStates->m_ColorState.GetFillColorRef());
  }
  return pStates;
}

#if defined _SKIA_SUPPORT_
void CPDF_RenderStatus::DebugVerifyDeviceIsPreMultiplied() const {
  m_pDevice->DebugVerifyBitmapIsPreMultiplied();
}
#endif

bool CPDF_RenderStatus::ProcessText(CPDF_TextObject* textobj,
                                    const CFX_Matrix& mtObj2Device,
                                    CFX_PathData* pClippingPath) {
  if (textobj->GetCharCodes().empty())
    return true;

  const TextRenderingMode text_render_mode = textobj->m_TextState.GetTextMode();
  if (text_render_mode == TextRenderingMode::MODE_INVISIBLE)
    return true;

  RetainPtr<CPDF_Font> pFont = textobj->m_TextState.GetFont();
  if (pFont->IsType3Font())
    return ProcessType3Text(textobj, mtObj2Device);

  bool bFill = false;
  bool bStroke = false;
  bool bClip = false;
  if (pClippingPath) {
    bClip = true;
  } else {
    switch (text_render_mode) {
      case TextRenderingMode::MODE_FILL:
      case TextRenderingMode::MODE_FILL_CLIP:
        bFill = true;
        break;
      case TextRenderingMode::MODE_STROKE:
      case TextRenderingMode::MODE_STROKE_CLIP:
        if (pFont->HasFace())
          bStroke = true;
        else
          bFill = true;
        break;
      case TextRenderingMode::MODE_FILL_STROKE:
      case TextRenderingMode::MODE_FILL_STROKE_CLIP:
        bFill = true;
        if (pFont->HasFace())
          bStroke = true;
        break;
      case TextRenderingMode::MODE_INVISIBLE:
        // Already handled above, but the compiler is not smart enough to
        // realize it.
        NOTREACHED();
        return true;
      case TextRenderingMode::MODE_CLIP:
        return true;
      case TextRenderingMode::MODE_UNKNOWN:
        NOTREACHED();
        return false;
    }
  }
  FX_ARGB stroke_argb = 0;
  FX_ARGB fill_argb = 0;
  bool bPattern = false;
  if (bStroke) {
    if (textobj->m_ColorState.GetStrokeColor()->IsPattern()) {
      bPattern = true;
    } else {
      stroke_argb = GetStrokeArgb(textobj);
    }
  }
  if (bFill) {
    if (textobj->m_ColorState.GetFillColor()->IsPattern()) {
      bPattern = true;
    } else {
      fill_argb = GetFillArgb(textobj);
    }
  }
  CFX_Matrix text_matrix = textobj->GetTextMatrix();
  if (!IsAvailableMatrix(text_matrix))
    return true;

  float font_size = textobj->m_TextState.GetFontSize();
  if (bPattern) {
    DrawTextPathWithPattern(textobj, mtObj2Device, pFont.Get(), font_size,
                            &text_matrix, bFill, bStroke);
    return true;
  }
  if (bClip || bStroke) {
    const CFX_Matrix* pDeviceMatrix = &mtObj2Device;
    CFX_Matrix device_matrix;
    if (bStroke) {
      const float* pCTM = textobj->m_TextState.GetCTM();
      if (pCTM[0] != 1.0f || pCTM[3] != 1.0f) {
        CFX_Matrix ctm(pCTM[0], pCTM[1], pCTM[2], pCTM[3], 0, 0);
        text_matrix *= ctm.GetInverse();
        device_matrix = ctm * mtObj2Device;
        pDeviceMatrix = &device_matrix;
      }
    }
    int flag = 0;
    if (bStroke && bFill) {
      flag |= FX_FILL_STROKE;
      flag |= FX_STROKE_TEXT_MODE;
    }
    if (textobj->m_GeneralState.GetStrokeAdjust())
      flag |= FX_STROKE_ADJUST;
    if (m_Options.GetOptions().bNoTextSmooth)
      flag |= FXFILL_NOPATHSMOOTH;
    return CPDF_TextRenderer::DrawTextPath(
        m_pDevice, textobj->GetCharCodes(), textobj->GetCharPositions(),
        pFont.Get(), font_size, text_matrix, pDeviceMatrix,
        textobj->m_GraphState.GetObject(), fill_argb, stroke_argb,
        pClippingPath, flag);
  }
  text_matrix.Concat(mtObj2Device);
  return CPDF_TextRenderer::DrawNormalText(
      m_pDevice, textobj->GetCharCodes(), textobj->GetCharPositions(),
      pFont.Get(), font_size, text_matrix, fill_argb, m_Options);
}

// TODO(npm): Font fallback for type 3 fonts? (Completely separate code!!)
bool CPDF_RenderStatus::ProcessType3Text(CPDF_TextObject* textobj,
                                         const CFX_Matrix& mtObj2Device) {
  CPDF_Type3Font* pType3Font = textobj->m_TextState.GetFont()->AsType3Font();
  if (pdfium::ContainsValue(m_Type3FontCache, pType3Font))
    return true;

  DeviceType device_type = m_pDevice->GetDeviceType();
  FX_ARGB fill_argb = GetFillArgbForType3(textobj);
  int fill_alpha = FXARGB_A(fill_argb);
  if (device_type != DeviceType::kDisplay && fill_alpha < 255)
    return false;

  CFX_Matrix text_matrix = textobj->GetTextMatrix();
  CFX_Matrix char_matrix = pType3Font->GetFontMatrix();
  float font_size = textobj->m_TextState.GetFontSize();
  char_matrix.Scale(font_size, font_size);

  // Must come before |glyphs|, because |glyphs| points into |refTypeCache|.
  std::set<RetainPtr<CPDF_Type3Cache>> refTypeCache;
  std::vector<TextGlyphPos> glyphs;
  if (device_type == DeviceType::kDisplay)
    glyphs.resize(textobj->GetCharCodes().size());

  for (size_t iChar = 0; iChar < textobj->GetCharCodes().size(); ++iChar) {
    uint32_t charcode = textobj->GetCharCodes()[iChar];
    if (charcode == static_cast<uint32_t>(-1))
      continue;

    CPDF_Type3Char* pType3Char = pType3Font->LoadChar(charcode);
    if (!pType3Char)
      continue;

    CFX_Matrix matrix = char_matrix;
    matrix.e += iChar > 0 ? textobj->GetCharPositions()[iChar - 1] : 0;
    matrix.Concat(text_matrix);
    matrix.Concat(mtObj2Device);
    if (!pType3Char->LoadBitmapFromSoleImageOfForm()) {
      if (!glyphs.empty()) {
        for (size_t i = 0; i < iChar; ++i) {
          const TextGlyphPos& glyph = glyphs[i];
          if (!glyph.m_pGlyph)
            continue;

          Optional<CFX_Point> point = glyph.GetOrigin({0, 0});
          if (!point.has_value())
            continue;

          m_pDevice->SetBitMask(glyph.m_pGlyph->GetBitmap(), point->x, point->y,
                                fill_argb);
        }
        glyphs.clear();
      }

      std::unique_ptr<CPDF_GraphicStates> pStates =
          CloneObjStates(textobj, false);
      CPDF_RenderOptions options = m_Options;
      options.GetOptions().bForceHalftone = true;
      options.GetOptions().bRectAA = true;

      const auto* pForm = static_cast<const CPDF_Form*>(pType3Char->form());
      const CPDF_Dictionary* pFormResource =
          pForm->GetDict()->GetDictFor("Resources");

      if (fill_alpha == 255) {
        CPDF_RenderStatus status(m_pContext.Get(), m_pDevice);
        status.SetOptions(options);
        status.SetTransparency(pForm->GetTransparency());
        status.SetType3Char(pType3Char);
        status.SetFillColor(fill_argb);
        status.SetDropObjects(m_bDropObjects);
        status.SetFormResource(pFormResource);
        status.Initialize(this, pStates.get());
        status.m_Type3FontCache = m_Type3FontCache;
        status.m_Type3FontCache.push_back(pType3Font);

        CFX_RenderDevice::StateRestorer restorer(m_pDevice);
        status.RenderObjectList(pForm, matrix);
      } else {
        FX_RECT rect =
            matrix.TransformRect(pForm->CalcBoundingBox()).GetOuterRect();
        if (!rect.Valid())
          continue;

        CFX_DefaultRenderDevice bitmap_device;
        if (!bitmap_device.Create(rect.Width(), rect.Height(), FXDIB_Argb,
                                  nullptr)) {
          return true;
        }
        bitmap_device.GetBitmap()->Clear(0);
        CPDF_RenderStatus status(m_pContext.Get(), &bitmap_device);
        status.SetOptions(options);
        status.SetTransparency(pForm->GetTransparency());
        status.SetType3Char(pType3Char);
        status.SetFillColor(fill_argb);
        status.SetDropObjects(m_bDropObjects);
        status.SetFormResource(pFormResource);
        status.Initialize(this, pStates.get());
        status.m_Type3FontCache = m_Type3FontCache;
        status.m_Type3FontCache.push_back(pType3Font);
        matrix.Translate(-rect.left, -rect.top);
        status.RenderObjectList(pForm, matrix);
        m_pDevice->SetDIBits(bitmap_device.GetBitmap(), rect.left, rect.top);
      }
    } else if (pType3Char->GetBitmap()) {
      if (device_type == DeviceType::kDisplay) {
        CPDF_Document* pDoc = pType3Font->GetDocument();
        RetainPtr<CPDF_Type3Cache> pCache =
            CPDF_DocRenderData::FromDocument(pDoc)->GetCachedType3(pType3Font);

        const CFX_GlyphBitmap* pBitmap = pCache->LoadGlyph(charcode, &matrix);
        if (!pBitmap)
          continue;

        refTypeCache.insert(std::move(pCache));

        CFX_Point origin(FXSYS_roundf(matrix.e), FXSYS_roundf(matrix.f));
        if (glyphs.empty()) {
          FX_SAFE_INT32 left = origin.x;
          left += pBitmap->left();
          if (!left.IsValid())
            continue;

          FX_SAFE_INT32 top = origin.y;
          top -= pBitmap->top();
          if (!top.IsValid())
            continue;

          m_pDevice->SetBitMask(pBitmap->GetBitmap(), left.ValueOrDie(),
                                top.ValueOrDie(), fill_argb);
        } else {
          glyphs[iChar].m_pGlyph = pBitmap;
          glyphs[iChar].m_Origin = origin;
        }
      } else {
        CFX_Matrix image_matrix = pType3Char->matrix() * matrix;
        CPDF_ImageRenderer renderer;
        if (renderer.Start(this, pType3Char->GetBitmap(), fill_argb, 255,
                           image_matrix, FXDIB_ResampleOptions(), false,
                           BlendMode::kNormal)) {
          renderer.Continue(nullptr);
        }
        if (!renderer.GetResult())
          return false;
      }
    }
  }

  if (glyphs.empty())
    return true;

  FX_RECT rect = GetGlyphsBBox(glyphs, 0);
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pBitmap->Create(rect.Width(), rect.Height(), FXDIB_8bppMask))
    return true;

  pBitmap->Clear(0);
  for (const TextGlyphPos& glyph : glyphs) {
    if (!glyph.m_pGlyph)
      continue;

    Optional<CFX_Point> point = glyph.GetOrigin({rect.left, rect.top});
    if (!point.has_value())
      continue;

    pBitmap->CompositeMask(
        point->x, point->y, glyph.m_pGlyph->GetBitmap()->GetWidth(),
        glyph.m_pGlyph->GetBitmap()->GetHeight(), glyph.m_pGlyph->GetBitmap(),
        fill_argb, 0, 0, BlendMode::kNormal, nullptr, false);
  }
  m_pDevice->SetBitMask(pBitmap, rect.left, rect.top, fill_argb);
  return true;
}

void CPDF_RenderStatus::DrawTextPathWithPattern(const CPDF_TextObject* textobj,
                                                const CFX_Matrix& mtObj2Device,
                                                CPDF_Font* pFont,
                                                float font_size,
                                                const CFX_Matrix* pTextMatrix,
                                                bool bFill,
                                                bool bStroke) {
  if (!bStroke) {
    std::vector<std::unique_ptr<CPDF_TextObject>> pCopy;
    pCopy.push_back(std::unique_ptr<CPDF_TextObject>(textobj->Clone()));

    CPDF_PathObject path;
    path.set_filltype(FXFILL_WINDING);
    path.m_ClipPath.CopyClipPath(m_LastClipPath);
    path.m_ClipPath.AppendTexts(&pCopy);
    path.m_ColorState = textobj->m_ColorState;
    path.m_GeneralState = textobj->m_GeneralState;
    path.path().AppendFloatRect(textobj->GetRect());
    path.SetRect(textobj->GetRect());

    AutoRestorer<UnownedPtr<const CPDF_PageObject>> restorer2(&m_pCurObj);
    RenderSingleObject(&path, mtObj2Device);
    return;
  }
  const CPDF_CharPosList CharPosList(
      textobj->GetCharCodes(), textobj->GetCharPositions(), pFont, font_size);
  for (const TextCharPos& charpos : CharPosList.Get()) {
    auto* font = charpos.m_FallbackFontPosition == -1
                     ? pFont->GetFont()
                     : pFont->GetFontFallback(charpos.m_FallbackFontPosition);
    const CFX_PathData* pPath =
        font->LoadGlyphPath(charpos.m_GlyphIndex, charpos.m_FontCharWidth);
    if (!pPath)
      continue;

    CPDF_PathObject path;
    path.m_GraphState = textobj->m_GraphState;
    path.m_ColorState = textobj->m_ColorState;

    CFX_Matrix matrix;
    if (charpos.m_bGlyphAdjust) {
      matrix = CFX_Matrix(charpos.m_AdjustMatrix[0], charpos.m_AdjustMatrix[1],
                          charpos.m_AdjustMatrix[2], charpos.m_AdjustMatrix[3],
                          0, 0);
    }
    matrix.Concat(CFX_Matrix(font_size, 0, 0, font_size, charpos.m_Origin.x,
                             charpos.m_Origin.y));
    path.set_stroke(bStroke);
    path.set_filltype(bFill ? FXFILL_WINDING : 0);
    path.path().Append(pPath, &matrix);
    path.set_matrix(*pTextMatrix);
    path.CalcBoundingBox();
    ProcessPath(&path, mtObj2Device);
  }
}

void CPDF_RenderStatus::DrawShadingPattern(CPDF_ShadingPattern* pattern,
                                           const CPDF_PageObject* pPageObj,
                                           const CFX_Matrix& mtObj2Device,
                                           bool bStroke) {
  if (!pattern->Load())
    return;

  CFX_RenderDevice::StateRestorer restorer(m_pDevice);
  if (!ClipPattern(pPageObj, mtObj2Device, bStroke))
    return;

  FX_RECT rect = GetObjectClippedRect(pPageObj, mtObj2Device);
  if (rect.IsEmpty())
    return;

  CFX_Matrix matrix = pattern->pattern_to_form() * mtObj2Device;
  int alpha =
      FXSYS_roundf(255 * (bStroke ? pPageObj->m_GeneralState.GetStrokeAlpha()
                                  : pPageObj->m_GeneralState.GetFillAlpha()));
  CPDF_RenderShading::Draw(m_pDevice, m_pContext.Get(), m_pCurObj.Get(),
                           pattern, matrix, rect, alpha, m_Options);
}

void CPDF_RenderStatus::ProcessShading(const CPDF_ShadingObject* pShadingObj,
                                       const CFX_Matrix& mtObj2Device) {
  FX_RECT rect = pShadingObj->GetTransformedBBox(mtObj2Device);
  FX_RECT clip_box = m_pDevice->GetClipBox();
  rect.Intersect(clip_box);
  if (rect.IsEmpty())
    return;

  CFX_Matrix matrix = pShadingObj->matrix() * mtObj2Device;
  CPDF_RenderShading::Draw(
      m_pDevice, m_pContext.Get(), m_pCurObj.Get(), pShadingObj->pattern(),
      matrix, rect,
      FXSYS_roundf(255 * pShadingObj->m_GeneralState.GetFillAlpha()),
      m_Options);
}

void CPDF_RenderStatus::DrawTilingPattern(CPDF_TilingPattern* pPattern,
                                          CPDF_PageObject* pPageObj,
                                          const CFX_Matrix& mtObj2Device,
                                          bool bStroke) {
  const std::unique_ptr<CPDF_Form> pPatternForm = pPattern->Load(pPageObj);
  if (!pPatternForm)
    return;

  CFX_RenderDevice::StateRestorer restorer(m_pDevice);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  ScopedSkiaDeviceFlush scoped_skia_device_flush(m_pDevice);
#endif
  if (!ClipPattern(pPageObj, mtObj2Device, bStroke))
    return;

  FX_RECT clip_box = m_pDevice->GetClipBox();
  if (clip_box.IsEmpty())
    return;

  const CFX_Matrix mtPattern2Device =
      pPattern->pattern_to_form() * mtObj2Device;

  CFX_FloatRect cell_bbox = mtPattern2Device.TransformRect(pPattern->bbox());

  float ceil_height = std::ceil(cell_bbox.Height());
  float ceil_width = std::ceil(cell_bbox.Width());

  // Validate the float will fit into the int when the conversion is done.
  if (!pdfium::base::IsValueInRangeForNumericType<int>(ceil_height) ||
      !pdfium::base::IsValueInRangeForNumericType<int>(ceil_width)) {
    return;
  }

  int width = static_cast<int>(ceil_width);
  int height = static_cast<int>(ceil_height);
  if (width <= 0)
    width = 1;
  if (height <= 0)
    height = 1;

  CFX_FloatRect clip_box_p =
      mtPattern2Device.GetInverse().TransformRect(CFX_FloatRect(clip_box));
  int min_col = static_cast<int>(
      ceil((clip_box_p.left - pPattern->bbox().right) / pPattern->x_step()));
  int max_col = static_cast<int>(
      floor((clip_box_p.right - pPattern->bbox().left) / pPattern->x_step()));
  int min_row = static_cast<int>(
      ceil((clip_box_p.bottom - pPattern->bbox().top) / pPattern->y_step()));
  int max_row = static_cast<int>(
      floor((clip_box_p.top - pPattern->bbox().bottom) / pPattern->y_step()));

  // Make sure we can fit the needed width * height into an int.
  if (height > std::numeric_limits<int>::max() / width)
    return;

  if (width > clip_box.Width() || height > clip_box.Height() ||
      width * height > clip_box.Width() * clip_box.Height()) {
    std::unique_ptr<CPDF_GraphicStates> pStates;
    if (!pPattern->colored())
      pStates = CloneObjStates(pPageObj, bStroke);

    const CPDF_Dictionary* pFormDict = pPatternForm->GetDict();
    const CPDF_Dictionary* pFormResource =
        pFormDict ? pFormDict->GetDictFor("Resources") : nullptr;
    for (int col = min_col; col <= max_col; col++) {
      for (int row = min_row; row <= max_row; row++) {
        CFX_PointF original = mtPattern2Device.Transform(
            CFX_PointF(col * pPattern->x_step(), row * pPattern->y_step()));
        CFX_Matrix matrix = mtObj2Device;
        matrix.Translate(original.x - mtPattern2Device.e,
                         original.y - mtPattern2Device.f);
        CFX_RenderDevice::StateRestorer restorer2(m_pDevice);
        CPDF_RenderStatus status(m_pContext.Get(), m_pDevice);
        status.SetOptions(m_Options);
        status.SetTransparency(pPatternForm->GetTransparency());
        status.SetFormResource(pFormResource);
        status.SetDropObjects(m_bDropObjects);
        status.Initialize(this, pStates.get());
        status.RenderObjectList(pPatternForm.get(), matrix);
      }
    }
    return;
  }

  bool bAligned =
      pPattern->bbox().left == 0 && pPattern->bbox().bottom == 0 &&
      pPattern->bbox().right == pPattern->x_step() &&
      pPattern->bbox().top == pPattern->y_step() &&
      (mtPattern2Device.IsScaled() || mtPattern2Device.Is90Rotated());
  if (bAligned) {
    int orig_x = FXSYS_roundf(mtPattern2Device.e);
    int orig_y = FXSYS_roundf(mtPattern2Device.f);
    min_col = (clip_box.left - orig_x) / width;
    if (clip_box.left < orig_x)
      min_col--;

    max_col = (clip_box.right - orig_x) / width;
    if (clip_box.right <= orig_x)
      max_col--;

    min_row = (clip_box.top - orig_y) / height;
    if (clip_box.top < orig_y)
      min_row--;

    max_row = (clip_box.bottom - orig_y) / height;
    if (clip_box.bottom <= orig_y)
      max_row--;
  }
  float left_offset = cell_bbox.left - mtPattern2Device.e;
  float top_offset = cell_bbox.bottom - mtPattern2Device.f;
  RetainPtr<CFX_DIBitmap> pPatternBitmap;
  if (width * height < 16) {
    RetainPtr<CFX_DIBitmap> pEnlargedBitmap = DrawPatternBitmap(
        m_pContext->GetDocument(), m_pContext->GetPageCache(), pPattern,
        pPatternForm.get(), mtObj2Device, 8, 8, m_Options.GetOptions());
    pPatternBitmap = pEnlargedBitmap->StretchTo(
        width, height, FXDIB_ResampleOptions(), nullptr);
  } else {
    pPatternBitmap =
        DrawPatternBitmap(m_pContext->GetDocument(), m_pContext->GetPageCache(),
                          pPattern, pPatternForm.get(), mtObj2Device, width,
                          height, m_Options.GetOptions());
  }
  if (!pPatternBitmap)
    return;

  if (m_Options.ColorModeIs(CPDF_RenderOptions::kGray))
    pPatternBitmap->ConvertColorScale(0, 0xffffff);

  FX_ARGB fill_argb = GetFillArgb(pPageObj);
  int clip_width = clip_box.right - clip_box.left;
  int clip_height = clip_box.bottom - clip_box.top;
  auto pScreen = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pScreen->Create(clip_width, clip_height, FXDIB_Argb))
    return;

  pScreen->Clear(0);
  const uint8_t* const src_buf = pPatternBitmap->GetBuffer();
  for (int col = min_col; col <= max_col; col++) {
    for (int row = min_row; row <= max_row; row++) {
      int start_x;
      int start_y;
      if (bAligned) {
        start_x =
            FXSYS_roundf(mtPattern2Device.e) + col * width - clip_box.left;
        start_y =
            FXSYS_roundf(mtPattern2Device.f) + row * height - clip_box.top;
      } else {
        CFX_PointF original = mtPattern2Device.Transform(
            CFX_PointF(col * pPattern->x_step(), row * pPattern->y_step()));

        pdfium::base::CheckedNumeric<int> safeStartX =
            FXSYS_roundf(original.x + left_offset);
        pdfium::base::CheckedNumeric<int> safeStartY =
            FXSYS_roundf(original.y + top_offset);

        safeStartX -= clip_box.left;
        safeStartY -= clip_box.top;
        if (!safeStartX.IsValid() || !safeStartY.IsValid())
          return;

        start_x = safeStartX.ValueOrDie();
        start_y = safeStartY.ValueOrDie();
      }
      if (width == 1 && height == 1) {
        if (start_x < 0 || start_x >= clip_box.Width() || start_y < 0 ||
            start_y >= clip_box.Height()) {
          continue;
        }
        uint32_t* dest_buf = reinterpret_cast<uint32_t*>(
            pScreen->GetBuffer() + pScreen->GetPitch() * start_y + start_x * 4);
        if (pPattern->colored()) {
          const auto* src_buf32 = reinterpret_cast<const uint32_t*>(src_buf);
          *dest_buf = *src_buf32;
        } else {
          *dest_buf = (*src_buf << 24) | (fill_argb & 0xffffff);
        }
      } else {
        if (pPattern->colored()) {
          pScreen->CompositeBitmap(start_x, start_y, width, height,
                                   pPatternBitmap, 0, 0, BlendMode::kNormal,
                                   nullptr, false);
        } else {
          pScreen->CompositeMask(start_x, start_y, width, height,
                                 pPatternBitmap, fill_argb, 0, 0,
                                 BlendMode::kNormal, nullptr, false);
        }
      }
    }
  }
  CompositeDIBitmap(pScreen, clip_box.left, clip_box.top, 0, 255,
                    BlendMode::kNormal, CPDF_Transparency());
}

void CPDF_RenderStatus::DrawPathWithPattern(CPDF_PathObject* pPathObj,
                                            const CFX_Matrix& mtObj2Device,
                                            const CPDF_Color* pColor,
                                            bool bStroke) {
  CPDF_Pattern* pattern = pColor->GetPattern();
  if (!pattern)
    return;

  if (CPDF_TilingPattern* pTilingPattern = pattern->AsTilingPattern())
    DrawTilingPattern(pTilingPattern, pPathObj, mtObj2Device, bStroke);
  else if (CPDF_ShadingPattern* pShadingPattern = pattern->AsShadingPattern())
    DrawShadingPattern(pShadingPattern, pPathObj, mtObj2Device, bStroke);
}

void CPDF_RenderStatus::ProcessPathPattern(CPDF_PathObject* pPathObj,
                                           const CFX_Matrix& mtObj2Device,
                                           int* filltype,
                                           bool* bStroke) {
  ASSERT(filltype);
  ASSERT(bStroke);

  if (*filltype) {
    const CPDF_Color& FillColor = *pPathObj->m_ColorState.GetFillColor();
    if (FillColor.IsPattern()) {
      DrawPathWithPattern(pPathObj, mtObj2Device, &FillColor, false);
      *filltype = 0;
    }
  }
  if (*bStroke) {
    const CPDF_Color& StrokeColor = *pPathObj->m_ColorState.GetStrokeColor();
    if (StrokeColor.IsPattern()) {
      DrawPathWithPattern(pPathObj, mtObj2Device, &StrokeColor, true);
      *bStroke = false;
    }
  }
}

bool CPDF_RenderStatus::ProcessImage(CPDF_ImageObject* pImageObj,
                                     const CFX_Matrix& mtObj2Device) {
  CPDF_ImageRenderer render;
  if (render.Start(this, pImageObj, mtObj2Device, m_bStdCS, m_curBlend))
    render.Continue(nullptr);
  return render.GetResult();
}

void CPDF_RenderStatus::CompositeDIBitmap(
    const RetainPtr<CFX_DIBitmap>& pDIBitmap,
    int left,
    int top,
    FX_ARGB mask_argb,
    int bitmap_alpha,
    BlendMode blend_mode,
    const CPDF_Transparency& transparency) {
  if (!pDIBitmap)
    return;

  if (blend_mode == BlendMode::kNormal) {
    if (!pDIBitmap->IsAlphaMask()) {
      if (bitmap_alpha < 255) {
#ifdef _SKIA_SUPPORT_
        std::unique_ptr<CFX_ImageRenderer> dummy;
        CFX_Matrix m = CFX_RenderDevice::GetFlipMatrix(
            pDIBitmap->GetWidth(), pDIBitmap->GetHeight(), left, top);
        m_pDevice->StartDIBits(pDIBitmap, bitmap_alpha, 0, m,
                               FXDIB_ResampleOptions(), &dummy);
        return;
#else
        pDIBitmap->MultiplyAlpha(bitmap_alpha);
#endif
      }
#ifdef _SKIA_SUPPORT_
      CFX_SkiaDeviceDriver::PreMultiply(pDIBitmap);
#endif
      if (m_pDevice->SetDIBits(pDIBitmap, left, top)) {
        return;
      }
    } else {
      uint32_t fill_argb = m_Options.TranslateColor(mask_argb);
      if (bitmap_alpha < 255) {
        uint8_t* fill_argb8 = reinterpret_cast<uint8_t*>(&fill_argb);
        fill_argb8[3] *= bitmap_alpha / 255;
      }
      if (m_pDevice->SetBitMask(pDIBitmap, left, top, fill_argb)) {
        return;
      }
    }
  }
  bool bIsolated = transparency.IsIsolated();
  bool bBackAlphaRequired =
      blend_mode != BlendMode::kNormal && bIsolated && !m_bDropObjects;
  bool bGetBackGround =
      ((m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT)) ||
      (!(m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT) &&
       (m_pDevice->GetRenderCaps() & FXRC_GET_BITS) && !bBackAlphaRequired);
  if (bGetBackGround) {
    if (bIsolated || !transparency.IsGroup()) {
      if (!pDIBitmap->IsAlphaMask())
        m_pDevice->SetDIBitsWithBlend(pDIBitmap, left, top, blend_mode);
      return;
    }

    FX_RECT rect(left, top, left + pDIBitmap->GetWidth(),
                 top + pDIBitmap->GetHeight());
    rect.Intersect(m_pDevice->GetClipBox());
    RetainPtr<CFX_DIBitmap> pClone;
    if (m_pDevice->GetBackDrop() && m_pDevice->GetBitmap()) {
      pClone = m_pDevice->GetBackDrop()->Clone(&rect);
      if (!pClone)
        return;

      RetainPtr<CFX_DIBitmap> pForeBitmap = m_pDevice->GetBitmap();
      pClone->CompositeBitmap(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                              pForeBitmap, rect.left, rect.top,
                              BlendMode::kNormal, nullptr, false);
      left = std::min(left, 0);
      top = std::min(top, 0);
      if (pDIBitmap->IsAlphaMask()) {
        pClone->CompositeMask(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                              pDIBitmap, mask_argb, left, top, blend_mode,
                              nullptr, false);
      } else {
        pClone->CompositeBitmap(0, 0, pClone->GetWidth(), pClone->GetHeight(),
                                pDIBitmap, left, top, blend_mode, nullptr,
                                false);
      }
    } else {
      pClone = pDIBitmap;
    }
    if (m_pDevice->GetBackDrop()) {
      m_pDevice->SetDIBits(pClone, rect.left, rect.top);
    } else {
      if (!pDIBitmap->IsAlphaMask()) {
        m_pDevice->SetDIBitsWithBlend(pDIBitmap, rect.left, rect.top,
                                      blend_mode);
      }
    }
    return;
  }
  int back_left;
  int back_top;
  FX_RECT rect(left, top, left + pDIBitmap->GetWidth(),
               top + pDIBitmap->GetHeight());
  RetainPtr<CFX_DIBitmap> pBackdrop = GetBackdrop(
      m_pCurObj.Get(), rect, blend_mode != BlendMode::kNormal && bIsolated,
      &back_left, &back_top);
  if (!pBackdrop)
    return;

  if (pDIBitmap->IsAlphaMask()) {
    pBackdrop->CompositeMask(left - back_left, top - back_top,
                             pDIBitmap->GetWidth(), pDIBitmap->GetHeight(),
                             pDIBitmap, mask_argb, 0, 0, blend_mode, nullptr,
                             false);
  } else {
    pBackdrop->CompositeBitmap(left - back_left, top - back_top,
                               pDIBitmap->GetWidth(), pDIBitmap->GetHeight(),
                               pDIBitmap, 0, 0, blend_mode, nullptr, false);
  }

  auto pBackdrop1 = pdfium::MakeRetain<CFX_DIBitmap>();
  pBackdrop1->Create(pBackdrop->GetWidth(), pBackdrop->GetHeight(),
                     FXDIB_Rgb32);
  pBackdrop1->Clear((uint32_t)-1);
  pBackdrop1->CompositeBitmap(0, 0, pBackdrop->GetWidth(),
                              pBackdrop->GetHeight(), pBackdrop, 0, 0,
                              BlendMode::kNormal, nullptr, false);
  pBackdrop = std::move(pBackdrop1);
  m_pDevice->SetDIBits(pBackdrop, back_left, back_top);
}

RetainPtr<CFX_DIBitmap> CPDF_RenderStatus::LoadSMask(
    CPDF_Dictionary* pSMaskDict,
    FX_RECT* pClipRect,
    const CFX_Matrix* pMatrix) {
  if (!pSMaskDict)
    return nullptr;

  CPDF_Stream* pGroup = pSMaskDict->GetStreamFor(pdfium::transparency::kG);
  if (!pGroup)
    return nullptr;

  std::unique_ptr<CPDF_Function> pFunc;
  const CPDF_Object* pFuncObj =
      pSMaskDict->GetDirectObjectFor(pdfium::transparency::kTR);
  if (pFuncObj && (pFuncObj->IsDictionary() || pFuncObj->IsStream()))
    pFunc = CPDF_Function::Load(pFuncObj);

  CFX_Matrix matrix = *pMatrix;
  matrix.Translate(-pClipRect->left, -pClipRect->top);

  CPDF_Form form(m_pContext->GetDocument(), m_pContext->GetPageResources(),
                 pGroup);
  form.ParseContent();

  CFX_DefaultRenderDevice bitmap_device;
  bool bLuminosity =
      pSMaskDict->GetStringFor(pdfium::transparency::kSoftMaskSubType) !=
      pdfium::transparency::kAlpha;
  int width = pClipRect->right - pClipRect->left;
  int height = pClipRect->bottom - pClipRect->top;
  FXDIB_Format format;
#if defined(OS_MACOSX) || defined _SKIA_SUPPORT_ || defined _SKIA_SUPPORT_PATHS_
  format = bLuminosity ? FXDIB_Rgb32 : FXDIB_8bppMask;
#else
  format = bLuminosity ? FXDIB_Rgb : FXDIB_8bppMask;
#endif
  if (!bitmap_device.Create(width, height, format, nullptr))
    return nullptr;

  RetainPtr<CFX_DIBitmap> bitmap = bitmap_device.GetBitmap();
  int nCSFamily = 0;
  if (bLuminosity) {
    FX_ARGB back_color =
        GetBackColor(pSMaskDict, pGroup->GetDict(), &nCSFamily);
    bitmap->Clear(back_color);
  } else {
    bitmap->Clear(0);
  }

  const CPDF_Dictionary* pFormResource =
      form.GetDict()->GetDictFor("Resources");
  CPDF_RenderOptions options;
  options.SetColorMode(bLuminosity ? CPDF_RenderOptions::kNormal
                                   : CPDF_RenderOptions::kAlpha);
  CPDF_RenderStatus status(m_pContext.Get(), &bitmap_device);
  status.SetOptions(options);
  status.SetGroupFamily(nCSFamily);
  status.SetLoadMask(bLuminosity);
  status.SetStdCS(true);
  status.SetFormResource(pFormResource);
  status.SetDropObjects(m_bDropObjects);
  status.Initialize(nullptr, nullptr);
  status.RenderObjectList(&form, matrix);

  auto pMask = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pMask->Create(width, height, FXDIB_8bppMask))
    return nullptr;

  uint8_t* dest_buf = pMask->GetBuffer();
  int dest_pitch = pMask->GetPitch();
  uint8_t* src_buf = bitmap->GetBuffer();
  int src_pitch = bitmap->GetPitch();
  std::vector<uint8_t> transfers(256);
  if (pFunc) {
    std::vector<float> results(pFunc->CountOutputs());
    for (size_t i = 0; i < transfers.size(); ++i) {
      float input = i / 255.0f;
      int nresult;
      pFunc->Call(&input, 1, results.data(), &nresult);
      transfers[i] = FXSYS_roundf(results[0] * 255);
    }
  } else {
    // Fill |transfers| with 0, 1, ... N.
    std::iota(transfers.begin(), transfers.end(), 0);
  }
  if (bLuminosity) {
    int Bpp = bitmap->GetBPP() / 8;
    for (int row = 0; row < height; row++) {
      uint8_t* dest_pos = dest_buf + row * dest_pitch;
      uint8_t* src_pos = src_buf + row * src_pitch;
      for (int col = 0; col < width; col++) {
        *dest_pos++ = transfers[FXRGB2GRAY(src_pos[2], src_pos[1], *src_pos)];
        src_pos += Bpp;
      }
    }
  } else if (pFunc) {
    int size = dest_pitch * height;
    for (int i = 0; i < size; i++) {
      dest_buf[i] = transfers[src_buf[i]];
    }
  } else {
    memcpy(dest_buf, src_buf, dest_pitch * height);
  }
  return pMask;
}

FX_ARGB CPDF_RenderStatus::GetBackColor(const CPDF_Dictionary* pSMaskDict,
                                        const CPDF_Dictionary* pGroupDict,
                                        int* pCSFamily) {
  static constexpr FX_ARGB kDefaultColor = ArgbEncode(255, 0, 0, 0);
  const CPDF_Array* pBC = pSMaskDict->GetArrayFor(pdfium::transparency::kBC);
  if (!pBC)
    return kDefaultColor;

  const CPDF_Object* pCSObj = nullptr;
  const CPDF_Dictionary* pGroup =
      pGroupDict ? pGroupDict->GetDictFor("Group") : nullptr;
  if (pGroup)
    pCSObj = pGroup->GetDirectObjectFor(pdfium::transparency::kCS);
  RetainPtr<CPDF_ColorSpace> pCS =
      CPDF_DocPageData::FromDocument(m_pContext->GetDocument())
          ->GetColorSpace(pCSObj, nullptr);
  if (!pCS)
    return kDefaultColor;

  int family = pCS->GetFamily();
  if (family == PDFCS_LAB || pCS->IsSpecial() ||
      (family == PDFCS_ICCBASED && !pCS->IsNormal())) {
    return kDefaultColor;
  }

  // Store Color Space Family to use in CPDF_RenderStatus::Initialize().
  *pCSFamily = family;

  uint32_t comps = std::max(8u, pCS->CountComponents());
  size_t count = std::min<size_t>(8, pBC->size());
  std::vector<float> floats = ReadArrayElementsToVector(pBC, count);
  floats.resize(comps);

  float R;
  float G;
  float B;
  pCS->GetRGB(floats.data(), &R, &G, &B);
  return ArgbEncode(255, static_cast<int>(R * 255), static_cast<int>(G * 255),
                    static_cast<int>(B * 255));
}

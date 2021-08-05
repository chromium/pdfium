// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/render/cpdf_renderstatus.h"

#include <algorithm>
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
#include "core/fpdfapi/render/charposlist.h"
#include "core/fpdfapi/render/cpdf_docrenderdata.h"
#include "core/fpdfapi/render/cpdf_imagerenderer.h"
#include "core/fpdfapi/render/cpdf_pagerendercache.h"
#include "core/fpdfapi/render/cpdf_rendercontext.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfapi/render/cpdf_rendershading.h"
#include "core/fpdfapi/render/cpdf_rendertiling.h"
#include "core/fpdfapi/render/cpdf_scaledrenderbuffer.h"
#include "core/fpdfapi/render/cpdf_textrenderer.h"
#include "core/fpdfapi/render/cpdf_type3cache.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/fx_memory.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_glyphbitmap.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_font.h"
#include "core/fxge/renderdevicedriver_iface.h"
#include "core/fxge/text_char_pos.h"
#include "core/fxge/text_glyph_pos.h"
#include "third_party/base/check.h"
#include "third_party/base/containers/contains.h"
#include "third_party/base/notreached.h"

#if defined(_SKIA_SUPPORT_)
#include "core/fxge/skia/fx_skia_device.h"
#endif

namespace {

constexpr int kRenderMaxRecursionDepth = 64;
int g_CurrentRecursionDepth = 0;

CFX_FillRenderOptions GetFillOptionsForDrawPathWithBlend(
    const CPDF_RenderOptions::Options& options,
    const CPDF_PathObject* path_obj,
    CFX_FillRenderOptions::FillType fill_type,
    bool is_stroke,
    bool is_type3_char) {
  CFX_FillRenderOptions fill_options(fill_type);
  if (fill_type != CFX_FillRenderOptions::FillType::kNoFill && options.bRectAA)
    fill_options.rect_aa = true;
  if (options.bNoPathSmooth)
    fill_options.aliased_path = true;
  if (path_obj->m_GeneralState.GetStrokeAdjust())
    fill_options.adjust_stroke = true;
  if (is_stroke)
    fill_options.stroke = true;
  if (is_type3_char)
    fill_options.text_mode = true;

  return fill_options;
}

CFX_FillRenderOptions GetFillOptionsForDrawTextPath(
    const CPDF_RenderOptions::Options& options,
    const CPDF_TextObject* text_obj,
    bool is_stroke,
    bool is_fill) {
  CFX_FillRenderOptions fill_options;
  if (is_stroke && is_fill) {
    fill_options.stroke = true;
    fill_options.stroke_text_mode = true;
  }
  if (text_obj->m_GeneralState.GetStrokeAdjust())
    fill_options.adjust_stroke = true;
  if (options.bNoTextSmooth)
    fill_options.aliased_path = true;

  return fill_options;
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
  FX_STACK_ALLOCATED();

  explicit ScopedSkiaDeviceFlush(CFX_RenderDevice* pDevice)
      : m_pDevice(pDevice) {}

  ScopedSkiaDeviceFlush(const ScopedSkiaDeviceFlush&) = delete;
  ScopedSkiaDeviceFlush& operator=(const ScopedSkiaDeviceFlush&) = delete;

  ~ScopedSkiaDeviceFlush() { m_pDevice->Flush(/*release=*/false); }

 private:
  UnownedPtr<CFX_RenderDevice> const m_pDevice;
};
#endif

}  // namespace

CPDF_RenderStatus::CPDF_RenderStatus(CPDF_RenderContext* pContext,
                                     CFX_RenderDevice* pDevice)
    : m_pContext(pContext), m_pDevice(pDevice) {}

CPDF_RenderStatus::~CPDF_RenderStatus() = default;

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
#if defined(_SKIA_SUPPORT_)
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
#if defined(_SKIA_SUPPORT_)
  DebugVerifyDeviceIsPreMultiplied();
#endif
}

void CPDF_RenderStatus::RenderSingleObject(CPDF_PageObject* pObj,
                                           const CFX_Matrix& mtObj2Device) {
#if defined(_SKIA_SUPPORT_)
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
#if defined(_SKIA_SUPPORT_)
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

  m_pImageRenderer = std::make_unique<CPDF_ImageRenderer>();
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
#if defined(_SKIA_SUPPORT_)
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
#if defined(_SKIA_SUPPORT_)
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

  int res = (pObj->IsImage() && m_bPrint) ? 0 : 300;
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
#if defined(_SKIA_SUPPORT_)
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
#if defined(_SKIA_SUPPORT_)
  DebugVerifyDeviceIsPreMultiplied();
#endif
  return true;
}

bool CPDF_RenderStatus::ProcessPath(CPDF_PathObject* path_obj,
                                    const CFX_Matrix& mtObj2Device) {
  CFX_FillRenderOptions::FillType fill_type = path_obj->filltype();
  bool stroke = path_obj->stroke();
  ProcessPathPattern(path_obj, mtObj2Device, &fill_type, &stroke);
  if (fill_type == CFX_FillRenderOptions::FillType::kNoFill && !stroke)
    return true;

  // If the option to convert fill paths to stroke is enabled for forced color,
  // set |fill_type| to FillType::kNoFill and |stroke| to true.
  CPDF_RenderOptions::Options& options = m_Options.GetOptions();
  if (m_Options.ColorModeIs(CPDF_RenderOptions::Type::kForcedColor) &&
      options.bConvertFillToStroke &&
      fill_type != CFX_FillRenderOptions::FillType::kNoFill) {
    stroke = true;
    fill_type = CFX_FillRenderOptions::FillType::kNoFill;
  }

  uint32_t fill_argb = fill_type != CFX_FillRenderOptions::FillType::kNoFill
                           ? GetFillArgb(path_obj)
                           : 0;
  uint32_t stroke_argb = stroke ? GetStrokeArgb(path_obj) : 0;
  CFX_Matrix path_matrix = path_obj->matrix() * mtObj2Device;
  if (!IsAvailableMatrix(path_matrix))
    return true;

  return m_pDevice->DrawPathWithBlend(
      path_obj->path().GetObject(), &path_matrix,
      path_obj->m_GraphState.GetObject(), fill_argb, stroke_argb,
      GetFillOptionsForDrawPathWithBlend(options, path_obj, fill_type, stroke,
                                         m_pType3Char),
      m_curBlend);
}

RetainPtr<CPDF_TransferFunc> CPDF_RenderStatus::GetTransferFunc(
    const CPDF_Object* pObj) const {
  DCHECK(pObj);
  auto* pDocCache = CPDF_DocRenderData::FromDocument(m_pContext->GetDocument());
  return pDocCache ? pDocCache->GetTransferFunc(pObj) : nullptr;
}

FX_ARGB CPDF_RenderStatus::GetFillArgb(CPDF_PageObject* pObj) const {
  if (Type3CharMissingFillColor(m_pType3Char.Get(), &pObj->m_ColorState))
    return m_T3FillColor;

  return GetFillArgbForType3(pObj);
}

FX_ARGB CPDF_RenderStatus::GetFillArgbForType3(CPDF_PageObject* pObj) const {
  const CPDF_ColorState* pColorState = &pObj->m_ColorState;
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
  return m_Options.TranslateObjectColor(AlphaAndColorRefToArgb(alpha, colorref),
                                        pObj->GetType(),
                                        CPDF_RenderOptions::RenderType::kFill);
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
  return m_Options.TranslateObjectColor(
      AlphaAndColorRefToArgb(alpha, colorref), pObj->GetType(),
      CPDF_RenderOptions::RenderType::kStroke);
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
    const CFX_Path* pPath = ClipPath.GetPath(i).GetObject();
    if (!pPath)
      continue;

    if (pPath->GetPoints().empty()) {
      CFX_Path EmptyPath;
      EmptyPath.AppendRect(-1, -1, 0, 0);
      m_pDevice->SetClip_PathFill(&EmptyPath, nullptr,
                                  CFX_FillRenderOptions::WindingOptions());
    } else {
      m_pDevice->SetClip_PathFill(
          pPath, &mtObj2Device, CFX_FillRenderOptions(ClipPath.GetClipType(i)));
    }
  }

  if (ClipPath.GetTextCount() == 0)
    return;

  if (!m_bPrint &&
      !(m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SOFT_CLIP)) {
    return;
  }

  std::unique_ptr<CFX_Path> pTextClippingPath;
  for (size_t i = 0; i < ClipPath.GetTextCount(); ++i) {
    CPDF_TextObject* pText = ClipPath.GetText(i);
    if (pText) {
      if (!pTextClippingPath)
        pTextClippingPath = std::make_unique<CFX_Path>();
      ProcessText(pText, mtObj2Device, pTextClippingPath.get());
      continue;
    }

    if (!pTextClippingPath)
      continue;

    CFX_FillRenderOptions fill_options(CFX_FillRenderOptions::WindingOptions());
    if (m_Options.GetOptions().bNoTextSmooth)
      fill_options.aliased_path = true;
    m_pDevice->SetClip_PathFill(pTextClippingPath.get(), nullptr, fill_options);
    pTextClippingPath.reset();
  }
}

bool CPDF_RenderStatus::ClipPattern(const CPDF_PageObject* page_obj,
                                    const CFX_Matrix& mtObj2Device,
                                    bool stroke) {
  if (page_obj->IsPath())
    return SelectClipPath(page_obj->AsPath(), mtObj2Device, stroke);
  if (page_obj->IsImage()) {
    m_pDevice->SetClip_Rect(page_obj->GetTransformedBBox(mtObj2Device));
    return true;
  }
  return false;
}

bool CPDF_RenderStatus::SelectClipPath(const CPDF_PathObject* path_obj,
                                       const CFX_Matrix& mtObj2Device,
                                       bool stroke) {
  CFX_Matrix path_matrix = path_obj->matrix() * mtObj2Device;
  if (stroke) {
    return m_pDevice->SetClip_PathStroke(path_obj->path().GetObject(),
                                         &path_matrix,
                                         path_obj->m_GraphState.GetObject());
  }
  CFX_FillRenderOptions fill_options(path_obj->filltype());
  if (m_Options.GetOptions().bNoPathSmooth) {
    fill_options.aliased_path = true;
  }
  return m_pDevice->SetClip_PathFill(path_obj->path().GetObject(), &path_matrix,
                                     fill_options);
}

bool CPDF_RenderStatus::ProcessTransparency(CPDF_PageObject* pPageObj,
                                            const CFX_Matrix& mtObj2Device) {
#if defined(_SKIA_SUPPORT_)
  DebugVerifyDeviceIsPreMultiplied();
#endif
  const BlendMode blend_type = pPageObj->m_GeneralState.GetBlendType();
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
       pPageObj->m_ClipPath.GetTextCount() > 0 && !m_bPrint &&
       !(m_pDevice->GetDeviceCaps(FXDC_RENDER_CAPS) & FXRC_SOFT_CLIP));
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
  if (!bitmap_device.Create(width, height, FXDIB_Format::kArgb, backdrop))
    return true;

  RetainPtr<CFX_DIBitmap> bitmap = bitmap_device.GetBitmap();
  bitmap->Clear(0);

  CFX_Matrix new_matrix = mtObj2Device;
  new_matrix.Translate(-rect.left, -rect.top);

  RetainPtr<CFX_DIBitmap> pTextMask;
  if (bTextClip) {
    pTextMask = pdfium::MakeRetain<CFX_DIBitmap>();
    if (!pTextMask->Create(width, height, FXDIB_Format::k8bppMask))
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
          nullptr, CFX_FillRenderOptions());
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
#if defined(_SKIA_SUPPORT_PATHS_)
  bitmap_device.Flush(true);
  bitmap->UnPreMultiply();
#endif
  m_bStopped = bitmap_render.m_bStopped;
  if (pSMaskDict) {
    CFX_Matrix smask_matrix =
        *pPageObj->m_GeneralState.GetSMaskMatrix() * mtObj2Device;
    RetainPtr<CFX_DIBBase> pSMaskSource =
        LoadSMask(pSMaskDict, &rect, smask_matrix);
    if (pSMaskSource)
      bitmap->MultiplyAlpha(pSMaskSource);
  }
  if (pTextMask) {
    bitmap->MultiplyAlpha(pTextMask);
    pTextMask.Reset();
  }
  int32_t blitAlpha = 255;
  if (group_alpha != 1.0f && transparency.IsGroup()) {
    blitAlpha = static_cast<int32_t>(group_alpha * 255);
#if !defined(_SKIA_SUPPORT_)
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
#if defined(_SKIA_SUPPORT_)
  DebugVerifyDeviceIsPreMultiplied();
#endif
  return true;
}

FX_RECT CPDF_RenderStatus::GetClippedBBox(const FX_RECT& rect) const {
  FX_RECT bbox = rect;
  bbox.Intersect(m_pDevice->GetClipBox());
  return bbox;
}

RetainPtr<CFX_DIBitmap> CPDF_RenderStatus::GetBackdrop(
    const CPDF_PageObject* pObj,
    const FX_RECT& bbox,
    bool bBackAlphaRequired) {
  int width = bbox.Width();
  int height = bbox.Height();
  auto pBackdrop = pdfium::MakeRetain<CFX_DIBitmap>();
  if (bBackAlphaRequired && !m_bDropObjects)
    pBackdrop->Create(width, height, FXDIB_Format::kArgb);
  else
    m_pDevice->CreateCompatibleBitmap(pBackdrop, width, height);

  if (!pBackdrop->GetBuffer())
    return nullptr;

  bool bNeedDraw;
  if (pBackdrop->IsAlphaFormat())
    bNeedDraw = !(m_pDevice->GetRenderCaps() & FXRC_ALPHA_OUTPUT);
  else
    bNeedDraw = !(m_pDevice->GetRenderCaps() & FXRC_GET_BITS);

  if (!bNeedDraw) {
    m_pDevice->GetDIBits(pBackdrop, bbox.left, bbox.top);
    return pBackdrop;
  }
  CFX_Matrix FinalMatrix = m_DeviceMatrix;
  FinalMatrix.Translate(-bbox.left, -bbox.top);
  pBackdrop->Clear(pBackdrop->IsAlphaFormat() ? 0 : 0xffffffff);

  CFX_DefaultRenderDevice device;
  device.Attach(pBackdrop, false, nullptr, false);
  m_pContext->Render(&device, pObj, &m_Options, &FinalMatrix);
  return pBackdrop;
}

std::unique_ptr<CPDF_GraphicStates> CPDF_RenderStatus::CloneObjStates(
    const CPDF_GraphicStates* pSrcStates,
    bool stroke) {
  if (!pSrcStates)
    return nullptr;

  auto pStates = std::make_unique<CPDF_GraphicStates>();
  pStates->CopyStates(*pSrcStates);
  const CPDF_Color* pObjColor = stroke
                                    ? pSrcStates->m_ColorState.GetStrokeColor()
                                    : pSrcStates->m_ColorState.GetFillColor();
  if (!pObjColor->IsNull()) {
    pStates->m_ColorState.SetFillColorRef(
        stroke ? pSrcStates->m_ColorState.GetStrokeColorRef()
               : pSrcStates->m_ColorState.GetFillColorRef());
    pStates->m_ColorState.SetStrokeColorRef(
        pStates->m_ColorState.GetFillColorRef());
  }
  return pStates;
}

#if defined(_SKIA_SUPPORT_)
void CPDF_RenderStatus::DebugVerifyDeviceIsPreMultiplied() const {
  m_pDevice->DebugVerifyBitmapIsPreMultiplied();
}
#endif

bool CPDF_RenderStatus::ProcessText(CPDF_TextObject* textobj,
                                    const CFX_Matrix& mtObj2Device,
                                    CFX_Path* clipping_path) {
  if (textobj->GetCharCodes().empty())
    return true;

  const TextRenderingMode text_render_mode = textobj->m_TextState.GetTextMode();
  if (text_render_mode == TextRenderingMode::MODE_INVISIBLE)
    return true;

  RetainPtr<CPDF_Font> pFont = textobj->m_TextState.GetFont();
  if (pFont->IsType3Font())
    return ProcessType3Text(textobj, mtObj2Device);

  bool is_fill = false;
  bool is_stroke = false;
  bool is_clip = false;
  if (clipping_path) {
    is_clip = true;
  } else {
    switch (text_render_mode) {
      case TextRenderingMode::MODE_FILL:
      case TextRenderingMode::MODE_FILL_CLIP:
        is_fill = true;
        break;
      case TextRenderingMode::MODE_STROKE:
      case TextRenderingMode::MODE_STROKE_CLIP:
        if (pFont->HasFace())
          is_stroke = true;
        else
          is_fill = true;
        break;
      case TextRenderingMode::MODE_FILL_STROKE:
      case TextRenderingMode::MODE_FILL_STROKE_CLIP:
        is_fill = true;
        if (pFont->HasFace())
          is_stroke = true;
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
  if (is_stroke) {
    if (textobj->m_ColorState.GetStrokeColor()->IsPattern()) {
      bPattern = true;
    } else {
      stroke_argb = GetStrokeArgb(textobj);
    }
  }
  if (is_fill) {
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
                            text_matrix, is_fill, is_stroke);
    return true;
  }
  if (is_clip || is_stroke) {
    const CFX_Matrix* pDeviceMatrix = &mtObj2Device;
    CFX_Matrix device_matrix;
    if (is_stroke) {
      const float* pCTM = textobj->m_TextState.GetCTM();
      if (pCTM[0] != 1.0f || pCTM[3] != 1.0f) {
        CFX_Matrix ctm(pCTM[0], pCTM[1], pCTM[2], pCTM[3], 0, 0);
        text_matrix *= ctm.GetInverse();
        device_matrix = ctm * mtObj2Device;
        pDeviceMatrix = &device_matrix;
      }
    }
    return CPDF_TextRenderer::DrawTextPath(
        m_pDevice, textobj->GetCharCodes(), textobj->GetCharPositions(),
        pFont.Get(), font_size, text_matrix, pDeviceMatrix,
        textobj->m_GraphState.GetObject(), fill_argb, stroke_argb,
        clipping_path,
        GetFillOptionsForDrawTextPath(m_Options.GetOptions(), textobj,
                                      is_stroke, is_fill));
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
  if (pdfium::Contains(m_Type3FontCache, pType3Font))
    return true;

  FX_ARGB fill_argb = GetFillArgbForType3(textobj);
  int fill_alpha = FXARGB_A(fill_argb);
  if (m_bPrint && fill_alpha < 255)
    return false;

  CFX_Matrix text_matrix = textobj->GetTextMatrix();
  CFX_Matrix char_matrix = pType3Font->GetFontMatrix();
  float font_size = textobj->m_TextState.GetFontSize();
  char_matrix.Scale(font_size, font_size);

  // Must come before |glyphs|, because |glyphs| points into |refTypeCache|.
  std::set<RetainPtr<CPDF_Type3Cache>> refTypeCache;
  std::vector<TextGlyphPos> glyphs;
  if (!m_bPrint)
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
        status.m_Type3FontCache.emplace_back(pType3Font);

        CFX_RenderDevice::StateRestorer restorer(m_pDevice);
        status.RenderObjectList(pForm, matrix);
      } else {
        FX_RECT rect =
            matrix.TransformRect(pForm->CalcBoundingBox()).GetOuterRect();
        if (!rect.Valid())
          continue;

        CFX_DefaultRenderDevice bitmap_device;
        if (!bitmap_device.Create(rect.Width(), rect.Height(),
                                  FXDIB_Format::kArgb, nullptr)) {
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
        status.m_Type3FontCache.emplace_back(pType3Font);
        matrix.Translate(-rect.left, -rect.top);
        status.RenderObjectList(pForm, matrix);
        m_pDevice->SetDIBits(bitmap_device.GetBitmap(), rect.left, rect.top);
      }
    } else if (pType3Char->GetBitmap()) {
      if (m_bPrint) {
        CFX_Matrix image_matrix = pType3Char->matrix() * matrix;
        CPDF_ImageRenderer renderer;
        if (renderer.Start(this, pType3Char->GetBitmap(), fill_argb,
                           image_matrix, FXDIB_ResampleOptions(), false)) {
          renderer.Continue(nullptr);
        }
        if (!renderer.GetResult())
          return false;
      } else {
        CPDF_Document* pDoc = pType3Font->GetDocument();
        RetainPtr<CPDF_Type3Cache> pCache =
            CPDF_DocRenderData::FromDocument(pDoc)->GetCachedType3(pType3Font);

        const CFX_GlyphBitmap* pBitmap = pCache->LoadGlyph(charcode, matrix);
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
      }
    }
  }

  if (glyphs.empty())
    return true;

  FX_RECT rect = GetGlyphsBBox(glyphs, 0);
  auto pBitmap = pdfium::MakeRetain<CFX_DIBitmap>();
  if (!pBitmap->Create(rect.Width(), rect.Height(), FXDIB_Format::k8bppMask))
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
                                                const CFX_Matrix& mtTextMatrix,
                                                bool fill,
                                                bool stroke) {
  if (!stroke) {
    std::vector<std::unique_ptr<CPDF_TextObject>> pCopy;
    pCopy.push_back(textobj->Clone());

    CPDF_PathObject path;
    path.set_filltype(CFX_FillRenderOptions::FillType::kWinding);
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

  std::vector<TextCharPos> char_pos_list = GetCharPosList(
      textobj->GetCharCodes(), textobj->GetCharPositions(), pFont, font_size);
  for (const TextCharPos& charpos : char_pos_list) {
    auto* font = charpos.m_FallbackFontPosition == -1
                     ? pFont->GetFont()
                     : pFont->GetFontFallback(charpos.m_FallbackFontPosition);
    const CFX_Path* pPath =
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
    path.set_stroke(stroke);
    path.set_filltype(fill ? CFX_FillRenderOptions::FillType::kWinding
                           : CFX_FillRenderOptions::FillType::kNoFill);
    path.path().Append(*pPath, &matrix);
    path.SetPathMatrix(mtTextMatrix);
    ProcessPath(&path, mtObj2Device);
  }
}

void CPDF_RenderStatus::DrawShadingPattern(CPDF_ShadingPattern* pattern,
                                           const CPDF_PageObject* pPageObj,
                                           const CFX_Matrix& mtObj2Device,
                                           bool stroke) {
  if (!pattern->Load())
    return;

  CFX_RenderDevice::StateRestorer restorer(m_pDevice);
  if (!ClipPattern(pPageObj, mtObj2Device, stroke))
    return;

  FX_RECT rect = GetObjectClippedRect(pPageObj, mtObj2Device);
  if (rect.IsEmpty())
    return;

  CFX_Matrix matrix = pattern->pattern_to_form() * mtObj2Device;
  int alpha =
      FXSYS_roundf(255 * (stroke ? pPageObj->m_GeneralState.GetStrokeAlpha()
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
                                          bool stroke) {
  const std::unique_ptr<CPDF_Form> pPatternForm = pPattern->Load(pPageObj);
  if (!pPatternForm)
    return;

  CFX_RenderDevice::StateRestorer restorer(m_pDevice);
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
  ScopedSkiaDeviceFlush scoped_skia_device_flush(m_pDevice);
#endif
  if (!ClipPattern(pPageObj, mtObj2Device, stroke))
    return;

  FX_RECT clip_box = m_pDevice->GetClipBox();
  if (clip_box.IsEmpty())
    return;

  RetainPtr<CFX_DIBitmap> pScreen =
      CPDF_RenderTiling::Draw(this, pPageObj, pPattern, pPatternForm.get(),
                              mtObj2Device, clip_box, stroke);
  if (!pScreen)
    return;

  CompositeDIBitmap(pScreen, clip_box.left, clip_box.top, 0, 255,
                    BlendMode::kNormal, CPDF_Transparency());
}

void CPDF_RenderStatus::DrawPathWithPattern(CPDF_PathObject* path_obj,
                                            const CFX_Matrix& mtObj2Device,
                                            const CPDF_Color* pColor,
                                            bool stroke) {
  CPDF_Pattern* pattern = pColor->GetPattern();
  if (!pattern)
    return;

  if (CPDF_TilingPattern* pTilingPattern = pattern->AsTilingPattern())
    DrawTilingPattern(pTilingPattern, path_obj, mtObj2Device, stroke);
  else if (CPDF_ShadingPattern* pShadingPattern = pattern->AsShadingPattern())
    DrawShadingPattern(pShadingPattern, path_obj, mtObj2Device, stroke);
}

void CPDF_RenderStatus::ProcessPathPattern(
    CPDF_PathObject* path_obj,
    const CFX_Matrix& mtObj2Device,
    CFX_FillRenderOptions::FillType* fill_type,
    bool* stroke) {
  DCHECK(fill_type);
  DCHECK(stroke);

  if (*fill_type != CFX_FillRenderOptions::FillType::kNoFill) {
    const CPDF_Color& FillColor = *path_obj->m_ColorState.GetFillColor();
    if (FillColor.IsPattern()) {
      DrawPathWithPattern(path_obj, mtObj2Device, &FillColor, false);
      *fill_type = CFX_FillRenderOptions::FillType::kNoFill;
    }
  }
  if (*stroke) {
    const CPDF_Color& StrokeColor = *path_obj->m_ColorState.GetStrokeColor();
    if (StrokeColor.IsPattern()) {
      DrawPathWithPattern(path_obj, mtObj2Device, &StrokeColor, true);
      *stroke = false;
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
    if (!pDIBitmap->IsMaskFormat()) {
      if (bitmap_alpha < 255) {
#if defined(_SKIA_SUPPORT_)
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
#if defined(_SKIA_SUPPORT_)
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
      if (!pDIBitmap->IsMaskFormat())
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
      if (pDIBitmap->IsMaskFormat()) {
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
      if (!pDIBitmap->IsMaskFormat()) {
        m_pDevice->SetDIBitsWithBlend(pDIBitmap, rect.left, rect.top,
                                      blend_mode);
      }
    }
    return;
  }
  FX_RECT bbox = GetClippedBBox(FX_RECT(left, top, left + pDIBitmap->GetWidth(),
                                        top + pDIBitmap->GetHeight()));
  RetainPtr<CFX_DIBitmap> pBackdrop = GetBackdrop(
      m_pCurObj.Get(), bbox, blend_mode != BlendMode::kNormal && bIsolated);
  if (!pBackdrop)
    return;

  if (pDIBitmap->IsMaskFormat()) {
    pBackdrop->CompositeMask(left - bbox.left, top - bbox.top,
                             pDIBitmap->GetWidth(), pDIBitmap->GetHeight(),
                             pDIBitmap, mask_argb, 0, 0, blend_mode, nullptr,
                             false);
  } else {
    pBackdrop->CompositeBitmap(left - bbox.left, top - bbox.top,
                               pDIBitmap->GetWidth(), pDIBitmap->GetHeight(),
                               pDIBitmap, 0, 0, blend_mode, nullptr, false);
  }

  auto pBackdrop1 = pdfium::MakeRetain<CFX_DIBitmap>();
  pBackdrop1->Create(pBackdrop->GetWidth(), pBackdrop->GetHeight(),
                     FXDIB_Format::kRgb32);
  pBackdrop1->Clear((uint32_t)-1);
  pBackdrop1->CompositeBitmap(0, 0, pBackdrop->GetWidth(),
                              pBackdrop->GetHeight(), pBackdrop, 0, 0,
                              BlendMode::kNormal, nullptr, false);
  pBackdrop = std::move(pBackdrop1);
  m_pDevice->SetDIBits(pBackdrop, bbox.left, bbox.top);
}

RetainPtr<CFX_DIBitmap> CPDF_RenderStatus::LoadSMask(
    CPDF_Dictionary* pSMaskDict,
    FX_RECT* pClipRect,
    const CFX_Matrix& mtMatrix) {
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

  CFX_Matrix matrix = mtMatrix;
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
#if defined(OS_APPLE) || defined(_SKIA_SUPPORT_) || \
    defined(_SKIA_SUPPORT_PATHS_)
  format = bLuminosity ? FXDIB_Format::kRgb32 : FXDIB_Format::k8bppMask;
#else
  format = bLuminosity ? FXDIB_Format::kRgb : FXDIB_Format::k8bppMask;
#endif
  if (!bitmap_device.Create(width, height, format, nullptr))
    return nullptr;

  RetainPtr<CFX_DIBitmap> bitmap = bitmap_device.GetBitmap();
  CPDF_ColorSpace::Family nCSFamily = CPDF_ColorSpace::Family::kUnknown;
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
  if (!pMask->Create(width, height, FXDIB_Format::k8bppMask))
    return nullptr;

  uint8_t* dest_buf = pMask->GetBuffer();
  int dest_pitch = pMask->GetPitch();
  uint8_t* src_buf = bitmap->GetBuffer();
  int src_pitch = bitmap->GetPitch();
  std::vector<uint8_t, FxAllocAllocator<uint8_t>> transfers(256);
  if (pFunc) {
    std::vector<float> results(pFunc->CountOutputs());
    for (size_t i = 0; i < transfers.size(); ++i) {
      float input = i / 255.0f;
      pFunc->Call(pdfium::make_span(&input, 1), results);
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
                                        CPDF_ColorSpace::Family* pCSFamily) {
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

  CPDF_ColorSpace::Family family = pCS->GetFamily();
  if (family == CPDF_ColorSpace::Family::kLab || pCS->IsSpecial() ||
      (family == CPDF_ColorSpace::Family::kICCBased && !pCS->IsNormal())) {
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
  pCS->GetRGB(floats, &R, &G, &B);
  return ArgbEncode(255, static_cast<int>(R * 255), static_cast<int>(G * 255),
                    static_cast<int>(B * 255));
}

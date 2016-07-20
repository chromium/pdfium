// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_annot.h"

#include "core/fpdfapi/fpdf_page/include/cpdf_form.h"
#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_array.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_render/include/cpdf_rendercontext.h"
#include "core/fpdfapi/fpdf_render/include/cpdf_renderoptions.h"
#include "core/fxge/include/fx_ge.h"

CPDF_Annot::CPDF_Annot(CPDF_Dictionary* pDict, CPDF_Document* pDocument)
    : m_pAnnotDict(pDict),
      m_pDocument(pDocument),
      m_sSubtype(m_pAnnotDict->GetStringBy("Subtype")) {}

CPDF_Annot::~CPDF_Annot() {
  ClearCachedAP();
}

void CPDF_Annot::ClearCachedAP() {
  for (const auto& pair : m_APMap) {
    delete pair.second;
  }
  m_APMap.clear();
}
CFX_ByteString CPDF_Annot::GetSubType() const {
  return m_sSubtype;
}

void CPDF_Annot::GetRect(CFX_FloatRect& rect) const {
  if (!m_pAnnotDict) {
    return;
  }
  rect = m_pAnnotDict->GetRectBy("Rect");
  rect.Normalize();
}

uint32_t CPDF_Annot::GetFlags() const {
  return m_pAnnotDict->GetIntegerBy("F");
}

CPDF_Stream* FPDFDOC_GetAnnotAP(CPDF_Dictionary* pAnnotDict,
                                CPDF_Annot::AppearanceMode mode) {
  CPDF_Dictionary* pAP = pAnnotDict->GetDictBy("AP");
  if (!pAP) {
    return nullptr;
  }
  const FX_CHAR* ap_entry = "N";
  if (mode == CPDF_Annot::Down)
    ap_entry = "D";
  else if (mode == CPDF_Annot::Rollover)
    ap_entry = "R";
  if (!pAP->KeyExist(ap_entry))
    ap_entry = "N";

  CPDF_Object* psub = pAP->GetDirectObjectBy(ap_entry);
  if (!psub)
    return nullptr;
  if (CPDF_Stream* pStream = psub->AsStream())
    return pStream;

  if (CPDF_Dictionary* pDict = psub->AsDictionary()) {
    CFX_ByteString as = pAnnotDict->GetStringBy("AS");
    if (as.IsEmpty()) {
      CFX_ByteString value = pAnnotDict->GetStringBy("V");
      if (value.IsEmpty()) {
        CPDF_Dictionary* pParentDict = pAnnotDict->GetDictBy("Parent");
        value = pParentDict ? pParentDict->GetStringBy("V") : CFX_ByteString();
      }
      if (value.IsEmpty() || !pDict->KeyExist(value))
        as = "Off";
      else
        as = value;
    }
    return pDict->GetStreamBy(as);
  }
  return nullptr;
}

CPDF_Form* CPDF_Annot::GetAPForm(const CPDF_Page* pPage, AppearanceMode mode) {
  CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(m_pAnnotDict, mode);
  if (!pStream)
    return nullptr;

  auto it = m_APMap.find(pStream);
  if (it != m_APMap.end())
    return it->second;

  CPDF_Form* pNewForm =
      new CPDF_Form(m_pDocument, pPage->m_pResources, pStream);
  pNewForm->ParseContent(nullptr, nullptr, nullptr);
  m_APMap[pStream] = pNewForm;
  return pNewForm;
}

static CPDF_Form* FPDFDOC_Annot_GetMatrix(const CPDF_Page* pPage,
                                          CPDF_Annot* pAnnot,
                                          CPDF_Annot::AppearanceMode mode,
                                          const CFX_Matrix* pUser2Device,
                                          CFX_Matrix& matrix) {
  CPDF_Form* pForm = pAnnot->GetAPForm(pPage, mode);
  if (!pForm) {
    return nullptr;
  }
  CFX_FloatRect form_bbox = pForm->m_pFormDict->GetRectBy("BBox");
  CFX_Matrix form_matrix = pForm->m_pFormDict->GetMatrixBy("Matrix");
  form_matrix.TransformRect(form_bbox);
  CFX_FloatRect arect;
  pAnnot->GetRect(arect);
  matrix.MatchRect(arect, form_bbox);
  matrix.Concat(*pUser2Device);
  return pForm;
}
FX_BOOL CPDF_Annot::DrawAppearance(CPDF_Page* pPage,
                                   CFX_RenderDevice* pDevice,
                                   const CFX_Matrix* pUser2Device,
                                   AppearanceMode mode,
                                   const CPDF_RenderOptions* pOptions) {
  CFX_Matrix matrix;
  CPDF_Form* pForm =
      FPDFDOC_Annot_GetMatrix(pPage, this, mode, pUser2Device, matrix);
  if (!pForm) {
    return FALSE;
  }
  CPDF_RenderContext context(pPage);
  context.AppendLayer(pForm, &matrix);
  context.Render(pDevice, pOptions, nullptr);
  return TRUE;
}
FX_BOOL CPDF_Annot::DrawInContext(const CPDF_Page* pPage,
                                  CPDF_RenderContext* pContext,
                                  const CFX_Matrix* pUser2Device,
                                  AppearanceMode mode) {
  CFX_Matrix matrix;
  CPDF_Form* pForm =
      FPDFDOC_Annot_GetMatrix(pPage, this, mode, pUser2Device, matrix);
  if (!pForm) {
    return FALSE;
  }
  pContext->AppendLayer(pForm, &matrix);
  return TRUE;
}
void CPDF_Annot::DrawBorder(CFX_RenderDevice* pDevice,
                            const CFX_Matrix* pUser2Device,
                            const CPDF_RenderOptions* pOptions) {
  if (GetSubType() == "Popup") {
    return;
  }
  uint32_t annot_flags = GetFlags();
  if (annot_flags & ANNOTFLAG_HIDDEN) {
    return;
  }
  bool bPrinting = pDevice->GetDeviceClass() == FXDC_PRINTER ||
                   (pOptions && (pOptions->m_Flags & RENDER_PRINTPREVIEW));
  if (bPrinting && (annot_flags & ANNOTFLAG_PRINT) == 0) {
    return;
  }
  if (!bPrinting && (annot_flags & ANNOTFLAG_NOVIEW)) {
    return;
  }
  CPDF_Dictionary* pBS = m_pAnnotDict->GetDictBy("BS");
  char style_char;
  FX_FLOAT width;
  CPDF_Array* pDashArray = nullptr;
  if (!pBS) {
    CPDF_Array* pBorderArray = m_pAnnotDict->GetArrayBy("Border");
    style_char = 'S';
    if (pBorderArray) {
      width = pBorderArray->GetNumberAt(2);
      if (pBorderArray->GetCount() == 4) {
        pDashArray = pBorderArray->GetArrayAt(3);
        if (!pDashArray) {
          return;
        }
        size_t nLen = pDashArray->GetCount();
        size_t i = 0;
        for (; i < nLen; ++i) {
          CPDF_Object* pObj = pDashArray->GetDirectObjectAt(i);
          if (pObj && pObj->GetInteger()) {
            break;
          }
        }
        if (i == nLen) {
          return;
        }
        style_char = 'D';
      }
    } else {
      width = 1;
    }
  } else {
    CFX_ByteString style = pBS->GetStringBy("S");
    pDashArray = pBS->GetArrayBy("D");
    style_char = style[1];
    width = pBS->GetNumberBy("W");
  }
  if (width <= 0) {
    return;
  }
  CPDF_Array* pColor = m_pAnnotDict->GetArrayBy("C");
  uint32_t argb = 0xff000000;
  if (pColor) {
    int R = (int32_t)(pColor->GetNumberAt(0) * 255);
    int G = (int32_t)(pColor->GetNumberAt(1) * 255);
    int B = (int32_t)(pColor->GetNumberAt(2) * 255);
    argb = ArgbEncode(0xff, R, G, B);
  }
  CFX_GraphStateData graph_state;
  graph_state.m_LineWidth = width;
  if (style_char == 'D') {
    if (pDashArray) {
      size_t dash_count = pDashArray->GetCount();
      if (dash_count % 2) {
        dash_count++;
      }
      graph_state.m_DashArray = FX_Alloc(FX_FLOAT, dash_count);
      graph_state.m_DashCount = dash_count;
      size_t i;
      for (i = 0; i < pDashArray->GetCount(); ++i) {
        graph_state.m_DashArray[i] = pDashArray->GetNumberAt(i);
      }
      if (i < dash_count) {
        graph_state.m_DashArray[i] = graph_state.m_DashArray[i - 1];
      }
    } else {
      graph_state.m_DashArray = FX_Alloc(FX_FLOAT, 2);
      graph_state.m_DashCount = 2;
      graph_state.m_DashArray[0] = graph_state.m_DashArray[1] = 3 * 1.0f;
    }
  }
  CFX_FloatRect rect;
  GetRect(rect);
  CFX_PathData path;
  width /= 2;
  path.AppendRect(rect.left + width, rect.bottom + width, rect.right - width,
                  rect.top - width);
  int fill_type = 0;
  if (pOptions && (pOptions->m_Flags & RENDER_NOPATHSMOOTH)) {
    fill_type |= FXFILL_NOPATHSMOOTH;
  }
  pDevice->DrawPath(&path, pUser2Device, &graph_state, argb, argb, fill_type);
}

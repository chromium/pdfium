// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_annotlist.h"

#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fpdfapi/fpdf_parser/include/cpdf_reference.h"
#include "core/fpdfapi/fpdf_render/include/cpdf_renderoptions.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpvt_generateap.h"

CPDF_AnnotList::CPDF_AnnotList(CPDF_Page* pPage)
    : m_pDocument(pPage->m_pDocument) {
  if (!pPage->m_pFormDict)
    return;

  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArrayBy("Annots");
  if (!pAnnots)
    return;

  CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
  CPDF_Dictionary* pAcroForm = pRoot->GetDictBy("AcroForm");
  FX_BOOL bRegenerateAP =
      pAcroForm && pAcroForm->GetBooleanBy("NeedAppearances");
  for (size_t i = 0; i < pAnnots->GetCount(); ++i) {
    CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetDirectObjectAt(i));
    if (!pDict)
      continue;

    uint32_t dwObjNum = pDict->GetObjNum();
    if (dwObjNum == 0) {
      dwObjNum = m_pDocument->AddIndirectObject(pDict);
      CPDF_Reference* pAction = new CPDF_Reference(m_pDocument, dwObjNum);
      pAnnots->InsertAt(i, pAction);
      pAnnots->RemoveAt(i + 1);
      pDict = pAnnots->GetDictAt(i);
    }
    m_AnnotList.push_back(
        std::unique_ptr<CPDF_Annot>(new CPDF_Annot(pDict, m_pDocument)));
    if (bRegenerateAP && pDict->GetStringBy("Subtype") == "Widget" &&
        CPDF_InterForm::IsUpdateAPEnabled()) {
      FPDF_GenerateAP(m_pDocument, pDict);
    }
  }
}

CPDF_AnnotList::~CPDF_AnnotList() {}

void CPDF_AnnotList::DisplayPass(CPDF_Page* pPage,
                                 CFX_RenderDevice* pDevice,
                                 CPDF_RenderContext* pContext,
                                 FX_BOOL bPrinting,
                                 CFX_Matrix* pMatrix,
                                 FX_BOOL bWidgetPass,
                                 CPDF_RenderOptions* pOptions,
                                 FX_RECT* clip_rect) {
  for (const auto& pAnnot : m_AnnotList) {
    bool bWidget = pAnnot->GetSubType() == "Widget";
    if ((bWidgetPass && !bWidget) || (!bWidgetPass && bWidget))
      continue;

    uint32_t annot_flags = pAnnot->GetFlags();
    if (annot_flags & ANNOTFLAG_HIDDEN)
      continue;

    if (bPrinting && (annot_flags & ANNOTFLAG_PRINT) == 0)
      continue;

    if (!bPrinting && (annot_flags & ANNOTFLAG_NOVIEW))
      continue;

    if (pOptions) {
      CPDF_OCContext* pOCContext = pOptions->m_pOCContext;
      CPDF_Dictionary* pAnnotDict = pAnnot->GetAnnotDict();
      if (pOCContext && pAnnotDict &&
          !pOCContext->CheckOCGVisible(pAnnotDict->GetDictBy("OC"))) {
        continue;
      }
    }
    CFX_FloatRect annot_rect_f;
    pAnnot->GetRect(annot_rect_f);
    CFX_Matrix matrix = *pMatrix;
    if (clip_rect) {
      annot_rect_f.Transform(&matrix);
      FX_RECT annot_rect = annot_rect_f.GetOutterRect();
      annot_rect.Intersect(*clip_rect);
      if (annot_rect.IsEmpty()) {
        continue;
      }
    }
    if (pContext) {
      pAnnot->DrawInContext(pPage, pContext, &matrix, CPDF_Annot::Normal);
    } else if (!pAnnot->DrawAppearance(pPage, pDevice, &matrix,
                                       CPDF_Annot::Normal, pOptions)) {
      pAnnot->DrawBorder(pDevice, &matrix, pOptions);
    }
  }
}

void CPDF_AnnotList::DisplayAnnots(CPDF_Page* pPage,
                                   CFX_RenderDevice* pDevice,
                                   CPDF_RenderContext* pContext,
                                   FX_BOOL bPrinting,
                                   CFX_Matrix* pUser2Device,
                                   uint32_t dwAnnotFlags,
                                   CPDF_RenderOptions* pOptions,
                                   FX_RECT* pClipRect) {
  if (dwAnnotFlags & ANNOTFLAG_INVISIBLE) {
    DisplayPass(pPage, pDevice, pContext, bPrinting, pUser2Device, FALSE,
                pOptions, pClipRect);
  }
  if (dwAnnotFlags & ANNOTFLAG_HIDDEN) {
    DisplayPass(pPage, pDevice, pContext, bPrinting, pUser2Device, TRUE,
                pOptions, pClipRect);
  }
}

void CPDF_AnnotList::DisplayAnnots(CPDF_Page* pPage,
                                   CPDF_RenderContext* pContext,
                                   FX_BOOL bPrinting,
                                   CFX_Matrix* pMatrix,
                                   FX_BOOL bShowWidget,
                                   CPDF_RenderOptions* pOptions) {
  uint32_t dwAnnotFlags = bShowWidget ? ANNOTFLAG_INVISIBLE | ANNOTFLAG_HIDDEN
                                      : ANNOTFLAG_INVISIBLE;
  DisplayAnnots(pPage, nullptr, pContext, bPrinting, pMatrix, dwAnnotFlags,
                pOptions, nullptr);
}

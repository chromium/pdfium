// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fpdfapi/fpdf_pageobj.h"
#include "../../include/fpdfdoc/fpdf_doc.h"
#include "third_party/base/nonstd_unique_ptr.h"

CPDF_AnnotList::CPDF_AnnotList(CPDF_Page* pPage)
    : m_pDocument(pPage->m_pDocument) {
  if (!pPage->m_pFormDict)
    return;

  CPDF_Array* pAnnots = pPage->m_pFormDict->GetArray("Annots");
  if (!pAnnots)
    return;

  CPDF_Dictionary* pRoot = m_pDocument->GetRoot();
  CPDF_Dictionary* pAcroForm = pRoot->GetDict("AcroForm");
  FX_BOOL bRegenerateAP = pAcroForm && pAcroForm->GetBoolean("NeedAppearances");
  for (FX_DWORD i = 0; i < pAnnots->GetCount(); ++i) {
    CPDF_Dictionary* pDict = ToDictionary(pAnnots->GetElementValue(i));
    if (!pDict)
      continue;

    FX_DWORD dwObjNum = pDict->GetObjNum();
    if (dwObjNum == 0) {
      dwObjNum = m_pDocument->AddIndirectObject(pDict);
      CPDF_Reference* pAction = new CPDF_Reference(m_pDocument, dwObjNum);
      pAnnots->InsertAt(i, pAction);
      pAnnots->RemoveAt(i + 1);
      pDict = pAnnots->GetDict(i);
    }
    m_AnnotList.push_back(new CPDF_Annot(pDict, this));
    if (bRegenerateAP &&
        pDict->GetConstString(FX_BSTRC("Subtype")) == FX_BSTRC("Widget") &&
        CPDF_InterForm::UpdatingAPEnabled()) {
      FPDF_GenerateAP(m_pDocument, pDict);
    }
  }
}

CPDF_AnnotList::~CPDF_AnnotList() {
  for (CPDF_Annot* annot : m_AnnotList)
    delete annot;
}

void CPDF_AnnotList::DisplayPass(const CPDF_Page* pPage,
                                 CFX_RenderDevice* pDevice,
                                 CPDF_RenderContext* pContext,
                                 FX_BOOL bPrinting,
                                 CFX_AffineMatrix* pMatrix,
                                 FX_BOOL bWidgetPass,
                                 CPDF_RenderOptions* pOptions,
                                 FX_RECT* clip_rect) {
  for (CPDF_Annot* pAnnot : m_AnnotList) {
    FX_BOOL bWidget = pAnnot->GetSubType() == "Widget";
    if ((bWidgetPass && !bWidget) || (!bWidgetPass && bWidget))
      continue;

    FX_DWORD annot_flags = pAnnot->GetFlags();
    if (annot_flags & ANNOTFLAG_HIDDEN)
      continue;

    if (bPrinting && (annot_flags & ANNOTFLAG_PRINT) == 0)
      continue;

    if (!bPrinting && (annot_flags & ANNOTFLAG_NOVIEW))
      continue;

    if (pOptions) {
      IPDF_OCContext* pOCContext = pOptions->m_pOCContext;
      CPDF_Dictionary* pAnnotDict = pAnnot->GetAnnotDict();
      if (pOCContext && pAnnotDict &&
          !pOCContext->CheckOCGVisible(pAnnotDict->GetDict(FX_BSTRC("OC")))) {
        continue;
      }
    }
    CPDF_Rect annot_rect_f;
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

void CPDF_AnnotList::DisplayAnnots(const CPDF_Page* pPage,
                                   CFX_RenderDevice* pDevice,
                                   CPDF_RenderContext* pContext,
                                   FX_BOOL bPrinting,
                                   CFX_AffineMatrix* pUser2Device,
                                   FX_DWORD dwAnnotFlags,
                                   CPDF_RenderOptions* pOptions,
                                   FX_RECT* pClipRect) {
  if (dwAnnotFlags & 0x01) {
    DisplayPass(pPage, pDevice, pContext, bPrinting, pUser2Device, FALSE,
                pOptions, pClipRect);
  }
  if (dwAnnotFlags & 0x02) {
    DisplayPass(pPage, pDevice, pContext, bPrinting, pUser2Device, TRUE,
                pOptions, pClipRect);
  }
}

CPDF_Annot::CPDF_Annot(CPDF_Dictionary* pDict, CPDF_AnnotList* pList)
    : m_pAnnotDict(pDict),
      m_pList(pList),
      m_sSubtype(m_pAnnotDict->GetConstString(FX_BSTRC("Subtype"))) {}
CPDF_Annot::~CPDF_Annot() {
  ClearCachedAP();
}
void CPDF_Annot::ClearCachedAP() {
  FX_POSITION pos = m_APMap.GetStartPosition();
  while (pos) {
    void* pForm;
    void* pObjects;
    m_APMap.GetNextAssoc(pos, pForm, pObjects);
    delete (CPDF_PageObjects*)pObjects;
  }
  m_APMap.RemoveAll();
}
CFX_ByteString CPDF_Annot::GetSubType() const {
  return m_sSubtype;
}

void CPDF_Annot::GetRect(CPDF_Rect& rect) const {
  if (m_pAnnotDict == NULL) {
    return;
  }
  rect = m_pAnnotDict->GetRect("Rect");
  rect.Normalize();
}

FX_DWORD CPDF_Annot::GetFlags() const {
  return m_pAnnotDict->GetInteger("F");
}

CPDF_Stream* FPDFDOC_GetAnnotAP(CPDF_Dictionary* pAnnotDict,
                                CPDF_Annot::AppearanceMode mode) {
  CPDF_Dictionary* pAP = pAnnotDict->GetDict("AP");
  if (pAP == NULL) {
    return NULL;
  }
  const FX_CHAR* ap_entry = "N";
  if (mode == CPDF_Annot::Down)
    ap_entry = "D";
  else if (mode == CPDF_Annot::Rollover)
    ap_entry = "R";
  if (!pAP->KeyExist(ap_entry))
    ap_entry = "N";

  CPDF_Object* psub = pAP->GetElementValue(ap_entry);
  if (!psub)
    return nullptr;
  if (CPDF_Stream* pStream = psub->AsStream())
    return pStream;

  if (CPDF_Dictionary* pDict = psub->AsDictionary()) {
    CFX_ByteString as = pAnnotDict->GetString("AS");
    if (as.IsEmpty()) {
      CFX_ByteString value = pAnnotDict->GetString(FX_BSTRC("V"));
      if (value.IsEmpty()) {
        CPDF_Dictionary* pDict = pAnnotDict->GetDict(FX_BSTRC("Parent"));
        value = pDict ? pDict->GetString(FX_BSTRC("V")) : CFX_ByteString();
      }
      if (value.IsEmpty() || !pDict->KeyExist(value))
        as = FX_BSTRC("Off");
      else
        as = value;
    }
    return pDict->GetStream(as);
  }
  return nullptr;
}

CPDF_Form* CPDF_Annot::GetAPForm(const CPDF_Page* pPage, AppearanceMode mode) {
  CPDF_Stream* pStream = FPDFDOC_GetAnnotAP(m_pAnnotDict, mode);
  if (!pStream)
    return nullptr;

  void* pForm;
  if (m_APMap.Lookup(pStream, pForm))
    return static_cast<CPDF_Form*>(pForm);

  CPDF_Form* pNewForm =
      new CPDF_Form(m_pList->GetDocument(), pPage->m_pResources, pStream);
  pNewForm->ParseContent(nullptr, nullptr, nullptr, nullptr);
  m_APMap.SetAt(pStream, pNewForm);
  return pNewForm;
}

static CPDF_Form* FPDFDOC_Annot_GetMatrix(const CPDF_Page* pPage,
                                          CPDF_Annot* pAnnot,
                                          CPDF_Annot::AppearanceMode mode,
                                          const CFX_AffineMatrix* pUser2Device,
                                          CFX_Matrix& matrix) {
  CPDF_Form* pForm = pAnnot->GetAPForm(pPage, mode);
  if (!pForm) {
    return NULL;
  }
  CFX_FloatRect form_bbox = pForm->m_pFormDict->GetRect(FX_BSTRC("BBox"));
  CFX_Matrix form_matrix = pForm->m_pFormDict->GetMatrix(FX_BSTRC("Matrix"));
  form_matrix.TransformRect(form_bbox);
  CPDF_Rect arect;
  pAnnot->GetRect(arect);
  matrix.MatchRect(arect, form_bbox);
  matrix.Concat(*pUser2Device);
  return pForm;
}
FX_BOOL CPDF_Annot::DrawAppearance(const CPDF_Page* pPage,
                                   CFX_RenderDevice* pDevice,
                                   const CFX_AffineMatrix* pUser2Device,
                                   AppearanceMode mode,
                                   const CPDF_RenderOptions* pOptions) {
  CFX_Matrix matrix;
  CPDF_Form* pForm =
      FPDFDOC_Annot_GetMatrix(pPage, this, mode, pUser2Device, matrix);
  if (!pForm) {
    return FALSE;
  }
  CPDF_RenderContext context;
  context.Create((CPDF_Page*)pPage);
  context.DrawObjectList(pDevice, pForm, &matrix, pOptions);
  return TRUE;
}
FX_BOOL CPDF_Annot::DrawInContext(const CPDF_Page* pPage,
                                  const CPDF_RenderContext* pContext,
                                  const CFX_AffineMatrix* pUser2Device,
                                  AppearanceMode mode) {
  CFX_Matrix matrix;
  CPDF_Form* pForm =
      FPDFDOC_Annot_GetMatrix(pPage, this, mode, pUser2Device, matrix);
  if (!pForm) {
    return FALSE;
  }
  ((CPDF_RenderContext*)pContext)->AppendObjectList(pForm, &matrix);
  return TRUE;
}
void CPDF_Annot::DrawBorder(CFX_RenderDevice* pDevice,
                            const CFX_AffineMatrix* pUser2Device,
                            const CPDF_RenderOptions* pOptions) {
  if (GetSubType() == "Popup") {
    return;
  }
  FX_DWORD annot_flags = GetFlags();
  if (annot_flags & ANNOTFLAG_HIDDEN) {
    return;
  }
  FX_BOOL bPrinting = pDevice->GetDeviceClass() == FXDC_PRINTER ||
                      (pOptions && (pOptions->m_Flags & RENDER_PRINTPREVIEW));
  if (bPrinting && (annot_flags & ANNOTFLAG_PRINT) == 0) {
    return;
  }
  if (!bPrinting && (annot_flags & ANNOTFLAG_NOVIEW)) {
    return;
  }
  CPDF_Dictionary* pBS = m_pAnnotDict->GetDict("BS");
  char style_char;
  FX_FLOAT width;
  CPDF_Array* pDashArray = NULL;
  if (pBS == NULL) {
    CPDF_Array* pBorderArray = m_pAnnotDict->GetArray("Border");
    style_char = 'S';
    if (pBorderArray) {
      width = pBorderArray->GetNumber(2);
      if (pBorderArray->GetCount() == 4) {
        pDashArray = pBorderArray->GetArray(3);
        if (pDashArray == NULL) {
          return;
        }
        int nLen = pDashArray->GetCount();
        int i = 0;
        for (; i < nLen; ++i) {
          CPDF_Object* pObj = pDashArray->GetElementValue(i);
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
    CFX_ByteString style = pBS->GetString("S");
    pDashArray = pBS->GetArray("D");
    style_char = style[1];
    width = pBS->GetNumber("W");
  }
  if (width <= 0) {
    return;
  }
  CPDF_Array* pColor = m_pAnnotDict->GetArray("C");
  FX_DWORD argb = 0xff000000;
  if (pColor != NULL) {
    int R = (int32_t)(pColor->GetNumber(0) * 255);
    int G = (int32_t)(pColor->GetNumber(1) * 255);
    int B = (int32_t)(pColor->GetNumber(2) * 255);
    argb = ArgbEncode(0xff, R, G, B);
  }
  CPDF_GraphStateData graph_state;
  graph_state.m_LineWidth = width;
  if (style_char == 'D') {
    if (pDashArray) {
      FX_DWORD dash_count = pDashArray->GetCount();
      if (dash_count % 2) {
        dash_count++;
      }
      graph_state.m_DashArray = FX_Alloc(FX_FLOAT, dash_count);
      graph_state.m_DashCount = dash_count;
      FX_DWORD i;
      for (i = 0; i < pDashArray->GetCount(); ++i) {
        graph_state.m_DashArray[i] = pDashArray->GetNumber(i);
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
  CPDF_PathData path;
  width /= 2;
  path.AppendRect(rect.left + width, rect.bottom + width, rect.right - width,
                  rect.top - width);
  int fill_type = 0;
  if (pOptions && (pOptions->m_Flags & RENDER_NOPATHSMOOTH)) {
    fill_type |= FXFILL_NOPATHSMOOTH;
  }
  pDevice->DrawPath(&path, pUser2Device, &graph_state, argb, argb, fill_type);
}

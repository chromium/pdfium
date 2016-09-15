// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_PAGEVIEW_H_
#define FPDFSDK_INCLUDE_CPDFSDK_PAGEVIEW_H_

#include <memory>
#include <vector>

#include "core/fpdfapi/fpdf_page/include/cpdf_page.h"
#include "core/fxcrt/include/fx_system.h"
#include "fpdfsdk/include/cpdfsdk_document.h"

class CFX_RenderDevice;
class CPDF_AnnotList;
class CPDF_RenderOptions;

class CPDFSDK_PageView final : public CPDF_Page::View {
 public:
  CPDFSDK_PageView(CPDFSDK_Document* pSDKDoc, UnderlyingPageType* page);
  ~CPDFSDK_PageView();

#ifdef PDF_ENABLE_XFA
  void PageView_OnDraw(CFX_RenderDevice* pDevice,
                       CFX_Matrix* pUser2Device,
                       CPDF_RenderOptions* pOptions,
                       const FX_RECT& pClip);
#else   // PDF_ENABLE_XFA
  void PageView_OnDraw(CFX_RenderDevice* pDevice,
                       CFX_Matrix* pUser2Device,
                       CPDF_RenderOptions* pOptions);
#endif  // PDF_ENABLE_XFA

  const CPDF_Annot* GetPDFAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  CPDFSDK_Annot* GetFXAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  const CPDF_Annot* GetPDFWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  CPDFSDK_Annot* GetFXWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  CPDFSDK_Annot* GetFocusAnnot();
  void SetFocusAnnot(CPDFSDK_Annot* pSDKAnnot, uint32_t nFlag = 0) {
    m_pSDKDoc->SetFocusAnnot(pSDKAnnot, nFlag);
  }
  FX_BOOL KillFocusAnnot(uint32_t nFlag = 0) {
    return m_pSDKDoc->KillFocusAnnot(nFlag);
  }
  void KillFocusAnnotIfNeeded();

  CPDFSDK_Annot* AddAnnot(CPDF_Dictionary* pDict);
  CPDFSDK_Annot* AddAnnot(const FX_CHAR* lpSubType, CPDF_Dictionary* pDict);
  CPDFSDK_Annot* AddAnnot(CPDF_Annot* pPDFAnnot);

  FX_BOOL DeleteAnnot(CPDFSDK_Annot* pAnnot);
  size_t CountAnnots() const;
  CPDFSDK_Annot* GetAnnot(size_t nIndex);
  CPDFSDK_Annot* GetAnnotByDict(CPDF_Dictionary* pDict);

#ifdef PDF_ENABLE_XFA
  CPDFSDK_Annot* AddAnnot(CXFA_FFWidget* pPDFAnnot);
  CPDFSDK_Annot* GetAnnotByXFAWidget(CXFA_FFWidget* hWidget);
  CPDFXFA_Page* GetPDFXFAPage() { return m_page; }
#endif  // PDF_ENABLE_XFA

  CPDF_Page* GetPDFPage() const;
  CPDF_Document* GetPDFDocument();
  CPDFSDK_Document* GetSDKDocument() { return m_pSDKDoc; }
  FX_BOOL OnLButtonDown(const CFX_FloatPoint& point, uint32_t nFlag);
  FX_BOOL OnLButtonUp(const CFX_FloatPoint& point, uint32_t nFlag);
#ifdef PDF_ENABLE_XFA
  FX_BOOL OnRButtonDown(const CFX_FloatPoint& point, uint32_t nFlag);
  FX_BOOL OnRButtonUp(const CFX_FloatPoint& point, uint32_t nFlag);
#endif  // PDF_ENABLE_XFA
  FX_BOOL OnChar(int nChar, uint32_t nFlag);
  FX_BOOL OnKeyDown(int nKeyCode, int nFlag);
  FX_BOOL OnKeyUp(int nKeyCode, int nFlag);

  FX_BOOL OnMouseMove(const CFX_FloatPoint& point, int nFlag);
  FX_BOOL OnMouseWheel(double deltaX,
                       double deltaY,
                       const CFX_FloatPoint& point,
                       int nFlag);
  bool IsValidAnnot(const CPDF_Annot* p) const;
  void GetCurrentMatrix(CFX_Matrix& matrix) { matrix = m_curMatrix; }
  void UpdateRects(const std::vector<CFX_FloatRect>& rects);
  void UpdateView(CPDFSDK_Annot* pAnnot);
  const std::vector<CPDFSDK_Annot*>& GetAnnotList() const {
    return m_fxAnnotArray;
  }

  int GetPageIndex() const;
  void LoadFXAnnots();
  void ClearFXAnnots();
  void SetValid(FX_BOOL bValid) { m_bValid = bValid; }
  FX_BOOL IsValid() { return m_bValid; }
  void SetLock(FX_BOOL bLocked) { m_bLocked = bLocked; }
  FX_BOOL IsLocked() { return m_bLocked; }
#ifndef PDF_ENABLE_XFA
  bool OwnsPage() const { return m_bOwnsPage; }
  void TakePageOwnership() { m_bOwnsPage = true; }
#endif  // PDF_ENABLE_XFA

 private:
  int GetPageIndexForStaticPDF() const;

  CFX_Matrix m_curMatrix;
  UnderlyingPageType* const m_page;
  std::unique_ptr<CPDF_AnnotList> m_pAnnotList;
  std::vector<CPDFSDK_Annot*> m_fxAnnotArray;
  CPDFSDK_Document* const m_pSDKDoc;
  CPDFSDK_Annot* m_CaptureWidget;
#ifndef PDF_ENABLE_XFA
  bool m_bOwnsPage;
#endif  // PDF_ENABLE_XFA
  FX_BOOL m_bEnterWidget;
  FX_BOOL m_bExitWidget;
  FX_BOOL m_bOnWidget;
  FX_BOOL m_bValid;
  FX_BOOL m_bLocked;
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_PAGEVIEW_H_

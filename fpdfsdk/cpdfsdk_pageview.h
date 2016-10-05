// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_PAGEVIEW_H_
#define FPDFSDK_CPDFSDK_PAGEVIEW_H_

#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fxcrt/fx_system.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_document.h"

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

  CPDFSDK_Annot* GetFXAnnotAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);
  CPDFSDK_Annot* GetFXWidgetAtPoint(FX_FLOAT pageX, FX_FLOAT pageY);

  void LoadFXAnnots();
  CPDFSDK_Annot* GetFocusAnnot();
  bool IsValidAnnot(const CPDF_Annot* p) const;
  bool IsValidSDKAnnot(const CPDFSDK_Annot* p) const;

  const std::vector<CPDFSDK_Annot*>& GetAnnotList() const {
    return m_SDKAnnotArray;
  }
  CPDFSDK_Annot* GetAnnotByDict(CPDF_Dictionary* pDict);

#ifdef PDF_ENABLE_XFA
  FX_BOOL DeleteAnnot(CPDFSDK_Annot* pAnnot);
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

  void GetCurrentMatrix(CFX_Matrix& matrix) { matrix = m_curMatrix; }
  void UpdateRects(const std::vector<CFX_FloatRect>& rects);
  void UpdateView(CPDFSDK_Annot* pAnnot);

  int GetPageIndex() const;

  void SetValid(FX_BOOL bValid) { m_bValid = bValid; }
  FX_BOOL IsValid() { return m_bValid; }

  void SetLock(FX_BOOL bLocked) { m_bLocked = bLocked; }
  FX_BOOL IsLocked() { return m_bLocked; }

  void SetBeingDestroyed() { m_bBeingDestroyed = true; }
  bool IsBeingDestroyed() const { return m_bBeingDestroyed; }

#ifndef PDF_ENABLE_XFA
  bool OwnsPage() const { return m_bOwnsPage; }
  void TakePageOwnership() { m_bOwnsPage = true; }
#endif  // PDF_ENABLE_XFA

 private:
  int GetPageIndexForStaticPDF() const;

  CFX_Matrix m_curMatrix;
  UnderlyingPageType* const m_page;
  std::unique_ptr<CPDF_AnnotList> m_pAnnotList;
  std::vector<CPDFSDK_Annot*> m_SDKAnnotArray;
  CPDFSDK_Document* const m_pSDKDoc;
  CPDFSDK_Annot::ObservedPtr m_pCaptureWidget;
#ifndef PDF_ENABLE_XFA
  bool m_bOwnsPage;
#endif  // PDF_ENABLE_XFA
  FX_BOOL m_bEnterWidget;
  FX_BOOL m_bExitWidget;
  FX_BOOL m_bOnWidget;
  FX_BOOL m_bValid;
  FX_BOOL m_bLocked;
  bool m_bBeingDestroyed;
};

#endif  // FPDFSDK_CPDFSDK_PAGEVIEW_H_

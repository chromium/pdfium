// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_PAGEVIEW_H_
#define FPDFSDK_CPDFSDK_PAGEVIEW_H_

#include <stdint.h>

#include <memory>
#include <vector>

#include "core/fpdfapi/page/cpdf_page.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_annothandlermgr.h"

class CFX_RenderDevice;
class CPDF_AnnotList;
class CPDF_RenderOptions;
class CPDFSDK_FormFillEnvironment;

class CPDFSDK_PageView final : public CPDF_Page::View {
 public:
  CPDFSDK_PageView(CPDFSDK_FormFillEnvironment* pFormFillEnv, IPDF_Page* page);
  ~CPDFSDK_PageView();

  void PageView_OnDraw(CFX_RenderDevice* pDevice,
                       const CFX_Matrix& mtUser2Device,
                       CPDF_RenderOptions* pOptions,
                       const FX_RECT& pClip);

  void LoadFXAnnots();
  CPDFSDK_Annot* GetFocusAnnot();
  bool IsValidAnnot(const CPDF_Annot* p) const;
  bool IsValidSDKAnnot(const CPDFSDK_Annot* p) const;

  const std::vector<CPDFSDK_Annot*>& GetAnnotList() const {
    return m_SDKAnnotArray;
  }
  CPDFSDK_Annot* GetAnnotByDict(CPDF_Dictionary* pDict);

#ifdef PDF_ENABLE_XFA
  bool DeleteAnnot(CPDFSDK_Annot* pAnnot);
  CPDFSDK_Annot* AddAnnot(CXFA_FFWidget* pPDFAnnot);
  CPDFSDK_Annot* GetAnnotByXFAWidget(CXFA_FFWidget* pWidget);
  IPDF_Page* GetXFAPage();
#endif  // PDF_ENABLE_XFA

  CPDF_Page* GetPDFPage() const;
  CPDF_Document* GetPDFDocument();
  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const {
    return m_pFormFillEnv.Get();
  }

  WideString GetFocusedFormText();
  WideString GetSelectedText();
  void ReplaceSelection(const WideString& text);
  bool SelectAllText();

  bool CanUndo();
  bool CanRedo();
  bool Undo();
  bool Redo();

  bool OnFocus(uint32_t nFlag, const CFX_PointF& point);
  bool OnLButtonDown(uint32_t nFlag, const CFX_PointF& point);
  bool OnLButtonUp(uint32_t nFlag, const CFX_PointF& point);
  bool OnLButtonDblClk(uint32_t nFlag, const CFX_PointF& point);
  bool OnRButtonDown(uint32_t nFlag, const CFX_PointF& point);
  bool OnRButtonUp(uint32_t nFlag, const CFX_PointF& point);
  bool OnChar(int nChar, uint32_t nFlag);
  bool OnKeyDown(int nKeyCode, int nFlag);
  bool OnKeyUp(int nKeyCode, int nFlag);
  bool OnMouseMove(int nFlag, const CFX_PointF& point);
  bool OnMouseWheel(int nFlag,
                    const CFX_PointF& point,
                    const CFX_Vector& delta);

  bool SetIndexSelected(int index, bool selected);
  bool IsIndexSelected(int index);

  const CFX_Matrix& GetCurrentMatrix() const { return m_curMatrix; }
  void UpdateRects(const std::vector<CFX_FloatRect>& rects);
  void UpdateView(CPDFSDK_Annot* pAnnot);

  int GetPageIndex() const;

  void SetValid(bool bValid) { m_bValid = bValid; }
  bool IsValid() const { return m_bValid; }
  bool IsLocked() const { return m_bLocked; }
  void SetBeingDestroyed() { m_bBeingDestroyed = true; }
  bool IsBeingDestroyed() const { return m_bBeingDestroyed; }
  void TakePageOwnership() { m_pOwnsPage.Reset(ToPDFPage(m_page)); }

 private:
  CPDFSDK_Annot* GetFXAnnotAtPoint(const CFX_PointF& point);
  CPDFSDK_Annot* GetFXWidgetAtPoint(const CFX_PointF& point);

  int GetPageIndexForStaticPDF() const;

  void EnterWidget(CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr,
                   ObservedPtr<CPDFSDK_Annot>* pAnnot,
                   uint32_t nFlag);
  void ExitWidget(CPDFSDK_AnnotHandlerMgr* pAnnotHandlerMgr,
                  bool callExitCallback,
                  uint32_t nFlag);

  CFX_Matrix m_curMatrix;
  UnownedPtr<IPDF_Page> const m_page;
  std::unique_ptr<CPDF_AnnotList> m_pAnnotList;
  std::vector<CPDFSDK_Annot*> m_SDKAnnotArray;
  UnownedPtr<CPDFSDK_FormFillEnvironment> const m_pFormFillEnv;
  ObservedPtr<CPDFSDK_Annot> m_pCaptureWidget;
  RetainPtr<CPDF_Page> m_pOwnsPage;
  bool m_bOnWidget = false;
  bool m_bValid = false;
  bool m_bLocked = false;
  bool m_bBeingDestroyed = false;
};

#endif  // FPDFSDK_CPDFSDK_PAGEVIEW_H_

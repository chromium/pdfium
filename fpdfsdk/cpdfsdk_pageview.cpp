// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_pageview.h"

#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfdoc/cpdf_annotlist.h"
#include "core/fpdfdoc/cpdf_interactiveform.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/containers/unique_ptr_adapters.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_annotiteration.h"
#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_interactiveform.h"

#ifdef PDF_ENABLE_XFA
#include "fpdfsdk/fpdfxfa/cpdfxfa_page.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_widget.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#endif  // PDF_ENABLE_XFA

CPDFSDK_PageView::CPDFSDK_PageView(CPDFSDK_FormFillEnvironment* pFormFillEnv,
                                   IPDF_Page* page)
    : page_(page), form_fill_env_(pFormFillEnv) {
  DCHECK(page_);
  CPDF_Page* pPDFPage = ToPDFPage(page);
  if (pPDFPage) {
    CPDFSDK_InteractiveForm* pForm = pFormFillEnv->GetInteractiveForm();
    CPDF_InteractiveForm* pPDFForm = pForm->GetInteractiveForm();
    pPDFForm->FixPageFields(pPDFPage);
    if (!page->AsXFAPage()) {
      pPDFPage->SetView(this);
    }
  }
}

CPDFSDK_PageView::~CPDFSDK_PageView() {
  if (!page_->AsXFAPage()) {
    // Deleting from `sdkannot_array_` below can cause the page pointed to by
    // `page_` to be freed, which will cause issues if we try to cleanup the
    // pageview pointer in `page_`. So, reset the pageview pointer before doing
    // anything else.
    page_->AsPDFPage()->SetView(nullptr);
  }

  // Manually reset elements to ensure they are deleted in order.
  for (std::unique_ptr<CPDFSDK_Annot>& pAnnot : sdkannot_array_) {
    pAnnot.reset();
  }

  sdkannot_array_.clear();
  annot_list_.reset();
}

void CPDFSDK_PageView::ClearPage(CPDF_Page* pPage) {
  if (!IsBeingDestroyed()) {
    GetFormFillEnv()->RemovePageView(pPage);
  }
}

void CPDFSDK_PageView::PageView_OnDraw(CFX_RenderDevice* pDevice,
                                       const CFX_Matrix& mtUser2Device,
                                       CPDF_RenderOptions* pOptions,
                                       const FX_RECT& pClip) {
  matrix_ = mtUser2Device;

#ifdef PDF_ENABLE_XFA
  IPDF_Page* pPage = GetXFAPage();
  CPDF_Document::Extension* pContext =
      pPage ? pPage->GetDocument()->GetExtension() : nullptr;
  if (pContext && pContext->ContainsExtensionFullForm()) {
    static_cast<CPDFXFA_Page*>(pPage)->DrawFocusAnnot(pDevice, GetFocusAnnot(),
                                                      mtUser2Device, pClip);
    return;
  }
#endif  // PDF_ENABLE_XFA

  // for pdf/static xfa.
  auto annot_iteration = CPDFSDK_AnnotIteration::CreateForDrawing(this);
  for (const auto& pSDKAnnot : annot_iteration) {
    pSDKAnnot->OnDraw(pDevice, mtUser2Device, pOptions->GetDrawAnnots());
  }
}

std::unique_ptr<CPDFSDK_Annot> CPDFSDK_PageView::NewAnnot(CPDF_Annot* annot) {
  const CPDF_Annot::Subtype sub_type = annot->GetSubtype();
  if (sub_type == CPDF_Annot::Subtype::WIDGET) {
    CPDFSDK_InteractiveForm* form = form_fill_env_->GetInteractiveForm();
    CPDF_InteractiveForm* pdf_form = form->GetInteractiveForm();
    CPDF_FormControl* form_control =
        pdf_form->GetControlByDict(annot->GetAnnotDict());
    if (!form_control) {
      return nullptr;
    }

    auto ret = std::make_unique<CPDFSDK_Widget>(annot, this, form);
    form->AddMap(form_control, ret.get());
    if (pdf_form->NeedConstructAP()) {
      ret->ResetAppearance(std::nullopt, CPDFSDK_Widget::kValueUnchanged);
    }
    return ret;
  }

#ifdef PDF_ENABLE_XFA
  if (sub_type == CPDF_Annot::Subtype::XFAWIDGET) {
    return nullptr;
  }
#endif  // PDF_ENABLE_XFA

  return std::make_unique<CPDFSDK_BAAnnot>(annot, this);
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFXAnnotAtPoint(const CFX_PointF& point) {
  CPDFSDK_AnnotIteration annot_iteration(this);
  for (const auto& pSDKAnnot : annot_iteration) {
    CFX_FloatRect rc = pSDKAnnot->GetViewBBox();
    if (pSDKAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::POPUP) {
      continue;
    }
    if (rc.Contains(point)) {
      return pSDKAnnot.Get();
    }
  }
  return nullptr;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFXWidgetAtPoint(const CFX_PointF& point) {
  CPDFSDK_AnnotIteration annot_iteration(this);
  for (const auto& pSDKAnnot : annot_iteration) {
    const CPDF_Annot::Subtype sub_type = pSDKAnnot->GetAnnotSubtype();
    bool do_hit_test = sub_type == CPDF_Annot::Subtype::WIDGET;
#ifdef PDF_ENABLE_XFA
    do_hit_test = do_hit_test || sub_type == CPDF_Annot::Subtype::XFAWIDGET;
#endif  // PDF_ENABLE_XFA
    if (do_hit_test && pSDKAnnot->DoHitTest(point)) {
      return pSDKAnnot.Get();
    }
  }
  return nullptr;
}

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_PageView::AddAnnotForFFWidget(CXFA_FFWidget* pWidget) {
  CPDFSDK_Annot* pSDKAnnot = GetAnnotForFFWidget(pWidget);
  if (pSDKAnnot) {
    return pSDKAnnot;
  }

  sdkannot_array_.push_back(std::make_unique<CPDFXFA_Widget>(pWidget, this));
  return sdkannot_array_.back().get();
}

void CPDFSDK_PageView::DeleteAnnotForFFWidget(CXFA_FFWidget* pWidget) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetAnnotForFFWidget(pWidget));
  if (!pAnnot) {
    return;
  }
  IPDF_Page* pPage = pAnnot->GetXFAPage();
  if (!pPage) {
    return;
  }
  CPDF_Document::Extension* pContext = pPage->GetDocument()->GetExtension();
  if (pContext && !pContext->ContainsExtensionForm()) {
    return;
  }
  if (GetFocusAnnot() == pAnnot) {
    // May invoke JS, invalidating pAnnot.
    form_fill_env_->KillFocusAnnot({});
  }
  if (pAnnot) {
    auto it = std::ranges::find_if(sdkannot_array_,
                                   pdfium::MatchesUniquePtr(pAnnot.Get()));
    if (it != sdkannot_array_.end()) {
      sdkannot_array_.erase(it);
    }
  }
  if (capture_widget_.Get() == pAnnot) {
    capture_widget_.Reset();
  }
}

CPDFXFA_Page* CPDFSDK_PageView::XFAPageIfNotBackedByPDFPage() {
  auto* pPage = static_cast<CPDFXFA_Page*>(GetXFAPage());
  return pPage && !pPage->AsPDFPage() ? pPage : nullptr;
}
#endif  // PDF_ENABLE_XFA

CPDF_Document* CPDFSDK_PageView::GetPDFDocument() {
  return page_->GetDocument();
}

CPDF_Page* CPDFSDK_PageView::GetPDFPage() const {
  return ToPDFPage(page_);
}

CPDFSDK_InteractiveForm* CPDFSDK_PageView::GetInteractiveForm() const {
  return form_fill_env_->GetInteractiveForm();
}

std::vector<CPDFSDK_Annot*> CPDFSDK_PageView::GetAnnotList() const {
  std::vector<CPDFSDK_Annot*> list;
  list.reserve(sdkannot_array_.size());
  for (const std::unique_ptr<CPDFSDK_Annot>& elem : sdkannot_array_) {
    list.push_back(elem.get());
  }
  return list;
}

CPDFSDK_Annot* CPDFSDK_PageView::GetAnnotByDict(const CPDF_Dictionary* pDict) {
  for (std::unique_ptr<CPDFSDK_Annot>& pAnnot : sdkannot_array_) {
    CPDF_Annot* pPDFAnnot = pAnnot->GetPDFAnnot();
    if (pPDFAnnot && pPDFAnnot->GetAnnotDict() == pDict) {
      return pAnnot.get();
    }
  }
  return nullptr;
}

#ifdef PDF_ENABLE_XFA
CPDFSDK_Annot* CPDFSDK_PageView::GetAnnotForFFWidget(CXFA_FFWidget* pWidget) {
  if (!pWidget) {
    return nullptr;
  }

  for (std::unique_ptr<CPDFSDK_Annot>& pAnnot : sdkannot_array_) {
    CPDFXFA_Widget* pCurrentWidget = pAnnot->AsXFAWidget();
    if (pCurrentWidget && pCurrentWidget->GetXFAFFWidget() == pWidget) {
      return pAnnot.get();
    }
  }
  return nullptr;
}

IPDF_Page* CPDFSDK_PageView::GetXFAPage() {
  return ToXFAPage(page_);
}
#endif  // PDF_ENABLE_XFA

WideString CPDFSDK_PageView::GetFocusedFormText() {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  return annot ? annot->GetText() : WideString();
}

CPDFSDK_Annot* CPDFSDK_PageView::GetNextAnnot(CPDFSDK_Annot* pAnnot) {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = XFAPageIfNotBackedByPDFPage();
  if (pXFAPage) {
    return pXFAPage->GetNextXFAAnnot(pAnnot);
  }
#endif  // PDF_ENABLE_XFA
  CPDFSDK_AnnotIterator ai(this, GetFormFillEnv()->GetFocusableAnnotSubtypes());
  return ai.GetNextAnnot(pAnnot);
}

CPDFSDK_Annot* CPDFSDK_PageView::GetPrevAnnot(CPDFSDK_Annot* pAnnot) {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = XFAPageIfNotBackedByPDFPage();
  if (pXFAPage) {
    return pXFAPage->GetPrevXFAAnnot(pAnnot);
  }
#endif  // PDF_ENABLE_XFA
  CPDFSDK_AnnotIterator ai(this, GetFormFillEnv()->GetFocusableAnnotSubtypes());
  return ai.GetPrevAnnot(pAnnot);
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFirstFocusableAnnot() {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = XFAPageIfNotBackedByPDFPage();
  if (pXFAPage) {
    return pXFAPage->GetFirstXFAAnnot(this);
  }
#endif  // PDF_ENABLE_XFA
  CPDFSDK_AnnotIterator ai(this, GetFormFillEnv()->GetFocusableAnnotSubtypes());
  return ai.GetFirstAnnot();
}

CPDFSDK_Annot* CPDFSDK_PageView::GetLastFocusableAnnot() {
#ifdef PDF_ENABLE_XFA
  CPDFXFA_Page* pXFAPage = XFAPageIfNotBackedByPDFPage();
  if (pXFAPage) {
    return pXFAPage->GetLastXFAAnnot(this);
  }
#endif  // PDF_ENABLE_XFA
  CPDFSDK_AnnotIterator ai(this, GetFormFillEnv()->GetFocusableAnnotSubtypes());
  return ai.GetLastAnnot();
}

WideString CPDFSDK_PageView::GetSelectedText() {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  if (!annot) {
    return WideString();
  }
  return annot->GetSelectedText();
}

void CPDFSDK_PageView::ReplaceAndKeepSelection(const WideString& text) {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  if (annot) {
    annot->ReplaceAndKeepSelection(text);
  }
}

void CPDFSDK_PageView::ReplaceSelection(const WideString& text) {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  if (annot) {
    annot->ReplaceSelection(text);
  }
}

bool CPDFSDK_PageView::SelectAllText() {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  return annot && annot->SelectAllText();
}

bool CPDFSDK_PageView::CanUndo() {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  return annot && annot->CanUndo();
}

bool CPDFSDK_PageView::CanRedo() {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  return annot && annot->CanRedo();
}

bool CPDFSDK_PageView::Undo() {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  return annot && annot->Undo();
}

bool CPDFSDK_PageView::Redo() {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  return annot && annot->Redo();
}

bool CPDFSDK_PageView::OnFocus(Mask<FWL_EVENTFLAG> nFlags,
                               const CFX_PointF& point) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetFXWidgetAtPoint(point));
  if (!pAnnot) {
    form_fill_env_->KillFocusAnnot(nFlags);
    return false;
  }

  form_fill_env_->SetFocusAnnot(pAnnot);
  return true;
}

bool CPDFSDK_PageView::OnLButtonDown(Mask<FWL_EVENTFLAG> nFlags,
                                     const CFX_PointF& point) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetFXWidgetAtPoint(point));
  if (!pAnnot) {
    form_fill_env_->KillFocusAnnot(nFlags);
    return false;
  }

  if (!CPDFSDK_Annot::OnLButtonDown(pAnnot, nFlags, point)) {
    return false;
  }

  if (!pAnnot) {
    return false;
  }

  form_fill_env_->SetFocusAnnot(pAnnot);
  return true;
}

bool CPDFSDK_PageView::OnLButtonUp(Mask<FWL_EVENTFLAG> nFlags,
                                   const CFX_PointF& point) {
  ObservedPtr<CPDFSDK_Annot> pFXAnnot(GetFXWidgetAtPoint(point));
  ObservedPtr<CPDFSDK_Annot> pFocusAnnot(GetFocusAnnot());
  if (pFocusAnnot && pFocusAnnot != pFXAnnot) {
    // Last focus Annot gets a chance to handle the event.
    if (CPDFSDK_Annot::OnLButtonUp(pFocusAnnot, nFlags, point)) {
      return true;
    }
  }
  return pFXAnnot && CPDFSDK_Annot::OnLButtonUp(pFXAnnot, nFlags, point);
}

bool CPDFSDK_PageView::OnLButtonDblClk(Mask<FWL_EVENTFLAG> nFlags,
                                       const CFX_PointF& point) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetFXWidgetAtPoint(point));
  if (!pAnnot) {
    form_fill_env_->KillFocusAnnot(nFlags);
    return false;
  }

  if (!CPDFSDK_Annot::OnLButtonDblClk(pAnnot, nFlags, point)) {
    return false;
  }

  if (!pAnnot) {
    return false;
  }

  form_fill_env_->SetFocusAnnot(pAnnot);
  return true;
}

bool CPDFSDK_PageView::OnRButtonDown(Mask<FWL_EVENTFLAG> nFlags,
                                     const CFX_PointF& point) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetFXWidgetAtPoint(point));
  if (!pAnnot) {
    return false;
  }

  bool ok = CPDFSDK_Annot::OnRButtonDown(pAnnot, nFlags, point);
  if (!pAnnot) {
    return false;
  }

  if (ok) {
    form_fill_env_->SetFocusAnnot(pAnnot);
  }

  return true;
}

bool CPDFSDK_PageView::OnRButtonUp(Mask<FWL_EVENTFLAG> nFlags,
                                   const CFX_PointF& point) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetFXWidgetAtPoint(point));
  if (!pAnnot) {
    return false;
  }

  bool ok = CPDFSDK_Annot::OnRButtonUp(pAnnot, nFlags, point);
  if (!pAnnot) {
    return false;
  }

  if (ok) {
    form_fill_env_->SetFocusAnnot(pAnnot);
  }

  return true;
}

bool CPDFSDK_PageView::OnMouseMove(Mask<FWL_EVENTFLAG> nFlags,
                                   const CFX_PointF& point) {
  ObservedPtr<CPDFSDK_Annot> pFXAnnot(GetFXAnnotAtPoint(point));
  ObservedPtr<CPDFSDK_PageView> pThis(this);

  if (pThis->on_widget_ && pThis->capture_widget_ != pFXAnnot) {
    pThis->ExitWidget(true, nFlags);
  }

  // ExitWidget() may have invalidated objects.
  if (!pThis || !pFXAnnot) {
    return false;
  }

  if (!pThis->on_widget_) {
    pThis->EnterWidget(pFXAnnot, nFlags);

    // EnterWidget() may have invalidated objects.
    if (!pThis) {
      return false;
    }

    if (!pFXAnnot) {
      pThis->ExitWidget(false, nFlags);
      return true;
    }
  }
  CPDFSDK_Annot::OnMouseMove(pFXAnnot, nFlags, point);
  return true;
}

void CPDFSDK_PageView::EnterWidget(ObservedPtr<CPDFSDK_Annot>& pAnnot,
                                   Mask<FWL_EVENTFLAG> nFlags) {
  on_widget_ = true;
  capture_widget_ = pAnnot;
  CPDFSDK_Annot::OnMouseEnter(capture_widget_, nFlags);
}

void CPDFSDK_PageView::ExitWidget(bool callExitCallback,
                                  Mask<FWL_EVENTFLAG> nFlags) {
  ObservedPtr<CPDFSDK_PageView> pThis(this);
  pThis->on_widget_ = false;
  if (!pThis->capture_widget_) {
    return;
  }

  if (callExitCallback) {
    CPDFSDK_Annot::OnMouseExit(pThis->capture_widget_, nFlags);

    // OnMouseExit() may have invalidated |this|.
    if (!pThis) {
      return;
    }
  }
  pThis->capture_widget_.Reset();
}

bool CPDFSDK_PageView::OnMouseWheel(Mask<FWL_EVENTFLAG> nFlags,
                                    const CFX_PointF& point,
                                    const CFX_Vector& delta) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetFXWidgetAtPoint(point));
  if (!pAnnot) {
    return false;
  }

  return CPDFSDK_Annot::OnMouseWheel(pAnnot, nFlags, point, delta);
}

bool CPDFSDK_PageView::SetIndexSelected(int index, bool selected) {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  return annot && annot->SetIndexSelected(index, selected);
}

bool CPDFSDK_PageView::IsIndexSelected(int index) {
  CPDFSDK_Annot* annot = GetFocusAnnot();
  return annot && annot->IsIndexSelected(index);
}

bool CPDFSDK_PageView::OnChar(uint32_t nChar, Mask<FWL_EVENTFLAG> nFlags) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetFocusAnnot());
  return pAnnot && CPDFSDK_Annot::OnChar(pAnnot, nChar, nFlags);
}

bool CPDFSDK_PageView::OnKeyDown(FWL_VKEYCODE nKeyCode,
                                 Mask<FWL_EVENTFLAG> nFlags) {
  ObservedPtr<CPDFSDK_Annot> pAnnot(GetFocusAnnot());
  if (!pAnnot) {
    // If pressed key is not tab then no action is needed.
    if (nKeyCode != FWL_VKEY_Tab) {
      return false;
    }

    // If ctrl key or alt key is pressed, then no action is needed.
    if (CPWL_Wnd::IsCTRLKeyDown(nFlags) || CPWL_Wnd::IsALTKeyDown(nFlags)) {
      return false;
    }

    ObservedPtr<CPDFSDK_Annot> end_annot(CPWL_Wnd::IsSHIFTKeyDown(nFlags)
                                             ? GetLastFocusableAnnot()
                                             : GetFirstFocusableAnnot());
    return end_annot && form_fill_env_->SetFocusAnnot(end_annot);
  }

  if (CPWL_Wnd::IsCTRLKeyDown(nFlags) || CPWL_Wnd::IsALTKeyDown(nFlags)) {
    return CPDFSDK_Annot::OnKeyDown(pAnnot, nKeyCode, nFlags);
  }

  CPDFSDK_Annot* pFocusAnnot = GetFocusAnnot();
  if (pFocusAnnot && (nKeyCode == FWL_VKEY_Tab)) {
    ObservedPtr<CPDFSDK_Annot> pNext(CPWL_Wnd::IsSHIFTKeyDown(nFlags)
                                         ? GetPrevAnnot(pFocusAnnot)
                                         : GetNextAnnot(pFocusAnnot));
    if (!pNext) {
      return false;
    }
    if (pNext.Get() != pFocusAnnot) {
      GetFormFillEnv()->SetFocusAnnot(pNext);
      return true;
    }
  }

  // Check |pAnnot| again because JS may have destroyed it in GetNextAnnot().
  if (!pAnnot) {
    return false;
  }

  return CPDFSDK_Annot::OnKeyDown(pAnnot, nKeyCode, nFlags);
}

void CPDFSDK_PageView::LoadFXAnnots() {
  AutoRestorer<bool> lock(&locked_);
  locked_ = true;

#ifdef PDF_ENABLE_XFA
  RetainPtr<CPDFXFA_Page> protector(ToXFAPage(page_));
  CPDF_Document::Extension* pContext = form_fill_env_->GetDocExtension();
  if (pContext && pContext->ContainsExtensionFullForm()) {
    CXFA_FFPageView* pageView = protector->GetXFAPageView();
    CXFA_FFPageWidgetIterator pWidgetHandler(
        pageView, Mask<XFA_WidgetStatus>{XFA_WidgetStatus::kVisible,
                                         XFA_WidgetStatus::kViewable});

    while (CXFA_FFWidget* pXFAAnnot = pWidgetHandler.MoveToNext()) {
      sdkannot_array_.push_back(
          std::make_unique<CPDFXFA_Widget>(pXFAAnnot, this));
      sdkannot_array_.back()->OnLoad();
    }
    return;
  }
#endif  // PDF_ENABLE_XFA

  CPDF_Page* pPage = GetPDFPage();
  DCHECK(pPage);
  bool bUpdateAP = CPDF_InteractiveForm::IsUpdateAPEnabled();
  // Disable the default AP construction.
  CPDF_InteractiveForm::SetUpdateAP(false);
  annot_list_ = std::make_unique<CPDF_AnnotList>(pPage);
  CPDF_InteractiveForm::SetUpdateAP(bUpdateAP);

  const size_t nCount = annot_list_->Count();
  for (size_t i = 0; i < nCount; ++i) {
    CPDF_Annot* pPDFAnnot = annot_list_->GetAt(i);
    CheckForUnsupportedAnnot(pPDFAnnot);
    std::unique_ptr<CPDFSDK_Annot> pAnnot = NewAnnot(pPDFAnnot);
    if (!pAnnot) {
      continue;
    }
    sdkannot_array_.push_back(std::move(pAnnot));
    sdkannot_array_.back()->OnLoad();
  }
}

void CPDFSDK_PageView::UpdateRects(const std::vector<CFX_FloatRect>& rects) {
  for (const auto& rc : rects) {
    form_fill_env_->Invalidate(page_, rc.GetOuterRect());
  }
}

void CPDFSDK_PageView::UpdateView(CPDFSDK_Annot* pAnnot) {
  CFX_FloatRect rcWindow = pAnnot->GetRect();
  form_fill_env_->Invalidate(page_, rcWindow.GetOuterRect());
}

int CPDFSDK_PageView::GetPageIndex() const {
#ifdef PDF_ENABLE_XFA
  CPDF_Document::Extension* pContext = page_->GetDocument()->GetExtension();
  if (pContext && pContext->ContainsExtensionFullForm()) {
    CXFA_FFPageView* pPageView = page_->AsXFAPage()->GetXFAPageView();
    return pPageView ? pPageView->GetLayoutItem()->GetPageIndex() : -1;
  }
#endif  // PDF_ENABLE_XFA
  return GetPageIndexForStaticPDF();
}

bool CPDFSDK_PageView::IsValidAnnot(const CPDF_Annot* p) const {
  return p && annot_list_->Contains(p);
}

bool CPDFSDK_PageView::IsValidSDKAnnot(const CPDFSDK_Annot* p) const {
  return p && std::ranges::find_if(
                  sdkannot_array_,
                  pdfium::MatchesUniquePtr(const_cast<CPDFSDK_Annot*>(p))) !=
                  sdkannot_array_.end();
}

CPDFSDK_Annot* CPDFSDK_PageView::GetFocusAnnot() {
  CPDFSDK_Annot* pFocusAnnot = form_fill_env_->GetFocusAnnot();
  return IsValidSDKAnnot(pFocusAnnot) ? pFocusAnnot : nullptr;
}

int CPDFSDK_PageView::GetPageIndexForStaticPDF() const {
  CPDF_Document* pDoc = form_fill_env_->GetPDFDocument();
  return pDoc->GetPageIndex(GetPDFPage()->GetDict()->GetObjNum());
}

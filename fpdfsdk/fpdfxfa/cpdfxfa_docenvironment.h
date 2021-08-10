// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FPDFXFA_CPDFXFA_DOCENVIRONMENT_H_
#define FPDFSDK_FPDFXFA_CPDFXFA_DOCENVIRONMENT_H_

#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "public/fpdfview.h"
#include "xfa/fxfa/cxfa_ffdoc.h"

class CFX_XMLDocument;
class CPDFXFA_Context;
class IJS_Runtime;

class CPDFXFA_DocEnvironment final : public CXFA_FFDoc::CallbackIface {
 public:
  explicit CPDFXFA_DocEnvironment(CPDFXFA_Context*);
  ~CPDFXFA_DocEnvironment() override;

  // CFXA_FFDoc::CallbackIface:
  void SetChangeMark(CXFA_FFDoc* hDoc) override;
  void InvalidateRect(CXFA_FFPageView* pPageView, const CFX_RectF& rt) override;
  void DisplayCaret(CXFA_FFWidget* hWidget,
                    bool bVisible,
                    const CFX_RectF* pRtAnchor) override;
  bool GetPopupPos(CXFA_FFWidget* hWidget,
                   float fMinPopup,
                   float fMaxPopup,
                   const CFX_RectF& rtAnchor,
                   CFX_RectF* pPopupRect) override;
  bool PopupMenu(CXFA_FFWidget* hWidget, const CFX_PointF& ptPopup) override;
  void OnPageViewEvent(CXFA_FFPageView* pPageView,
                       CXFA_FFDoc::PageViewEvent eEvent) override;
  void WidgetPostAdd(CXFA_FFWidget* hWidget) override;
  void WidgetPreRemove(CXFA_FFWidget* hWidget) override;
  int32_t CountPages(const CXFA_FFDoc* hDoc) const override;
  int32_t GetCurrentPage(const CXFA_FFDoc* hDoc) const override;
  void SetCurrentPage(CXFA_FFDoc* hDoc, int32_t iCurPage) override;
  bool IsCalculationsEnabled(const CXFA_FFDoc* hDoc) const override;
  void SetCalculationsEnabled(CXFA_FFDoc* hDoc, bool bEnabled) override;
  WideString GetTitle(const CXFA_FFDoc* hDoc) const override;
  void SetTitle(CXFA_FFDoc* hDoc, const WideString& wsTitle) override;
  void ExportData(CXFA_FFDoc* hDoc,
                  const WideString& wsFilePath,
                  bool bXDP) override;
  void GotoURL(CXFA_FFDoc* hDoc, const WideString& bsURL) override;
  bool IsValidationsEnabled(const CXFA_FFDoc* hDoc) const override;
  void SetValidationsEnabled(CXFA_FFDoc* hDoc, bool bEnabled) override;
  void SetFocusWidget(CXFA_FFDoc* hDoc, CXFA_FFWidget* hWidget) override;
  void Print(CXFA_FFDoc* hDoc,
             int32_t nStartPage,
             int32_t nEndPage,
             Mask<XFA_PrintOpt> dwOptions) override;
  FX_ARGB GetHighlightColor(const CXFA_FFDoc* hDoc) const override;
  IJS_Runtime* GetIJSRuntime(const CXFA_FFDoc* hDoc) const override;
  CFX_XMLDocument* GetXMLDoc() const override;
  RetainPtr<IFX_SeekableReadStream> OpenLinkedFile(
      CXFA_FFDoc* hDoc,
      const WideString& wsLink) override;

#ifdef PDF_XFA_ELEMENT_SUBMIT_ENABLED
  bool Submit(CXFA_FFDoc* hDoc, CXFA_Submit* submit) override;
#endif  // PDF_XFA_ELEMENT_SUBMIT_ENABLED

 private:
#ifdef PDF_XFA_ELEMENT_SUBMIT_ENABLED
  bool MailToInfo(WideString& csURL,
                  WideString& csToAddress,
                  WideString& csCCAddress,
                  WideString& csBCCAddress,
                  WideString& csSubject,
                  WideString& csMsg);
  bool ExportSubmitFile(FPDF_FILEHANDLER* ppFileHandler,
                        int fileType,
                        FPDF_DWORD encodeType,
                        FPDF_DWORD flag);
  void ToXFAContentFlags(WideString csSrcContent, FPDF_DWORD& flag);
  bool OnBeforeNotifySubmit();
  void OnAfterNotifySubmit();
  bool SubmitInternal(CXFA_FFDoc* hDoc, CXFA_Submit* submit);
#endif  // PDF_XFA_ELEMENT_SUBMIT_ENABLED

  UnownedPtr<CPDFXFA_Context> const m_pContext;
};

#endif  // FPDFSDK_FPDFXFA_CPDFXFA_DOCENVIRONMENT_H_

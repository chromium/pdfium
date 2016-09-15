// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_DOCUMENT_H_
#define FPDFSDK_INCLUDE_CPDFSDK_DOCUMENT_H_

#include <map>
#include <memory>

#include "core/fpdfapi/fpdf_parser/include/cpdf_document.h"
#include "core/fxcrt/include/cfx_observable.h"
#include "fpdfsdk/include/fsdk_define.h"
#include "public/fpdf_formfill.h"

class CPDF_OCContext;
class CPDFSDK_Environment;
class CPDFSDK_Annot;
class CPDFSDK_InterForm;
class CPDFSDK_PageView;
class IJS_Runtime;

class CPDFSDK_Document : public CFX_Observable<CPDFSDK_Document> {
 public:
  static CPDFSDK_Document* FromFPDFFormHandle(FPDF_FORMHANDLE hHandle);

  CPDFSDK_Document(UnderlyingDocumentType* pDoc, CPDFSDK_Environment* pEnv);
  ~CPDFSDK_Document();

  CPDFSDK_InterForm* GetInterForm();

  // Gets the document object for the next layer down; for master this is
  // a CPDF_Document, but for XFA it is a CPDFXFA_Document.
  UnderlyingDocumentType* GetUnderlyingDocument() const {
#ifdef PDF_ENABLE_XFA
    return GetXFADocument();
#else   // PDF_ENABLE_XFA
    return GetPDFDocument();
#endif  // PDF_ENABLE_XFA
  }

  // Gets the CPDF_Document, either directly in master, or from the
  // CPDFXFA_Document for XFA.
  CPDF_Document* GetPDFDocument() const {
#ifdef PDF_ENABLE_XFA
    return m_pDoc ? m_pDoc->GetPDFDoc() : nullptr;
#else   // PDF_ENABLE_XFA
    return m_pDoc;
#endif  // PDF_ENABLE_XFA
  }

#ifdef PDF_ENABLE_XFA
  // Gets the XFA document directly (XFA-only).
  CPDFXFA_Document* GetXFADocument() const { return m_pDoc; }

  int GetPageViewCount() const { return m_pageMap.size(); }
#endif  // PDF_ENABLE_XFA

  CPDFSDK_PageView* GetPageView(UnderlyingPageType* pPage, bool ReNew);
  CPDFSDK_PageView* GetPageView(int nIndex);
  CPDFSDK_PageView* GetCurrentView();
  void RemovePageView(UnderlyingPageType* pPage);
  void UpdateAllViews(CPDFSDK_PageView* pSender, CPDFSDK_Annot* pAnnot);

  CPDFSDK_Annot* GetFocusAnnot();

  IJS_Runtime* GetJsRuntime();

  FX_BOOL SetFocusAnnot(CPDFSDK_Annot* pAnnot, uint32_t nFlag = 0);
  FX_BOOL KillFocusAnnot(uint32_t nFlag = 0);

  FX_BOOL ExtractPages(const std::vector<uint16_t>& arrExtraPages,
                       CPDF_Document* pDstDoc);
  FX_BOOL InsertPages(int nInsertAt,
                      const CPDF_Document* pSrcDoc,
                      const std::vector<uint16_t>& arrSrcPages);
  FX_BOOL ReplacePages(int nPage,
                       const CPDF_Document* pSrcDoc,
                       const std::vector<uint16_t>& arrSrcPages);

  void OnCloseDocument();

  int GetPageCount() { return m_pDoc->GetPageCount(); }
  FX_BOOL GetPermissions(int nFlag);
  FX_BOOL GetChangeMark() { return m_bChangeMask; }
  void SetChangeMark() { m_bChangeMask = TRUE; }
  void ClearChangeMark() { m_bChangeMask = FALSE; }
  CFX_WideString GetPath();
  UnderlyingPageType* GetPage(int nIndex);
  CPDFSDK_Environment* GetEnv() { return m_pEnv; }
  void ProcJavascriptFun();
  FX_BOOL ProcOpenAction();
  CPDF_OCContext* GetOCContext();

 private:
  std::map<UnderlyingPageType*, CPDFSDK_PageView*> m_pageMap;
  UnderlyingDocumentType* m_pDoc;
  std::unique_ptr<CPDFSDK_InterForm> m_pInterForm;
  CPDFSDK_Annot* m_pFocusAnnot;
  CPDFSDK_Environment* m_pEnv;
  std::unique_ptr<CPDF_OCContext> m_pOccontent;
  FX_BOOL m_bChangeMask;
  FX_BOOL m_bBeingDestroyed;
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_DOCUMENT_H_

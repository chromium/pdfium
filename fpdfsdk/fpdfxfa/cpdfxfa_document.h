// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_FPDFXFA_CPDFXFA_DOCUMENT_H_
#define FPDFSDK_FPDFXFA_CPDFXFA_DOCUMENT_H_

#include <memory>

#include "fpdfsdk/fpdfxfa/cpdfxfa_app.h"
#include "fpdfsdk/fpdfxfa/cpdfxfa_docenvironment.h"
#include "xfa/fxfa/xfa_ffdoc.h"

class CPDFSDK_FormFillEnvironment;
class CPDFXFA_Page;
class CXFA_FFDocHandler;
class IJS_Runtime;
class IJS_Context;

enum LoadStatus {
  FXFA_LOADSTATUS_PRELOAD = 0,
  FXFA_LOADSTATUS_LOADING,
  FXFA_LOADSTATUS_LOADED,
  FXFA_LOADSTATUS_CLOSING,
  FXFA_LOADSTATUS_CLOSED
};

class CPDFXFA_Document {
 public:
  CPDFXFA_Document(std::unique_ptr<CPDF_Document> pPDFDoc);
  ~CPDFXFA_Document();

  FX_BOOL LoadXFADoc();
  CPDF_Document* GetPDFDoc() { return m_pPDFDoc.get(); }
  CXFA_FFDoc* GetXFADoc() { return m_pXFADoc.get(); }
  CXFA_FFDocView* GetXFADocView() { return m_pXFADocView; }
  int GetDocType() const { return m_iDocType; }
  CPDFXFA_App* GetApp();  // Creates if needed.

  CPDFSDK_FormFillEnvironment* GetFormFillEnv() const { return m_pFormFillEnv; }
  void SetFormFillEnv(CPDFSDK_FormFillEnvironment* pFormFillEnv);

  void DeletePage(int page_index);
  int GetPageCount() const;

  CPDFXFA_Page* GetXFAPage(int page_index);
  CPDFXFA_Page* GetXFAPage(CXFA_FFPageView* pPage) const;

  void RemovePage(CPDFXFA_Page* page);

  void ClearChangeMark();

 protected:
  friend class CPDFXFA_DocEnvironment;

  int GetOriginalPageCount() const { return m_nPageCount; }
  void SetOriginalPageCount(int count) {
    m_nPageCount = count;
    m_XFAPageList.SetSize(count);
  }

  LoadStatus GetLoadStatus() const { return m_nLoadStatus; }

  CFX_ArrayTemplate<CPDFXFA_Page*>* GetXFAPageList() { return &m_XFAPageList; }

 private:
  void CloseXFADoc(CXFA_FFDocHandler* pDoc);

  int m_iDocType;

  std::unique_ptr<CPDF_Document> m_pPDFDoc;
  std::unique_ptr<CXFA_FFDoc> m_pXFADoc;
  CPDFSDK_FormFillEnvironment* m_pFormFillEnv;  // not owned.
  CXFA_FFDocView* m_pXFADocView;  // not owned.
  std::unique_ptr<CPDFXFA_App> m_pApp;
  CFX_ArrayTemplate<CPDFXFA_Page*> m_XFAPageList;
  LoadStatus m_nLoadStatus;
  int m_nPageCount;

  // Must be destroyed before |m_pFormFillEnv|.
  CPDFXFA_DocEnvironment m_DocEnv;
};

#endif  // FPDFSDK_FPDFXFA_CPDFXFA_DOCUMENT_H_

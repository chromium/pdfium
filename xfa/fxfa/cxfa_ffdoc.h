// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFDOC_H_
#define XFA_FXFA_CXFA_FFDOC_H_

#include <map>
#include <memory>

#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_document_parser.h"

class CFGAS_PDFFontMgr;
class CFX_ChecksumContext;
class CPDF_Document;
class CXFA_FFApp;
class CXFA_FFNotify;
class CXFA_FFDocView;

struct FX_IMAGEDIB_AND_DPI {
  FX_IMAGEDIB_AND_DPI();
  FX_IMAGEDIB_AND_DPI(const FX_IMAGEDIB_AND_DPI& that);
  FX_IMAGEDIB_AND_DPI(const RetainPtr<CFX_DIBSource>& pDib,
                      int32_t xDpi,
                      int32_t yDpi);
  ~FX_IMAGEDIB_AND_DPI();

  RetainPtr<CFX_DIBSource> pDibSource;
  int32_t iImageXDpi;
  int32_t iImageYDpi;
};

inline FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI() = default;
inline FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI(
    const FX_IMAGEDIB_AND_DPI& that) = default;

inline FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI(
    const RetainPtr<CFX_DIBSource>& pDib,
    int32_t xDpi,
    int32_t yDpi)
    : pDibSource(pDib), iImageXDpi(xDpi), iImageYDpi(yDpi) {}

inline FX_IMAGEDIB_AND_DPI::~FX_IMAGEDIB_AND_DPI() = default;

class CXFA_FFDoc {
 public:
  CXFA_FFDoc(CXFA_FFApp* pApp, IXFA_DocEnvironment* pDocEnvironment);
  ~CXFA_FFDoc();

  IXFA_DocEnvironment* GetDocEnvironment() const {
    return m_pDocEnvironment.Get();
  }
  FormType GetFormType() const { return m_FormType; }

  int32_t StartLoad();
  int32_t DoLoad();
  void StopLoad();

  CXFA_FFDocView* CreateDocView();

  bool OpenDoc(CPDF_Document* pPDFDoc);
  void CloseDoc();

  CXFA_Document* GetXFADoc() const { return m_pDocumentParser->GetDocument(); }
  CXFA_FFApp* GetApp() const { return m_pApp.Get(); }
  CPDF_Document* GetPDFDoc() const { return m_pPDFDoc.Get(); }
  CXFA_FFDocView* GetDocView(CXFA_LayoutProcessor* pLayout);
  CXFA_FFDocView* GetDocView();
  RetainPtr<CFX_DIBitmap> GetPDFNamedImage(const WideStringView& wsName,
                                           int32_t& iImageXDpi,
                                           int32_t& iImageYDpi);
  CFGAS_PDFFontMgr* GetPDFFontMgr() const { return m_pPDFFontMgr.get(); }

  bool SavePackage(CXFA_Node* pNode,
                   const RetainPtr<IFX_SeekableStream>& pFile,
                   CFX_ChecksumContext* pCSContext);
  bool ImportData(const RetainPtr<IFX_SeekableStream>& pStream,
                  bool bXDP = true);

 private:
  UnownedPtr<IXFA_DocEnvironment> const m_pDocEnvironment;
  std::unique_ptr<CXFA_DocumentParser> m_pDocumentParser;
  RetainPtr<IFX_SeekableStream> m_pStream;
  UnownedPtr<CXFA_FFApp> const m_pApp;
  std::unique_ptr<CXFA_FFNotify> m_pNotify;
  UnownedPtr<CPDF_Document> m_pPDFDoc;
  std::map<uint32_t, FX_IMAGEDIB_AND_DPI> m_HashToDibDpiMap;
  std::unique_ptr<CXFA_FFDocView> m_DocView;
  std::unique_ptr<CFGAS_PDFFontMgr> m_pPDFFontMgr;
  FormType m_FormType = FormType::kXFAForeground;
};

#endif  // XFA_FXFA_CXFA_FFDOC_H_

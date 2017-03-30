// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFDOC_H_
#define XFA_FXFA_CXFA_FFDOC_H_

#include <map>
#include <memory>

#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_document_parser.h"

class CFX_ChecksumContext;
class CXFA_FFApp;
class CXFA_FFNotify;
class CXFA_FFDocView;

struct FX_IMAGEDIB_AND_DPI {
  FX_IMAGEDIB_AND_DPI();
  FX_IMAGEDIB_AND_DPI(const FX_IMAGEDIB_AND_DPI& that);
  FX_IMAGEDIB_AND_DPI(const CFX_RetainPtr<CFX_DIBSource>& pDib,
                      int32_t xDpi,
                      int32_t yDpi);
  ~FX_IMAGEDIB_AND_DPI();

  CFX_RetainPtr<CFX_DIBSource> pDibSource;
  int32_t iImageXDpi;
  int32_t iImageYDpi;
};

inline FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI() = default;
inline FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI(
    const FX_IMAGEDIB_AND_DPI& that) = default;

inline FX_IMAGEDIB_AND_DPI::FX_IMAGEDIB_AND_DPI(
    const CFX_RetainPtr<CFX_DIBSource>& pDib,
    int32_t xDpi,
    int32_t yDpi)
    : pDibSource(pDib), iImageXDpi(xDpi), iImageYDpi(yDpi) {}

inline FX_IMAGEDIB_AND_DPI::~FX_IMAGEDIB_AND_DPI() = default;

class CXFA_FFDoc {
 public:
  CXFA_FFDoc(CXFA_FFApp* pApp, IXFA_DocEnvironment* pDocEnvironment);
  ~CXFA_FFDoc();

  IXFA_DocEnvironment* GetDocEnvironment() const { return m_pDocEnvironment; }
  XFA_DocType GetDocType() const { return m_dwDocType; }

  int32_t StartLoad();
  int32_t DoLoad(IFX_Pause* pPause = nullptr);
  void StopLoad();

  CXFA_FFDocView* CreateDocView();

  bool OpenDoc(const CFX_RetainPtr<IFX_SeekableReadStream>& pStream);
  bool OpenDoc(CPDF_Document* pPDFDoc);
  bool CloseDoc();

  CXFA_Document* GetXFADoc() { return m_pDocumentParser->GetDocument(); }
  CXFA_FFApp* GetApp() { return m_pApp; }
  CXFA_FFDocView* GetDocView(CXFA_LayoutProcessor* pLayout);
  CXFA_FFDocView* GetDocView();
  CPDF_Document* GetPDFDoc();
  CFX_RetainPtr<CFX_DIBitmap> GetPDFNamedImage(const CFX_WideStringC& wsName,
                                               int32_t& iImageXDpi,
                                               int32_t& iImageYDpi);

  bool SavePackage(XFA_HashCode code,
                   const CFX_RetainPtr<IFX_SeekableWriteStream>& pFile,
                   CFX_ChecksumContext* pCSContext);
  bool ImportData(const CFX_RetainPtr<IFX_SeekableReadStream>& pStream,
                  bool bXDP = true);

 private:
  IXFA_DocEnvironment* const m_pDocEnvironment;
  std::unique_ptr<CXFA_DocumentParser> m_pDocumentParser;
  CFX_RetainPtr<IFX_SeekableReadStream> m_pStream;
  CXFA_FFApp* m_pApp;
  std::unique_ptr<CXFA_FFNotify> m_pNotify;
  CPDF_Document* m_pPDFDoc;
  std::map<uint32_t, FX_IMAGEDIB_AND_DPI> m_HashToDibDpiMap;
  std::unique_ptr<CXFA_FFDocView> m_DocView;
  XFA_DocType m_dwDocType;
};

#endif  // XFA_FXFA_CXFA_FFDOC_H_

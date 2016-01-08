// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXFA_SRC_APP_XFA_FFDOC_H_
#define FXFA_SRC_APP_XFA_FFDOC_H_

#include "xfa/include/fxfa/fxfa.h"

class CXFA_FFApp;
class CXFA_FFNotify;
class CXFA_FFDocView;
class IXFA_Locale;

typedef struct _FX_IMAGEDIB_AND_DPI {
  CFX_DIBSource* pDibSource;
  int32_t iImageXDpi;
  int32_t iImageYDpi;
} FX_IMAGEDIB_AND_DPI;

class CXFA_FFDoc : public IXFA_Doc {
 public:
  CXFA_FFDoc(CXFA_FFApp* pApp, IXFA_DocProvider* pDocProvider);
  ~CXFA_FFDoc();
  IXFA_DocProvider* GetDocProvider() { return m_pDocProvider; }
  FX_DWORD GetDocType();
  int32_t StartLoad();
  int32_t DoLoad(IFX_Pause* pPause = NULL);
  void StopLoad();
  IXFA_DocView* CreateDocView(FX_DWORD dwView = 0);
  FX_BOOL OpenDoc(IFX_FileRead* pStream, FX_BOOL bTakeOverFile);
  FX_BOOL OpenDoc(CPDF_Document* pPDFDoc);
  FX_BOOL CloseDoc();
  void SetDocType(FX_DWORD dwType);
  CXFA_Document* GetXFADoc() { return m_pDocument; }
  CXFA_FFApp* GetApp() { return m_pApp; }
  CXFA_FFDocView* GetDocView(IXFA_DocLayout* pLayout);
  CXFA_FFDocView* GetDocView();
  CPDF_Document* GetPDFDoc();
  CFX_DIBitmap* GetPDFNamedImage(const CFX_WideStringC& wsName,
                                 int32_t& iImageXDpi,
                                 int32_t& iImageYDpi);
  IFDE_XMLElement* GetPackageData(const CFX_WideStringC& wsPackage);
  FX_BOOL SavePackage(const CFX_WideStringC& wsPackage,
                      IFX_FileWrite* pFile,
                      IXFA_ChecksumContext* pCSContext = NULL);
  FX_BOOL ImportData(IFX_FileRead* pStream, FX_BOOL bXDP = TRUE);

 protected:
  IXFA_DocProvider* m_pDocProvider;
  CXFA_Document* m_pDocument;
  IFX_FileRead* m_pStream;
  CXFA_FFApp* m_pApp;
  CXFA_FFNotify* m_pNotify;
  CPDF_Document* m_pPDFDoc;
  CFX_MapPtrToPtr m_mapNamedImages;
  CFX_MapPtrToPtr m_mapTypeToDocView;
  FX_DWORD m_dwDocType;
  FX_BOOL m_bOwnStream;
};

#endif  // FXFA_SRC_APP_XFA_FFDOC_H_

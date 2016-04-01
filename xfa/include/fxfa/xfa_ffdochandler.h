// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FXFA_XFA_FFDOCHANDLER_H_
#define XFA_INCLUDE_FXFA_XFA_FFDOCHANDLER_H_

#include "xfa/include/fxfa/fxfa.h"

class CXFA_ChecksumContext;

class CXFA_FFDocHandler {
 public:
  CXFA_FFDocHandler();
  ~CXFA_FFDocHandler();

  void ReleaseDoc(CXFA_FFDoc* hDoc);
  IXFA_DocProvider* GetDocProvider(CXFA_FFDoc* hDoc);
  uint32_t GetDocType(CXFA_FFDoc* hDoc);
  int32_t StartLoad(CXFA_FFDoc* hDoc);
  int32_t DoLoad(CXFA_FFDoc* hDoc, IFX_Pause* pPause = NULL);
  void StopLoad(CXFA_FFDoc* hDoc);

  CXFA_FFDocView* CreateDocView(CXFA_FFDoc* hDoc, uint32_t dwView = 0);
  int32_t CountPackages(CXFA_FFDoc* hDoc);
  void GetPackageName(CXFA_FFDoc* hDoc,
                      int32_t iPackage,
                      CFX_WideStringC& wsPackage);
  CFDE_XMLElement* GetPackageData(CXFA_FFDoc* hDoc,
                                  const CFX_WideStringC& wsPackage);
  FX_BOOL SavePackage(CXFA_FFDoc* hDoc,
                      const CFX_WideStringC& wsPackage,
                      IFX_FileWrite* pFile,
                      CXFA_ChecksumContext* pCSContext = NULL);
  FX_BOOL CloseDoc(CXFA_FFDoc* hDoc);
  FX_BOOL ImportData(CXFA_FFDoc* hDoc,
                     IFX_FileRead* pStream,
                     FX_BOOL bXDP = TRUE);
  void SetJSERuntime(CXFA_FFDoc* hDoc, FXJSE_HRUNTIME hRuntime);
  FXJSE_HVALUE GetXFAScriptObject(CXFA_FFDoc* hDoc);
  XFA_ATTRIBUTEENUM GetRestoreState(CXFA_FFDoc* hDoc);
  FX_BOOL RunDocScript(CXFA_FFDoc* hDoc,
                       XFA_SCRIPTTYPE eScriptType,
                       const CFX_WideStringC& wsScript,
                       FXJSE_HVALUE hRetValue,
                       FXJSE_HVALUE hThisObject);

 protected:
};

#endif  // XFA_INCLUDE_FXFA_XFA_FFDOCHANDLER_H_

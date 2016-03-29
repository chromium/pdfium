// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFDOCHANDLER_H_
#define XFA_FXFA_APP_XFA_FFDOCHANDLER_H_

#include "xfa/include/fxfa/fxfa.h"

class CXFA_FFDocHandler : public IXFA_DocHandler {
 public:
  CXFA_FFDocHandler();
  ~CXFA_FFDocHandler();

  void ReleaseDoc(IXFA_Doc* hDoc) override;
  IXFA_DocProvider* GetDocProvider(IXFA_Doc* hDoc) override;
  uint32_t GetDocType(IXFA_Doc* hDoc) override;
  int32_t StartLoad(IXFA_Doc* hDoc) override;
  int32_t DoLoad(IXFA_Doc* hDoc, IFX_Pause* pPause = NULL) override;
  void StopLoad(IXFA_Doc* hDoc) override;

  IXFA_DocView* CreateDocView(IXFA_Doc* hDoc, uint32_t dwView = 0) override;
  int32_t CountPackages(IXFA_Doc* hDoc) override;
  void GetPackageName(IXFA_Doc* hDoc,
                      int32_t iPackage,
                      CFX_WideStringC& wsPackage) override;
  CFDE_XMLElement* GetPackageData(IXFA_Doc* hDoc,
                                  const CFX_WideStringC& wsPackage);
  FX_BOOL SavePackage(IXFA_Doc* hDoc,
                      const CFX_WideStringC& wsPackage,
                      IFX_FileWrite* pFile,
                      IXFA_ChecksumContext* pCSContext = NULL) override;
  FX_BOOL CloseDoc(IXFA_Doc* hDoc) override;
  FX_BOOL ImportData(IXFA_Doc* hDoc,
                     IFX_FileRead* pStream,
                     FX_BOOL bXDP = TRUE) override;
  void SetJSERuntime(IXFA_Doc* hDoc, FXJSE_HRUNTIME hRuntime) override;
  FXJSE_HVALUE GetXFAScriptObject(IXFA_Doc* hDoc) override;
  XFA_ATTRIBUTEENUM GetRestoreState(IXFA_Doc* hDoc) override;
  FX_BOOL RunDocScript(IXFA_Doc* hDoc,
                       XFA_SCRIPTTYPE eScriptType,
                       const CFX_WideStringC& wsScript,
                       FXJSE_HVALUE hRetValue,
                       FXJSE_HVALUE hThisObject) override;

 protected:
};

#endif  // XFA_FXFA_APP_XFA_FFDOCHANDLER_H_

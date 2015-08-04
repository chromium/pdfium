// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FFDOCHANDLER_H_
#define XFA_FFDOCHANDLER_H_

class CXFA_FFDocHandler : public IXFA_DocHandler {
 public:
  CXFA_FFDocHandler();
  ~CXFA_FFDocHandler();
  virtual void ReleaseDoc(IXFA_Doc* hDoc);
  virtual IXFA_DocProvider* GetDocProvider(IXFA_Doc* hDoc);
  virtual FX_DWORD GetDocType(IXFA_Doc* hDoc);
  virtual int32_t StartLoad(IXFA_Doc* hDoc);
  virtual int32_t DoLoad(IXFA_Doc* hDoc, IFX_Pause* pPause = NULL);
  virtual void StopLoad(IXFA_Doc* hDoc);

  virtual IXFA_DocView* CreateDocView(IXFA_Doc* hDoc, FX_DWORD dwView = 0);
  virtual int32_t CountPackages(IXFA_Doc* hDoc);
  virtual void GetPackageName(IXFA_Doc* hDoc,
                              int32_t iPackage,
                              CFX_WideStringC& wsPackage);
  virtual IFDE_XMLElement* GetPackageData(IXFA_Doc* hDoc,
                                          const CFX_WideStringC& wsPackage);
  virtual FX_BOOL SavePackage(IXFA_Doc* hDoc,
                              const CFX_WideStringC& wsPackage,
                              IFX_FileWrite* pFile,
                              IXFA_ChecksumContext* pCSContext = NULL);
  virtual FX_BOOL CloseDoc(IXFA_Doc* hDoc);
  virtual FX_BOOL ImportData(IXFA_Doc* hDoc,
                             IFX_FileRead* pStream,
                             FX_BOOL bXDP = TRUE);
  virtual void SetJSERuntime(IXFA_Doc* hDoc, FXJSE_HRUNTIME hRuntime);
  virtual FXJSE_HVALUE GetXFAScriptObject(IXFA_Doc* hDoc);
  virtual XFA_ATTRIBUTEENUM GetRestoreState(IXFA_Doc* hDoc);
  virtual FX_BOOL RunDocScript(IXFA_Doc* hDoc,
                               XFA_SCRIPTTYPE eScriptType,
                               const CFX_WideStringC& wsScript,
                               FXJSE_HVALUE hRetValue,
                               FXJSE_HVALUE hThisObject);

 protected:
};

#endif  // XFA_FFDOCHANDLER_H_

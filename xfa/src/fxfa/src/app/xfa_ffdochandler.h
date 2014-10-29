// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_DOCHANDLER_IMP_H
#define _FXFA_FORMFILLER_DOCHANDLER_IMP_H
class CXFA_FFDocHandler : public IXFA_DocHandler, public CFX_Object
{
public:
    CXFA_FFDocHandler();
    ~CXFA_FFDocHandler();
    virtual void				ReleaseDoc(XFA_HDOC hDoc);
    virtual IXFA_DocProvider*	GetDocProvider(XFA_HDOC hDoc);
    virtual FX_DWORD		GetDocType(XFA_HDOC hDoc);
    virtual	FX_INT32		StartLoad(XFA_HDOC hDoc);
    virtual FX_INT32		DoLoad(XFA_HDOC hDoc, IFX_Pause *pPause = NULL);
    virtual void			StopLoad(XFA_HDOC hDoc);

    virtual IXFA_DocView*	CreateDocView(XFA_HDOC hDoc, FX_DWORD dwView = 0);
    virtual FX_INT32			CountPackages(XFA_HDOC hDoc);
    virtual	void				GetPackageName(XFA_HDOC hDoc, FX_INT32 iPackage, CFX_WideStringC &wsPackage);
    virtual IFDE_XMLElement*	GetPackageData(XFA_HDOC hDoc, FX_WSTR wsPackage);
    virtual FX_BOOL			SavePackage(XFA_HDOC hDoc, FX_WSTR wsPackage, IFX_FileWrite* pFile, IXFA_ChecksumContext *pCSContext = NULL);
    virtual FX_BOOL			CloseDoc(XFA_HDOC hDoc);
    virtual FX_BOOL			ImportData(XFA_HDOC hDoc, IFX_FileRead* pStream, FX_BOOL bXDP = TRUE);
    virtual	void			SetJSERuntime(XFA_HDOC hDoc, FXJSE_HRUNTIME hRuntime);
    virtual FXJSE_HVALUE		GetXFAScriptObject(XFA_HDOC hDoc);
    virtual XFA_ATTRIBUTEENUM	GetRestoreState(XFA_HDOC hDoc);
    virtual FX_BOOL			RunDocScript(XFA_HDOC hDoc, XFA_SCRIPTTYPE eScriptType, FX_WSTR wsScript, FXJSE_HVALUE hRetValue, FXJSE_HVALUE hThisObject);
protected:
};
#endif

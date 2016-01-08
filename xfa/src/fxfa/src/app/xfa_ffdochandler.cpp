// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_common.h"
#include "xfa_ffdochandler.h"
#include "xfa_ffdoc.h"
CXFA_FFDocHandler::CXFA_FFDocHandler() {}
CXFA_FFDocHandler::~CXFA_FFDocHandler() {}
void CXFA_FFDocHandler::ReleaseDoc(IXFA_Doc* hDoc) {
  delete hDoc;  // virtual dtor.
}
IXFA_DocProvider* CXFA_FFDocHandler::GetDocProvider(IXFA_Doc* hDoc) {
  return static_cast<CXFA_FFDoc*>(hDoc)->GetDocProvider();
}
FX_DWORD CXFA_FFDocHandler::GetDocType(IXFA_Doc* hDoc) {
  return static_cast<CXFA_FFDoc*>(hDoc)->GetDocType();
}
int32_t CXFA_FFDocHandler::StartLoad(IXFA_Doc* hDoc) {
  return static_cast<CXFA_FFDoc*>(hDoc)->StartLoad();
}
int32_t CXFA_FFDocHandler::DoLoad(IXFA_Doc* hDoc, IFX_Pause* pPause) {
  return static_cast<CXFA_FFDoc*>(hDoc)->DoLoad(pPause);
}
void CXFA_FFDocHandler::StopLoad(IXFA_Doc* hDoc) {
  static_cast<CXFA_FFDoc*>(hDoc)->StopLoad();
}

IXFA_DocView* CXFA_FFDocHandler::CreateDocView(IXFA_Doc* hDoc,
                                               FX_DWORD dwView) {
  return static_cast<CXFA_FFDoc*>(hDoc)->CreateDocView(dwView);
}
int32_t CXFA_FFDocHandler::CountPackages(IXFA_Doc* hDoc) {
  return 0;
}
void CXFA_FFDocHandler::GetPackageName(IXFA_Doc* hDoc,
                                       int32_t iPackage,
                                       CFX_WideStringC& wsPackage) {}
IFDE_XMLElement* CXFA_FFDocHandler::GetPackageData(
    IXFA_Doc* hDoc,
    const CFX_WideStringC& wsPackage) {
  return static_cast<CXFA_FFDoc*>(hDoc)->GetPackageData(wsPackage);
}
FX_BOOL CXFA_FFDocHandler::SavePackage(IXFA_Doc* hDoc,
                                       const CFX_WideStringC& wsPackage,
                                       IFX_FileWrite* pFile,
                                       IXFA_ChecksumContext* pCSContext) {
  return static_cast<CXFA_FFDoc*>(hDoc)
      ->SavePackage(wsPackage, pFile, pCSContext);
}
FX_BOOL CXFA_FFDocHandler::CloseDoc(IXFA_Doc* hDoc) {
  return static_cast<CXFA_FFDoc*>(hDoc)->CloseDoc();
}

FX_BOOL CXFA_FFDocHandler::ImportData(IXFA_Doc* hDoc,
                                      IFX_FileRead* pStream,
                                      FX_BOOL bXDP) {
  return static_cast<CXFA_FFDoc*>(hDoc)->ImportData(pStream, bXDP);
}
void CXFA_FFDocHandler::SetJSERuntime(IXFA_Doc* hDoc, FXJSE_HRUNTIME hRuntime) {
  static_cast<CXFA_FFDoc*>(hDoc)->GetXFADoc()->InitScriptContext(hRuntime);
}
FXJSE_HVALUE CXFA_FFDocHandler::GetXFAScriptObject(IXFA_Doc* hDoc) {
  CXFA_Document* pXFADoc = static_cast<CXFA_FFDoc*>(hDoc)->GetXFADoc();
  if (!pXFADoc) {
    return NULL;
  }
  IXFA_ScriptContext* pScriptContext = pXFADoc->GetScriptContext();
  if (!pScriptContext) {
    return NULL;
  }
  return pScriptContext->GetJSValueFromMap(pXFADoc->GetRoot());
}
XFA_ATTRIBUTEENUM CXFA_FFDocHandler::GetRestoreState(IXFA_Doc* hDoc) {
  CXFA_Document* pXFADoc = static_cast<CXFA_FFDoc*>(hDoc)->GetXFADoc();
  if (!pXFADoc) {
    return XFA_ATTRIBUTEENUM_Unknown;
  }
  CXFA_Node* pForm = (CXFA_Node*)pXFADoc->GetXFANode(XFA_HASHCODE_Form);
  if (!pForm) {
    return XFA_ATTRIBUTEENUM_Unknown;
  }
  CXFA_Node* pSubForm = pForm->GetFirstChildByClass(XFA_ELEMENT_Subform);
  if (!pSubForm) {
    return XFA_ATTRIBUTEENUM_Unknown;
  }
  return pSubForm->GetEnum(XFA_ATTRIBUTE_RestoreState);
}
FX_BOOL CXFA_FFDocHandler::RunDocScript(IXFA_Doc* hDoc,
                                        XFA_SCRIPTTYPE eScriptType,
                                        const CFX_WideStringC& wsScript,
                                        FXJSE_HVALUE hRetValue,
                                        FXJSE_HVALUE hThisObject) {
  CXFA_Document* pXFADoc = static_cast<CXFA_FFDoc*>(hDoc)->GetXFADoc();
  if (!pXFADoc) {
    return FALSE;
  }
  IXFA_ScriptContext* pScriptContext = pXFADoc->GetScriptContext();
  if (!pScriptContext) {
    return FALSE;
  }
  return pScriptContext->RunScript(
      (XFA_SCRIPTLANGTYPE)eScriptType, wsScript, hRetValue,
      hThisObject ? (CXFA_Object*)FXJSE_Value_ToObject(hThisObject, NULL)
                  : NULL);
}

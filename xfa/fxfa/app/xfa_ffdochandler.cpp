// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/include/fxfa/xfa_ffdochandler.h"

#include "xfa/fxfa/parser/xfa_script.h"
#include "xfa/fxfa/parser/xfa_script_imp.h"
#include "xfa/include/fxfa/xfa_checksum.h"
#include "xfa/include/fxfa/xfa_ffdoc.h"

CXFA_FFDocHandler::CXFA_FFDocHandler() {}

CXFA_FFDocHandler::~CXFA_FFDocHandler() {}

void CXFA_FFDocHandler::ReleaseDoc(CXFA_FFDoc* hDoc) {
  delete hDoc;  // virtual dtor.
}
IXFA_DocProvider* CXFA_FFDocHandler::GetDocProvider(CXFA_FFDoc* hDoc) {
  return hDoc->GetDocProvider();
}
uint32_t CXFA_FFDocHandler::GetDocType(CXFA_FFDoc* hDoc) {
  return hDoc->GetDocType();
}
int32_t CXFA_FFDocHandler::StartLoad(CXFA_FFDoc* hDoc) {
  return hDoc->StartLoad();
}
int32_t CXFA_FFDocHandler::DoLoad(CXFA_FFDoc* hDoc, IFX_Pause* pPause) {
  return hDoc->DoLoad(pPause);
}
void CXFA_FFDocHandler::StopLoad(CXFA_FFDoc* hDoc) {
  hDoc->StopLoad();
}

CXFA_FFDocView* CXFA_FFDocHandler::CreateDocView(CXFA_FFDoc* hDoc,
                                                 uint32_t dwView) {
  return hDoc->CreateDocView(dwView);
}
int32_t CXFA_FFDocHandler::CountPackages(CXFA_FFDoc* hDoc) {
  return 0;
}
void CXFA_FFDocHandler::GetPackageName(CXFA_FFDoc* hDoc,
                                       int32_t iPackage,
                                       CFX_WideStringC& wsPackage) {}
CFDE_XMLElement* CXFA_FFDocHandler::GetPackageData(
    CXFA_FFDoc* hDoc,
    const CFX_WideStringC& wsPackage) {
  return hDoc->GetPackageData(wsPackage);
}
FX_BOOL CXFA_FFDocHandler::SavePackage(CXFA_FFDoc* hDoc,
                                       const CFX_WideStringC& wsPackage,
                                       IFX_FileWrite* pFile,
                                       CXFA_ChecksumContext* pCSContext) {
  return hDoc->SavePackage(wsPackage, pFile, pCSContext);
}
FX_BOOL CXFA_FFDocHandler::CloseDoc(CXFA_FFDoc* hDoc) {
  return hDoc->CloseDoc();
}

FX_BOOL CXFA_FFDocHandler::ImportData(CXFA_FFDoc* hDoc,
                                      IFX_FileRead* pStream,
                                      FX_BOOL bXDP) {
  return hDoc->ImportData(pStream, bXDP);
}
void CXFA_FFDocHandler::SetJSERuntime(CXFA_FFDoc* hDoc,
                                      FXJSE_HRUNTIME hRuntime) {
  hDoc->GetXFADoc()->InitScriptContext(hRuntime);
}
FXJSE_HVALUE CXFA_FFDocHandler::GetXFAScriptObject(CXFA_FFDoc* hDoc) {
  CXFA_Document* pXFADoc = hDoc->GetXFADoc();
  if (!pXFADoc) {
    return NULL;
  }
  CXFA_ScriptContext* pScriptContext = pXFADoc->GetScriptContext();
  if (!pScriptContext) {
    return NULL;
  }
  return pScriptContext->GetJSValueFromMap(pXFADoc->GetRoot());
}
XFA_ATTRIBUTEENUM CXFA_FFDocHandler::GetRestoreState(CXFA_FFDoc* hDoc) {
  CXFA_Document* pXFADoc = hDoc->GetXFADoc();
  if (!pXFADoc) {
    return XFA_ATTRIBUTEENUM_Unknown;
  }
  CXFA_Node* pForm = ToNode(pXFADoc->GetXFAObject(XFA_HASHCODE_Form));
  if (!pForm) {
    return XFA_ATTRIBUTEENUM_Unknown;
  }
  CXFA_Node* pSubForm = pForm->GetFirstChildByClass(XFA_ELEMENT_Subform);
  if (!pSubForm) {
    return XFA_ATTRIBUTEENUM_Unknown;
  }
  return pSubForm->GetEnum(XFA_ATTRIBUTE_RestoreState);
}
FX_BOOL CXFA_FFDocHandler::RunDocScript(CXFA_FFDoc* hDoc,
                                        XFA_SCRIPTTYPE eScriptType,
                                        const CFX_WideStringC& wsScript,
                                        FXJSE_HVALUE hRetValue,
                                        FXJSE_HVALUE hThisObject) {
  CXFA_Document* pXFADoc = hDoc->GetXFADoc();
  if (!pXFADoc) {
    return FALSE;
  }
  CXFA_ScriptContext* pScriptContext = pXFADoc->GetScriptContext();
  if (!pScriptContext) {
    return FALSE;
  }
  return pScriptContext->RunScript(
      (XFA_SCRIPTLANGTYPE)eScriptType, wsScript, hRetValue,
      hThisObject ? (CXFA_Object*)FXJSE_Value_ToObject(hThisObject, NULL)
                  : NULL);
}

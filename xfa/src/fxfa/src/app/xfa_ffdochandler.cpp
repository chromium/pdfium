// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "../common/xfa_common.h"
#include "xfa_ffdochandler.h"
#include "xfa_ffdoc.h"
CXFA_FFDocHandler::CXFA_FFDocHandler()
{
}
CXFA_FFDocHandler::~CXFA_FFDocHandler()
{
}
void CXFA_FFDocHandler::ReleaseDoc(XFA_HDOC hDoc)
{
    delete (CXFA_FFDoc*)hDoc;
}
IXFA_DocProvider* CXFA_FFDocHandler::GetDocProvider(XFA_HDOC hDoc)
{
    return ((CXFA_FFDoc*)hDoc)->GetDocProvider();
}
FX_DWORD CXFA_FFDocHandler::GetDocType(XFA_HDOC hDoc)
{
    return ((CXFA_FFDoc*)hDoc)->GetDocType();
}
FX_INT32 CXFA_FFDocHandler::StartLoad(XFA_HDOC hDoc)
{
    return ((CXFA_FFDoc*)hDoc)->StartLoad();
}
FX_INT32 CXFA_FFDocHandler::DoLoad(XFA_HDOC hDoc, IFX_Pause *pPause )
{
    return ((CXFA_FFDoc*)hDoc)->DoLoad(pPause);
}
void CXFA_FFDocHandler::StopLoad(XFA_HDOC hDoc)
{
    ((CXFA_FFDoc*)hDoc)->StopLoad();
}

IXFA_DocView* CXFA_FFDocHandler::CreateDocView(XFA_HDOC hDoc, FX_DWORD dwView )
{
    return ((CXFA_FFDoc*)hDoc)->CreateDocView(dwView);
}
FX_INT32 CXFA_FFDocHandler::CountPackages(XFA_HDOC hDoc)
{
    return 0;
}
void CXFA_FFDocHandler::GetPackageName(XFA_HDOC hDoc, FX_INT32 iPackage, CFX_WideStringC &wsPackage)
{
}
IFDE_XMLElement* CXFA_FFDocHandler::GetPackageData(XFA_HDOC hDoc, FX_WSTR wsPackage)
{
    return ((CXFA_FFDoc*)hDoc)->GetPackageData(wsPackage);
}
FX_BOOL CXFA_FFDocHandler::SavePackage(XFA_HDOC hDoc, FX_WSTR wsPackage, IFX_FileWrite* pFile, IXFA_ChecksumContext *pCSContext )
{
    return ((CXFA_FFDoc*)hDoc)->SavePackage(wsPackage, pFile, pCSContext);
}
FX_BOOL CXFA_FFDocHandler::CloseDoc(XFA_HDOC hDoc)
{
    return ((CXFA_FFDoc*)hDoc)->CloseDoc();
}

FX_BOOL CXFA_FFDocHandler::ImportData(XFA_HDOC hDoc, IFX_FileRead* pStream, FX_BOOL bXDP )
{
    return ((CXFA_FFDoc*)hDoc)->ImportData(pStream, bXDP);
}
void CXFA_FFDocHandler::SetJSERuntime(XFA_HDOC hDoc, FXJSE_HRUNTIME hRuntime)
{
    ((CXFA_FFDoc*)hDoc)->GetXFADoc()->InitScriptContext(hRuntime);
}
FXJSE_HVALUE CXFA_FFDocHandler::GetXFAScriptObject(XFA_HDOC hDoc)
{
    CXFA_Document* pXFADoc = ((CXFA_FFDoc*)hDoc)->GetXFADoc();
    if (!pXFADoc) {
        return NULL;
    }
    IXFA_ScriptContext* pScriptContext = pXFADoc->GetScriptContext();
    if (!pScriptContext) {
        return NULL;
    }
    return pScriptContext->GetJSValueFromMap(pXFADoc->GetRoot());
}
XFA_ATTRIBUTEENUM CXFA_FFDocHandler::GetRestoreState(XFA_HDOC hDoc)
{
    CXFA_Document* pXFADoc = ((CXFA_FFDoc*)hDoc)->GetXFADoc();
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
FX_BOOL	CXFA_FFDocHandler::RunDocScript(XFA_HDOC hDoc, XFA_SCRIPTTYPE eScriptType, FX_WSTR wsScript, FXJSE_HVALUE hRetValue, FXJSE_HVALUE hThisObject)
{
    CXFA_Document* pXFADoc = ((CXFA_FFDoc*)hDoc)->GetXFADoc();
    if (!pXFADoc) {
        return FALSE;
    }
    IXFA_ScriptContext* pScriptContext = pXFADoc->GetScriptContext();
    if (!pScriptContext) {
        return FALSE;
    }
    return pScriptContext->RunScript((XFA_SCRIPTLANGTYPE)eScriptType, wsScript, hRetValue, hThisObject ? (CXFA_Object*)FXJSE_Value_ToObject(hThisObject, NULL) : NULL);
}

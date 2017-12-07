// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_tree.h"

#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_attachnodelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"

const CJX_MethodSpec CJX_Tree::MethodSpecs[] = {
    {"resolveNode", resolveNode_static},
    {"resolveNodes", resolveNodes_static},
    {"", nullptr}};

CJX_Tree::CJX_Tree(CXFA_Object* obj) : CJX_Object(obj) {
  DefineMethods(MethodSpecs);
}

CJX_Tree::~CJX_Tree() {}

void CJX_Tree::resolveNode(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"resolveNode");
    return;
  }
  WideString wsExpression =
      WideString::FromUTF8(pArguments->GetUTF8String(0).AsStringView());
  CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
  if (!pScriptContext)
    return;
  CXFA_Object* refNode = GetXFAObject();
  if (refNode->GetElementType() == XFA_Element::Xfa)
    refNode = pScriptContext->GetThisObject();
  uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                    XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                    XFA_RESOLVENODE_Siblings;
  XFA_RESOLVENODE_RS resolveNodeRS;
  if (!pScriptContext->ResolveObjects(ToNode(refNode),
                                      wsExpression.AsStringView(),
                                      &resolveNodeRS, dwFlag, nullptr)) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }
  if (resolveNodeRS.dwFlags == XFA_ResolveNode_RSType_Nodes) {
    CXFA_Object* pObject = resolveNodeRS.objects.front();
    pArguments->GetReturnValue()->Assign(
        pScriptContext->GetJSValueFromMap(pObject));
  } else {
    const XFA_SCRIPTATTRIBUTEINFO* lpAttributeInfo =
        resolveNodeRS.pScriptAttribute;
    if (lpAttributeInfo &&
        lpAttributeInfo->eValueType == XFA_ScriptType::Object) {
      auto pValue =
          pdfium::MakeUnique<CFXJSE_Value>(pScriptContext->GetRuntime());
      CJX_Object* jsObject = resolveNodeRS.objects.front()->JSObject();
      (jsObject->*(lpAttributeInfo->callback))(pValue.get(), false,
                                               lpAttributeInfo->attribute);
      pArguments->GetReturnValue()->Assign(pValue.get());
    } else {
      pArguments->GetReturnValue()->SetNull();
    }
  }
}

void CJX_Tree::resolveNodes(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"resolveNodes");
    return;
  }

  WideString wsExpression =
      WideString::FromUTF8(pArguments->GetUTF8String(0).AsStringView());
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (!pValue)
    return;

  CXFA_Object* refNode = GetXFAObject();
  if (refNode->GetElementType() == XFA_Element::Xfa)
    refNode = GetDocument()->GetScriptContext()->GetThisObject();

  ResolveNodeList(pValue, wsExpression,
                  XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                      XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                      XFA_RESOLVENODE_Siblings,
                  ToNode(refNode));
}

void CJX_Tree::all(CFXJSE_Value* pValue,
                   bool bSetting,
                   XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  uint32_t dwFlag = XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_ALL;
  WideString wsExpression = GetAttribute(XFA_Attribute::Name) + L"[*]";
  ResolveNodeList(pValue, wsExpression, dwFlag, nullptr);
}

void CJX_Tree::classAll(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  WideString wsExpression = L"#" + GetXFAObject()->GetClassName() + L"[*]";
  ResolveNodeList(pValue, wsExpression,
                  XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_ALL, nullptr);
}

void CJX_Tree::nodes(CFXJSE_Value* pValue,
                     bool bSetting,
                     XFA_Attribute eAttribute) {
  CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
  if (!pScriptContext)
    return;

  if (bSetting) {
    WideString wsMessage = L"Unable to set ";
    FXJSE_ThrowMessage(wsMessage.UTF8Encode().AsStringView());
    return;
  }

  CXFA_AttachNodeList* pNodeList =
      new CXFA_AttachNodeList(GetDocument(), ToNode(GetXFAObject()));
  pValue->SetObject(pNodeList, pScriptContext->GetJseNormalClass());
}

void CJX_Tree::parent(CFXJSE_Value* pValue,
                      bool bSetting,
                      XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  CXFA_Node* pParent = ToNode(GetXFAObject())->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pParent) {
    pValue->SetNull();
    return;
  }

  pValue->Assign(GetDocument()->GetScriptContext()->GetJSValueFromMap(pParent));
}

void CJX_Tree::index(CFXJSE_Value* pValue,
                     bool bSetting,
                     XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(ToNode(GetXFAObject())->GetNodeSameNameIndex());
}

void CJX_Tree::classIndex(CFXJSE_Value* pValue,
                          bool bSetting,
                          XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(ToNode(GetXFAObject())->GetNodeSameClassIndex());
}

void CJX_Tree::somExpression(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  WideString wsSOMExpression;
  GetXFAObject()->GetSOMExpression(wsSOMExpression);
  pValue->SetString(wsSOMExpression.UTF8Encode().AsStringView());
}

void CJX_Tree::ResolveNodeList(CFXJSE_Value* pValue,
                               WideString wsExpression,
                               uint32_t dwFlag,
                               CXFA_Node* refNode) {
  CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
  if (!pScriptContext)
    return;
  if (!refNode)
    refNode = ToNode(GetXFAObject());

  XFA_RESOLVENODE_RS resolveNodeRS;
  pScriptContext->ResolveObjects(refNode, wsExpression.AsStringView(),
                                 &resolveNodeRS, dwFlag, nullptr);
  CXFA_ArrayNodeList* pNodeList = new CXFA_ArrayNodeList(GetDocument());
  if (resolveNodeRS.dwFlags == XFA_ResolveNode_RSType_Nodes) {
    for (CXFA_Object* pObject : resolveNodeRS.objects) {
      if (pObject->IsNode())
        pNodeList->Append(pObject->AsNode());
    }
  } else {
    if (resolveNodeRS.pScriptAttribute &&
        resolveNodeRS.pScriptAttribute->eValueType == XFA_ScriptType::Object) {
      for (CXFA_Object* pObject : resolveNodeRS.objects) {
        auto pValue =
            pdfium::MakeUnique<CFXJSE_Value>(pScriptContext->GetRuntime());
        CJX_Object* jsObject = pObject->JSObject();
        (jsObject->*(resolveNodeRS.pScriptAttribute->callback))(
            pValue.get(), false, resolveNodeRS.pScriptAttribute->attribute);

        CXFA_Object* obj = CFXJSE_Engine::ToObject(pValue.get(), nullptr);
        if (obj->IsNode())
          pNodeList->Append(obj->AsNode());
      }
    }
  }
  pValue->SetObject(pNodeList, pScriptContext->GetJseNormalClass());
}

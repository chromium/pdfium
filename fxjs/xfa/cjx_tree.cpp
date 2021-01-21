// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_tree.h"

#include <memory>
#include <vector>

#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "third_party/base/numerics/safe_conversions.h"
#include "v8/include/cppgc/allocation.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_attachnodelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_object.h"

const CJX_MethodSpec CJX_Tree::MethodSpecs[] = {
    {"resolveNode", resolveNode_static},
    {"resolveNodes", resolveNodes_static}};

CJX_Tree::CJX_Tree(CXFA_Object* obj) : CJX_Object(obj) {
  DefineMethods(MethodSpecs);
}

CJX_Tree::~CJX_Tree() = default;

bool CJX_Tree::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Tree::resolveNode(
    CFX_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  WideString expression = runtime->ToWideString(params[0]);
  CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
  CXFA_Object* refNode = GetXFAObject();
  if (refNode->GetElementType() == XFA_Element::Xfa)
    refNode = pScriptContext->GetThisObject();

  uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                    XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                    XFA_RESOLVENODE_Siblings;
  Optional<CFXJSE_Engine::ResolveResult> maybeResult =
      pScriptContext->ResolveObjects(ToNode(refNode), expression.AsStringView(),
                                     dwFlag);
  if (!maybeResult.has_value())
    return CJS_Result::Success(runtime->NewNull());

  if (maybeResult.value().type == CFXJSE_Engine::ResolveResult::Type::kNodes) {
    return CJS_Result::Success(
        GetDocument()->GetScriptContext()->GetOrCreateJSBindingFromMap(
            maybeResult.value().objects.front().Get()));
  }

  if (!maybeResult.value().script_attribute.callback ||
      maybeResult.value().script_attribute.eValueType !=
          XFA_ScriptType::Object) {
    return CJS_Result::Success(runtime->NewNull());
  }

  v8::Local<v8::Value> pValue;
  CJX_Object* jsObject = maybeResult.value().objects.front()->JSObject();
  (*maybeResult.value().script_attribute.callback)(
      runtime->GetIsolate(), jsObject, &pValue, false,
      maybeResult.value().script_attribute.attribute);
  return CJS_Result::Success(pValue);
}

CJS_Result CJX_Tree::resolveNodes(
    CFX_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Object* refNode = GetXFAObject();
  if (refNode->GetElementType() == XFA_Element::Xfa)
    refNode = GetDocument()->GetScriptContext()->GetThisObject();

  CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
  v8::Local<v8::Value> pValue;
  ResolveNodeList(pScriptContext->GetIsolate(), &pValue,
                  runtime->ToWideString(params[0]),
                  XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                      XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                      XFA_RESOLVENODE_Siblings,
                  ToNode(refNode));
  return CJS_Result::Success(pValue);
}

void CJX_Tree::all(v8::Isolate* pIsolate,
                   v8::Local<v8::Value>* pValue,
                   bool bSetting,
                   XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  uint32_t dwFlag = XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_ALL;
  WideString wsExpression = GetAttributeByEnum(XFA_Attribute::Name) + L"[*]";
  ResolveNodeList(pIsolate, pValue, wsExpression, dwFlag, nullptr);
}

void CJX_Tree::classAll(v8::Isolate* pIsolate,
                        v8::Local<v8::Value>* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  WideString wsExpression =
      L"#" + WideString::FromASCII(GetXFAObject()->GetClassName()) + L"[*]";
  ResolveNodeList(pIsolate, pValue, wsExpression,
                  XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_ALL, nullptr);
}

void CJX_Tree::nodes(v8::Isolate* pIsolate,
                     v8::Local<v8::Value>* pValue,
                     bool bSetting,
                     XFA_Attribute eAttribute) {
  if (bSetting) {
    WideString wsMessage = L"Unable to set ";
    FXJSE_ThrowMessage(wsMessage.ToUTF8().AsStringView());
    return;
  }

  CXFA_Document* pDoc = GetDocument();
  auto* pNodeList = cppgc::MakeGarbageCollected<CXFA_AttachNodeList>(
      pDoc->GetHeap()->GetAllocationHandle(), pDoc, GetXFANode());
  pDoc->GetNodeOwner()->PersistList(pNodeList);

  CFXJSE_Engine* pEngine = pDoc->GetScriptContext();
  *pValue = pNodeList->JSObject()->NewBoundV8Object(
      pIsolate, pEngine->GetJseNormalClass()->GetTemplate(pIsolate));
}

void CJX_Tree::parent(v8::Isolate* pIsolate,
                      v8::Local<v8::Value>* pValue,
                      bool bSetting,
                      XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  CXFA_Node* pParent = GetXFANode()->GetParent();
  *pValue = pParent ? GetDocument()
                          ->GetScriptContext()
                          ->GetOrCreateJSBindingFromMap(pParent)
                          .As<v8::Value>()
                    : fxv8::NewNullHelper(pIsolate).As<v8::Value>();
}

void CJX_Tree::index(v8::Isolate* pIsolate,
                     v8::Local<v8::Value>* pValue,
                     bool bSetting,
                     XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  CXFA_Node* pNode = GetXFANode();
  size_t iIndex = pNode ? pNode->GetIndexByName() : 0;
  *pValue = fxv8::NewNumberHelper(pIsolate,
                                  pdfium::base::checked_cast<int32_t>(iIndex));
}

void CJX_Tree::classIndex(v8::Isolate* pIsolate,
                          v8::Local<v8::Value>* pValue,
                          bool bSetting,
                          XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  CXFA_Node* pNode = GetXFANode();
  size_t iIndex = pNode ? pNode->GetIndexByClassName() : 0;
  *pValue = fxv8::NewNumberHelper(pIsolate,
                                  pdfium::base::checked_cast<int32_t>(iIndex));
}

void CJX_Tree::somExpression(v8::Isolate* pIsolate,
                             v8::Local<v8::Value>* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  ByteString bsSOMExpression = GetXFAObject()->GetSOMExpression().ToUTF8();
  *pValue = fxv8::NewStringHelper(pIsolate, bsSOMExpression.AsStringView());
}

void CJX_Tree::ResolveNodeList(v8::Isolate* pIsolate,
                               v8::Local<v8::Value>* pValue,
                               WideString wsExpression,
                               uint32_t dwFlag,
                               CXFA_Node* refNode) {
  if (!refNode)
    refNode = GetXFANode();

  CXFA_Document* pDoc = GetDocument();
  CFXJSE_Engine* pScriptContext = pDoc->GetScriptContext();
  Optional<CFXJSE_Engine::ResolveResult> maybeResult =
      pScriptContext->ResolveObjects(refNode, wsExpression.AsStringView(),
                                     dwFlag);

  auto* pNodeList = cppgc::MakeGarbageCollected<CXFA_ArrayNodeList>(
      pDoc->GetHeap()->GetAllocationHandle(), pDoc);
  pDoc->GetNodeOwner()->PersistList(pNodeList);

  if (maybeResult.has_value()) {
    if (maybeResult.value().type ==
        CFXJSE_Engine::ResolveResult::Type::kNodes) {
      for (auto& pObject : maybeResult.value().objects) {
        if (pObject->IsNode())
          pNodeList->Append(pObject->AsNode());
      }
    } else {
      if (maybeResult.value().script_attribute.callback &&
          maybeResult.value().script_attribute.eValueType ==
              XFA_ScriptType::Object) {
        for (auto& pObject : maybeResult.value().objects) {
          v8::Local<v8::Value> innerValue;
          CJX_Object* jsObject = pObject->JSObject();
          (*maybeResult.value().script_attribute.callback)(
              pIsolate, jsObject, &innerValue, false,
              maybeResult.value().script_attribute.attribute);
          CXFA_Object* obj =
              CFXJSE_Engine::ToObject(pScriptContext->GetIsolate(), innerValue);
          if (obj->IsNode())
            pNodeList->Append(obj->AsNode());
        }
      }
    }
  }
  *pValue = pNodeList->JSObject()->NewBoundV8Object(
      pIsolate, pScriptContext->GetJseNormalClass()->GetTemplate(pIsolate));
}

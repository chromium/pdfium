// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_engine.h"

#include <utility>

#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/containers/contains.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxcrt/widetext_buffer.h"
#include "fxjs/cjs_runtime.h"
#include "fxjs/fxv8.h"
#include "fxjs/xfa/cfxjse_class.h"
#include "fxjs/xfa/cfxjse_context.h"
#include "fxjs/xfa/cfxjse_formcalc_context.h"
#include "fxjs/xfa/cfxjse_isolatetracker.h"
#include "fxjs/xfa/cfxjse_nodehelper.h"
#include "fxjs/xfa/cfxjse_resolveprocessor.h"
#include "fxjs/xfa/cfxjse_value.h"
#include "fxjs/xfa/cjx_object.h"
#include "v8/include/v8-function-callback.h"
#include "v8/include/v8-function.h"
#include "v8/include/v8-local-handle.h"
#include "v8/include/v8-object.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_object.h"
#include "xfa/fxfa/parser/cxfa_thisproxy.h"
#include "xfa/fxfa/parser/cxfa_variables.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_utils.h"

using pdfium::fxjse::kClassTag;

const FXJSE_CLASS_DESCRIPTOR kGlobalClassDescriptor = {
    kClassTag,  // tag
    "Root",     // name
    {},         // methods
    CFXJSE_Engine::GlobalPropTypeGetter,
    CFXJSE_Engine::GlobalPropertyGetter,
    CFXJSE_Engine::GlobalPropertySetter,
    CFXJSE_Engine::NormalMethodCall,
};

const FXJSE_CLASS_DESCRIPTOR kNormalClassDescriptor = {
    kClassTag,    // tag
    "XFAObject",  // name
    {},           // methods
    CFXJSE_Engine::NormalPropTypeGetter,
    CFXJSE_Engine::NormalPropertyGetter,
    CFXJSE_Engine::NormalPropertySetter,
    CFXJSE_Engine::NormalMethodCall,
};

const FXJSE_CLASS_DESCRIPTOR kVariablesClassDescriptor = {
    kClassTag,          // tag
    "XFAScriptObject",  // name
    {},                 // methods
    CFXJSE_Engine::NormalPropTypeGetter,
    CFXJSE_Engine::GlobalPropertyGetter,
    CFXJSE_Engine::GlobalPropertySetter,
    CFXJSE_Engine::NormalMethodCall,
};

namespace {

const char kFormCalcRuntime[] = "pfm_rt";

}  // namespace

CFXJSE_Engine::ResolveResult::ResolveResult() = default;

CFXJSE_Engine::ResolveResult::ResolveResult(ResolveResult&& that) noexcept =
    default;

CFXJSE_Engine::ResolveResult& CFXJSE_Engine::ResolveResult::operator=(
    ResolveResult&& that) noexcept = default;

CFXJSE_Engine::ResolveResult::~ResolveResult() = default;

// static
CXFA_Object* CFXJSE_Engine::ToObject(
    const v8::FunctionCallbackInfo<v8::Value>& info) {
  return ToObject(info.GetIsolate(), info.This());
}

// static
CXFA_Object* CFXJSE_Engine::ToObject(v8::Isolate* pIsolate,
                                     v8::Local<v8::Value> value) {
  if (!value->IsObject()) {
    return nullptr;
  }

  return ToObject(FXJSE_RetrieveObjectBinding(value.As<v8::Object>()));
}

// static.
CXFA_Object* CFXJSE_Engine::ToObject(v8::Isolate* pIsolate,
                                     CFXJSE_Value* pValue) {
  return ToObject(pValue->ToHostObject(pIsolate));
}

// static
CXFA_Object* CFXJSE_Engine::ToObject(CFXJSE_HostObject* pHostObj) {
  if (!pHostObj) {
    return nullptr;
  }

  CJX_Object* pJSObject = pHostObj->AsCJXObject();
  return pJSObject ? pJSObject->GetXFAObject() : nullptr;
}

CFXJSE_Engine::CFXJSE_Engine(CXFA_Document* pDocument,
                             CJS_Runtime* fxjs_runtime)
    : CFX_V8(fxjs_runtime->GetIsolate()),
      subordinate_runtime_(fxjs_runtime),
      document_(pDocument),
      js_context_(CFXJSE_Context::Create(fxjs_runtime->GetIsolate(),
                                         &kGlobalClassDescriptor,
                                         pDocument->GetRoot()->JSObject(),
                                         nullptr)),
      node_helper_(std::make_unique<CFXJSE_NodeHelper>()),
      resolve_processor_(
          std::make_unique<CFXJSE_ResolveProcessor>(this, node_helper_.get())) {
  RemoveBuiltInObjs(js_context_.get());
  js_context_->EnableCompatibleMode();

  // Don't know if this can happen before we remove the builtin objs and set
  // compatibility mode.
  js_class_ =
      CFXJSE_Class::Create(js_context_.get(), &kNormalClassDescriptor, false);
}

CFXJSE_Engine::~CFXJSE_Engine() {
  // This is what ensures that the v8 object bound to a CJX_Object
  // no longer retains that binding since it will outlive that object.
  CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
  for (const auto& pair : map_object_to_object_) {
    const v8::Global<v8::Object>& binding = pair.second;
    FXJSE_ClearObjectBinding(v8::Local<v8::Object>::New(GetIsolate(), binding));
  }
}

CFXJSE_Engine::EventParamScope::EventParamScope(CFXJSE_Engine* pEngine,
                                                CXFA_Node* pTarget,
                                                CXFA_EventParam* pEventParam)
    : engine_(pEngine),
      prev_target_(pEngine->GetEventTarget()),
      prev_event_param_(pEngine->GetEventParam()) {
  engine_->target_ = pTarget;
  engine_->event_param_ = pEventParam;
}

CFXJSE_Engine::EventParamScope::~EventParamScope() {
  engine_->target_ = prev_target_;
  engine_->event_param_ = prev_event_param_;
}

CFXJSE_Context::ExecutionResult CFXJSE_Engine::RunScript(
    CXFA_Script::Type eScriptType,
    WideStringView wsScript,
    CXFA_Object* pThisObject) {
  CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
  AutoRestorer<CXFA_Script::Type> typeRestorer(&script_type_);
  script_type_ = eScriptType;

  ByteString btScript;
  if (eScriptType == CXFA_Script::Type::Formcalc) {
    if (!form_calc_context_) {
      form_calc_context_ = std::make_unique<CFXJSE_FormCalcContext>(
          GetIsolate(), js_context_.get(), document_.Get());
    }
    std::optional<WideTextBuffer> wsJavaScript =
        CFXJSE_FormCalcContext::Translate(document_->GetHeap(), wsScript);
    if (!wsJavaScript.has_value()) {
      auto undefined_value = std::make_unique<CFXJSE_Value>();
      undefined_value->SetUndefined(GetIsolate());
      return CFXJSE_Context::ExecutionResult(false, std::move(undefined_value));
    }
    btScript = FX_UTF8Encode(wsJavaScript.value().AsStringView());
  } else {
    btScript = FX_UTF8Encode(wsScript);
  }
  AutoRestorer<cppgc::Persistent<CXFA_Object>> nodeRestorer(&this_object_);
  this_object_ = pThisObject;

  v8::Local<v8::Object> pThisBinding;
  if (pThisObject) {
    pThisBinding = GetOrCreateJSBindingFromMap(pThisObject);
  }

  IJS_Runtime::ScopedEventContext ctx(subordinate_runtime_);
  return js_context_->ExecuteScript(btScript.AsStringView(), pThisBinding);
}

bool CFXJSE_Engine::QueryNodeByFlag(CXFA_Node* refNode,
                                    WideStringView propname,
                                    v8::Local<v8::Value>* pValue,
                                    Mask<XFA_ResolveFlag> dwFlag) {
  if (!refNode) {
    return false;
  }

  std::optional<CFXJSE_Engine::ResolveResult> maybeResult =
      ResolveObjects(refNode, propname, dwFlag);
  if (!maybeResult.has_value()) {
    return false;
  }

  if (maybeResult.value().type == ResolveResult::Type::kNodes) {
    *pValue =
        GetOrCreateJSBindingFromMap(maybeResult.value().objects.front().Get());
    return true;
  }
  if (maybeResult.value().type == ResolveResult::Type::kAttribute &&
      maybeResult.value().script_attribute.callback) {
    CJX_Object* jsObject = maybeResult.value().objects.front()->JSObject();
    (*maybeResult.value().script_attribute.callback)(
        GetIsolate(), jsObject, pValue, false,
        maybeResult.value().script_attribute.attribute);
  }
  return true;
}

bool CFXJSE_Engine::UpdateNodeByFlag(CXFA_Node* refNode,
                                     WideStringView propname,
                                     v8::Local<v8::Value> pValue,
                                     Mask<XFA_ResolveFlag> dwFlag) {
  if (!refNode) {
    return false;
  }

  std::optional<CFXJSE_Engine::ResolveResult> maybeResult =
      ResolveObjects(refNode, propname, dwFlag);
  if (!maybeResult.has_value()) {
    return false;
  }

  if (maybeResult.value().type == ResolveResult::Type::kAttribute &&
      maybeResult.value().script_attribute.callback) {
    CJX_Object* jsObject = maybeResult.value().objects.front()->JSObject();
    (*maybeResult.value().script_attribute.callback)(
        GetIsolate(), jsObject, &pValue, true,
        maybeResult.value().script_attribute.attribute);
  }
  return true;
}

// static
void CFXJSE_Engine::GlobalPropertySetter(v8::Isolate* pIsolate,
                                         v8::Local<v8::Object> pObject,
                                         ByteStringView szPropName,
                                         v8::Local<v8::Value> pValue) {
  CXFA_Object* pOriginalNode = ToObject(pIsolate, pObject);
  CXFA_Document* pDoc = pOriginalNode->GetDocument();
  CFXJSE_Engine* pScriptContext = pDoc->GetScriptContext();
  CXFA_Node* pRefNode = ToNode(pScriptContext->GetThisObject());
  if (pOriginalNode->IsThisProxy()) {
    pRefNode = ToNode(pScriptContext->GetVariablesThis(pOriginalNode));
  }

  WideString wsPropName = WideString::FromUTF8(szPropName);
  if (pScriptContext->UpdateNodeByFlag(
          pRefNode, wsPropName.AsStringView(), pValue,
          Mask<XFA_ResolveFlag>{
              XFA_ResolveFlag::kParent, XFA_ResolveFlag::kSiblings,
              XFA_ResolveFlag::kChildren, XFA_ResolveFlag::kProperties,
              XFA_ResolveFlag::kAttributes})) {
    return;
  }
  if (pOriginalNode->IsThisProxy() && fxv8::IsUndefined(pValue)) {
    fxv8::ReentrantDeleteObjectPropertyHelper(pScriptContext->GetIsolate(),
                                              pObject, szPropName);
    return;
  }
  CXFA_FFNotify* pNotify = pDoc->GetNotify();
  if (!pNotify) {
    return;
  }

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  auto* pCJSRuntime = static_cast<CJS_Runtime*>(hDoc->GetIJSRuntime());
  if (!pCJSRuntime) {
    return;
  }

  IJS_Runtime::ScopedEventContext pContext(pCJSRuntime);
  pCJSRuntime->SetValueByNameInGlobalObject(szPropName, pValue);
}

// static
v8::Local<v8::Value> CFXJSE_Engine::GlobalPropertyGetter(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pObject,
    ByteStringView szPropName) {
  CXFA_Object* pOriginalObject = ToObject(pIsolate, pObject);
  CXFA_Document* pDoc = pOriginalObject->GetDocument();
  CFXJSE_Engine* pScriptContext = pDoc->GetScriptContext();
  WideString wsPropName = WideString::FromUTF8(szPropName);

  // Assume failure.
  v8::Local<v8::Value> pValue = fxv8::NewUndefinedHelper(pIsolate);

  if (pScriptContext->GetType() == CXFA_Script::Type::Formcalc) {
    if (szPropName == kFormCalcRuntime) {
      return pScriptContext->form_calc_context_->GlobalPropertyGetter();
    }

    XFA_HashCode uHashCode =
        static_cast<XFA_HashCode>(FX_HashCode_GetW(wsPropName.AsStringView()));
    if (uHashCode != XFA_HASHCODE_Layout) {
      CXFA_Object* pObj =
          pScriptContext->GetDocument()->GetXFAObject(uHashCode);
      if (pObj) {
        return pScriptContext->GetOrCreateJSBindingFromMap(pObj);
      }
    }
  }

  CXFA_Node* pRefNode = ToNode(pScriptContext->GetThisObject());
  if (pOriginalObject->IsThisProxy()) {
    pRefNode = ToNode(pScriptContext->GetVariablesThis(pOriginalObject));
  }

  if (pScriptContext->QueryNodeByFlag(
          pRefNode, wsPropName.AsStringView(), &pValue,
          Mask<XFA_ResolveFlag>{XFA_ResolveFlag::kChildren,
                                XFA_ResolveFlag::kProperties,
                                XFA_ResolveFlag::kAttributes})) {
    return pValue;
  }
  if (pScriptContext->QueryNodeByFlag(
          pRefNode, wsPropName.AsStringView(), &pValue,
          Mask<XFA_ResolveFlag>{XFA_ResolveFlag::kParent,
                                XFA_ResolveFlag::kSiblings})) {
    return pValue;
  }

  CXFA_Object* pScriptObject =
      pScriptContext->GetVariablesScript(pOriginalObject);
  if (pScriptObject && pScriptContext->QueryVariableValue(
                           CXFA_Script::FromNode(pScriptObject->AsNode()),
                           szPropName, &pValue)) {
    return pValue;
  }

  CXFA_FFNotify* pNotify = pDoc->GetNotify();
  if (!pNotify) {
    return pValue;
  }

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  auto* pCJSRuntime = static_cast<CJS_Runtime*>(hDoc->GetIJSRuntime());
  if (!pCJSRuntime) {
    return pValue;
  }

  IJS_Runtime::ScopedEventContext pContext(pCJSRuntime);
  v8::Local<v8::Value> temp_value =
      pCJSRuntime->GetValueByNameFromGlobalObject(szPropName);

  return !temp_value.IsEmpty() ? temp_value : pValue;
}

// static
FXJSE_ClassPropType CFXJSE_Engine::GlobalPropTypeGetter(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pHolder,
    ByteStringView szPropName,
    bool bQueryIn) {
  CXFA_Object* pObject = ToObject(pIsolate, pHolder);
  if (!pObject) {
    return FXJSE_ClassPropType::kNone;
  }

  CFXJSE_Engine* pScriptContext = pObject->GetDocument()->GetScriptContext();
  pObject = pScriptContext->GetVariablesThis(pObject);
  WideString wsPropName = WideString::FromUTF8(szPropName);
  if (pObject->JSObject()->HasMethod(wsPropName)) {
    return FXJSE_ClassPropType::kMethod;
  }

  return FXJSE_ClassPropType::kProperty;
}

// static
v8::Local<v8::Value> CFXJSE_Engine::NormalPropertyGetter(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pHolder,
    ByteStringView szPropName) {
  CXFA_Object* pOriginalObject = ToObject(pIsolate, pHolder);
  if (!pOriginalObject) {
    return fxv8::NewUndefinedHelper(pIsolate);
  }

  CFXJSE_Engine* pScriptContext =
      pOriginalObject->GetDocument()->GetScriptContext();

  WideString wsPropName = WideString::FromUTF8(szPropName);
  if (wsPropName.EqualsASCII("xfa")) {
    return pScriptContext->GetOrCreateJSBindingFromMap(
        pScriptContext->GetDocument()->GetRoot());
  }

  v8::Local<v8::Value> pReturnValue = fxv8::NewUndefinedHelper(pIsolate);
  CXFA_Object* pObject = pScriptContext->GetVariablesThis(pOriginalObject);
  CXFA_Node* pRefNode = ToNode(pObject);
  if (pScriptContext->QueryNodeByFlag(
          pRefNode, wsPropName.AsStringView(), &pReturnValue,
          Mask<XFA_ResolveFlag>{XFA_ResolveFlag::kChildren,
                                XFA_ResolveFlag::kProperties,
                                XFA_ResolveFlag::kAttributes})) {
    return pReturnValue;
  }
  if (pObject == pScriptContext->GetThisObject() ||
      (pScriptContext->GetType() == CXFA_Script::Type::Javascript &&
       !pScriptContext->IsStrictScopeInJavaScript())) {
    if (pScriptContext->QueryNodeByFlag(
            pRefNode, wsPropName.AsStringView(), &pReturnValue,
            Mask<XFA_ResolveFlag>{XFA_ResolveFlag::kParent,
                                  XFA_ResolveFlag::kSiblings})) {
      return pReturnValue;
    }
  }
  CXFA_Object* pScriptObject =
      pScriptContext->GetVariablesScript(pOriginalObject);
  if (!pScriptObject) {
    return pReturnValue;
  }

  if (pScriptContext->QueryVariableValue(
          CXFA_Script::FromNode(pScriptObject->AsNode()), szPropName,
          &pReturnValue)) {
    return pReturnValue;
  }
  std::optional<XFA_SCRIPTATTRIBUTEINFO> info = XFA_GetScriptAttributeByName(
      pObject->GetElementType(), wsPropName.AsStringView());
  if (info.has_value()) {
    (*info.value().callback)(pIsolate, pObject->JSObject(), &pReturnValue,
                             false, info.value().attribute);
    return pReturnValue;
  }

  CXFA_FFNotify* pNotify = pObject->GetDocument()->GetNotify();
  if (!pNotify) {
    return pReturnValue;
  }

  CXFA_FFDoc* hDoc = pNotify->GetFFDoc();
  auto* pCJSRuntime = static_cast<CJS_Runtime*>(hDoc->GetIJSRuntime());
  if (!pCJSRuntime) {
    return pReturnValue;
  }

  IJS_Runtime::ScopedEventContext pContext(pCJSRuntime);
  v8::Local<v8::Value> temp_local =
      pCJSRuntime->GetValueByNameFromGlobalObject(szPropName);

  return !temp_local.IsEmpty() ? temp_local : pReturnValue;
}

// static
void CFXJSE_Engine::NormalPropertySetter(v8::Isolate* pIsolate,
                                         v8::Local<v8::Object> pHolder,
                                         ByteStringView szPropName,
                                         v8::Local<v8::Value> pValue) {
  CXFA_Object* pOriginalObject = ToObject(pIsolate, pHolder);
  if (!pOriginalObject) {
    return;
  }

  CFXJSE_Engine* pScriptContext =
      pOriginalObject->GetDocument()->GetScriptContext();
  if (pScriptContext->IsResolvingNodes()) {
    return;
  }

  CXFA_Object* pObject = pScriptContext->GetVariablesThis(pOriginalObject);
  WideString wsPropName = WideString::FromUTF8(szPropName);
  WideStringView wsPropNameView = wsPropName.AsStringView();
  std::optional<XFA_SCRIPTATTRIBUTEINFO> info =
      XFA_GetScriptAttributeByName(pObject->GetElementType(), wsPropNameView);
  if (info.has_value()) {
    CJX_Object* jsObject = pObject->JSObject();
    (*info.value().callback)(pIsolate, jsObject, &pValue, true,
                             info.value().attribute);
    return;
  }

  if (pObject->IsNode()) {
    if (wsPropNameView[0] == '#') {
      wsPropNameView = wsPropNameView.Last(wsPropNameView.GetLength() - 1);
    }

    CXFA_Node* pNode = ToNode(pObject);
    CXFA_Node* pPropOrChild = nullptr;
    XFA_Element eType = XFA_GetElementByName(wsPropNameView);
    if (eType != XFA_Element::Unknown) {
      pPropOrChild =
          pNode->JSObject()->GetOrCreateProperty<CXFA_Node>(0, eType);
    } else {
      pPropOrChild = pNode->GetFirstChildByName(wsPropNameView);
    }

    if (pPropOrChild) {
      info = XFA_GetScriptAttributeByName(pPropOrChild->GetElementType(),
                                          L"{default}");
      if (info.has_value()) {
        pPropOrChild->JSObject()->ScriptSomDefaultValue(pIsolate, &pValue, true,
                                                        XFA_Attribute::Unknown);
        return;
      }
    }
  }

  CXFA_Object* pScriptObject =
      pScriptContext->GetVariablesScript(pOriginalObject);
  if (pScriptObject) {
    pScriptContext->UpdateVariableValue(
        CXFA_Script::FromNode(pScriptObject->AsNode()), szPropName, pValue);
  }
}

FXJSE_ClassPropType CFXJSE_Engine::NormalPropTypeGetter(
    v8::Isolate* pIsolate,
    v8::Local<v8::Object> pHolder,
    ByteStringView szPropName,
    bool bQueryIn) {
  CXFA_Object* pObject = ToObject(pIsolate, pHolder);
  if (!pObject) {
    return FXJSE_ClassPropType::kNone;
  }

  CFXJSE_Engine* pScriptContext = pObject->GetDocument()->GetScriptContext();
  pObject = pScriptContext->GetVariablesThis(pObject);
  XFA_Element eType = pObject->GetElementType();
  WideString wsPropName = WideString::FromUTF8(szPropName);
  if (pObject->JSObject()->HasMethod(wsPropName)) {
    return FXJSE_ClassPropType::kMethod;
  }

  if (bQueryIn) {
    std::optional<XFA_SCRIPTATTRIBUTEINFO> maybe_info =
        XFA_GetScriptAttributeByName(eType, wsPropName.AsStringView());
    if (!maybe_info.has_value()) {
      return FXJSE_ClassPropType::kNone;
    }
  }
  return FXJSE_ClassPropType::kProperty;
}

CJS_Result CFXJSE_Engine::NormalMethodCall(
    const v8::FunctionCallbackInfo<v8::Value>& info,
    const WideString& functionName) {
  CXFA_Object* pObject = ToObject(info);
  if (!pObject) {
    return CJS_Result::Failure(WideString::FromASCII("no Holder() present."));
  }
  CFXJSE_Engine* pScriptContext = pObject->GetDocument()->GetScriptContext();
  pObject = pScriptContext->GetVariablesThis(pObject);

  v8::LocalVector<v8::Value> parameters(info.GetIsolate());
  for (int i = 0; i < info.Length(); i++) {
    parameters.push_back(info[i]);
  }
  return pObject->JSObject()->RunMethod(pScriptContext, functionName,
                                        parameters);
}

bool CFXJSE_Engine::IsStrictScopeInJavaScript() {
  return document_->is_strict_scoping();
}

CXFA_Script::Type CFXJSE_Engine::GetType() {
  return script_type_;
}

void CFXJSE_Engine::AddObjectToUpArray(CXFA_Node* pNode) {
  up_object_array_.push_back(pNode);
}

CXFA_Node* CFXJSE_Engine::LastObjectFromUpArray() {
  return !up_object_array_.empty() ? up_object_array_.back() : nullptr;
}

CFXJSE_Context* CFXJSE_Engine::CreateVariablesContext(CXFA_Script* pScriptNode,
                                                      CXFA_Node* pSubform) {
  if (!pScriptNode || !pSubform) {
    return nullptr;
  }

  auto* proxy = cppgc::MakeGarbageCollected<CXFA_ThisProxy>(
      pScriptNode->GetDocument()->GetHeap()->GetAllocationHandle(), pSubform,
      pScriptNode);
  auto pNewContext = CFXJSE_Context::Create(
      GetIsolate(), &kVariablesClassDescriptor, proxy->JSObject(), proxy);
  RemoveBuiltInObjs(pNewContext.get());
  pNewContext->EnableCompatibleMode();
  CFXJSE_Context* pResult = pNewContext.get();
  map_variable_to_context_[pScriptNode->JSObject()] = std::move(pNewContext);
  return pResult;
}

CXFA_Object* CFXJSE_Engine::GetVariablesThis(CXFA_Object* pObject) {
  CXFA_ThisProxy* pProxy = ToThisProxy(pObject);
  return pProxy ? pProxy->GetThisNode() : pObject;
}

CXFA_Object* CFXJSE_Engine::GetVariablesScript(CXFA_Object* pObject) {
  CXFA_ThisProxy* pProxy = ToThisProxy(pObject);
  return pProxy ? pProxy->GetScriptNode() : pObject;
}

void CFXJSE_Engine::RunVariablesScript(CXFA_Script* pScriptNode) {
  if (!pScriptNode) {
    return;
  }

  auto* pParent = CXFA_Variables::FromNode(pScriptNode->GetParent());
  if (!pParent) {
    return;
  }

  auto it = map_variable_to_context_.find(pScriptNode->JSObject());
  if (it != map_variable_to_context_.end() && it->second) {
    return;
  }

  CXFA_Node* pTextNode = pScriptNode->GetFirstChild();
  if (!pTextNode) {
    return;
  }

  std::optional<WideString> wsScript =
      pTextNode->JSObject()->TryCData(XFA_Attribute::Value, true);
  if (!wsScript.has_value()) {
    return;
  }

  ByteString btScript = wsScript->ToUTF8();
  CXFA_Node* pThisObject = pParent->GetParent();
  CFXJSE_Context* pVariablesContext =
      CreateVariablesContext(pScriptNode, pThisObject);
  AutoRestorer<cppgc::Persistent<CXFA_Object>> nodeRestorer(&this_object_);
  this_object_ = pThisObject;
  pVariablesContext->ExecuteScript(btScript.AsStringView(),
                                   v8::Local<v8::Object>());
}

CFXJSE_Context* CFXJSE_Engine::VariablesContextForScriptNode(
    CXFA_Script* pScriptNode) {
  if (!pScriptNode) {
    return nullptr;
  }

  auto* variablesNode = CXFA_Variables::FromNode(pScriptNode->GetParent());
  if (!variablesNode) {
    return nullptr;
  }

  auto it = map_variable_to_context_.find(pScriptNode->JSObject());
  return it != map_variable_to_context_.end() ? it->second.get() : nullptr;
}

bool CFXJSE_Engine::QueryVariableValue(CXFA_Script* pScriptNode,
                                       ByteStringView szPropName,
                                       v8::Local<v8::Value>* pValue) {
  CFXJSE_Context* pVariableContext = VariablesContextForScriptNode(pScriptNode);
  if (!pVariableContext) {
    return false;
  }

  v8::Local<v8::Object> pObject = pVariableContext->GetGlobalObject();
  if (!fxv8::ReentrantHasObjectOwnPropertyHelper(GetIsolate(), pObject,
                                                 szPropName)) {
    return false;
  }

  v8::Local<v8::Value> hVariableValue =
      fxv8::ReentrantGetObjectPropertyHelper(GetIsolate(), pObject, szPropName);
  if (fxv8::IsFunction(hVariableValue)) {
    v8::Local<v8::Function> maybeFunc = CFXJSE_Value::NewBoundFunction(
        GetIsolate(), hVariableValue.As<v8::Function>(), pObject);
    if (!maybeFunc.IsEmpty()) {
      *pValue = maybeFunc;
    }
  } else {
    *pValue = hVariableValue;
  }
  return true;
}

bool CFXJSE_Engine::UpdateVariableValue(CXFA_Script* pScriptNode,
                                        ByteStringView szPropName,
                                        v8::Local<v8::Value> pValue) {
  CFXJSE_Context* pVariableContext = VariablesContextForScriptNode(pScriptNode);
  if (!pVariableContext) {
    return false;
  }

  v8::Local<v8::Object> pObject = pVariableContext->GetGlobalObject();
  fxv8::ReentrantSetObjectOwnPropertyHelper(GetIsolate(), pObject, szPropName,
                                            pValue);
  return true;
}

void CFXJSE_Engine::RemoveBuiltInObjs(CFXJSE_Context* pContext) {
  CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
  v8::Local<v8::Object> pObject = pContext->GetGlobalObject();
  fxv8::ReentrantDeleteObjectPropertyHelper(GetIsolate(), pObject, "Number");
  fxv8::ReentrantDeleteObjectPropertyHelper(GetIsolate(), pObject, "Date");
}

std::optional<CFXJSE_Engine::ResolveResult> CFXJSE_Engine::ResolveObjects(
    CXFA_Object* refObject,
    WideStringView wsExpression,
    Mask<XFA_ResolveFlag> dwStyles) {
  return ResolveObjectsWithBindNode(refObject, wsExpression, dwStyles, nullptr);
}

std::optional<CFXJSE_Engine::ResolveResult>
CFXJSE_Engine::ResolveObjectsWithBindNode(CXFA_Object* refObject,
                                          WideStringView wsExpression,
                                          Mask<XFA_ResolveFlag> dwStyles,
                                          CXFA_Node* bindNode) {
  if (wsExpression.IsEmpty()) {
    return std::nullopt;
  }

  AutoRestorer<bool> resolving_restorer(&resolving_nodes_);
  resolving_nodes_ = true;

  const bool bParentOrSiblings =
      !!(dwStyles & Mask<XFA_ResolveFlag>{XFA_ResolveFlag::kParent,
                                          XFA_ResolveFlag::kSiblings});
  if (script_type_ != CXFA_Script::Type::Formcalc || bParentOrSiblings) {
    up_object_array_.clear();
  }
  if (refObject && refObject->IsNode() && bParentOrSiblings) {
    up_object_array_.push_back(refObject->AsNode());
  }

  ResolveResult result;
  bool bNextCreate = false;
  if (dwStyles & XFA_ResolveFlag::kCreateNode) {
    node_helper_->SetCreateNodeType(bindNode);
  }

  node_helper_->create_parent_ = nullptr;
  node_helper_->cur_all_start_ = -1;

  CFXJSE_ResolveProcessor::NodeData rndFind;
  int32_t nStart = 0;
  int32_t nLevel = 0;

  std::vector<cppgc::Member<CXFA_Object>> findObjects;
  findObjects.emplace_back(refObject ? refObject : document_->GetRoot());
  int32_t nNodes = 0;
  CFXJSE_ScopeUtil_IsolateHandleContext scope(GetJseContext());
  while (true) {
    nNodes = fxcrt::CollectionSize<int32_t>(findObjects);
    int32_t i = 0;
    rndFind.styles_ = dwStyles;
    resolve_processor_->SetCurStart(nStart);
    nStart = resolve_processor_->GetFilter(wsExpression, nStart, rndFind);
    if (nStart < 1) {
      if ((dwStyles & XFA_ResolveFlag::kCreateNode) && !bNextCreate) {
        CXFA_Node* pDataNode = nullptr;
        nStart = node_helper_->cur_all_start_;
        if (nStart != -1) {
          pDataNode = document_->GetNotBindNode(findObjects);
          if (pDataNode) {
            findObjects.clear();
            findObjects.emplace_back(pDataNode);
            break;
          }
        } else {
          pDataNode = findObjects.front()->AsNode();
          findObjects.clear();
          findObjects.emplace_back(pDataNode);
          break;
        }
        dwStyles |= XFA_ResolveFlag::kBind;
        findObjects.clear();
        findObjects.emplace_back(node_helper_->all_start_parent_.Get());
        continue;
      }
      break;
    }
    if (bNextCreate) {
      int32_t checked_length =
          pdfium::checked_cast<int32_t>(wsExpression.GetLength());
      if (node_helper_->CreateNode(rndFind.name_, rndFind.condition_,
                                   nStart == checked_length, this)) {
        continue;
      }
      break;
    }
    std::vector<cppgc::Member<CXFA_Object>> retObjects;
    while (i < nNodes) {
      bool bDataBind = false;
      if (((dwStyles & XFA_ResolveFlag::kBind) ||
           (dwStyles & XFA_ResolveFlag::kCreateNode)) &&
          nNodes > 1) {
        CFXJSE_ResolveProcessor::NodeData rndBind;
        resolve_processor_->GetFilter(wsExpression, nStart, rndBind);
        i = resolve_processor_->IndexForDataBind(rndBind.condition_, nNodes);
        bDataBind = true;
      }
      rndFind.cur_object_ = findObjects[i++].Get();
      rndFind.level_ = nLevel;
      rndFind.result_.type = ResolveResult::Type::kNodes;
      if (!resolve_processor_->Resolve(GetIsolate(), rndFind)) {
        continue;
      }

      if (rndFind.result_.type == ResolveResult::Type::kAttribute &&
          rndFind.result_.script_attribute.callback &&
          nStart < pdfium::checked_cast<int32_t>(wsExpression.GetLength())) {
        v8::Local<v8::Value> pValue;
        CJX_Object* jsObject = rndFind.result_.objects.front()->JSObject();
        (*rndFind.result_.script_attribute.callback)(
            GetIsolate(), jsObject, &pValue, false,
            rndFind.result_.script_attribute.attribute);
        if (!pValue.IsEmpty()) {
          rndFind.result_.objects.front() = ToObject(GetIsolate(), pValue);
        }
      }
      if (!up_object_array_.empty()) {
        up_object_array_.pop_back();
      }
      retObjects.insert(retObjects.end(), rndFind.result_.objects.begin(),
                        rndFind.result_.objects.end());
      rndFind.result_.objects.clear();
      if (bDataBind) {
        break;
      }
    }
    findObjects.clear();

    nNodes = fxcrt::CollectionSize<int32_t>(retObjects);
    if (nNodes < 1) {
      if (dwStyles & XFA_ResolveFlag::kCreateNode) {
        bNextCreate = true;
        if (!node_helper_->create_parent_) {
          node_helper_->create_parent_ = ToNode(rndFind.cur_object_);
          node_helper_->create_count_ = 1;
        }
        int32_t checked_length =
            pdfium::checked_cast<int32_t>(wsExpression.GetLength());
        if (node_helper_->CreateNode(rndFind.name_, rndFind.condition_,
                                     nStart == checked_length, this)) {
          continue;
        }
      }
      break;
    }

    findObjects = std::move(retObjects);
    rndFind.result_.objects.clear();
    if (nLevel == 0) {
      dwStyles.Clear(XFA_ResolveFlag::kParent);
      dwStyles.Clear(XFA_ResolveFlag::kSiblings);
    }
    nLevel++;
  }

  if (!bNextCreate) {
    result.type = rndFind.result_.type;
    if (nNodes > 0) {
      result.objects.insert(result.objects.end(), findObjects.begin(),
                            findObjects.end());
    }
    if (rndFind.result_.type == ResolveResult::Type::kAttribute) {
      result.script_attribute = rndFind.result_.script_attribute;
      return result;
    }
  }
  if ((dwStyles & XFA_ResolveFlag::kCreateNode) ||
      (dwStyles & XFA_ResolveFlag::kBind) ||
      (dwStyles & XFA_ResolveFlag::kBindNew)) {
    if (node_helper_->create_parent_) {
      result.objects.emplace_back(node_helper_->create_parent_.Get());
    } else {
      node_helper_->CreateNodeForCondition(rndFind.condition_);
    }

    result.type = node_helper_->create_flag_;
    if (result.type == ResolveResult::Type::kCreateNodeOne) {
      if (node_helper_->cur_all_start_ != -1) {
        result.type = ResolveResult::Type::kCreateNodeMidAll;
      }
    }

    if (!bNextCreate && (dwStyles & XFA_ResolveFlag::kCreateNode)) {
      result.type = ResolveResult::Type::kExistNodes;
    }

    if (result.objects.empty()) {
      return std::nullopt;
    }

    return result;
  }
  if (nNodes == 0) {
    return std::nullopt;
  }

  return result;
}

v8::Local<v8::Object> CFXJSE_Engine::GetOrCreateJSBindingFromMap(
    CXFA_Object* pObject) {
  RunVariablesScript(CXFA_Script::FromNode(pObject->AsNode()));

  CJX_Object* pCJXObject = pObject->JSObject();
  auto iter = map_object_to_object_.find(pCJXObject);
  if (iter != map_object_to_object_.end()) {
    return v8::Local<v8::Object>::New(GetIsolate(), iter->second);
  }

  v8::Local<v8::Object> binding = pCJXObject->NewBoundV8Object(
      GetIsolate(), js_class_->GetTemplate(GetIsolate()));

  map_object_to_object_[pCJXObject].Reset(GetIsolate(), binding);
  return binding;
}

void CFXJSE_Engine::SetNodesOfRunScript(
    std::vector<cppgc::Persistent<CXFA_Node>>* pArray) {
  script_node_array_ = pArray;
}

void CFXJSE_Engine::AddNodesOfRunScript(CXFA_Node* pNode) {
  if (script_node_array_ &&
      !pdfium::Contains(*script_node_array_, pNode,
                        &cppgc::Persistent<CXFA_Node>::Get)) {
    script_node_array_->emplace_back(pNode);
  }
}

CXFA_Object* CFXJSE_Engine::ToXFAObject(v8::Local<v8::Value> obj) {
  if (!fxv8::IsObject(obj)) {
    return nullptr;
  }

  CFXJSE_HostObject* pHostObj =
      FXJSE_RetrieveObjectBinding(obj.As<v8::Object>());
  if (!pHostObj) {
    return nullptr;
  }

  CJX_Object* pJSObject = pHostObj->AsCJXObject();
  return pJSObject ? pJSObject->GetXFAObject() : nullptr;
}

v8::Local<v8::Object> CFXJSE_Engine::NewNormalXFAObject(CXFA_Object* obj) {
  v8::EscapableHandleScope scope(GetIsolate());
  v8::Local<v8::Object> object = obj->JSObject()->NewBoundV8Object(
      GetIsolate(), GetJseNormalClass()->GetTemplate(GetIsolate()));
  return scope.Escape(object);
}

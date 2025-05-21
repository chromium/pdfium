// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_ENGINE_H_
#define FXJS_XFA_CFXJSE_ENGINE_H_

#include <map>
#include <memory>
#include <type_traits>
#include <vector>

#include "core/fxcrt/mask.h"
#include "core/fxcrt/unowned_ptr.h"
#include "fxjs/cfx_v8.h"
#include "fxjs/xfa/cfxjse_context.h"
#include "v8/include/cppgc/persistent.h"
#include "v8/include/v8-forward.h"
#include "v8/include/v8-persistent-handle.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_script.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"

class CFXJSE_Class;
class CFXJSE_FormCalcContext;
class CFXJSE_HostObject;
class CFXJSE_NodeHelper;
class CFXJSE_ResolveProcessor;
class CFXJSE_Value;
class CJS_Runtime;

enum class XFA_ResolveFlag : uint16_t {
  kChildren = 1 << 0,
  kTagName = 1 << 1,
  kAttributes = 1 << 2,
  kProperties = 1 << 3,
  kSiblings = 1 << 5,
  kParent = 1 << 6,
  kAnyChild = 1 << 7,
  kALL = 1 << 8,
  kCreateNode = 1 << 10,
  kBind = 1 << 11,
  kBindNew = 1 << 12,
};

class CFXJSE_Engine final : public CFX_V8 {
 public:
  class ResolveResult {
    CPPGC_STACK_ALLOCATED();  // Allow raw/unowned pointers.

   public:
    enum class Type {
      kNodes = 0,
      kAttribute,
      kCreateNodeOne,
      kCreateNodeAll,
      kCreateNodeMidAll,
      kExistNodes,
    };

    ResolveResult();
    ResolveResult(ResolveResult&& that) noexcept;
    ResolveResult& operator=(ResolveResult&& that) noexcept;

    // Move-only type.
    ResolveResult(const ResolveResult& that) = delete;
    ResolveResult& operator=(const ResolveResult& that) = delete;

    ~ResolveResult();

    Type type = Type::kNodes;
    XFA_SCRIPTATTRIBUTEINFO script_attribute = {};

    // Vector of Member would be correct for stack-based vectors, if
    // STL worked with cppgc.
    std::vector<cppgc::Member<CXFA_Object>> objects;
  };

  static CXFA_Object* ToObject(const v8::FunctionCallbackInfo<v8::Value>& info);
  static CXFA_Object* ToObject(v8::Isolate* pIsolate,
                               v8::Local<v8::Value> value);
  static CXFA_Object* ToObject(v8::Isolate* pIsolate, CFXJSE_Value* pValue);
  static CXFA_Object* ToObject(CFXJSE_HostObject* pHostObj);
  static v8::Local<v8::Value> GlobalPropertyGetter(
      v8::Isolate* pIsolate,
      v8::Local<v8::Object> pObject,
      ByteStringView szPropName);
  static void GlobalPropertySetter(v8::Isolate* pIsolate,
                                   v8::Local<v8::Object> pObject,
                                   ByteStringView szPropName,
                                   v8::Local<v8::Value> pValue);
  static v8::Local<v8::Value> NormalPropertyGetter(
      v8::Isolate* pIsolate,
      v8::Local<v8::Object> pObject,
      ByteStringView szPropName);
  static void NormalPropertySetter(v8::Isolate* pIsolate,
                                   v8::Local<v8::Object> pObject,
                                   ByteStringView szPropName,
                                   v8::Local<v8::Value> pValue);
  static CJS_Result NormalMethodCall(
      const v8::FunctionCallbackInfo<v8::Value>& info,
      const WideString& functionName);
  static FXJSE_ClassPropType NormalPropTypeGetter(v8::Isolate* pIsolate,
                                                  v8::Local<v8::Object> pObject,
                                                  ByteStringView szPropName,
                                                  bool bQueryIn);
  static FXJSE_ClassPropType GlobalPropTypeGetter(v8::Isolate* pIsolate,
                                                  v8::Local<v8::Object> pObject,
                                                  ByteStringView szPropName,
                                                  bool bQueryIn);

  CFXJSE_Engine(CXFA_Document* document, CJS_Runtime* fxjs_runtime);
  ~CFXJSE_Engine() override;

  class EventParamScope {
    CPPGC_STACK_ALLOCATED();

   public:
    EventParamScope(CFXJSE_Engine* pEngine,
                    CXFA_Node* pTarget,
                    CXFA_EventParam* pEventParam);
    ~EventParamScope();

   private:
    UnownedPtr<CFXJSE_Engine> engine_;
    UnownedPtr<CXFA_Node> prev_target_;
    UnownedPtr<CXFA_EventParam> prev_event_param_;
  };
  friend class EventParamScope;

  CXFA_Node* GetEventTarget() const { return target_; }
  CXFA_EventParam* GetEventParam() const { return event_param_; }

  CFXJSE_Context::ExecutionResult RunScript(CXFA_Script::Type eScriptType,
                                            WideStringView wsScript,
                                            CXFA_Object* pThisObject);

  std::optional<ResolveResult> ResolveObjects(CXFA_Object* refObject,
                                              WideStringView wsExpression,
                                              Mask<XFA_ResolveFlag> dwStyles);

  std::optional<ResolveResult> ResolveObjectsWithBindNode(
      CXFA_Object* refObject,
      WideStringView wsExpression,
      Mask<XFA_ResolveFlag> dwStyles,
      CXFA_Node* bindNode);

  v8::Local<v8::Object> GetOrCreateJSBindingFromMap(CXFA_Object* pObject);

  CXFA_Object* GetThisObject() const { return this_object_; }
  CFXJSE_Class* GetJseNormalClass() const { return js_class_; }
  CXFA_Document* GetDocument() const { return document_.Get(); }

  void SetNodesOfRunScript(std::vector<cppgc::Persistent<CXFA_Node>>* pArray);
  void AddNodesOfRunScript(CXFA_Node* pNode);

  void SetRunAtType(XFA_AttributeValue eRunAt) { run_at_type_ = eRunAt; }
  bool IsRunAtClient() { return run_at_type_ != XFA_AttributeValue::Server; }

  CXFA_Script::Type GetType();

  void AddObjectToUpArray(CXFA_Node* pNode);
  CXFA_Node* LastObjectFromUpArray();

  CXFA_Object* ToXFAObject(v8::Local<v8::Value> obj);
  v8::Local<v8::Object> NewNormalXFAObject(CXFA_Object* obj);

  bool IsResolvingNodes() const { return resolving_nodes_; }

  CFXJSE_Context* GetJseContextForTest() const { return GetJseContext(); }

 private:
  CFXJSE_Context* GetJseContext() const { return js_context_.get(); }
  CFXJSE_Context* CreateVariablesContext(CXFA_Script* pScriptNode,
                                         CXFA_Node* pSubform);
  void RemoveBuiltInObjs(CFXJSE_Context* pContext);
  bool QueryNodeByFlag(CXFA_Node* refNode,
                       WideStringView propname,
                       v8::Local<v8::Value>* pValue,
                       Mask<XFA_ResolveFlag> dwFlag);
  bool UpdateNodeByFlag(CXFA_Node* refNode,
                        WideStringView propname,
                        v8::Local<v8::Value> pValue,
                        Mask<XFA_ResolveFlag> dwFlag);
  bool IsStrictScopeInJavaScript();
  CXFA_Object* GetVariablesThis(CXFA_Object* pObject);
  CXFA_Object* GetVariablesScript(CXFA_Object* pObject);
  CFXJSE_Context* VariablesContextForScriptNode(CXFA_Script* pScriptNode);
  bool QueryVariableValue(CXFA_Script* pScriptNode,
                          ByteStringView szPropName,
                          v8::Local<v8::Value>* pValue);
  bool UpdateVariableValue(CXFA_Script* pScriptNode,
                           ByteStringView szPropName,
                           v8::Local<v8::Value> pValue);
  void RunVariablesScript(CXFA_Script* pScriptNode);

  UnownedPtr<CJS_Runtime> const subordinate_runtime_;
  cppgc::WeakPersistent<CXFA_Document> const document_;
  std::unique_ptr<CFXJSE_Context> js_context_;
  UnownedPtr<CFXJSE_Class> js_class_;
  CXFA_Script::Type script_type_ = CXFA_Script::Type::Unknown;
  // |map_object_to_value_| is what ensures the v8 object bound to a
  // CJX_Object remains valid for the lifetime of the engine.
  std::map<cppgc::Persistent<CJX_Object>, v8::Global<v8::Object>>
      map_object_to_object_;
  std::map<cppgc::Persistent<CJX_Object>, std::unique_ptr<CFXJSE_Context>>
      map_variable_to_context_;
  cppgc::Persistent<CXFA_Node> target_;
  UnownedPtr<CXFA_EventParam> event_param_;
  std::vector<cppgc::Persistent<CXFA_Node>> up_object_array_;
  UnownedPtr<std::vector<cppgc::Persistent<CXFA_Node>>> script_node_array_;
  std::unique_ptr<CFXJSE_NodeHelper> const node_helper_;
  std::unique_ptr<CFXJSE_ResolveProcessor> const resolve_processor_;
  std::unique_ptr<CFXJSE_FormCalcContext> form_calc_context_;
  cppgc::Persistent<CXFA_Object> this_object_;
  XFA_AttributeValue run_at_type_ = XFA_AttributeValue::Client;
  bool resolving_nodes_ = false;
};

#endif  //  FXJS_XFA_CFXJSE_ENGINE_H_

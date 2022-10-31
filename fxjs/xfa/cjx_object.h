// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_OBJECT_H_
#define FXJS_XFA_CJX_OBJECT_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/widestring.h"
#include "fxjs/gc/heap.h"
#include "fxjs/xfa/fxjse.h"
#include "fxjs/xfa/jse_define.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/base/span.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/v8-forward.h"
#include "xfa/fxfa/fxfa_basic.h"

class CFXJSE_Engine;
class CFXJSE_MapModule;
class CFX_XMLElement;
class CJX_Object;
class CXFA_Document;
class CXFA_LayoutItem;
class CXFA_Measurement;
class CXFA_Node;
class CXFA_Object;

using CJX_MethodCall =
    CJS_Result (*)(CJX_Object* obj,
                   CFXJSE_Engine* runtime,
                   const std::vector<v8::Local<v8::Value>>& params);

struct CJX_MethodSpec {
  const char* pName;
  CJX_MethodCall pMethodCall;
};

class CJX_Object : public cppgc::GarbageCollected<CJX_Object>,
                   public CFXJSE_HostObject {
 public:
  // Corresponds 1:1 with CJX_ subclasses.
  enum class TypeTag {
    Boolean,
    Container,
    DataWindow,
    Delta,
    Desc,
    Draw,
    Encrypt,
    EventPseudoModel,
    ExclGroup,
    Extras,
    Field,
    Form,
    Handler,
    HostPseudoModel,
    InstanceManager,
    LayoutPseudoModel,
    List,
    LogPseudoModel,
    Manifest,
    Model,
    Node,
    Object,
    Occur,
    Packet,
    Script,
    SignaturePesudoModel,
    Source,
    Subform,
    SubformSet,
    Template,
    TextNode,
    Tree,
    TreeList,
    WsdlConnection,
    Xfa,
  };

  class CalcData : public cppgc::GarbageCollected<CalcData> {
   public:
    CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
    ~CalcData();

    void Trace(cppgc::Visitor* visitor) const;

    std::vector<cppgc::Member<CXFA_Node>> m_Globals;

   private:
    CalcData();
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CJX_Object() override;

  // CFXJSE_HostObject:
  CJX_Object* AsCJXObject() override;

  virtual void Trace(cppgc::Visitor* visitor) const;
  virtual bool DynamicTypeIs(TypeTag eType) const;

  JSE_PROP(className);

  CXFA_Document* GetDocument() const;
  CXFA_Node* GetXFANode() const;
  CXFA_Object* GetXFAObject() const { return object_; }

  void SetCalcRecursionCount(size_t count) { calc_recursion_count_ = count; }
  size_t GetCalcRecursionCount() const { return calc_recursion_count_; }

  void SetLayoutItem(CXFA_LayoutItem* item) { layout_item_ = item; }
  CXFA_LayoutItem* GetLayoutItem() const { return layout_item_; }

  bool HasMethod(const WideString& func) const;
  CJS_Result RunMethod(const WideString& func,
                       const std::vector<v8::Local<v8::Value>>& params);

  bool HasAttribute(XFA_Attribute eAttr) const;
  WideString GetAttributeByString(WideStringView attr) const;
  WideString GetAttributeByEnum(XFA_Attribute attr) const;
  absl::optional<WideString> TryAttribute(XFA_Attribute eAttr,
                                          bool bUseDefault) const;
  void SetAttributeByEnum(XFA_Attribute eAttr,
                          const WideString& wsValue,
                          bool bNotify);
  void SetAttributeByString(WideStringView wsAttr, const WideString& wsValue);
  void RemoveAttribute(WideStringView wsAttr);

  WideString GetContent(bool bScriptModify) const;
  absl::optional<WideString> TryContent(bool bScriptModify, bool bProto) const;
  void SetContent(const WideString& wsContent,
                  const WideString& wsXMLValue,
                  bool bNotify,
                  bool bScriptModify,
                  bool bSyncData);

  template <typename T>
  T* GetProperty(int32_t index, XFA_Element eType) const {
    return static_cast<T*>(GetPropertyInternal(index, eType));
  }
  template <typename T>
  T* GetOrCreateProperty(int32_t index, XFA_Element eType) {
    return static_cast<T*>(GetOrCreatePropertyInternal(index, eType));
  }

  void SetAttributeValue(const WideString& wsValue,
                         const WideString& wsXMLValue);

  // Not actual properties, but invoked as property handlers to cover
  // a broad range of underlying properties.
  JSE_PROP(ScriptAttributeString);
  JSE_PROP(ScriptAttributeBool);
  JSE_PROP(ScriptAttributeInteger);
  JSE_PROP(ScriptSomFontColor);
  JSE_PROP(ScriptSomFillColor);
  JSE_PROP(ScriptSomBorderColor);
  JSE_PROP(ScriptSomBorderWidth);
  JSE_PROP(ScriptSomValidationMessage);
  JSE_PROP(ScriptSomMandatoryMessage);
  JSE_PROP(ScriptSomDefaultValue);
  JSE_PROP(ScriptSomDefaultValue_Read);
  JSE_PROP(ScriptSomDataNode);
  JSE_PROP(ScriptSomMandatory);
  JSE_PROP(ScriptSomInstanceIndex);
  JSE_PROP(ScriptSubmitFormatMode);

  absl::optional<WideString> TryNamespace() const;

  int32_t GetInteger(XFA_Attribute eAttr) const;
  absl::optional<int32_t> TryInteger(XFA_Attribute eAttr,
                                     bool bUseDefault) const;
  void SetInteger(XFA_Attribute eAttr, int32_t iValue, bool bNotify);

  WideString GetCData(XFA_Attribute eAttr) const;
  absl::optional<WideString> TryCData(XFA_Attribute eAttr,
                                      bool bUseDefault) const;
  void SetCData(XFA_Attribute eAttr, const WideString& wsValue);

  XFA_AttributeValue GetEnum(XFA_Attribute eAttr) const;
  absl::optional<XFA_AttributeValue> TryEnum(XFA_Attribute eAttr,
                                             bool bUseDefault) const;
  void SetEnum(XFA_Attribute eAttr, XFA_AttributeValue eValue, bool bNotify);

  bool GetBoolean(XFA_Attribute eAttr) const;
  absl::optional<bool> TryBoolean(XFA_Attribute eAttr, bool bUseDefault) const;
  void SetBoolean(XFA_Attribute eAttr, bool bValue, bool bNotify);

  CXFA_Measurement GetMeasure(XFA_Attribute eAttr) const;
  float GetMeasureInUnit(XFA_Attribute eAttr, XFA_Unit unit) const;
  absl::optional<CXFA_Measurement> TryMeasure(XFA_Attribute eAttr,
                                              bool bUseDefault) const;
  absl::optional<float> TryMeasureAsFloat(XFA_Attribute attr) const;
  void SetMeasure(XFA_Attribute eAttr,
                  const CXFA_Measurement& mValue,
                  bool bNotify);

  void MergeAllData(CXFA_Object* pDstObj);

  CalcData* GetCalcData() const { return calc_data_; }
  CalcData* GetOrCreateCalcData(cppgc::Heap* heap);
  void TakeCalcDataFrom(CJX_Object* that);

  void ThrowInvalidPropertyException(v8::Isolate* pIsolate) const;
  void ThrowArgumentMismatchException(v8::Isolate* pIsolate) const;
  void ThrowIndexOutOfBoundsException(v8::Isolate* pIsolate) const;
  void ThrowParamCountMismatchException(v8::Isolate* pIsolate,
                                        const WideString& method) const;
  void ThrowTooManyOccurrencesException(v8::Isolate* pIsolate,
                                        const WideString& obj) const;

 protected:
  enum class SOMMessageType {
    kValidationMessage,
    kFormatMessage,
    kMandatoryMessage
  };

  explicit CJX_Object(CXFA_Object* obj);

  void ScriptSomMessage(v8::Isolate* pIsolate,
                        v8::Local<v8::Value>* pValue,
                        bool bSetting,
                        SOMMessageType iMessageType);
  void SetAttributeValueImpl(const WideString& wsValue,
                             const WideString& wsXMLValue,
                             bool bNotify,
                             bool bScriptModify);
  void SetCDataImpl(XFA_Attribute eAttr,
                    const WideString& wsValue,
                    bool bNotify,
                    bool bScriptModify);
  void DefineMethods(pdfium::span<const CJX_MethodSpec> methods);
  void MoveBufferMapData(CXFA_Object* pSrcObj, CXFA_Object* pDstObj);
  void ThrowException(v8::Isolate* pIsolate, const WideString& str) const;

 private:
  using Type__ = CJX_Object;
  static const TypeTag static_type__ = TypeTag::Object;

  CXFA_Node* GetPropertyInternal(int32_t index, XFA_Element eType) const;
  CXFA_Node* GetOrCreatePropertyInternal(int32_t index, XFA_Element eType);

  void OnChanging(XFA_Attribute eAttr);
  void OnChanged(XFA_Attribute eAttr, bool bScriptModify);

  // Returns a pointer to the XML node that needs to be updated with the new
  // attribute value. |nullptr| if no update is needed.
  CFX_XMLElement* SetValue(XFA_Attribute eAttr, int32_t value, bool bNotify);
  int32_t Subform_and_SubformSet_InstanceIndex();

  CFXJSE_MapModule* CreateMapModule();
  CFXJSE_MapModule* GetMapModule() const;
  void SetMapModuleValue(uint32_t key, int32_t value);
  void SetMapModuleString(uint32_t key, const WideString& wsValue);
  void SetMapModuleMeasurement(uint32_t key, const CXFA_Measurement& value);
  absl::optional<int32_t> GetMapModuleValue(uint32_t key) const;
  absl::optional<WideString> GetMapModuleString(uint32_t key) const;
  absl::optional<CXFA_Measurement> GetMapModuleMeasurement(uint32_t key) const;
  absl::optional<int32_t> GetMapModuleValueFollowingChain(uint32_t key) const;
  absl::optional<WideString> GetMapModuleStringFollowingChain(
      uint32_t key) const;
  absl::optional<CXFA_Measurement> GetMapModuleMeasurementFollowingChain(
      uint32_t key) const;
  bool HasMapModuleKey(uint32_t key) const;
  void RemoveMapModuleKey(uint32_t key);
  void MoveBufferMapData(CXFA_Object* pDstObj);

  cppgc::Member<CXFA_Object> object_;
  cppgc::Member<CXFA_LayoutItem> layout_item_;
  cppgc::Member<CalcData> calc_data_;
  std::unique_ptr<CFXJSE_MapModule> map_module_;
  std::map<ByteString, CJX_MethodCall> method_specs_;
  size_t calc_recursion_count_ = 0;
};

#endif  // FXJS_XFA_CJX_OBJECT_H_

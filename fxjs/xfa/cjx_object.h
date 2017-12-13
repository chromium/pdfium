// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_OBJECT_H_
#define FXJS_XFA_CJX_OBJECT_H_

#include <map>
#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "third_party/base/optional.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/cxfa_widgetdata.h"

class CFXJSE_Value;
class CJS_V8;
class CXFA_CalcData;
class CXFA_Document;
class CXFA_Object;
struct XFA_MAPMODULEDATA;

typedef CJS_Return (*CJX_MethodCall)(
    CJX_Object* obj,
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params);
struct CJX_MethodSpec {
  const char* pName;
  CJX_MethodCall pMethodCall;
};

typedef void (*PD_CALLBACK_FREEDATA)(void* pData);
typedef void (*PD_CALLBACK_DUPLICATEDATA)(void*& pData);

struct XFA_MAPDATABLOCKCALLBACKINFO {
  PD_CALLBACK_FREEDATA pFree;
  PD_CALLBACK_DUPLICATEDATA pCopy;
};

class CJX_Object {
 public:
  explicit CJX_Object(CXFA_Object* obj);
  virtual ~CJX_Object();

  CXFA_Object* GetXFAObject() { return object_.Get(); }
  const CXFA_Object* GetXFAObject() const { return object_.Get(); }

  CXFA_Document* GetDocument() const;

  void SetWidgetData(std::unique_ptr<CXFA_WidgetData> data) {
    widget_data_ = std::move(data);
  }
  CXFA_WidgetData* GetWidgetData() const { return widget_data_.get(); }

  bool HasMethod(const WideString& func) const;
  CJS_Return RunMethod(const WideString& func,
                       const std::vector<v8::Local<v8::Value>>& params);

  bool HasAttribute(XFA_Attribute eAttr);
  bool SetAttribute(XFA_Attribute eAttr,
                    const WideStringView& wsValue,
                    bool bNotify);
  bool SetAttribute(const WideStringView& wsAttr,
                    const WideStringView& wsValue,
                    bool bNotify);
  void RemoveAttribute(const WideStringView& wsAttr);
  WideString GetAttribute(const WideStringView& attr);
  WideString GetAttribute(XFA_Attribute attr);
  pdfium::Optional<WideString> TryAttribute(const WideStringView& wsAttr,
                                            bool bUseDefault);
  pdfium::Optional<WideString> TryAttribute(XFA_Attribute eAttr,
                                            bool bUseDefault);

  pdfium::Optional<WideString> TryContent(bool bScriptModify, bool bProto);
  bool SetContent(const WideString& wsContent,
                  const WideString& wsXMLValue,
                  bool bNotify,
                  bool bScriptModify,
                  bool bSyncData);
  WideString GetContent(bool bScriptModify);

  CXFA_Node* GetProperty(int32_t index,
                         XFA_Element eType,
                         bool bCreateProperty);

  void SetAttributeValue(const WideString& wsValue,
                         const WideString& wsXMLValue,
                         bool bNotify,
                         bool bScriptModify);

  void Script_Attribute_String(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_Attribute_BOOL(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute);
  void Script_Attribute_Integer(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute);

  void Script_Som_FontColor(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_Som_FillColor(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_Som_BorderColor(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_Som_BorderWidth(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_Som_ValidationMessage(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);
  void Script_Som_MandatoryMessage(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_Field_FormatMessage(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_Som_DefaultValue(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_Som_DefaultValue_Read(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);
  void Script_Som_DataNode(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void Script_Som_Mandatory(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);
  void Script_Som_InstanceIndex(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute);

  void Script_NodeClass_Ns(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void Script_NodeClass_Model(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_NodeClass_IsContainer(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);
  void Script_NodeClass_IsNull(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_NodeClass_OneOfChild(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_ModelClass_Context(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute);
  void Script_ModelClass_AliasNode(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_Delta_CurrentValue(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute);
  void Script_Delta_SavedValue(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);
  void Script_Delta_Target(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void Script_Field_Length(CFXJSE_Value* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute);
  void Script_Field_EditValue(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_Field_FormattedValue(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_Field_ParentSubform(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_Field_SelectedIndex(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_ExclGroup_DefaultAndRawValue(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute);
  void Script_ExclGroup_ErrorText(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_ExclGroup_Transient(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);

  void Script_Subform_InstanceManager(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute);
  void Script_Subform_Locale(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute);
  void Script_InstanceManager_Count(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);
  void Script_InstanceManager_Max(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);
  void Script_InstanceManager_Min(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute);

  void Script_Occur_Max(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute);
  void Script_Occur_Min(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute);

  void Script_Form_Checksum(CFXJSE_Value* pValue,
                            bool bSetting,
                            XFA_Attribute eAttribute);

  void Script_Packet_Content(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute);

  void Script_Source_Db(CFXJSE_Value* pValue,
                        bool bSetting,
                        XFA_Attribute eAttribute);
  void Script_Xfa_This(CFXJSE_Value* pValue,
                       bool bSetting,
                       XFA_Attribute eAttribute);
  void Script_Handler_Version(CFXJSE_Value* pValue,
                              bool bSetting,
                              XFA_Attribute eAttribute);
  void Script_SubmitFormat_Mode(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute);
  void Script_Extras_Type(CFXJSE_Value* pValue,
                          bool bSetting,
                          XFA_Attribute eAttribute);
  void Script_Encrypt_Format(CFXJSE_Value* pValue,
                             bool bSetting,
                             XFA_Attribute eAttribute);
  void Script_Script_Stateless(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute);

  pdfium::Optional<WideString> TryNamespace();

  pdfium::Optional<int32_t> TryInteger(XFA_Attribute eAttr, bool bUseDefault);
  bool SetInteger(XFA_Attribute eAttr, int32_t iValue, bool bNotify);
  int32_t GetInteger(XFA_Attribute eAttr);

  pdfium::Optional<WideString> TryCData(XFA_Attribute eAttr, bool bUseDefault);
  bool SetCData(XFA_Attribute eAttr,
                const WideString& wsValue,
                bool bNotify,
                bool bScriptModify);
  WideString GetCData(XFA_Attribute eAttr);

  pdfium::Optional<XFA_AttributeEnum> TryEnum(XFA_Attribute eAttr,
                                              bool bUseDefault);
  bool SetEnum(XFA_Attribute eAttr, XFA_AttributeEnum eValue, bool bNotify);
  XFA_AttributeEnum GetEnum(XFA_Attribute eAttr);

  pdfium::Optional<bool> TryBoolean(XFA_Attribute eAttr, bool bUseDefault);
  bool SetBoolean(XFA_Attribute eAttr, bool bValue, bool bNotify);
  bool GetBoolean(XFA_Attribute eAttr);

  pdfium::Optional<CXFA_Measurement> TryMeasure(XFA_Attribute eAttr,
                                                bool bUseDefault) const;
  bool SetMeasure(XFA_Attribute eAttr, CXFA_Measurement mValue, bool bNotify);
  CXFA_Measurement GetMeasure(XFA_Attribute eAttr) const;

  void Script_ObjectClass_ClassName(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute);

  void MergeAllData(CXFA_Object* pDstModule);

  void SetCalcData(std::unique_ptr<CXFA_CalcData> data);
  CXFA_CalcData* GetCalcData() const { return calc_data_.get(); }
  std::unique_ptr<CXFA_CalcData> ReleaseCalcData();

  int32_t InstanceManager_SetInstances(int32_t iDesired);
  int32_t InstanceManager_MoveInstance(int32_t iTo, int32_t iFrom);

  void ThrowInvalidPropertyException() const;
  void ThrowArgumentMismatchException() const;
  void ThrowIndexOutOfBoundsException() const;
  void ThrowParamCountMismatchException(const WideString& method) const;
  void ThrowTooManyOccurancesException(const WideString& obj) const;

 protected:
  void DefineMethods(const CJX_MethodSpec method_specs[], size_t count);

  void MoveBufferMapData(CXFA_Object* pSrcModule, CXFA_Object* pDstModule);
  void SetMapModuleString(void* pKey, const WideStringView& wsValue);
  void ThrowException(const wchar_t* str, ...) const;

 private:
  enum XFA_SOM_MESSAGETYPE {
    XFA_SOM_ValidationMessage,
    XFA_SOM_FormatMessage,
    XFA_SOM_MandatoryMessage
  };
  void Script_Som_Message(CFXJSE_Value* pValue,
                          bool bSetting,
                          XFA_SOM_MESSAGETYPE iMessageType);
  void Script_Boolean_DefaultValue(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute);
  void Script_Draw_DefaultValue(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute);
  void Script_Field_DefaultValue(CFXJSE_Value* pValue,
                                 bool bSetting,
                                 XFA_Attribute eAttribute);
  int32_t Subform_and_SubformSet_InstanceIndex();

  void OnChanged(XFA_Attribute eAttr, bool bNotify, bool bScriptModify);
  void OnChanging(XFA_Attribute eAttr, bool bNotify);
  bool SetUserData(void* pKey,
                   void* pData,
                   XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo);

  // Returns a pointer to the XML node that needs to be updated with the new
  // attribute value. |nullptr| if no update is needed.
  CFX_XMLElement* SetValue(XFA_Attribute eAttr,
                           XFA_AttributeType eType,
                           void* pValue,
                           bool bNotify);

  XFA_MAPMODULEDATA* CreateMapModuleData();
  XFA_MAPMODULEDATA* GetMapModuleData() const;
  void SetMapModuleValue(void* pKey, void* pValue);
  bool GetMapModuleValue(void* pKey, void*& pValue);
  bool GetMapModuleString(void* pKey, WideStringView& wsValue);
  void SetMapModuleBuffer(void* pKey,
                          void* pValue,
                          int32_t iBytes,
                          XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo);
  bool GetMapModuleBuffer(void* pKey,
                          void*& pValue,
                          int32_t& iBytes,
                          bool bProtoAlso) const;
  bool HasMapModuleKey(void* pKey);
  void ClearMapModuleBuffer();
  void RemoveMapModuleKey(void* pKey);
  void MoveBufferMapData(CXFA_Object* pDstModule);

  std::unique_ptr<CXFA_WidgetData> widget_data_;
  UnownedPtr<CXFA_Object> object_;
  std::unique_ptr<XFA_MAPMODULEDATA> map_module_data_;
  std::unique_ptr<CXFA_CalcData> calc_data_;
  std::map<ByteString, CJX_MethodCall> method_specs_;
};

#endif  // FXJS_XFA_CJX_OBJECT_H_

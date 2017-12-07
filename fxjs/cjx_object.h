// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJX_OBJECT_H_
#define FXJS_CJX_OBJECT_H_

#include <map>
#include <memory>

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "xfa/fxfa/fxfa_basic.h"

typedef void (*CJX_MethodCall)(CJX_Object* obj, CFXJSE_Arguments* args);

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

class CFXJSE_Value;
class CXFA_CalcData;
class CXFA_Document;
class CXFA_Object;
struct XFA_MAPMODULEDATA;

class CJX_Object {
 public:
  virtual ~CJX_Object();

  CXFA_Object* GetXFAObject() { return object_.Get(); }
  const CXFA_Object* GetXFAObject() const { return object_.Get(); }

  CXFA_Document* GetDocument() const;

  bool HasMethod(const WideString& func) const;
  void RunMethod(const WideString& func, CFXJSE_Arguments* args);

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

  void SetAttributeValue(const WideString& wsValue,
                         const WideString& wsXMLValue,
                         bool bNotify,
                         bool bScriptModify);

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

  void ThrowInvalidPropertyException() const;
  void ThrowArgumentMismatchException() const;
  void ThrowIndexOutOfBoundsException() const;
  void ThrowParamCountMismatchException(const WideString& method) const;

 protected:
  explicit CJX_Object(CXFA_Object* obj);

  void DefineMethods(const CJX_MethodSpec method_specs[]);

  void MoveBufferMapData(CXFA_Object* pSrcModule, CXFA_Object* pDstModule);
  void SetMapModuleString(void* pKey, const WideStringView& wsValue);
  void ThrowException(const wchar_t* str, ...) const;

 private:
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

  UnownedPtr<CXFA_Object> object_;
  std::unique_ptr<XFA_MAPMODULEDATA> map_module_data_;
  std::unique_ptr<CXFA_CalcData> calc_data_;
  std::map<ByteString, CJX_MethodCall> method_specs_;
};

#endif  // FXJS_CJX_OBJECT_H_

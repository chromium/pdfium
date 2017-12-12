// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_object.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "fxjs/cjs_return.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_object.h"

namespace {

void XFA_DeleteWideString(void* pData) {
  delete static_cast<WideString*>(pData);
}

void XFA_CopyWideString(void*& pData) {
  if (!pData)
    return;
  pData = new WideString(*reinterpret_cast<WideString*>(pData));
}

XFA_MAPDATABLOCKCALLBACKINFO deleteWideStringCallBack = {XFA_DeleteWideString,
                                                         XFA_CopyWideString};

enum XFA_KEYTYPE {
  XFA_KEYTYPE_Custom,
  XFA_KEYTYPE_Element,
};

void* GetMapKey_Custom(const WideStringView& wsKey) {
  uint32_t dwKey = FX_HashCode_GetW(wsKey, false);
  return (void*)(uintptr_t)((dwKey << 1) | XFA_KEYTYPE_Custom);
}

void* GetMapKey_Element(XFA_Element eType, XFA_Attribute eAttribute) {
  return (void*)(uintptr_t)((static_cast<uint32_t>(eType) << 16) |
                            (static_cast<uint32_t>(eAttribute) << 8) |
                            XFA_KEYTYPE_Element);
}

void XFA_DefaultFreeData(void* pData) {}

XFA_MAPDATABLOCKCALLBACKINFO gs_XFADefaultFreeData = {XFA_DefaultFreeData,
                                                      nullptr};

}  // namespace

struct XFA_MAPDATABLOCK {
  uint8_t* GetData() const { return (uint8_t*)this + sizeof(XFA_MAPDATABLOCK); }

  XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo;
  int32_t iBytes;
};

struct XFA_MAPMODULEDATA {
  XFA_MAPMODULEDATA() {}
  ~XFA_MAPMODULEDATA() {}

  std::map<void*, void*> m_ValueMap;
  std::map<void*, XFA_MAPDATABLOCK*> m_BufferMap;
};

CJX_Object::CJX_Object(CXFA_Object* obj) : object_(obj) {}

CJX_Object::~CJX_Object() {
  ClearMapModuleBuffer();
}

void CJX_Object::DefineMethods(const CJX_MethodSpec method_specs[],
                               size_t count) {
  for (size_t i = 0; i < count; ++i)
    method_specs_[method_specs[i].pName] = method_specs[i].pMethodCall;
}

CXFA_Document* CJX_Object::GetDocument() const {
  return object_->GetDocument();
}

void CJX_Object::Script_ObjectClass_ClassName(CFXJSE_Value* pValue,
                                              bool bSetting,
                                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString(
      FX_UTF8Encode(GetXFAObject()->GetClassName()).AsStringView());
}

bool CJX_Object::HasMethod(const WideString& func) const {
  return pdfium::ContainsKey(method_specs_, func.UTF8Encode());
}

CJS_Return CJX_Object::RunMethod(
    const WideString& func,
    const std::vector<v8::Local<v8::Value>>& params) {
  auto it = method_specs_.find(func.UTF8Encode());
  if (it == method_specs_.end())
    return CJS_Return(false);
  return it->second(this, GetXFAObject()->GetDocument()->GetScriptContext(),
                    params);
}

void CJX_Object::ThrowInvalidPropertyException() const {
  ThrowException(L"Invalid property set operation.");
}

void CJX_Object::ThrowIndexOutOfBoundsException() const {
  ThrowException(L"Index value is out of bounds.");
}

void CJX_Object::ThrowParamCountMismatchException(
    const WideString& method) const {
  ThrowException(L"Incorrect number of parameters calling method '%.16s'.",
                 method.c_str());
}

void CJX_Object::ThrowArgumentMismatchException() const {
  ThrowException(L"Argument mismatch in property or function argument.");
}

void CJX_Object::ThrowException(const wchar_t* str, ...) const {
  va_list arg_ptr;
  va_start(arg_ptr, str);
  WideString wsMessage = WideString::FormatV(str, arg_ptr);
  va_end(arg_ptr);

  ASSERT(!wsMessage.IsEmpty());
  FXJSE_ThrowMessage(wsMessage.UTF8Encode().AsStringView());
}

bool CJX_Object::HasAttribute(XFA_Attribute eAttr) {
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  return HasMapModuleKey(pKey);
}

bool CJX_Object::SetAttribute(XFA_Attribute eAttr,
                              const WideStringView& wsValue,
                              bool bNotify) {
  switch (ToNode(GetXFAObject())->GetAttributeType(eAttr)) {
    case XFA_AttributeType::Enum: {
      pdfium::Optional<XFA_AttributeEnum> item =
          CXFA_Node::NameToAttributeEnum(wsValue);
      return SetEnum(
          eAttr,
          item ? *item : *(ToNode(GetXFAObject())->GetDefaultEnum(eAttr)),
          bNotify);
    }
    case XFA_AttributeType::CData:
      return SetCData(eAttr, WideString(wsValue), bNotify, false);
    case XFA_AttributeType::Boolean:
      return SetBoolean(eAttr, wsValue != L"0", bNotify);
    case XFA_AttributeType::Integer:
      return SetInteger(eAttr,
                        FXSYS_round(FXSYS_wcstof(wsValue.unterminated_c_str(),
                                                 wsValue.GetLength(), nullptr)),
                        bNotify);
    case XFA_AttributeType::Measure:
      return SetMeasure(eAttr, CXFA_Measurement(wsValue), bNotify);
    default:
      break;
  }
  return false;
}

void CJX_Object::SetMapModuleString(void* pKey, const WideStringView& wsValue) {
  SetMapModuleBuffer(pKey, (void*)wsValue.unterminated_c_str(),
                     wsValue.GetLength() * sizeof(wchar_t), nullptr);
}

bool CJX_Object::SetAttribute(const WideStringView& wsAttr,
                              const WideStringView& wsValue,
                              bool bNotify) {
  XFA_Attribute attr = CXFA_Node::NameToAttribute(wsValue);
  if (attr != XFA_Attribute::Unknown)
    return SetAttribute(attr, wsValue, bNotify);

  void* pKey = GetMapKey_Custom(wsAttr);
  SetMapModuleString(pKey, wsValue);
  return true;
}

WideString CJX_Object::GetAttribute(const WideStringView& attr) {
  return TryAttribute(attr, true).value_or(WideString());
}

WideString CJX_Object::GetAttribute(XFA_Attribute attr) {
  return TryAttribute(attr, true).value_or(WideString());
}

pdfium::Optional<WideString> CJX_Object::TryAttribute(XFA_Attribute eAttr,
                                                      bool bUseDefault) {
  switch (ToNode(GetXFAObject())->GetAttributeType(eAttr)) {
    case XFA_AttributeType::Enum: {
      pdfium::Optional<XFA_AttributeEnum> value = TryEnum(eAttr, bUseDefault);
      if (!value)
        return {};

      return {CXFA_Node::AttributeEnumToName(*value)};
    }
    case XFA_AttributeType::CData:
      return TryCData(eAttr, bUseDefault);

    case XFA_AttributeType::Boolean: {
      pdfium::Optional<bool> value = TryBoolean(eAttr, bUseDefault);
      if (!value)
        return {};
      return {*value ? L"1" : L"0"};
    }
    case XFA_AttributeType::Integer: {
      pdfium::Optional<int32_t> iValue = TryInteger(eAttr, bUseDefault);
      if (!iValue)
        return {};
      return {WideString::Format(L"%d", *iValue)};
    }
    case XFA_AttributeType::Measure: {
      pdfium::Optional<CXFA_Measurement> value = TryMeasure(eAttr, bUseDefault);
      if (!value)
        return {};

      return {value->ToString()};
    }
    default:
      break;
  }
  return {};
}

pdfium::Optional<WideString> CJX_Object::TryAttribute(
    const WideStringView& wsAttr,
    bool bUseDefault) {
  XFA_Attribute attr = CXFA_Node::NameToAttribute(wsAttr);
  if (attr != XFA_Attribute::Unknown)
    return TryAttribute(attr, bUseDefault);

  void* pKey = GetMapKey_Custom(wsAttr);
  WideStringView wsValueC;
  if (!GetMapModuleString(pKey, wsValueC))
    return {};

  return {WideString(wsValueC)};
}

void CJX_Object::RemoveAttribute(const WideStringView& wsAttr) {
  void* pKey = GetMapKey_Custom(wsAttr);
  if (pKey)
    RemoveMapModuleKey(pKey);
}

pdfium::Optional<bool> CJX_Object::TryBoolean(XFA_Attribute eAttr,
                                              bool bUseDefault) {
  void* pValue = nullptr;
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  if (GetMapModuleValue(pKey, pValue))
    return {!!pValue};
  if (!bUseDefault)
    return {};

  return ToNode(GetXFAObject())->GetDefaultBoolean(eAttr);
}

bool CJX_Object::SetBoolean(XFA_Attribute eAttr, bool bValue, bool bNotify) {
  CFX_XMLElement* elem = SetValue(eAttr, XFA_AttributeType::Boolean,
                                  (void*)(uintptr_t)bValue, bNotify);
  if (elem)
    elem->SetString(CXFA_Node::AttributeToName(eAttr), bValue ? L"1" : L"0");
  return true;
}

bool CJX_Object::GetBoolean(XFA_Attribute eAttr) {
  return TryBoolean(eAttr, true).value_or(false);
}

bool CJX_Object::SetInteger(XFA_Attribute eAttr, int32_t iValue, bool bNotify) {
  CFX_XMLElement* elem = SetValue(eAttr, XFA_AttributeType::Integer,
                                  (void*)(uintptr_t)iValue, bNotify);
  if (elem) {
    elem->SetString(CXFA_Node::AttributeToName(eAttr),
                    WideString::Format(L"%d", iValue));
  }
  return true;
}

int32_t CJX_Object::GetInteger(XFA_Attribute eAttr) {
  return TryInteger(eAttr, true).value_or(0);
}

pdfium::Optional<int32_t> CJX_Object::TryInteger(XFA_Attribute eAttr,
                                                 bool bUseDefault) {
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  void* pValue = nullptr;
  if (GetMapModuleValue(pKey, pValue))
    return {static_cast<int32_t>(reinterpret_cast<uintptr_t>(pValue))};
  if (!bUseDefault)
    return {};

  return ToNode(GetXFAObject())->GetDefaultInteger(eAttr);
}

pdfium::Optional<XFA_AttributeEnum> CJX_Object::TryEnum(XFA_Attribute eAttr,
                                                        bool bUseDefault) {
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  void* pValue = nullptr;
  if (GetMapModuleValue(pKey, pValue)) {
    return {
        static_cast<XFA_AttributeEnum>(reinterpret_cast<uintptr_t>(pValue))};
  }
  if (!bUseDefault)
    return {};

  return ToNode(GetXFAObject())->GetDefaultEnum(eAttr);
}

bool CJX_Object::SetEnum(XFA_Attribute eAttr,
                         XFA_AttributeEnum eValue,
                         bool bNotify) {
  CFX_XMLElement* elem = SetValue(eAttr, XFA_AttributeType::Enum,
                                  (void*)(uintptr_t)eValue, bNotify);
  if (elem) {
    elem->SetString(CXFA_Node::AttributeToName(eAttr),
                    CXFA_Node::AttributeEnumToName(eValue));
  }
  return true;
}

XFA_AttributeEnum CJX_Object::GetEnum(XFA_Attribute eAttr) {
  return TryEnum(eAttr, true).value_or(XFA_AttributeEnum::Unknown);
}

bool CJX_Object::SetMeasure(XFA_Attribute eAttr,
                            CXFA_Measurement mValue,
                            bool bNotify) {
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  OnChanging(eAttr, bNotify);
  SetMapModuleBuffer(pKey, &mValue, sizeof(CXFA_Measurement), nullptr);
  OnChanged(eAttr, bNotify, false);
  return true;
}

pdfium::Optional<CXFA_Measurement> CJX_Object::TryMeasure(
    XFA_Attribute eAttr,
    bool bUseDefault) const {
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  void* pValue;
  int32_t iBytes;
  if (GetMapModuleBuffer(pKey, pValue, iBytes, true) &&
      iBytes == sizeof(CXFA_Measurement)) {
    return {*reinterpret_cast<CXFA_Measurement*>(pValue)};
  }
  if (!bUseDefault)
    return {};

  return ToNode(GetXFAObject())->GetDefaultMeasurement(eAttr);
}

CXFA_Measurement CJX_Object::GetMeasure(XFA_Attribute eAttr) const {
  return TryMeasure(eAttr, true).value_or(CXFA_Measurement());
}

WideString CJX_Object::GetCData(XFA_Attribute eAttr) {
  return TryCData(eAttr, true).value_or(WideString());
}

bool CJX_Object::SetCData(XFA_Attribute eAttr,
                          const WideString& wsValue,
                          bool bNotify,
                          bool bScriptModify) {
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  OnChanging(eAttr, bNotify);
  if (eAttr == XFA_Attribute::Value) {
    WideString* pClone = new WideString(wsValue);
    SetUserData(pKey, pClone, &deleteWideStringCallBack);
  } else {
    SetMapModuleString(pKey, wsValue.AsStringView());
    if (eAttr == XFA_Attribute::Name)
      ToNode(GetXFAObject())->UpdateNameHash();
  }
  OnChanged(eAttr, bNotify, bScriptModify);

  if (!ToNode(GetXFAObject())->IsNeedSavingXMLNode() ||
      eAttr == XFA_Attribute::QualifiedName ||
      eAttr == XFA_Attribute::BindingNode) {
    return true;
  }

  if (eAttr == XFA_Attribute::Name &&
      (GetXFAObject()->GetElementType() == XFA_Element::DataValue ||
       GetXFAObject()->GetElementType() == XFA_Element::DataGroup)) {
    return true;
  }

  auto* elem =
      static_cast<CFX_XMLElement*>(ToNode(GetXFAObject())->GetXMLMappingNode());
  if (eAttr == XFA_Attribute::Value) {
    FX_XMLNODETYPE eXMLType = elem->GetType();
    switch (eXMLType) {
      case FX_XMLNODE_Element:
        if (ToNode(GetXFAObject())->IsAttributeInXML()) {
          elem->SetString(WideString(GetCData(XFA_Attribute::QualifiedName)),
                          wsValue);
        } else {
          bool bDeleteChildren = true;
          if (ToNode(GetXFAObject())->GetPacketType() ==
              XFA_PacketType::Datasets) {
            for (CXFA_Node* pChildDataNode =
                     ToNode(GetXFAObject())
                         ->GetNodeItem(XFA_NODEITEM_FirstChild);
                 pChildDataNode; pChildDataNode = pChildDataNode->GetNodeItem(
                                     XFA_NODEITEM_NextSibling)) {
              if (!pChildDataNode->GetBindItems()->empty()) {
                bDeleteChildren = false;
                break;
              }
            }
          }
          if (bDeleteChildren)
            elem->DeleteChildren();

          elem->SetTextData(wsValue);
        }
        break;
      case FX_XMLNODE_Text:
        static_cast<CFX_XMLText*>(ToNode(GetXFAObject())->GetXMLMappingNode())
            ->SetText(wsValue);
        break;
      default:
        NOTREACHED();
    }
    return true;
  }
  ASSERT(elem->GetType() == FX_XMLNODE_Element);

  WideString wsAttrName = CXFA_Node::AttributeToName(eAttr);
  if (eAttr == XFA_Attribute::ContentType)
    wsAttrName = L"xfa:" + wsAttrName;

  elem->SetString(wsAttrName, wsValue);
  return true;
}

void CJX_Object::SetAttributeValue(const WideString& wsValue,
                                   const WideString& wsXMLValue,
                                   bool bNotify,
                                   bool bScriptModify) {
  void* pKey =
      GetMapKey_Element(GetXFAObject()->GetElementType(), XFA_Attribute::Value);
  OnChanging(XFA_Attribute::Value, bNotify);
  WideString* pClone = new WideString(wsValue);
  SetUserData(pKey, pClone, &deleteWideStringCallBack);
  OnChanged(XFA_Attribute::Value, bNotify, bScriptModify);
  if (!ToNode(GetXFAObject())->IsNeedSavingXMLNode())
    return;

  auto* elem =
      static_cast<CFX_XMLElement*>(ToNode(GetXFAObject())->GetXMLMappingNode());
  FX_XMLNODETYPE eXMLType = elem->GetType();
  switch (eXMLType) {
    case FX_XMLNODE_Element:
      if (ToNode(GetXFAObject())->IsAttributeInXML()) {
        elem->SetString(WideString(GetCData(XFA_Attribute::QualifiedName)),
                        wsXMLValue);
      } else {
        bool bDeleteChildren = true;
        if (ToNode(GetXFAObject())->GetPacketType() ==
            XFA_PacketType::Datasets) {
          for (CXFA_Node* pChildDataNode =
                   ToNode(GetXFAObject())->GetNodeItem(XFA_NODEITEM_FirstChild);
               pChildDataNode; pChildDataNode = pChildDataNode->GetNodeItem(
                                   XFA_NODEITEM_NextSibling)) {
            if (!pChildDataNode->GetBindItems()->empty()) {
              bDeleteChildren = false;
              break;
            }
          }
        }
        if (bDeleteChildren)
          elem->DeleteChildren();

        elem->SetTextData(wsXMLValue);
      }
      break;
    case FX_XMLNODE_Text:
      static_cast<CFX_XMLText*>(ToNode(GetXFAObject())->GetXMLMappingNode())
          ->SetText(wsXMLValue);
      break;
    default:
      ASSERT(0);
  }
}

pdfium::Optional<WideString> CJX_Object::TryCData(XFA_Attribute eAttr,
                                                  bool bUseDefault) {
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  if (eAttr == XFA_Attribute::Value) {
    void* pData;
    int32_t iBytes = 0;
    WideString* pStr = nullptr;
    if (GetMapModuleBuffer(pKey, pData, iBytes, true) &&
        iBytes == sizeof(void*)) {
      memcpy(&pData, pData, iBytes);
      pStr = reinterpret_cast<WideString*>(pData);
    }
    if (pStr)
      return {*pStr};
  } else {
    WideStringView wsValueC;
    if (GetMapModuleString(pKey, wsValueC))
      return {WideString(wsValueC)};
  }
  if (!bUseDefault)
    return {};

  return ToNode(GetXFAObject())->GetDefaultCData(eAttr);
}

CFX_XMLElement* CJX_Object::SetValue(XFA_Attribute eAttr,
                                     XFA_AttributeType eType,
                                     void* pValue,
                                     bool bNotify) {
  void* pKey = GetMapKey_Element(GetXFAObject()->GetElementType(), eAttr);
  OnChanging(eAttr, bNotify);
  SetMapModuleValue(pKey, pValue);
  OnChanged(eAttr, bNotify, false);
  if (!ToNode(GetXFAObject())->IsNeedSavingXMLNode())
    return nullptr;

  auto* elem =
      static_cast<CFX_XMLElement*>(ToNode(GetXFAObject())->GetXMLMappingNode());
  ASSERT(elem->GetType() == FX_XMLNODE_Element);

  return elem;
}

bool CJX_Object::SetUserData(void* pKey,
                             void* pData,
                             XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo) {
  SetMapModuleBuffer(pKey, &pData, sizeof(void*),
                     pCallbackInfo ? pCallbackInfo : &gs_XFADefaultFreeData);
  return true;
}

XFA_MAPMODULEDATA* CJX_Object::CreateMapModuleData() {
  if (!map_module_data_)
    map_module_data_ = pdfium::MakeUnique<XFA_MAPMODULEDATA>();
  return map_module_data_.get();
}

XFA_MAPMODULEDATA* CJX_Object::GetMapModuleData() const {
  return map_module_data_.get();
}

void CJX_Object::SetMapModuleValue(void* pKey, void* pValue) {
  CreateMapModuleData()->m_ValueMap[pKey] = pValue;
}

bool CJX_Object::GetMapModuleValue(void* pKey, void*& pValue) {
  for (CXFA_Node* pNode = ToNode(GetXFAObject()); pNode;
       pNode = pNode->GetTemplateNode()) {
    XFA_MAPMODULEDATA* pModule = pNode->JSNode()->GetMapModuleData();
    if (pModule) {
      auto it = pModule->m_ValueMap.find(pKey);
      if (it != pModule->m_ValueMap.end()) {
        pValue = it->second;
        return true;
      }
    }
    if (pNode->GetPacketType() == XFA_PacketType::Datasets)
      break;
  }
  return false;
}

bool CJX_Object::GetMapModuleString(void* pKey, WideStringView& wsValue) {
  void* pValue;
  int32_t iBytes;
  if (!GetMapModuleBuffer(pKey, pValue, iBytes, true))
    return false;

  // Defensive measure: no out-of-bounds pointers even if zero length.
  int32_t iChars = iBytes / sizeof(wchar_t);
  wsValue = WideStringView(iChars ? (const wchar_t*)pValue : nullptr, iChars);
  return true;
}

void CJX_Object::SetMapModuleBuffer(
    void* pKey,
    void* pValue,
    int32_t iBytes,
    XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo) {
  XFA_MAPDATABLOCK*& pBuffer = CreateMapModuleData()->m_BufferMap[pKey];
  if (!pBuffer) {
    pBuffer = reinterpret_cast<XFA_MAPDATABLOCK*>(
        FX_Alloc(uint8_t, sizeof(XFA_MAPDATABLOCK) + iBytes));
  } else if (pBuffer->iBytes != iBytes) {
    if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree)
      pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());

    pBuffer = reinterpret_cast<XFA_MAPDATABLOCK*>(
        FX_Realloc(uint8_t, pBuffer, sizeof(XFA_MAPDATABLOCK) + iBytes));
  } else if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree) {
    pBuffer->pCallbackInfo->pFree(
        *reinterpret_cast<void**>(pBuffer->GetData()));
  }
  if (!pBuffer)
    return;

  pBuffer->pCallbackInfo = pCallbackInfo;
  pBuffer->iBytes = iBytes;
  memcpy(pBuffer->GetData(), pValue, iBytes);
}

bool CJX_Object::GetMapModuleBuffer(void* pKey,
                                    void*& pValue,
                                    int32_t& iBytes,
                                    bool bProtoAlso) const {
  XFA_MAPDATABLOCK* pBuffer = nullptr;
  for (const CXFA_Node* pNode = ToNode(GetXFAObject()); pNode;
       pNode = pNode->GetTemplateNode()) {
    XFA_MAPMODULEDATA* pModule = pNode->JSNode()->GetMapModuleData();
    if (pModule) {
      auto it = pModule->m_BufferMap.find(pKey);
      if (it != pModule->m_BufferMap.end()) {
        pBuffer = it->second;
        break;
      }
    }
    if (!bProtoAlso || pNode->GetPacketType() == XFA_PacketType::Datasets)
      break;
  }
  if (!pBuffer)
    return false;

  pValue = pBuffer->GetData();
  iBytes = pBuffer->iBytes;
  return true;
}

bool CJX_Object::HasMapModuleKey(void* pKey) {
  XFA_MAPMODULEDATA* pModule = GetMapModuleData();
  return pModule && (pdfium::ContainsKey(pModule->m_ValueMap, pKey) ||
                     pdfium::ContainsKey(pModule->m_BufferMap, pKey));
}

void CJX_Object::ClearMapModuleBuffer() {
  XFA_MAPMODULEDATA* pModule = GetMapModuleData();
  if (!pModule)
    return;

  for (auto& pair : pModule->m_BufferMap) {
    XFA_MAPDATABLOCK* pBuffer = pair.second;
    if (pBuffer) {
      if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree)
        pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());

      FX_Free(pBuffer);
    }
  }
  pModule->m_BufferMap.clear();
  pModule->m_ValueMap.clear();
}

void CJX_Object::RemoveMapModuleKey(void* pKey) {
  ASSERT(pKey);

  XFA_MAPMODULEDATA* pModule = GetMapModuleData();
  if (!pModule)
    return;

  auto it = pModule->m_BufferMap.find(pKey);
  if (it != pModule->m_BufferMap.end()) {
    XFA_MAPDATABLOCK* pBuffer = it->second;
    if (pBuffer) {
      if (pBuffer->pCallbackInfo && pBuffer->pCallbackInfo->pFree)
        pBuffer->pCallbackInfo->pFree(*(void**)pBuffer->GetData());

      FX_Free(pBuffer);
    }
    pModule->m_BufferMap.erase(it);
  }
  pModule->m_ValueMap.erase(pKey);
  return;
}

void CJX_Object::MergeAllData(CXFA_Object* pDstModule) {
  XFA_MAPMODULEDATA* pDstModuleData =
      ToNode(pDstModule)->JSNode()->CreateMapModuleData();
  XFA_MAPMODULEDATA* pSrcModuleData = GetMapModuleData();
  if (!pSrcModuleData)
    return;

  for (const auto& pair : pSrcModuleData->m_ValueMap)
    pDstModuleData->m_ValueMap[pair.first] = pair.second;

  for (const auto& pair : pSrcModuleData->m_BufferMap) {
    XFA_MAPDATABLOCK* pSrcBuffer = pair.second;
    XFA_MAPDATABLOCK*& pDstBuffer = pDstModuleData->m_BufferMap[pair.first];
    if (pSrcBuffer->pCallbackInfo && pSrcBuffer->pCallbackInfo->pFree &&
        !pSrcBuffer->pCallbackInfo->pCopy) {
      if (pDstBuffer) {
        pDstBuffer->pCallbackInfo->pFree(*(void**)pDstBuffer->GetData());
        pDstModuleData->m_BufferMap.erase(pair.first);
      }
      continue;
    }
    if (!pDstBuffer) {
      pDstBuffer = (XFA_MAPDATABLOCK*)FX_Alloc(
          uint8_t, sizeof(XFA_MAPDATABLOCK) + pSrcBuffer->iBytes);
    } else if (pDstBuffer->iBytes != pSrcBuffer->iBytes) {
      if (pDstBuffer->pCallbackInfo && pDstBuffer->pCallbackInfo->pFree) {
        pDstBuffer->pCallbackInfo->pFree(*(void**)pDstBuffer->GetData());
      }
      pDstBuffer = (XFA_MAPDATABLOCK*)FX_Realloc(
          uint8_t, pDstBuffer, sizeof(XFA_MAPDATABLOCK) + pSrcBuffer->iBytes);
    } else if (pDstBuffer->pCallbackInfo && pDstBuffer->pCallbackInfo->pFree) {
      pDstBuffer->pCallbackInfo->pFree(*(void**)pDstBuffer->GetData());
    }
    if (!pDstBuffer)
      continue;

    pDstBuffer->pCallbackInfo = pSrcBuffer->pCallbackInfo;
    pDstBuffer->iBytes = pSrcBuffer->iBytes;
    memcpy(pDstBuffer->GetData(), pSrcBuffer->GetData(), pSrcBuffer->iBytes);
    if (pDstBuffer->pCallbackInfo && pDstBuffer->pCallbackInfo->pCopy) {
      pDstBuffer->pCallbackInfo->pCopy(*(void**)pDstBuffer->GetData());
    }
  }
}

void CJX_Object::MoveBufferMapData(CXFA_Object* pDstModule) {
  if (!pDstModule)
    return;

  bool bNeedMove = true;
  if (pDstModule->GetElementType() != GetXFAObject()->GetElementType())
    bNeedMove = false;

  if (bNeedMove)
    ToNode(pDstModule)->JSNode()->SetCalcData(ReleaseCalcData());
  if (!pDstModule->IsNodeV())
    return;

  WideString wsValue = ToNode(pDstModule)->JSNode()->GetContent(false);
  WideString wsFormatValue(wsValue);
  CXFA_WidgetData* pWidgetData = ToNode(pDstModule)->GetContainerWidgetData();
  if (pWidgetData)
    wsFormatValue = pWidgetData->GetFormatDataValue(wsValue);

  ToNode(pDstModule)
      ->JSNode()
      ->SetContent(wsValue, wsFormatValue, true, true, true);
}

void CJX_Object::MoveBufferMapData(CXFA_Object* pSrcModule,
                                   CXFA_Object* pDstModule) {
  if (!pSrcModule || !pDstModule)
    return;

  CXFA_Node* pSrcChild =
      ToNode(pSrcModule)->GetNodeItem(XFA_NODEITEM_FirstChild);
  CXFA_Node* pDstChild =
      ToNode(pDstModule)->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pSrcChild && pDstChild) {
    MoveBufferMapData(pSrcChild, pDstChild);

    pSrcChild = pSrcChild->GetNodeItem(XFA_NODEITEM_NextSibling);
    pDstChild = pDstChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  ToNode(pSrcModule)->JSNode()->MoveBufferMapData(pDstModule);
}

void CJX_Object::OnChanging(XFA_Attribute eAttr, bool bNotify) {
  if (!bNotify || !ToNode(GetXFAObject())->IsInitialized())
    return;

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->OnValueChanging(ToNode(GetXFAObject()), eAttr);
}

void CJX_Object::OnChanged(XFA_Attribute eAttr,
                           bool bNotify,
                           bool bScriptModify) {
  if (bNotify && ToNode(GetXFAObject())->IsInitialized())
    ToNode(GetXFAObject())->SendAttributeChangeMessage(eAttr, bScriptModify);
}

void CJX_Object::SetCalcData(std::unique_ptr<CXFA_CalcData> data) {
  calc_data_ = std::move(data);
}

std::unique_ptr<CXFA_CalcData> CJX_Object::ReleaseCalcData() {
  return std::move(calc_data_);
}

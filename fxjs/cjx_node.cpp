// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjx_node.h"

#include <map>
#include <tuple>
#include <vector>

#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "fxjs/cfxjse_arguments.h"
#include "fxjs/cfxjse_engine.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_attachnodelist.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/cxfa_occurdata.h"
#include "xfa/fxfa/parser/cxfa_simple_parser.h"
#include "xfa/fxfa/parser/xfa_utils.h"

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

std::tuple<int32_t, int32_t, int32_t> StrToRGB(const WideString& strRGB) {
  int32_t r = 0;
  int32_t g = 0;
  int32_t b = 0;

  size_t iIndex = 0;
  for (size_t i = 0; i < strRGB.GetLength(); ++i) {
    wchar_t ch = strRGB[i];
    if (ch == L',')
      ++iIndex;
    if (iIndex > 2)
      break;

    int32_t iValue = ch - L'0';
    if (iValue >= 0 && iValue <= 9) {
      switch (iIndex) {
        case 0:
          r = r * 10 + iValue;
          break;
        case 1:
          g = g * 10 + iValue;
          break;
        default:
          b = b * 10 + iValue;
          break;
      }
    }
  }
  return {r, g, b};
}

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

struct XFA_ExecEventParaInfo {
 public:
  uint32_t m_uHash;
  const wchar_t* m_lpcEventName;
  XFA_EVENTTYPE m_eventType;
  uint32_t m_validFlags;
};

const XFA_ExecEventParaInfo gs_eventParaInfos[] = {
    {0x02a6c55a, L"postSubmit", XFA_EVENT_PostSubmit, 0},
    {0x0ab466bb, L"preSubmit", XFA_EVENT_PreSubmit, 0},
    {0x109d7ce7, L"mouseEnter", XFA_EVENT_MouseEnter, 5},
    {0x17fad373, L"postPrint", XFA_EVENT_PostPrint, 0},
    {0x1bfc72d9, L"preOpen", XFA_EVENT_PreOpen, 7},
    {0x2196a452, L"initialize", XFA_EVENT_Initialize, 1},
    {0x27410f03, L"mouseExit", XFA_EVENT_MouseExit, 5},
    {0x33c43dec, L"docClose", XFA_EVENT_DocClose, 0},
    {0x361fa1b6, L"preSave", XFA_EVENT_PreSave, 0},
    {0x36f1c6d8, L"preSign", XFA_EVENT_PreSign, 6},
    {0x4731d6ba, L"exit", XFA_EVENT_Exit, 2},
    {0x56bf456b, L"docReady", XFA_EVENT_DocReady, 0},
    {0x7233018a, L"validate", XFA_EVENT_Validate, 1},
    {0x8808385e, L"indexChange", XFA_EVENT_IndexChange, 3},
    {0x891f4606, L"change", XFA_EVENT_Change, 4},
    {0x9528a7b4, L"prePrint", XFA_EVENT_PrePrint, 0},
    {0x9f693b21, L"mouseDown", XFA_EVENT_MouseDown, 5},
    {0xcdce56b3, L"full", XFA_EVENT_Full, 4},
    {0xd576d08e, L"mouseUp", XFA_EVENT_MouseUp, 5},
    {0xd95657a6, L"click", XFA_EVENT_Click, 4},
    {0xdbfbe02e, L"calculate", XFA_EVENT_Calculate, 1},
    {0xe25fa7b8, L"postOpen", XFA_EVENT_PostOpen, 7},
    {0xe28dce7e, L"enter", XFA_EVENT_Enter, 2},
    {0xfc82d695, L"postSave", XFA_EVENT_PostSave, 0},
    {0xfd54fbb7, L"postSign", XFA_EVENT_PostSign, 6},
};

const XFA_ExecEventParaInfo* GetEventParaInfoByName(
    const WideStringView& wsEventName) {
  uint32_t uHash = FX_HashCode_GetW(wsEventName, false);
  int32_t iStart = 0;
  int32_t iEnd = (sizeof(gs_eventParaInfos) / sizeof(gs_eventParaInfos[0])) - 1;
  do {
    int32_t iMid = (iStart + iEnd) / 2;
    const XFA_ExecEventParaInfo* eventParaInfo = &gs_eventParaInfos[iMid];
    if (uHash == eventParaInfo->m_uHash)
      return eventParaInfo;
    if (uHash < eventParaInfo->m_uHash)
      iEnd = iMid - 1;
    else
      iStart = iMid + 1;
  } while (iStart <= iEnd);
  return nullptr;
}

}  // namespace

static void XFA_DefaultFreeData(void* pData) {}

static XFA_MAPDATABLOCKCALLBACKINFO gs_XFADefaultFreeData = {
    XFA_DefaultFreeData, nullptr};

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

CJX_Node::CJX_Node(CXFA_Node* node) : CJX_Object(node) {}

CJX_Node::~CJX_Node() {
  ClearMapModuleBuffer();
}

CXFA_Node* CJX_Node::GetXFANode() {
  return static_cast<CXFA_Node*>(GetXFAObject());
}

const CXFA_Node* CJX_Node::GetXFANode() const {
  return static_cast<const CXFA_Node*>(GetXFAObject());
}

bool CJX_Node::HasAttribute(XFA_Attribute eAttr) {
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
  return HasMapModuleKey(pKey);
}

bool CJX_Node::SetAttribute(XFA_Attribute eAttr,
                            const WideStringView& wsValue,
                            bool bNotify) {
  XFA_AttributeType eType = GetXFANode()->GetAttributeType(eAttr);
  switch (eType) {
    case XFA_AttributeType::Enum: {
      pdfium::Optional<XFA_AttributeEnum> item =
          CXFA_Node::NameToAttributeEnum(wsValue);
      return SetEnum(eAttr,
                     item ? *item : *(GetXFANode()->GetDefaultEnum(eAttr)),
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

bool CJX_Node::SetAttribute(const WideStringView& wsAttr,
                            const WideStringView& wsValue,
                            bool bNotify) {
  XFA_Attribute attr = CXFA_Node::NameToAttribute(wsValue);
  if (attr != XFA_Attribute::Unknown)
    return SetAttribute(attr, wsValue, bNotify);

  void* pKey = GetMapKey_Custom(wsAttr);
  SetMapModuleString(pKey, wsValue);
  return true;
}

WideString CJX_Node::GetAttribute(const WideStringView& attr) {
  return TryAttribute(attr, true).value_or(WideString());
}

WideString CJX_Node::GetAttribute(XFA_Attribute attr) {
  return TryAttribute(attr, true).value_or(WideString());
}

pdfium::Optional<WideString> CJX_Node::TryAttribute(XFA_Attribute eAttr,
                                                    bool bUseDefault) {
  XFA_AttributeType eType = GetXFANode()->GetAttributeType(eAttr);
  switch (eType) {
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

pdfium::Optional<WideString> CJX_Node::TryAttribute(
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

void CJX_Node::RemoveAttribute(const WideStringView& wsAttr) {
  void* pKey = GetMapKey_Custom(wsAttr);
  if (pKey)
    RemoveMapModuleKey(pKey);
}

int32_t CJX_Node::Subform_and_SubformSet_InstanceIndex() {
  int32_t index = 0;
  for (CXFA_Node* pNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_PrevSibling);
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
    if ((pNode->GetElementType() != XFA_Element::Subform) &&
        (pNode->GetElementType() != XFA_Element::SubformSet)) {
      break;
    }
    index++;
  }
  return index;
}

int32_t CJX_Node::InstanceManager_SetInstances(int32_t iDesired) {
  CXFA_OccurData occurData(GetXFANode()->GetOccurNode());
  if (iDesired < occurData.GetMin()) {
    ThrowTooManyOccurancesException(L"min");
    return 1;
  }

  int32_t iMax = occurData.GetMax();
  if (iMax >= 0 && iDesired > iMax) {
    ThrowTooManyOccurancesException(L"max");
    return 2;
  }

  int32_t iCount = GetXFANode()->GetCount();
  if (iDesired == iCount)
    return 0;

  if (iDesired < iCount) {
    WideString wsInstManagerName = GetCData(XFA_Attribute::Name);
    WideString wsInstanceName = WideString(
        wsInstManagerName.IsEmpty()
            ? wsInstManagerName
            : wsInstManagerName.Right(wsInstManagerName.GetLength() - 1));
    uint32_t dInstanceNameHash =
        FX_HashCode_GetW(wsInstanceName.AsStringView(), false);
    CXFA_Node* pPrevSibling =
        iDesired == 0 ? GetXFANode() : GetXFANode()->GetItem(iDesired - 1);
    while (iCount > iDesired) {
      CXFA_Node* pRemoveInstance =
          pPrevSibling->GetNodeItem(XFA_NODEITEM_NextSibling);
      if (pRemoveInstance->GetElementType() != XFA_Element::Subform &&
          pRemoveInstance->GetElementType() != XFA_Element::SubformSet) {
        continue;
      }
      if (pRemoveInstance->GetElementType() == XFA_Element::InstanceManager) {
        NOTREACHED();
        break;
      }
      if (pRemoveInstance->GetNameHash() == dInstanceNameHash) {
        GetXFANode()->RemoveItem(pRemoveInstance, true);
        iCount--;
      }
    }
  } else {
    while (iCount < iDesired) {
      CXFA_Node* pNewInstance = GetXFANode()->CreateInstance(true);
      GetXFANode()->InsertItem(pNewInstance, iCount, iCount, false);
      iCount++;
      CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
      if (!pNotify)
        return 0;

      pNotify->RunNodeInitialize(pNewInstance);
    }
  }

  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (pLayoutPro) {
    pLayoutPro->AddChangedContainer(
        ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
  }
  return 0;
}

int32_t CJX_Node::InstanceManager_MoveInstance(int32_t iTo, int32_t iFrom) {
  int32_t iCount = GetXFANode()->GetCount();
  if (iFrom > iCount || iTo > iCount - 1) {
    ThrowIndexOutOfBoundsException();
    return 1;
  }
  if (iFrom < 0 || iTo < 0 || iFrom == iTo)
    return 0;

  CXFA_Node* pMoveInstance = GetXFANode()->GetItem(iFrom);
  GetXFANode()->RemoveItem(pMoveInstance, false);
  GetXFANode()->InsertItem(pMoveInstance, iTo, iCount - 1, true);
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (pLayoutPro) {
    pLayoutPro->AddChangedContainer(
        ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
  }
  return 0;
}

void CJX_Node::Script_TreeClass_ResolveNode(CFXJSE_Arguments* pArguments) {
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
  CXFA_Node* refNode = GetXFANode();
  if (refNode->GetElementType() == XFA_Element::Xfa)
    refNode = ToNode(pScriptContext->GetThisObject());
  uint32_t dwFlag = XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                    XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                    XFA_RESOLVENODE_Siblings;
  XFA_RESOLVENODE_RS resolveNodeRS;
  int32_t iRet = pScriptContext->ResolveObjects(
      refNode, wsExpression.AsStringView(), resolveNodeRS, dwFlag);
  if (iRet < 1) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }
  if (resolveNodeRS.dwFlags == XFA_RESOLVENODE_RSTYPE_Nodes) {
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

void CJX_Node::Script_TreeClass_ResolveNodes(CFXJSE_Arguments* pArguments) {
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

  CXFA_Node* refNode = GetXFANode();
  if (refNode->GetElementType() == XFA_Element::Xfa)
    refNode = ToNode(GetDocument()->GetScriptContext()->GetThisObject());

  ResolveNodeList(pValue, wsExpression,
                  XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
                      XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
                      XFA_RESOLVENODE_Siblings,
                  refNode);
}

void CJX_Node::ResolveNodeList(CFXJSE_Value* pValue,
                               WideString wsExpression,
                               uint32_t dwFlag,
                               CXFA_Node* refNode) {
  CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
  if (!pScriptContext)
    return;

  XFA_RESOLVENODE_RS resolveNodeRS;
  if (!refNode)
    refNode = GetXFANode();

  pScriptContext->ResolveObjects(refNode, wsExpression.AsStringView(),
                                 resolveNodeRS, dwFlag);
  CXFA_ArrayNodeList* pNodeList = new CXFA_ArrayNodeList(GetDocument());
  if (resolveNodeRS.dwFlags == XFA_RESOLVENODE_RSTYPE_Nodes) {
    for (CXFA_Object* pObject : resolveNodeRS.objects) {
      if (pObject->IsNode())
        pNodeList->Append(pObject->AsNode());
    }
  } else {
    CXFA_ValueArray valueArray(pScriptContext->GetRuntime());
    if (resolveNodeRS.GetAttributeResult(&valueArray) > 0) {
      for (CXFA_Object* pObject : valueArray.GetAttributeObject()) {
        if (pObject->IsNode())
          pNodeList->Append(pObject->AsNode());
      }
    }
  }
  pValue->SetObject(pNodeList, pScriptContext->GetJseNormalClass());
}

void CJX_Node::Script_TreeClass_All(CFXJSE_Value* pValue,
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

void CJX_Node::Script_TreeClass_Nodes(CFXJSE_Value* pValue,
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
      new CXFA_AttachNodeList(GetDocument(), GetXFANode());
  pValue->SetObject(pNodeList, pScriptContext->GetJseNormalClass());
}

void CJX_Node::Script_TreeClass_ClassAll(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  WideString wsExpression = L"#" + GetXFANode()->GetClassName() + L"[*]";
  ResolveNodeList(pValue, wsExpression,
                  XFA_RESOLVENODE_Siblings | XFA_RESOLVENODE_ALL, nullptr);
}

void CJX_Node::Script_TreeClass_Parent(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  CXFA_Node* pParent = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
  if (!pParent) {
    pValue->SetNull();
    return;
  }

  pValue->Assign(GetDocument()->GetScriptContext()->GetJSValueFromMap(pParent));
}

void CJX_Node::Script_TreeClass_Index(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(GetXFANode()->GetNodeSameNameIndex());
}

void CJX_Node::Script_TreeClass_ClassIndex(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(GetXFANode()->GetNodeSameClassIndex());
}

void CJX_Node::Script_TreeClass_SomExpression(CFXJSE_Value* pValue,
                                              bool bSetting,
                                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  WideString wsSOMExpression;
  GetXFANode()->GetSOMExpression(wsSOMExpression);
  pValue->SetString(wsSOMExpression.UTF8Encode().AsStringView());
}

void CJX_Node::Script_NodeClass_ApplyXSL(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"applyXSL");
    return;
  }

  // TODO(weili): check whether we need to implement this, pdfium:501.
}

void CJX_Node::Script_NodeClass_AssignNode(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 3) {
    ThrowParamCountMismatchException(L"assignNode");
    return;
  }

  // TODO(weili): check whether we need to implement this, pdfium:501.
}

void CJX_Node::Script_NodeClass_Clone(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"clone");
    return;
  }

  CXFA_Node* pCloneNode = GetXFANode()->Clone(!!pArguments->GetInt32(0));
  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pCloneNode));
}

void CJX_Node::Script_NodeClass_GetAttribute(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"getAttribute");
    return;
  }

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (!pValue)
    return;

  WideString wsExpression =
      WideString::FromUTF8(pArguments->GetUTF8String(0).AsStringView());
  pValue->SetString(
      GetAttribute(wsExpression.AsStringView()).UTF8Encode().AsStringView());
}

void CJX_Node::Script_NodeClass_GetElement(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 2) {
    ThrowParamCountMismatchException(L"getElement");
    return;
  }

  WideString wsExpression =
      WideString::FromUTF8(pArguments->GetUTF8String(0).AsStringView());
  int32_t iValue = iLength >= 2 ? pArguments->GetInt32(1) : 0;

  CXFA_Node* pNode =
      GetProperty(iValue, CXFA_Node::NameToElement(wsExpression), true);
  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNode));
}

void CJX_Node::Script_NodeClass_IsPropertySpecified(
    CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 3) {
    ThrowParamCountMismatchException(L"isPropertySpecified");
    return;
  }

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (!pValue)
    return;

  WideString wsExpression =
      WideString::FromUTF8(pArguments->GetUTF8String(0).AsStringView());
  XFA_Attribute attr = CXFA_Node::NameToAttribute(wsExpression.AsStringView());
  if (attr != XFA_Attribute::Unknown && HasAttribute(attr)) {
    pValue->SetBoolean(true);
    return;
  }

  bool bParent = iLength < 2 || pArguments->GetInt32(1);
  int32_t iIndex = iLength == 3 ? pArguments->GetInt32(2) : 0;
  XFA_Element eType = CXFA_Node::NameToElement(wsExpression);
  bool bHas = !!GetProperty(iIndex, eType, true);
  if (!bHas && bParent && GetXFANode()->GetParent()) {
    // Also check on the parent.
    auto* jsnode = GetXFANode()->GetParent()->JSNode();
    bHas = jsnode->HasAttribute(attr) ||
           !!jsnode->GetProperty(iIndex, eType, true);
  }
  pValue->SetBoolean(bHas);
}

void CJX_Node::Script_NodeClass_LoadXML(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 3) {
    ThrowParamCountMismatchException(L"loadXML");
    return;
  }

  ByteString wsExpression = pArguments->GetUTF8String(0);
  if (wsExpression.IsEmpty())
    return;

  bool bIgnoreRoot = true;
  if (iLength >= 2)
    bIgnoreRoot = !!pArguments->GetInt32(1);

  bool bOverwrite = 0;
  if (iLength >= 3)
    bOverwrite = !!pArguments->GetInt32(2);

  auto pParser = pdfium::MakeUnique<CXFA_SimpleParser>(GetDocument(), false);
  if (!pParser)
    return;

  CFX_XMLNode* pXMLNode = pParser->ParseXMLData(wsExpression);
  if (!pXMLNode)
    return;

  if (bIgnoreRoot &&
      (pXMLNode->GetType() != FX_XMLNODE_Element ||
       XFA_RecognizeRichText(static_cast<CFX_XMLElement*>(pXMLNode)))) {
    bIgnoreRoot = false;
  }

  CXFA_Node* pFakeRoot = GetXFANode()->Clone(false);
  WideString wsContentType = GetCData(XFA_Attribute::ContentType);
  if (!wsContentType.IsEmpty()) {
    pFakeRoot->JSNode()->SetCData(XFA_Attribute::ContentType,
                                  WideString(wsContentType), false, false);
  }

  std::unique_ptr<CFX_XMLNode> pFakeXMLRoot(pFakeRoot->GetXMLMappingNode());
  if (!pFakeXMLRoot) {
    CFX_XMLNode* pThisXMLRoot = GetXFANode()->GetXMLMappingNode();
    pFakeXMLRoot = pThisXMLRoot ? pThisXMLRoot->Clone() : nullptr;
  }
  if (!pFakeXMLRoot) {
    pFakeXMLRoot = pdfium::MakeUnique<CFX_XMLElement>(
        WideString(GetXFANode()->GetClassName()));
  }

  if (bIgnoreRoot) {
    CFX_XMLNode* pXMLChild = pXMLNode->GetNodeItem(CFX_XMLNode::FirstChild);
    while (pXMLChild) {
      CFX_XMLNode* pXMLSibling =
          pXMLChild->GetNodeItem(CFX_XMLNode::NextSibling);
      pXMLNode->RemoveChildNode(pXMLChild);
      pFakeXMLRoot->InsertChildNode(pXMLChild);
      pXMLChild = pXMLSibling;
    }
  } else {
    CFX_XMLNode* pXMLParent = pXMLNode->GetNodeItem(CFX_XMLNode::Parent);
    if (pXMLParent)
      pXMLParent->RemoveChildNode(pXMLNode);

    pFakeXMLRoot->InsertChildNode(pXMLNode);
  }

  pParser->ConstructXFANode(pFakeRoot, pFakeXMLRoot.get());
  pFakeRoot = pParser->GetRootNode();
  if (!pFakeRoot)
    return;

  if (bOverwrite) {
    CXFA_Node* pChild = GetXFANode()->GetNodeItem(XFA_NODEITEM_FirstChild);
    CXFA_Node* pNewChild = pFakeRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
    int32_t index = 0;
    while (pNewChild) {
      CXFA_Node* pItem = pNewChild->GetNodeItem(XFA_NODEITEM_NextSibling);
      pFakeRoot->RemoveChild(pNewChild, true);
      GetXFANode()->InsertChild(index++, pNewChild);
      pNewChild->SetFlag(XFA_NodeFlag_Initialized, true);
      pNewChild = pItem;
    }

    while (pChild) {
      CXFA_Node* pItem = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
      GetXFANode()->RemoveChild(pChild, true);
      pFakeRoot->InsertChild(pChild, nullptr);
      pChild = pItem;
    }

    if (GetXFANode()->GetPacketType() == XFA_PacketType::Form &&
        GetXFANode()->GetElementType() == XFA_Element::ExData) {
      CFX_XMLNode* pTempXMLNode = GetXFANode()->GetXMLMappingNode();
      GetXFANode()->SetXMLMappingNode(pFakeXMLRoot.release());
      GetXFANode()->SetFlag(XFA_NodeFlag_OwnXMLNode, false);
      if (pTempXMLNode && !pTempXMLNode->GetNodeItem(CFX_XMLNode::Parent))
        pFakeXMLRoot.reset(pTempXMLNode);
      else
        pFakeXMLRoot = nullptr;
    }
    MoveBufferMapData(pFakeRoot, GetXFANode());
  } else {
    CXFA_Node* pChild = pFakeRoot->GetNodeItem(XFA_NODEITEM_FirstChild);
    while (pChild) {
      CXFA_Node* pItem = pChild->GetNodeItem(XFA_NODEITEM_NextSibling);
      pFakeRoot->RemoveChild(pChild, true);
      GetXFANode()->InsertChild(pChild, nullptr);
      pChild->SetFlag(XFA_NodeFlag_Initialized, true);
      pChild = pItem;
    }
  }

  if (pFakeXMLRoot) {
    pFakeRoot->SetXMLMappingNode(pFakeXMLRoot.release());
    pFakeRoot->SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  pFakeRoot->SetFlag(XFA_NodeFlag_HasRemovedChildren, false);
}

void CJX_Node::Script_NodeClass_SaveFilteredXML(CFXJSE_Arguments* pArguments) {
  // TODO(weili): Check whether we need to implement this, pdfium:501.
}

void CJX_Node::Script_NodeClass_SaveXML(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 0 || iLength > 1) {
    ThrowParamCountMismatchException(L"saveXML");
    return;
  }

  if (iLength == 1 && pArguments->GetUTF8String(0) != "pretty") {
    ThrowArgumentMismatchException();
    return;
  }

  // TODO(weili): Check whether we need to save pretty print XML, pdfium:501.

  WideString bsXMLHeader = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  if (GetXFANode()->GetPacketType() != XFA_PacketType::Form &&
      GetXFANode()->GetPacketType() != XFA_PacketType::Datasets) {
    pArguments->GetReturnValue()->SetString("");
    return;
  }

  CFX_XMLNode* pElement = nullptr;
  if (GetXFANode()->GetPacketType() == XFA_PacketType::Datasets) {
    pElement = GetXFANode()->GetXMLMappingNode();
    if (!pElement || pElement->GetType() != FX_XMLNODE_Element) {
      pArguments->GetReturnValue()->SetString(
          bsXMLHeader.UTF8Encode().AsStringView());
      return;
    }
    XFA_DataExporter_DealWithDataGroupNode(GetXFANode());
  }

  auto pMemoryStream = pdfium::MakeRetain<CFX_MemoryStream>(true);
  auto pStream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(pMemoryStream, true);
  pStream->SetCodePage(FX_CODEPAGE_UTF8);
  pStream->WriteString(bsXMLHeader.AsStringView());

  if (GetXFANode()->GetPacketType() == XFA_PacketType::Form)
    XFA_DataExporter_RegenerateFormFile(GetXFANode(), pStream, nullptr, true);
  else
    pElement->SaveXMLNode(pStream);

  pArguments->GetReturnValue()->SetString(
      ByteStringView(pMemoryStream->GetBuffer(), pMemoryStream->GetSize()));
}

void CJX_Node::Script_NodeClass_SetAttribute(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 2) {
    ThrowParamCountMismatchException(L"setAttribute");
    return;
  }

  WideString wsAttributeValue =
      WideString::FromUTF8(pArguments->GetUTF8String(0).AsStringView());
  WideString wsAttribute =
      WideString::FromUTF8(pArguments->GetUTF8String(1).AsStringView());
  SetAttribute(wsAttribute.AsStringView(), wsAttributeValue.AsStringView(),
               true);
}

void CJX_Node::Script_NodeClass_SetElement(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1 && iLength != 2) {
    ThrowParamCountMismatchException(L"setElement");
    return;
  }

  // TODO(weili): check whether we need to implement this, pdfium:501.
}

void CJX_Node::Script_NodeClass_Ns(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString(
      TryNamespace().value_or(WideString()).UTF8Encode().AsStringView());
}

void CJX_Node::Script_NodeClass_Model(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  pValue->Assign(GetDocument()->GetScriptContext()->GetJSValueFromMap(
      GetXFANode()->GetModelNode()));
}

void CJX_Node::Script_NodeClass_IsContainer(CFXJSE_Value* pValue,
                                            bool bSetting,
                                            XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  pValue->SetBoolean(GetXFANode()->IsContainerNode());
}

void CJX_Node::Script_NodeClass_IsNull(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  if (GetXFANode()->GetElementType() == XFA_Element::Subform) {
    pValue->SetBoolean(false);
    return;
  }
  pValue->SetBoolean(GetContent(false).IsEmpty());
}

void CJX_Node::Script_NodeClass_OneOfChild(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  std::vector<CXFA_Node*> properties = GetXFANode()->GetNodeList(
      XFA_NODEFILTER_OneOfProperty, XFA_Element::Unknown);
  if (!properties.empty()) {
    pValue->Assign(GetDocument()->GetScriptContext()->GetJSValueFromMap(
        properties.front()));
  }
}

void CJX_Node::Script_ContainerClass_GetDelta(CFXJSE_Arguments* pArguments) {}

void CJX_Node::Script_ContainerClass_GetDeltas(CFXJSE_Arguments* pArguments) {
  CXFA_ArrayNodeList* pFormNodes = new CXFA_ArrayNodeList(GetDocument());
  pArguments->GetReturnValue()->SetObject(
      pFormNodes, GetDocument()->GetScriptContext()->GetJseNormalClass());
}

void CJX_Node::Script_ModelClass_ClearErrorList(CFXJSE_Arguments* pArguments) {}

void CJX_Node::Script_ModelClass_CreateNode(CFXJSE_Arguments* pArguments) {
  Script_Template_CreateNode(pArguments);
}

void CJX_Node::Script_ModelClass_IsCompatibleNS(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1) {
    ThrowParamCountMismatchException(L"isCompatibleNS");
    return;
  }

  WideString wsNameSpace;
  if (iLength >= 1) {
    ByteString bsNameSpace = pArguments->GetUTF8String(0);
    wsNameSpace = WideString::FromUTF8(bsNameSpace.AsStringView());
  }

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (!pValue)
    return;

  pValue->SetBoolean(TryNamespace().value_or(WideString()) == wsNameSpace);
}

void CJX_Node::Script_ModelClass_Context(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {}

void CJX_Node::Script_ModelClass_AliasNode(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {}

void CJX_Node::Script_Attribute_Integer(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_Attribute eAttribute) {
  if (bSetting) {
    SetInteger(eAttribute, pValue->ToInteger(), true);
    return;
  }
  pValue->SetInteger(GetInteger(eAttribute));
}

void CJX_Node::Script_Attribute_IntegerRead(CFXJSE_Value* pValue,
                                            bool bSetting,
                                            XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(GetInteger(eAttribute));
}

void CJX_Node::Script_Attribute_BOOL(CFXJSE_Value* pValue,
                                     bool bSetting,
                                     XFA_Attribute eAttribute) {
  if (bSetting) {
    SetBoolean(eAttribute, pValue->ToBoolean(), true);
    return;
  }
  pValue->SetString(GetBoolean(eAttribute) ? "1" : "0");
}

void CJX_Node::Script_Attribute_BOOLRead(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString(GetBoolean(eAttribute) ? "1" : "0");
}

void CJX_Node::OnChanging(XFA_Attribute eAttr, bool bNotify) {
  if (!bNotify || !GetXFANode()->IsInitialized())
    return;

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify)
    pNotify->OnValueChanging(GetXFANode(), eAttr);
}

void CJX_Node::OnChanged(XFA_Attribute eAttr,
                         bool bNotify,
                         bool bScriptModify) {
  if (bNotify && GetXFANode()->IsInitialized())
    SendAttributeChangeMessage(eAttr, bScriptModify);
}

void CJX_Node::SendAttributeChangeMessage(XFA_Attribute eAttribute,
                                          bool bScriptModify) {
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (!pLayoutPro)
    return;

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (GetXFANode()->GetPacketType() != XFA_PacketType::Form) {
    pNotify->OnValueChanged(GetXFANode(), eAttribute, GetXFANode(),
                            GetXFANode());
    return;
  }

  bool bNeedFindContainer = false;
  switch (GetXFANode()->GetElementType()) {
    case XFA_Element::Caption:
      bNeedFindContainer = true;
      pNotify->OnValueChanged(GetXFANode(), eAttribute, GetXFANode(),
                              GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent));
      break;
    case XFA_Element::Font:
    case XFA_Element::Para: {
      bNeedFindContainer = true;
      CXFA_Node* pParentNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
      if (pParentNode->GetElementType() == XFA_Element::Caption) {
        pNotify->OnValueChanged(GetXFANode(), eAttribute, pParentNode,
                                pParentNode->GetNodeItem(XFA_NODEITEM_Parent));
      } else {
        pNotify->OnValueChanged(GetXFANode(), eAttribute, GetXFANode(),
                                pParentNode);
      }
      break;
    }
    case XFA_Element::Margin: {
      bNeedFindContainer = true;
      CXFA_Node* pParentNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
      XFA_Element eParentType = pParentNode->GetElementType();
      if (pParentNode->IsContainerNode()) {
        pNotify->OnValueChanged(GetXFANode(), eAttribute, GetXFANode(),
                                pParentNode);
      } else if (eParentType == XFA_Element::Caption) {
        pNotify->OnValueChanged(GetXFANode(), eAttribute, pParentNode,
                                pParentNode->GetNodeItem(XFA_NODEITEM_Parent));
      } else {
        CXFA_Node* pNode = pParentNode->GetNodeItem(XFA_NODEITEM_Parent);
        if (pNode && pNode->GetElementType() == XFA_Element::Ui) {
          pNotify->OnValueChanged(GetXFANode(), eAttribute, pNode,
                                  pNode->GetNodeItem(XFA_NODEITEM_Parent));
        }
      }
      break;
    }
    case XFA_Element::Comb: {
      CXFA_Node* pEditNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
      XFA_Element eUIType = pEditNode->GetElementType();
      if (pEditNode && (eUIType == XFA_Element::DateTimeEdit ||
                        eUIType == XFA_Element::NumericEdit ||
                        eUIType == XFA_Element::TextEdit)) {
        CXFA_Node* pUINode = pEditNode->GetNodeItem(XFA_NODEITEM_Parent);
        if (pUINode) {
          pNotify->OnValueChanged(GetXFANode(), eAttribute, pUINode,
                                  pUINode->GetNodeItem(XFA_NODEITEM_Parent));
        }
      }
      break;
    }
    case XFA_Element::Button:
    case XFA_Element::Barcode:
    case XFA_Element::ChoiceList:
    case XFA_Element::DateTimeEdit:
    case XFA_Element::NumericEdit:
    case XFA_Element::PasswordEdit:
    case XFA_Element::TextEdit: {
      CXFA_Node* pUINode = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
      if (pUINode) {
        pNotify->OnValueChanged(GetXFANode(), eAttribute, pUINode,
                                pUINode->GetNodeItem(XFA_NODEITEM_Parent));
      }
      break;
    }
    case XFA_Element::CheckButton: {
      bNeedFindContainer = true;
      CXFA_Node* pUINode = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
      if (pUINode) {
        pNotify->OnValueChanged(GetXFANode(), eAttribute, pUINode,
                                pUINode->GetNodeItem(XFA_NODEITEM_Parent));
      }
      break;
    }
    case XFA_Element::Keep:
    case XFA_Element::Bookend:
    case XFA_Element::Break:
    case XFA_Element::BreakAfter:
    case XFA_Element::BreakBefore:
    case XFA_Element::Overflow:
      bNeedFindContainer = true;
      break;
    case XFA_Element::Area:
    case XFA_Element::Draw:
    case XFA_Element::ExclGroup:
    case XFA_Element::Field:
    case XFA_Element::Subform:
    case XFA_Element::SubformSet:
      pLayoutPro->AddChangedContainer(GetXFANode());
      pNotify->OnValueChanged(GetXFANode(), eAttribute, GetXFANode(),
                              GetXFANode());
      break;
    case XFA_Element::Sharptext:
    case XFA_Element::Sharpxml:
    case XFA_Element::SharpxHTML: {
      CXFA_Node* pTextNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
      if (!pTextNode)
        return;

      CXFA_Node* pValueNode = pTextNode->GetNodeItem(XFA_NODEITEM_Parent);
      if (!pValueNode)
        return;

      XFA_Element eType = pValueNode->GetElementType();
      if (eType == XFA_Element::Value) {
        bNeedFindContainer = true;
        CXFA_Node* pNode = pValueNode->GetNodeItem(XFA_NODEITEM_Parent);
        if (pNode && pNode->IsContainerNode()) {
          if (bScriptModify)
            pValueNode = pNode;

          pNotify->OnValueChanged(GetXFANode(), eAttribute, pValueNode, pNode);
        } else {
          pNotify->OnValueChanged(GetXFANode(), eAttribute, pNode,
                                  pNode->GetNodeItem(XFA_NODEITEM_Parent));
        }
      } else {
        if (eType == XFA_Element::Items) {
          CXFA_Node* pNode = pValueNode->GetNodeItem(XFA_NODEITEM_Parent);
          if (pNode && pNode->IsContainerNode()) {
            pNotify->OnValueChanged(GetXFANode(), eAttribute, pValueNode,
                                    pNode);
          }
        }
      }
      break;
    }
    default:
      break;
  }

  if (!bNeedFindContainer)
    return;

  CXFA_Node* pParent = GetXFANode();
  while (pParent && !pParent->IsContainerNode())
    pParent = pParent->GetNodeItem(XFA_NODEITEM_Parent);

  if (pParent)
    pLayoutPro->AddChangedContainer(pParent);
}

void CJX_Node::Script_Attribute_String(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_Attribute eAttribute) {
  if (!bSetting) {
    pValue->SetString(GetAttribute(eAttribute).UTF8Encode().AsStringView());
    return;
  }

  WideString wsValue = pValue->ToWideString();
  SetAttribute(eAttribute, wsValue.AsStringView(), true);
  if (eAttribute != XFA_Attribute::Use ||
      GetXFANode()->GetElementType() != XFA_Element::Desc) {
    return;
  }

  CXFA_Node* pTemplateNode =
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Template));
  CXFA_Node* pProtoRoot =
      pTemplateNode->GetFirstChildByClass(XFA_Element::Subform)
          ->GetFirstChildByClass(XFA_Element::Proto);

  WideString wsID;
  WideString wsSOM;
  if (!wsValue.IsEmpty()) {
    if (wsValue[0] == '#')
      wsID = WideString(wsValue.c_str() + 1, wsValue.GetLength() - 1);
    else
      wsSOM = wsValue;
  }

  CXFA_Node* pProtoNode = nullptr;
  if (!wsSOM.IsEmpty()) {
    XFA_RESOLVENODE_RS resolveNodeRS;
    int32_t iRet = GetDocument()->GetScriptContext()->ResolveObjects(
        pProtoRoot, wsSOM.AsStringView(), resolveNodeRS,
        XFA_RESOLVENODE_Children | XFA_RESOLVENODE_Attributes |
            XFA_RESOLVENODE_Properties | XFA_RESOLVENODE_Parent |
            XFA_RESOLVENODE_Siblings);
    if (iRet > 0 && resolveNodeRS.objects.front()->IsNode())
      pProtoNode = resolveNodeRS.objects.front()->AsNode();

  } else if (!wsID.IsEmpty()) {
    pProtoNode = GetDocument()->GetNodeByID(pProtoRoot, wsID.AsStringView());
  }
  if (!pProtoNode)
    return;

  CXFA_Node* pHeadChild = GetXFANode()->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pHeadChild) {
    CXFA_Node* pSibling = pHeadChild->GetNodeItem(XFA_NODEITEM_NextSibling);
    GetXFANode()->RemoveChild(pHeadChild, true);
    pHeadChild = pSibling;
  }

  std::unique_ptr<CXFA_Node> pProtoForm(pProtoNode->CloneTemplateToForm(true));
  pHeadChild = pProtoForm->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pHeadChild) {
    CXFA_Node* pSibling = pHeadChild->GetNodeItem(XFA_NODEITEM_NextSibling);
    pProtoForm->RemoveChild(pHeadChild, true);
    GetXFANode()->InsertChild(pHeadChild, nullptr);
    pHeadChild = pSibling;
  }

  GetDocument()->RemovePurgeNode(pProtoForm.get());
}

void CJX_Node::Script_Attribute_StringRead(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString(GetAttribute(eAttribute).UTF8Encode().AsStringView());
}

void CJX_Node::Script_WsdlConnection_Execute(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 0 && argc != 1) {
    ThrowParamCountMismatchException(L"execute");
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(false);
}

void CJX_Node::Script_Delta_Restore(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"restore");
}

void CJX_Node::Script_Delta_CurrentValue(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {}

void CJX_Node::Script_Delta_SavedValue(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_Attribute eAttribute) {}

void CJX_Node::Script_Delta_Target(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {}

void CJX_Node::Script_Som_Message(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_SOM_MESSAGETYPE iMessageType) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  bool bNew = false;
  CXFA_ValidateData validateData = pWidgetData->GetValidateData(false);
  if (!validateData.HasValidNode()) {
    validateData = pWidgetData->GetValidateData(true);
    bNew = true;
  }

  if (bSetting) {
    switch (iMessageType) {
      case XFA_SOM_ValidationMessage:
        validateData.SetScriptMessageText(pValue->ToWideString());
        break;
      case XFA_SOM_FormatMessage:
        validateData.SetFormatMessageText(pValue->ToWideString());
        break;
      case XFA_SOM_MandatoryMessage:
        validateData.SetNullMessageText(pValue->ToWideString());
        break;
      default:
        break;
    }
    if (!bNew) {
      CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
      if (!pNotify) {
        return;
      }
      pNotify->AddCalcValidate(GetXFANode());
    }
    return;
  }

  WideString wsMessage;
  switch (iMessageType) {
    case XFA_SOM_ValidationMessage:
      wsMessage = validateData.GetScriptMessageText();
      break;
    case XFA_SOM_FormatMessage:
      wsMessage = validateData.GetFormatMessageText();
      break;
    case XFA_SOM_MandatoryMessage:
      wsMessage = validateData.GetNullMessageText();
      break;
    default:
      break;
  }
  pValue->SetString(wsMessage.UTF8Encode().AsStringView());
}

void CJX_Node::Script_Som_ValidationMessage(CFXJSE_Value* pValue,
                                            bool bSetting,
                                            XFA_Attribute eAttribute) {
  Script_Som_Message(pValue, bSetting, XFA_SOM_ValidationMessage);
}

void CJX_Node::Script_Field_Length(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pValue->SetInteger(0);
    return;
  }
  pValue->SetInteger(pWidgetData->CountChoiceListItems(true));
}

void CJX_Node::Script_Som_DefaultValue(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_Attribute eAttribute) {
  XFA_Element eType = GetXFANode()->GetElementType();
  if (eType == XFA_Element::Field) {
    Script_Field_DefaultValue(pValue, bSetting, eAttribute);
    return;
  }
  if (eType == XFA_Element::Draw) {
    Script_Draw_DefaultValue(pValue, bSetting, eAttribute);
    return;
  }
  if (eType == XFA_Element::Boolean) {
    Script_Boolean_Value(pValue, bSetting, eAttribute);
    return;
  }
  if (bSetting) {
    WideString wsNewValue;
    if (pValue && !(pValue->IsNull() || pValue->IsUndefined()))
      wsNewValue = pValue->ToWideString();

    WideString wsFormatValue(wsNewValue);
    CXFA_WidgetData* pContainerWidgetData = nullptr;
    if (GetXFANode()->GetPacketType() == XFA_PacketType::Datasets) {
      WideString wsPicture;
      for (const auto& pFormNode : *(GetXFANode()->GetBindItems())) {
        if (!pFormNode || pFormNode->HasRemovedChildren())
          continue;

        pContainerWidgetData = pFormNode->GetContainerWidgetData();
        if (pContainerWidgetData) {
          wsPicture = pContainerWidgetData->GetPictureContent(
              XFA_VALUEPICTURE_DataBind);
        }
        if (!wsPicture.IsEmpty())
          break;

        pContainerWidgetData = nullptr;
      }
    } else if (GetXFANode()->GetPacketType() == XFA_PacketType::Form) {
      pContainerWidgetData = GetXFANode()->GetContainerWidgetData();
    }

    if (pContainerWidgetData)
      wsFormatValue = pContainerWidgetData->GetFormatDataValue(wsNewValue);

    SetContent(wsNewValue, wsFormatValue, true, true, true);
    return;
  }

  WideString content = GetContent(true);
  if (content.IsEmpty() && eType != XFA_Element::Text &&
      eType != XFA_Element::SubmitUrl) {
    pValue->SetNull();
  } else if (eType == XFA_Element::Integer) {
    pValue->SetInteger(FXSYS_wtoi(content.c_str()));
  } else if (eType == XFA_Element::Float || eType == XFA_Element::Decimal) {
    CFX_Decimal decimal(content.AsStringView());
    pValue->SetFloat((float)(double)decimal);
  } else {
    pValue->SetString(content.UTF8Encode().AsStringView());
  }
}

void CJX_Node::Script_Som_DefaultValue_Read(CFXJSE_Value* pValue,
                                            bool bSetting,
                                            XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  WideString content = GetContent(true);
  if (content.IsEmpty()) {
    pValue->SetNull();
    return;
  }
  pValue->SetString(content.UTF8Encode().AsStringView());
}

void CJX_Node::Script_Boolean_Value(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  if (!bSetting) {
    WideString wsValue = GetContent(true);
    pValue->SetBoolean(wsValue == L"1");
    return;
  }

  ByteString newValue;
  if (pValue && !(pValue->IsNull() || pValue->IsUndefined()))
    newValue = pValue->ToString();

  int32_t iValue = FXSYS_atoi(newValue.c_str());
  WideString wsNewValue(iValue == 0 ? L"0" : L"1");
  WideString wsFormatValue(wsNewValue);
  CXFA_WidgetData* pContainerWidgetData =
      GetXFANode()->GetContainerWidgetData();
  if (pContainerWidgetData)
    wsFormatValue = pContainerWidgetData->GetFormatDataValue(wsNewValue);

  SetContent(wsNewValue, wsFormatValue, true, true, true);
}

void CJX_Node::Script_Som_BorderColor(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  CXFA_BorderData borderData = pWidgetData->GetBorderData(true);
  int32_t iSize = borderData.CountEdges();
  if (bSetting) {
    int32_t r = 0;
    int32_t g = 0;
    int32_t b = 0;
    std::tie(r, g, b) = StrToRGB(pValue->ToWideString());
    FX_ARGB rgb = ArgbEncode(100, r, g, b);
    for (int32_t i = 0; i < iSize; ++i)
      borderData.GetEdgeData(i).SetColor(rgb);

    return;
  }

  FX_ARGB color = borderData.GetEdgeData(0).GetColor();
  int32_t a;
  int32_t r;
  int32_t g;
  int32_t b;
  std::tie(a, r, g, b) = ArgbDecode(color);
  pValue->SetString(
      WideString::Format(L"%d,%d,%d", r, g, b).UTF8Encode().AsStringView());
}

void CJX_Node::Script_Som_BorderWidth(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  CXFA_BorderData borderData = pWidgetData->GetBorderData(true);
  if (bSetting) {
    CXFA_Measurement thickness = borderData.GetEdgeData(0).GetMSThickness();
    pValue->SetString(thickness.ToString().UTF8Encode().AsStringView());
    return;
  }

  WideString wsThickness = pValue->ToWideString();
  for (int32_t i = 0; i < borderData.CountEdges(); ++i) {
    borderData.GetEdgeData(i).SetMSThickness(
        CXFA_Measurement(wsThickness.AsStringView()));
  }
}

void CJX_Node::Script_Som_FillColor(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  CXFA_BorderData borderData = pWidgetData->GetBorderData(true);
  CXFA_FillData borderfillData = borderData.GetFillData(true);
  CXFA_Node* pNode = borderfillData.GetNode();
  if (!pNode)
    return;

  if (bSetting) {
    int32_t r;
    int32_t g;
    int32_t b;
    std::tie(r, g, b) = StrToRGB(pValue->ToWideString());
    FX_ARGB color = ArgbEncode(0xff, r, g, b);
    borderfillData.SetColor(color);
    return;
  }

  FX_ARGB color = borderfillData.GetColor(false);
  int32_t a;
  int32_t r;
  int32_t g;
  int32_t b;
  std::tie(a, r, g, b) = ArgbDecode(color);
  pValue->SetString(
      WideString::Format(L"%d,%d,%d", r, g, b).UTF8Encode().AsStringView());
}

void CJX_Node::Script_Som_DataNode(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  CXFA_Node* pDataNode = GetXFANode()->GetBindData();
  if (!pDataNode) {
    pValue->SetNull();
    return;
  }

  pValue->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pDataNode));
}

void CJX_Node::Script_Draw_DefaultValue(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_Attribute eAttribute) {
  if (!bSetting) {
    WideString content = GetContent(true);
    if (content.IsEmpty())
      pValue->SetNull();
    else
      pValue->SetString(content.UTF8Encode().AsStringView());

    return;
  }

  if (!pValue || !pValue->IsString())
    return;

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  XFA_Element uiType = pWidgetData->GetUIType();
  if (uiType != XFA_Element::Text)
    return;

  WideString wsNewValue = pValue->ToWideString();
  SetContent(wsNewValue, wsNewValue, true, true, true);
}

void CJX_Node::Script_Field_DefaultValue(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  if (bSetting) {
    if (pValue) {
      pWidgetData->SetPreNull(pWidgetData->IsNull());
      pWidgetData->SetIsNull(pValue->IsNull());
    }

    WideString wsNewText;
    if (pValue && !(pValue->IsNull() || pValue->IsUndefined()))
      wsNewText = pValue->ToWideString();

    CXFA_Node* pUIChild = pWidgetData->GetUIChild();
    if (pUIChild->GetElementType() == XFA_Element::NumericEdit) {
      wsNewText =
          pWidgetData->NumericLimit(wsNewText, pWidgetData->GetLeadDigits(),
                                    pWidgetData->GetFracDigits());
    }

    CXFA_WidgetData* pContainerWidgetData =
        GetXFANode()->GetContainerWidgetData();
    WideString wsFormatText(wsNewText);
    if (pContainerWidgetData)
      wsFormatText = pContainerWidgetData->GetFormatDataValue(wsNewText);

    SetContent(wsNewText, wsFormatText, true, true, true);
    return;
  }

  WideString content = GetContent(true);
  if (content.IsEmpty()) {
    pValue->SetNull();
    return;
  }

  CXFA_Node* pUIChild = pWidgetData->GetUIChild();
  CXFA_Node* pNode = pWidgetData->GetFormValueData().GetNode()->GetNodeItem(
      XFA_NODEITEM_FirstChild);
  if (pNode && pNode->GetElementType() == XFA_Element::Decimal) {
    if (pUIChild->GetElementType() == XFA_Element::NumericEdit &&
        (pNode->JSNode()->GetInteger(XFA_Attribute::FracDigits) == -1)) {
      pValue->SetString(content.UTF8Encode().AsStringView());
    } else {
      CFX_Decimal decimal(content.AsStringView());
      pValue->SetFloat((float)(double)decimal);
    }
  } else if (pNode && pNode->GetElementType() == XFA_Element::Integer) {
    pValue->SetInteger(FXSYS_wtoi(content.c_str()));
  } else if (pNode && pNode->GetElementType() == XFA_Element::Boolean) {
    pValue->SetBoolean(FXSYS_wtoi(content.c_str()) == 0 ? false : true);
  } else if (pNode && pNode->GetElementType() == XFA_Element::Float) {
    CFX_Decimal decimal(content.AsStringView());
    pValue->SetFloat((float)(double)decimal);
  } else {
    pValue->SetString(content.UTF8Encode().AsStringView());
  }
}

void CJX_Node::Script_Field_EditValue(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  if (bSetting) {
    pWidgetData->SetValue(XFA_VALUEPICTURE_Edit, pValue->ToWideString());
    return;
  }
  pValue->SetString(
      pWidgetData->GetValue(XFA_VALUEPICTURE_Edit).UTF8Encode().AsStringView());
}

void CJX_Node::Script_Som_FontColor(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  CXFA_FontData fontData = pWidgetData->GetFontData(true);
  CXFA_Node* pNode = fontData.GetNode();
  if (!pNode)
    return;

  if (bSetting) {
    int32_t r;
    int32_t g;
    int32_t b;
    std::tie(r, g, b) = StrToRGB(pValue->ToWideString());
    FX_ARGB color = ArgbEncode(0xff, r, g, b);
    fontData.SetColor(color);
    return;
  }

  int32_t a;
  int32_t r;
  int32_t g;
  int32_t b;
  std::tie(a, r, g, b) = ArgbDecode(fontData.GetColor());
  pValue->SetString(ByteString::Format("%d,%d,%d", r, g, b).AsStringView());
}

void CJX_Node::Script_Field_FormatMessage(CFXJSE_Value* pValue,
                                          bool bSetting,
                                          XFA_Attribute eAttribute) {
  Script_Som_Message(pValue, bSetting, XFA_SOM_FormatMessage);
}

void CJX_Node::Script_Field_FormattedValue(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  if (bSetting) {
    pWidgetData->SetValue(XFA_VALUEPICTURE_Display, pValue->ToWideString());
    return;
  }
  pValue->SetString(pWidgetData->GetValue(XFA_VALUEPICTURE_Display)
                        .UTF8Encode()
                        .AsStringView());
}

void CJX_Node::Script_Som_Mandatory(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  CXFA_ValidateData validateData = pWidgetData->GetValidateData(true);
  if (bSetting) {
    validateData.SetNullTest(pValue->ToWideString());
    return;
  }

  WideString str = CXFA_Node::AttributeEnumToName(validateData.GetNullTest());
  pValue->SetString(str.UTF8Encode().AsStringView());
}

void CJX_Node::Script_Som_MandatoryMessage(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {
  Script_Som_Message(pValue, bSetting, XFA_SOM_MandatoryMessage);
}

void CJX_Node::Script_Field_ParentSubform(CFXJSE_Value* pValue,
                                          bool bSetting,
                                          XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetNull();
}

void CJX_Node::Script_Field_SelectedIndex(CFXJSE_Value* pValue,
                                          bool bSetting,
                                          XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  if (!bSetting) {
    pValue->SetInteger(pWidgetData->GetSelectedItem(0));
    return;
  }

  int32_t iIndex = pValue->ToInteger();
  if (iIndex == -1) {
    pWidgetData->ClearAllSelections();
    return;
  }

  pWidgetData->SetItemState(iIndex, true, true, true, true);
}

void CJX_Node::Script_Field_ClearItems(CFXJSE_Arguments* pArguments) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  pWidgetData->DeleteItem(-1, true, false);
}

void CJX_Node::Script_Field_ExecEvent(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"execEvent");
    return;
  }

  ByteString eventString = pArguments->GetUTF8String(0);
  int32_t iRet = execSingleEventByName(
      WideString::FromUTF8(eventString.AsStringView()).AsStringView(),
      XFA_Element::Field);
  if (eventString != "validate")
    return;

  pArguments->GetReturnValue()->SetBoolean(
      (iRet == XFA_EVENTERROR_Error) ? false : true);
}

void CJX_Node::Script_Field_ExecInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize, false,
                                false);
}

void CJX_Node::Script_Field_DeleteItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"deleteItem");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  int32_t iIndex = pArguments->GetInt32(0);
  bool bValue = pWidgetData->DeleteItem(iIndex, true, true);
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetBoolean(bValue);
}

void CJX_Node::Script_Field_GetSaveItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"getSaveItem");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  pdfium::Optional<WideString> value =
      pWidgetData->GetChoiceListItem(iIndex, true);
  if (!value) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }
  pArguments->GetReturnValue()->SetString(value->UTF8Encode().AsStringView());
}

void CJX_Node::Script_Field_BoundItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"boundItem");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  ByteString bsValue = pArguments->GetUTF8String(0);
  WideString wsValue = WideString::FromUTF8(bsValue.AsStringView());
  WideString wsBoundValue = pWidgetData->GetItemValue(wsValue.AsStringView());
  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetString(wsBoundValue.UTF8Encode().AsStringView());
}

void CJX_Node::Script_Field_GetItemState(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"getItemState");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  CFXJSE_Value* pValue = pArguments->GetReturnValue();
  if (pValue)
    pValue->SetBoolean(pWidgetData->GetItemState(pArguments->GetInt32(0)));
}

void CJX_Node::Script_Field_ExecCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate, false,
                                false);
}

void CJX_Node::Script_Field_SetItems(CFXJSE_Arguments* pArguments) {}

void CJX_Node::Script_Field_GetDisplayItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength != 1) {
    ThrowParamCountMismatchException(L"getDisplayItem");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  if (iIndex < 0) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  pdfium::Optional<WideString> value =
      pWidgetData->GetChoiceListItem(iIndex, false);
  if (!value) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }
  pArguments->GetReturnValue()->SetString(value->UTF8Encode().AsStringView());
}

void CJX_Node::Script_Field_SetItemState(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 2) {
    ThrowParamCountMismatchException(L"setItemState");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  int32_t iIndex = pArguments->GetInt32(0);
  if (pArguments->GetInt32(1) != 0) {
    pWidgetData->SetItemState(iIndex, true, true, true, true);
    return;
  }

  if (pWidgetData->GetItemState(iIndex))
    pWidgetData->SetItemState(iIndex, false, true, true, true);
}

void CJX_Node::Script_Field_AddItem(CFXJSE_Arguments* pArguments) {
  int32_t iLength = pArguments->GetLength();
  if (iLength < 1 || iLength > 2) {
    ThrowParamCountMismatchException(L"addItem");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  WideString wsLabel;
  if (iLength >= 1) {
    ByteString bsLabel = pArguments->GetUTF8String(0);
    wsLabel = WideString::FromUTF8(bsLabel.AsStringView());
  }

  WideString wsValue;
  if (iLength >= 2) {
    ByteString bsValue = pArguments->GetUTF8String(1);
    wsValue = WideString::FromUTF8(bsValue.AsStringView());
  }

  pWidgetData->InsertItem(wsLabel, wsValue, true);
}

void CJX_Node::Script_Field_ExecValidate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execValidate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }

  int32_t iRet = pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate,
                                               false, false);
  pArguments->GetReturnValue()->SetBoolean(
      (iRet == XFA_EVENTERROR_Error) ? false : true);
}

void CJX_Node::Script_ExclGroup_ErrorText(CFXJSE_Value* pValue,
                                          bool bSetting,
                                          XFA_Attribute eAttribute) {
  if (bSetting)
    ThrowInvalidPropertyException();
}

void CJX_Node::Script_ExclGroup_DefaultAndRawValue(CFXJSE_Value* pValue,
                                                   bool bSetting,
                                                   XFA_Attribute eAttribute) {
  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData)
    return;

  if (bSetting) {
    pWidgetData->SetSelectedMemberByValue(pValue->ToWideString().AsStringView(),
                                          true, true, true);
    return;
  }

  WideString wsValue = GetContent(true);
  XFA_VERSION curVersion = GetDocument()->GetCurVersionMode();
  if (wsValue.IsEmpty() && curVersion >= XFA_VERSION_300) {
    pValue->SetNull();
    return;
  }
  pValue->SetString(wsValue.UTF8Encode().AsStringView());
}

void CJX_Node::Script_ExclGroup_Transient(CFXJSE_Value* pValue,
                                          bool bSetting,
                                          XFA_Attribute eAttribute) {}

void CJX_Node::Script_ExclGroup_ExecEvent(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"execEvent");
    return;
  }

  ByteString eventString = pArguments->GetUTF8String(0);
  execSingleEventByName(
      WideString::FromUTF8(eventString.AsStringView()).AsStringView(),
      XFA_Element::ExclGroup);
}

void CJX_Node::Script_ExclGroup_SelectedMember(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc < 0 || argc > 1) {
    ThrowParamCountMismatchException(L"selectedMember");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  CXFA_Node* pReturnNode = nullptr;
  if (argc == 0) {
    pReturnNode = pWidgetData->GetSelectedMember();
  } else {
    ByteString szName;
    szName = pArguments->GetUTF8String(0);
    pReturnNode = pWidgetData->SetSelectedMember(
        WideString::FromUTF8(szName.AsStringView()).AsStringView(), true);
  }
  if (!pReturnNode) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pReturnNode));
}

void CJX_Node::Script_ExclGroup_ExecInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize);
}

void CJX_Node::Script_ExclGroup_ExecCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
}

void CJX_Node::Script_ExclGroup_ExecValidate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execValidate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }

  int32_t iRet =
      pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate);
  pArguments->GetReturnValue()->SetBoolean(
      (iRet == XFA_EVENTERROR_Error) ? false : true);
}

void CJX_Node::Script_Som_InstanceIndex(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_Attribute eAttribute) {
  if (!bSetting) {
    pValue->SetInteger(Subform_and_SubformSet_InstanceIndex());
    return;
  }

  int32_t iTo = pValue->ToInteger();
  int32_t iFrom = Subform_and_SubformSet_InstanceIndex();
  CXFA_Node* pManagerNode = nullptr;
  for (CXFA_Node* pNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_PrevSibling);
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
    if (pNode->GetElementType() == XFA_Element::InstanceManager) {
      pManagerNode = pNode;
      break;
    }
  }
  if (!pManagerNode)
    return;

  pManagerNode->JSNode()->InstanceManager_MoveInstance(iTo, iFrom);
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_Node* pToInstance = pManagerNode->GetItem(iTo);
  if (pToInstance && pToInstance->GetElementType() == XFA_Element::Subform) {
    pNotify->RunSubformIndexChange(pToInstance);
  }

  CXFA_Node* pFromInstance = pManagerNode->GetItem(iFrom);
  if (pFromInstance &&
      pFromInstance->GetElementType() == XFA_Element::Subform) {
    pNotify->RunSubformIndexChange(pFromInstance);
  }
}

void CJX_Node::Script_Subform_InstanceManager(CFXJSE_Value* pValue,
                                              bool bSetting,
                                              XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }

  WideString wsName = GetCData(XFA_Attribute::Name);
  CXFA_Node* pInstanceMgr = nullptr;
  for (CXFA_Node* pNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_PrevSibling);
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
    if (pNode->GetElementType() == XFA_Element::InstanceManager) {
      WideString wsInstMgrName = pNode->JSNode()->GetCData(XFA_Attribute::Name);
      if (wsInstMgrName.GetLength() >= 1 && wsInstMgrName[0] == '_' &&
          wsInstMgrName.Right(wsInstMgrName.GetLength() - 1) == wsName) {
        pInstanceMgr = pNode;
      }
      break;
    }
  }
  if (!pInstanceMgr) {
    pValue->SetNull();
    return;
  }

  pValue->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pInstanceMgr));
}

void CJX_Node::Script_Subform_Locale(CFXJSE_Value* pValue,
                                     bool bSetting,
                                     XFA_Attribute eAttribute) {
  if (bSetting) {
    SetCData(XFA_Attribute::Locale, pValue->ToWideString(), true, true);
    return;
  }

  WideString wsLocaleName;
  GetXFANode()->GetLocaleName(wsLocaleName);
  pValue->SetString(wsLocaleName.UTF8Encode().AsStringView());
}

void CJX_Node::Script_Subform_ExecEvent(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"execEvent");
    return;
  }

  ByteString eventString = pArguments->GetUTF8String(0);
  execSingleEventByName(
      WideString::FromUTF8(eventString.AsStringView()).AsStringView(),
      XFA_Element::Subform);
}

void CJX_Node::Script_Subform_ExecInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize);
}

void CJX_Node::Script_Subform_ExecCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
}

void CJX_Node::Script_Subform_ExecValidate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execValidate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }

  int32_t iRet =
      pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate);
  pArguments->GetReturnValue()->SetBoolean(
      (iRet == XFA_EVENTERROR_Error) ? false : true);
}

void CJX_Node::Script_Subform_GetInvalidObjects(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"getInvalidObjects");
}

void CJX_Node::Script_Template_FormNodes(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"formNodes");
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Node::Script_Template_Remerge(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"remerge");
    return;
  }
  GetDocument()->DoDataRemerge(true);
}

void CJX_Node::Script_Template_ExecInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Node::Script_Template_CreateNode(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc <= 0 || argc >= 4) {
    ThrowParamCountMismatchException(L"createNode");
    return;
  }

  WideString strName;
  WideString strNameSpace;
  if (argc > 1) {
    ByteString bsName = pArguments->GetUTF8String(1);
    strName = WideString::FromUTF8(bsName.AsStringView());
    if (argc == 3) {
      ByteString bsNameSpace = pArguments->GetUTF8String(2);
      strNameSpace = WideString::FromUTF8(bsNameSpace.AsStringView());
    }
  }

  ByteString bsTagName = pArguments->GetUTF8String(0);
  WideString strTagName = WideString::FromUTF8(bsTagName.AsStringView());
  XFA_Element eType = CXFA_Node::NameToElement(strTagName);
  CXFA_Node* pNewNode = GetXFANode()->CreateSamePacketNode(eType);
  if (!pNewNode) {
    pArguments->GetReturnValue()->SetNull();
    return;
  }

  if (strName.IsEmpty()) {
    pArguments->GetReturnValue()->Assign(
        GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewNode));
    return;
  }

  if (!pNewNode->HasAttribute(XFA_Attribute::Name)) {
    ThrowMissingPropertyException(strTagName, L"name");
    return;
  }

  pNewNode->JSNode()->SetAttribute(XFA_Attribute::Name, strName.AsStringView(),
                                   true);
  if (pNewNode->GetPacketType() == XFA_PacketType::Datasets)
    pNewNode->CreateXMLMappingNode();

  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewNode));
}

void CJX_Node::Script_Template_Recalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"recalculate");
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Node::Script_Template_ExecCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Node::Script_Template_ExecValidate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execValidate");
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(!!GetXFANode()->GetWidgetData());
}

void CJX_Node::Script_Manifest_Evaluate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"evaluate");
    return;
  }

  CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
  if (!pWidgetData) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }
  pArguments->GetReturnValue()->SetBoolean(true);
}

void CJX_Node::Script_InstanceManager_Max(CFXJSE_Value* pValue,
                                          bool bSetting,
                                          XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMax());
}

void CJX_Node::Script_InstanceManager_Min(CFXJSE_Value* pValue,
                                          bool bSetting,
                                          XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetInteger(CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMin());
}

void CJX_Node::Script_InstanceManager_Count(CFXJSE_Value* pValue,
                                            bool bSetting,
                                            XFA_Attribute eAttribute) {
  if (bSetting) {
    pValue->SetInteger(GetXFANode()->GetCount());
    return;
  }
  InstanceManager_SetInstances(pValue->ToInteger());
}

void CJX_Node::Script_InstanceManager_MoveInstance(
    CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 2) {
    pArguments->GetReturnValue()->SetUndefined();
    return;
  }

  int32_t iFrom = pArguments->GetInt32(0);
  int32_t iTo = pArguments->GetInt32(1);
  InstanceManager_MoveInstance(iTo, iFrom);
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  CXFA_Node* pToInstance = GetXFANode()->GetItem(iTo);
  if (pToInstance && pToInstance->GetElementType() == XFA_Element::Subform)
    pNotify->RunSubformIndexChange(pToInstance);

  CXFA_Node* pFromInstance = GetXFANode()->GetItem(iFrom);
  if (pFromInstance &&
      pFromInstance->GetElementType() == XFA_Element::Subform) {
    pNotify->RunSubformIndexChange(pFromInstance);
  }
}

void CJX_Node::Script_InstanceManager_RemoveInstance(
    CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    pArguments->GetReturnValue()->SetUndefined();
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  int32_t iCount = GetXFANode()->GetCount();
  if (iIndex < 0 || iIndex >= iCount) {
    ThrowIndexOutOfBoundsException();
    return;
  }

  int32_t iMin = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMin();
  if (iCount - 1 < iMin) {
    ThrowTooManyOccurancesException(L"min");
    return;
  }

  CXFA_Node* pRemoveInstance = GetXFANode()->GetItem(iIndex);
  GetXFANode()->RemoveItem(pRemoveInstance, true);
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (pNotify) {
    for (int32_t i = iIndex; i < iCount - 1; i++) {
      CXFA_Node* pSubformInstance = GetXFANode()->GetItem(i);
      if (pSubformInstance &&
          pSubformInstance->GetElementType() == XFA_Element::Subform) {
        pNotify->RunSubformIndexChange(pSubformInstance);
      }
    }
  }
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (!pLayoutPro)
    return;

  pLayoutPro->AddChangedContainer(
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
}

void CJX_Node::Script_InstanceManager_SetInstances(
    CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    pArguments->GetReturnValue()->SetUndefined();
    return;
  }

  int32_t iDesired = pArguments->GetInt32(0);
  InstanceManager_SetInstances(iDesired);
}

void CJX_Node::Script_InstanceManager_AddInstance(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 0 && argc != 1) {
    ThrowParamCountMismatchException(L"addInstance");
    return;
  }

  bool fFlags = true;
  if (argc == 1)
    fFlags = pArguments->GetInt32(0) == 0 ? false : true;

  int32_t iCount = GetXFANode()->GetCount();
  int32_t iMax = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMax();
  if (iMax >= 0 && iCount >= iMax) {
    ThrowTooManyOccurancesException(L"max");
    return;
  }

  CXFA_Node* pNewInstance = GetXFANode()->CreateInstance(fFlags);
  GetXFANode()->InsertItem(pNewInstance, iCount, iCount, false);
  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewInstance));
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->RunNodeInitialize(pNewInstance);
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (!pLayoutPro)
    return;

  pLayoutPro->AddChangedContainer(
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
}

void CJX_Node::Script_InstanceManager_InsertInstance(
    CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 1 && argc != 2) {
    ThrowParamCountMismatchException(L"insertInstance");
    return;
  }

  int32_t iIndex = pArguments->GetInt32(0);
  bool bBind = false;
  if (argc == 2)
    bBind = pArguments->GetInt32(1) == 0 ? false : true;

  int32_t iCount = GetXFANode()->GetCount();
  if (iIndex < 0 || iIndex > iCount) {
    ThrowIndexOutOfBoundsException();
    return;
  }

  int32_t iMax = CXFA_OccurData(GetXFANode()->GetOccurNode()).GetMax();
  if (iMax >= 0 && iCount >= iMax) {
    ThrowTooManyOccurancesException(L"max");
    return;
  }

  CXFA_Node* pNewInstance = GetXFANode()->CreateInstance(bBind);
  GetXFANode()->InsertItem(pNewInstance, iIndex, iCount, true);
  pArguments->GetReturnValue()->Assign(
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNewInstance));
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->RunNodeInitialize(pNewInstance);
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (!pLayoutPro)
    return;

  pLayoutPro->AddChangedContainer(
      ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Form)));
}

void CJX_Node::Script_Occur_Max(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute) {
  CXFA_OccurData occurData(GetXFANode());
  if (!bSetting) {
    pValue->SetInteger(occurData.GetMax());
    return;
  }
  occurData.SetMax(pValue->ToInteger());
}

void CJX_Node::Script_Occur_Min(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute) {
  CXFA_OccurData occurData(GetXFANode());
  if (!bSetting) {
    pValue->SetInteger(occurData.GetMin());
    return;
  }
  occurData.SetMin(pValue->ToInteger());
}

void CJX_Node::Script_Desc_Metadata(CFXJSE_Arguments* pArguments) {
  int32_t argc = pArguments->GetLength();
  if (argc != 0 && argc != 1) {
    ThrowParamCountMismatchException(L"metadata");
    return;
  }
  pArguments->GetReturnValue()->SetString("");
}

void CJX_Node::Script_Form_FormNodes(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"formNodes");
    return;
  }

  CXFA_Node* pDataNode = static_cast<CXFA_Node*>(pArguments->GetObject(0));
  if (!pDataNode) {
    ThrowArgumentMismatchException();
    return;
  }

  std::vector<CXFA_Node*> formItems;
  CXFA_ArrayNodeList* pFormNodes = new CXFA_ArrayNodeList(GetDocument());
  pFormNodes->SetArrayNodeList(formItems);
  pArguments->GetReturnValue()->SetObject(
      pFormNodes, GetDocument()->GetScriptContext()->GetJseNormalClass());
}

void CJX_Node::Script_Form_Remerge(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"remerge");
    return;
  }

  GetDocument()->DoDataRemerge(true);
}

void CJX_Node::Script_Form_ExecInitialize(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execInitialize");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Initialize);
}

void CJX_Node::Script_Form_Recalculate(CFXJSE_Arguments* pArguments) {
  CXFA_EventParam* pEventParam =
      GetDocument()->GetScriptContext()->GetEventParam();
  if (pEventParam->m_eType == XFA_EVENT_Calculate ||
      pEventParam->m_eType == XFA_EVENT_InitCalculate) {
    return;
  }
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"recalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;
  if (pArguments->GetInt32(0) != 0)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate);
  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Ready, true);
}

void CJX_Node::Script_Form_ExecCalculate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execCalculate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Calculate);
}

void CJX_Node::Script_Form_ExecValidate(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0) {
    ThrowParamCountMismatchException(L"execValidate");
    return;
  }

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify) {
    pArguments->GetReturnValue()->SetBoolean(false);
    return;
  }

  int32_t iRet =
      pNotify->ExecEventByDeepFirst(GetXFANode(), XFA_EVENT_Validate);
  pArguments->GetReturnValue()->SetBoolean(
      (iRet == XFA_EVENTERROR_Error) ? false : true);
}

void CJX_Node::Script_Form_Checksum(CFXJSE_Value* pValue,
                                    bool bSetting,
                                    XFA_Attribute eAttribute) {
  if (bSetting) {
    SetAttribute(XFA_Attribute::Checksum, pValue->ToWideString().AsStringView(),
                 false);
    return;
  }

  pdfium::Optional<WideString> checksum =
      TryAttribute(XFA_Attribute::Checksum, false);
  pValue->SetString(checksum ? checksum->UTF8Encode().AsStringView() : "");
}

void CJX_Node::Script_Packet_GetAttribute(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"getAttribute");
    return;
  }

  WideString wsAttributeValue;
  CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
  if (pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element) {
    ByteString bsAttributeName = pArguments->GetUTF8String(0);
    wsAttributeValue = static_cast<CFX_XMLElement*>(pXMLNode)->GetString(
        WideString::FromUTF8(bsAttributeName.AsStringView()).c_str());
  }

  pArguments->GetReturnValue()->SetString(
      wsAttributeValue.UTF8Encode().AsStringView());
}

void CJX_Node::Script_Packet_SetAttribute(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 2) {
    ThrowParamCountMismatchException(L"setAttribute");
    return;
  }

  CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
  if (pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element) {
    ByteString bsValue = pArguments->GetUTF8String(0);
    ByteString bsName = pArguments->GetUTF8String(1);
    static_cast<CFX_XMLElement*>(pXMLNode)->SetString(
        WideString::FromUTF8(bsName.AsStringView()),
        WideString::FromUTF8(bsValue.AsStringView()));
  }
  pArguments->GetReturnValue()->SetNull();
}

void CJX_Node::Script_Packet_RemoveAttribute(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 1) {
    ThrowParamCountMismatchException(L"removeAttribute");
    return;
  }

  CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
  if (pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element) {
    ByteString bsName = pArguments->GetUTF8String(0);
    WideString wsName = WideString::FromUTF8(bsName.AsStringView());
    CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
    if (pXMLElement->HasAttribute(wsName.c_str()))
      pXMLElement->RemoveAttribute(wsName.c_str());
  }
  pArguments->GetReturnValue()->SetNull();
}

void CJX_Node::Script_Packet_Content(CFXJSE_Value* pValue,
                                     bool bSetting,
                                     XFA_Attribute eAttribute) {
  CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
  if (bSetting) {
    if (pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element) {
      CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
      pXMLElement->SetTextData(pValue->ToWideString());
    }
    return;
  }

  WideString wsTextData;
  if (pXMLNode && pXMLNode->GetType() == FX_XMLNODE_Element) {
    CFX_XMLElement* pXMLElement = static_cast<CFX_XMLElement*>(pXMLNode);
    wsTextData = pXMLElement->GetTextData();
  }

  pValue->SetString(wsTextData.UTF8Encode().AsStringView());
}

void CJX_Node::Script_Source_Next(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"next");
}

void CJX_Node::Script_Source_CancelBatch(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"cancelBatch");
}

void CJX_Node::Script_Source_First(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"first");
}

void CJX_Node::Script_Source_UpdateBatch(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"updateBatch");
}

void CJX_Node::Script_Source_Previous(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"previous");
}

void CJX_Node::Script_Source_IsBOF(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"isBOF");
}

void CJX_Node::Script_Source_IsEOF(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"isEOF");
}

void CJX_Node::Script_Source_Cancel(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"cancel");
}

void CJX_Node::Script_Source_Update(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"update");
}

void CJX_Node::Script_Source_Open(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"open");
}

void CJX_Node::Script_Source_Delete(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"delete");
}

void CJX_Node::Script_Source_AddNew(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"addNew");
}

void CJX_Node::Script_Source_Requery(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"requery");
}

void CJX_Node::Script_Source_Resync(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"resync");
}

void CJX_Node::Script_Source_Close(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"close");
}

void CJX_Node::Script_Source_Last(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"last");
}

void CJX_Node::Script_Source_HasDataChanged(CFXJSE_Arguments* pArguments) {
  if (pArguments->GetLength() != 0)
    ThrowParamCountMismatchException(L"hasDataChanged");
}

void CJX_Node::Script_Source_Db(CFXJSE_Value* pValue,
                                bool bSetting,
                                XFA_Attribute eAttribute) {}

void CJX_Node::Script_Xfa_This(CFXJSE_Value* pValue,
                               bool bSetting,
                               XFA_Attribute eAttribute) {
  if (bSetting)
    return;

  CXFA_Object* pThis = GetDocument()->GetScriptContext()->GetThisObject();
  ASSERT(pThis);
  pValue->Assign(GetDocument()->GetScriptContext()->GetJSValueFromMap(pThis));
}

void CJX_Node::Script_Handler_Version(CFXJSE_Value* pValue,
                                      bool bSetting,
                                      XFA_Attribute eAttribute) {}

void CJX_Node::Script_SubmitFormat_Mode(CFXJSE_Value* pValue,
                                        bool bSetting,
                                        XFA_Attribute eAttribute) {}

void CJX_Node::Script_Extras_Type(CFXJSE_Value* pValue,
                                  bool bSetting,
                                  XFA_Attribute eAttribute) {}

void CJX_Node::Script_Script_Stateless(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException();
    return;
  }
  pValue->SetString(FX_UTF8Encode(WideStringView(L"0", 1)).AsStringView());
}

void CJX_Node::Script_Encrypt_Format(CFXJSE_Value* pValue,
                                     bool bSetting,
                                     XFA_Attribute eAttribute) {}

pdfium::Optional<bool> CJX_Node::TryBoolean(XFA_Attribute eAttr,
                                            bool bUseDefault) {
  void* pValue = nullptr;
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
  if (GetMapModuleValue(pKey, pValue))
    return {!!pValue};
  if (!bUseDefault)
    return {};

  return GetXFANode()->GetDefaultBoolean(eAttr);
}

bool CJX_Node::SetBoolean(XFA_Attribute eAttr, bool bValue, bool bNotify) {
  CFX_XMLElement* elem = SetValue(eAttr, XFA_AttributeType::Boolean,
                                  (void*)(uintptr_t)bValue, bNotify);
  if (elem)
    elem->SetString(CXFA_Node::AttributeToName(eAttr), bValue ? L"1" : L"0");
  return true;
}

bool CJX_Node::GetBoolean(XFA_Attribute eAttr) {
  return TryBoolean(eAttr, true).value_or(false);
}

bool CJX_Node::SetInteger(XFA_Attribute eAttr, int32_t iValue, bool bNotify) {
  CFX_XMLElement* elem = SetValue(eAttr, XFA_AttributeType::Integer,
                                  (void*)(uintptr_t)iValue, bNotify);
  if (elem) {
    elem->SetString(CXFA_Node::AttributeToName(eAttr),
                    WideString::Format(L"%d", iValue));
  }
  return true;
}

int32_t CJX_Node::GetInteger(XFA_Attribute eAttr) {
  return TryInteger(eAttr, true).value_or(0);
}

pdfium::Optional<int32_t> CJX_Node::TryInteger(XFA_Attribute eAttr,
                                               bool bUseDefault) {
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
  void* pValue = nullptr;
  if (GetMapModuleValue(pKey, pValue))
    return {static_cast<int32_t>(reinterpret_cast<uintptr_t>(pValue))};
  if (!bUseDefault)
    return {};

  return GetXFANode()->GetDefaultInteger(eAttr);
}

pdfium::Optional<XFA_AttributeEnum> CJX_Node::TryEnum(XFA_Attribute eAttr,
                                                      bool bUseDefault) {
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
  void* pValue = nullptr;
  if (GetMapModuleValue(pKey, pValue)) {
    return {
        static_cast<XFA_AttributeEnum>(reinterpret_cast<uintptr_t>(pValue))};
  }
  if (!bUseDefault)
    return {};

  return GetXFANode()->GetDefaultEnum(eAttr);
}

bool CJX_Node::SetEnum(XFA_Attribute eAttr,
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

XFA_AttributeEnum CJX_Node::GetEnum(XFA_Attribute eAttr) {
  return TryEnum(eAttr, true).value_or(XFA_AttributeEnum::Unknown);
}

bool CJX_Node::SetMeasure(XFA_Attribute eAttr,
                          CXFA_Measurement mValue,
                          bool bNotify) {
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
  OnChanging(eAttr, bNotify);
  SetMapModuleBuffer(pKey, &mValue, sizeof(CXFA_Measurement), nullptr);
  OnChanged(eAttr, bNotify, false);
  return true;
}

pdfium::Optional<CXFA_Measurement> CJX_Node::TryMeasure(
    XFA_Attribute eAttr,
    bool bUseDefault) const {
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
  void* pValue;
  int32_t iBytes;
  if (GetMapModuleBuffer(pKey, pValue, iBytes, true) &&
      iBytes == sizeof(CXFA_Measurement)) {
    return {*reinterpret_cast<CXFA_Measurement*>(pValue)};
  }
  if (!bUseDefault)
    return {};

  return GetXFANode()->GetDefaultMeasurement(eAttr);
}

CXFA_Measurement CJX_Node::GetMeasure(XFA_Attribute eAttr) const {
  return TryMeasure(eAttr, true).value_or(CXFA_Measurement());
}

WideString CJX_Node::GetCData(XFA_Attribute eAttr) {
  return TryCData(eAttr, true).value_or(WideString());
}

bool CJX_Node::SetCData(XFA_Attribute eAttr,
                        const WideString& wsValue,
                        bool bNotify,
                        bool bScriptModify) {
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
  OnChanging(eAttr, bNotify);
  if (eAttr == XFA_Attribute::Value) {
    WideString* pClone = new WideString(wsValue);
    SetUserData(pKey, pClone, &deleteWideStringCallBack);
  } else {
    SetMapModuleString(pKey, wsValue.AsStringView());
    if (eAttr == XFA_Attribute::Name)
      GetXFANode()->UpdateNameHash();
  }
  OnChanged(eAttr, bNotify, bScriptModify);

  if (!GetXFANode()->IsNeedSavingXMLNode() ||
      eAttr == XFA_Attribute::QualifiedName ||
      eAttr == XFA_Attribute::BindingNode) {
    return true;
  }

  if (eAttr == XFA_Attribute::Name &&
      (GetXFANode()->GetElementType() == XFA_Element::DataValue ||
       GetXFANode()->GetElementType() == XFA_Element::DataGroup)) {
    return true;
  }

  auto* elem = static_cast<CFX_XMLElement*>(GetXFANode()->GetXMLMappingNode());
  if (eAttr == XFA_Attribute::Value) {
    FX_XMLNODETYPE eXMLType = elem->GetType();
    switch (eXMLType) {
      case FX_XMLNODE_Element:
        if (GetXFANode()->IsAttributeInXML()) {
          elem->SetString(WideString(GetCData(XFA_Attribute::QualifiedName)),
                          wsValue);
        } else {
          bool bDeleteChildren = true;
          if (GetXFANode()->GetPacketType() == XFA_PacketType::Datasets) {
            for (CXFA_Node* pChildDataNode =
                     GetXFANode()->GetNodeItem(XFA_NODEITEM_FirstChild);
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
        static_cast<CFX_XMLText*>(GetXFANode()->GetXMLMappingNode())
            ->SetText(wsValue);
        break;
      default:
        ASSERT(0);
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

void CJX_Node::SetAttributeValue(const WideString& wsValue,
                                 const WideString& wsXMLValue,
                                 bool bNotify,
                                 bool bScriptModify) {
  void* pKey =
      GetMapKey_Element(GetXFANode()->GetElementType(), XFA_Attribute::Value);
  OnChanging(XFA_Attribute::Value, bNotify);
  WideString* pClone = new WideString(wsValue);
  SetUserData(pKey, pClone, &deleteWideStringCallBack);
  OnChanged(XFA_Attribute::Value, bNotify, bScriptModify);
  if (!GetXFANode()->IsNeedSavingXMLNode())
    return;

  auto* elem = static_cast<CFX_XMLElement*>(GetXFANode()->GetXMLMappingNode());
  FX_XMLNODETYPE eXMLType = elem->GetType();
  switch (eXMLType) {
    case FX_XMLNODE_Element:
      if (GetXFANode()->IsAttributeInXML()) {
        elem->SetString(WideString(GetCData(XFA_Attribute::QualifiedName)),
                        wsXMLValue);
      } else {
        bool bDeleteChildren = true;
        if (GetXFANode()->GetPacketType() == XFA_PacketType::Datasets) {
          for (CXFA_Node* pChildDataNode =
                   GetXFANode()->GetNodeItem(XFA_NODEITEM_FirstChild);
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
      static_cast<CFX_XMLText*>(GetXFANode()->GetXMLMappingNode())
          ->SetText(wsXMLValue);
      break;
    default:
      ASSERT(0);
  }
}

pdfium::Optional<WideString> CJX_Node::TryCData(XFA_Attribute eAttr,
                                                bool bUseDefault) {
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
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

  return GetXFANode()->GetDefaultCData(eAttr);
}

CFX_XMLElement* CJX_Node::SetValue(XFA_Attribute eAttr,
                                   XFA_AttributeType eType,
                                   void* pValue,
                                   bool bNotify) {
  void* pKey = GetMapKey_Element(GetXFANode()->GetElementType(), eAttr);
  OnChanging(eAttr, bNotify);
  SetMapModuleValue(pKey, pValue);
  OnChanged(eAttr, bNotify, false);
  if (!GetXFANode()->IsNeedSavingXMLNode())
    return nullptr;

  auto* elem = static_cast<CFX_XMLElement*>(GetXFANode()->GetXMLMappingNode());
  ASSERT(elem->GetType() == FX_XMLNODE_Element);

  return elem;
}

// TODO(dsinclair): This should not be needed. Nodes should get un-bound when
// they're deleted instead of us pointing to bad objects.
void CJX_Node::ReleaseBindingNodes() {
  for (auto& node : binding_nodes_)
    node.Release();
}

bool CJX_Node::SetUserData(void* pKey,
                           void* pData,
                           XFA_MAPDATABLOCKCALLBACKINFO* pCallbackInfo) {
  SetMapModuleBuffer(pKey, &pData, sizeof(void*),
                     pCallbackInfo ? pCallbackInfo : &gs_XFADefaultFreeData);
  return true;
}

bool CJX_Node::SetContent(const WideString& wsContent,
                          const WideString& wsXMLValue,
                          bool bNotify,
                          bool bScriptModify,
                          bool bSyncData) {
  CXFA_Node* pNode = nullptr;
  CXFA_Node* pBindNode = nullptr;
  switch (GetXFANode()->GetObjectType()) {
    case XFA_ObjectType::ContainerNode: {
      if (XFA_FieldIsMultiListBox(GetXFANode())) {
        CXFA_Node* pValue = GetProperty(0, XFA_Element::Value, true);
        if (!pValue)
          break;

        CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
        ASSERT(pChildValue);
        pChildValue->JSNode()->SetCData(XFA_Attribute::ContentType, L"text/xml",
                                        false, false);
        pChildValue->JSNode()->SetContent(wsContent, wsContent, bNotify,
                                          bScriptModify, false);
        CXFA_Node* pBind = GetXFANode()->GetBindData();
        if (bSyncData && pBind) {
          std::vector<WideString> wsSaveTextArray;
          size_t iSize = 0;
          if (!wsContent.IsEmpty()) {
            size_t iStart = 0;
            size_t iLength = wsContent.GetLength();
            auto iEnd = wsContent.Find(L'\n', iStart);
            iEnd = !iEnd.has_value() ? iLength : iEnd;
            while (iEnd.value() >= iStart) {
              wsSaveTextArray.push_back(
                  wsContent.Mid(iStart, iEnd.value() - iStart));
              iStart = iEnd.value() + 1;
              if (iStart >= iLength)
                break;

              iEnd = wsContent.Find(L'\n', iStart);
              if (!iEnd.has_value()) {
                wsSaveTextArray.push_back(
                    wsContent.Mid(iStart, iLength - iStart));
              }
            }
            iSize = wsSaveTextArray.size();
          }
          if (iSize == 0) {
            while (CXFA_Node* pChildNode =
                       pBind->GetNodeItem(XFA_NODEITEM_FirstChild)) {
              pBind->RemoveChild(pChildNode, true);
            }
          } else {
            std::vector<CXFA_Node*> valueNodes = pBind->GetNodeList(
                XFA_NODEFILTER_Children, XFA_Element::DataValue);
            size_t iDatas = valueNodes.size();
            if (iDatas < iSize) {
              size_t iAddNodes = iSize - iDatas;
              CXFA_Node* pValueNodes = nullptr;
              while (iAddNodes-- > 0) {
                pValueNodes =
                    pBind->CreateSamePacketNode(XFA_Element::DataValue);
                pValueNodes->JSNode()->SetCData(XFA_Attribute::Name, L"value",
                                                false, false);
                pValueNodes->CreateXMLMappingNode();
                pBind->InsertChild(pValueNodes, nullptr);
              }
              pValueNodes = nullptr;
            } else if (iDatas > iSize) {
              size_t iDelNodes = iDatas - iSize;
              while (iDelNodes-- > 0) {
                pBind->RemoveChild(pBind->GetNodeItem(XFA_NODEITEM_FirstChild),
                                   true);
              }
            }
            int32_t i = 0;
            for (CXFA_Node* pValueNode =
                     pBind->GetNodeItem(XFA_NODEITEM_FirstChild);
                 pValueNode; pValueNode = pValueNode->GetNodeItem(
                                 XFA_NODEITEM_NextSibling)) {
              pValueNode->JSNode()->SetAttributeValue(
                  wsSaveTextArray[i], wsSaveTextArray[i], false, false);
              i++;
            }
          }
          for (const auto& pArrayNode : *(pBind->GetBindItems())) {
            if (pArrayNode.Get() != GetXFANode()) {
              pArrayNode->JSNode()->SetContent(wsContent, wsContent, bNotify,
                                               bScriptModify, false);
            }
          }
        }
        break;
      }
      if (GetXFANode()->GetElementType() == XFA_Element::ExclGroup) {
        pNode = GetXFANode();
      } else {
        CXFA_Node* pValue = GetProperty(0, XFA_Element::Value, true);
        if (!pValue)
          break;

        CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
        ASSERT(pChildValue);
        pChildValue->JSNode()->SetContent(wsContent, wsContent, bNotify,
                                          bScriptModify, false);
      }
      pBindNode = GetXFANode()->GetBindData();
      if (pBindNode && bSyncData) {
        pBindNode->JSNode()->SetContent(wsContent, wsXMLValue, bNotify,
                                        bScriptModify, false);
        for (const auto& pArrayNode : *(pBindNode->GetBindItems())) {
          if (pArrayNode.Get() != GetXFANode()) {
            pArrayNode->JSNode()->SetContent(wsContent, wsContent, bNotify,
                                             true, false);
          }
        }
      }
      pBindNode = nullptr;
      break;
    }
    case XFA_ObjectType::ContentNode: {
      WideString wsContentType;
      if (GetXFANode()->GetElementType() == XFA_Element::ExData) {
        pdfium::Optional<WideString> ret =
            TryAttribute(XFA_Attribute::ContentType, false);
        if (ret)
          wsContentType = *ret;
        if (wsContentType == L"text/html") {
          wsContentType = L"";
          SetAttribute(XFA_Attribute::ContentType, wsContentType.AsStringView(),
                       false);
        }
      }

      CXFA_Node* pContentRawDataNode =
          GetXFANode()->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!pContentRawDataNode) {
        pContentRawDataNode = GetXFANode()->CreateSamePacketNode(
            (wsContentType == L"text/xml") ? XFA_Element::Sharpxml
                                           : XFA_Element::Sharptext);
        GetXFANode()->InsertChild(pContentRawDataNode, nullptr);
      }
      return pContentRawDataNode->JSNode()->SetContent(
          wsContent, wsXMLValue, bNotify, bScriptModify, bSyncData);
    }
    case XFA_ObjectType::NodeC:
    case XFA_ObjectType::TextNode:
      pNode = GetXFANode();
      break;
    case XFA_ObjectType::NodeV:
      pNode = GetXFANode();
      if (bSyncData && GetXFANode()->GetPacketType() == XFA_PacketType::Form) {
        CXFA_Node* pParent = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
        if (pParent) {
          pParent = pParent->GetNodeItem(XFA_NODEITEM_Parent);
        }
        if (pParent && pParent->GetElementType() == XFA_Element::Value) {
          pParent = pParent->GetNodeItem(XFA_NODEITEM_Parent);
          if (pParent && pParent->IsContainerNode()) {
            pBindNode = pParent->GetBindData();
            if (pBindNode) {
              pBindNode->JSNode()->SetContent(wsContent, wsXMLValue, bNotify,
                                              bScriptModify, false);
            }
          }
        }
      }
      break;
    default:
      if (GetXFANode()->GetElementType() == XFA_Element::DataValue) {
        pNode = GetXFANode();
        pBindNode = GetXFANode();
      }
      break;
  }
  if (!pNode)
    return false;

  SetAttributeValue(wsContent, wsXMLValue, bNotify, bScriptModify);
  if (pBindNode && bSyncData) {
    for (const auto& pArrayNode : *(pBindNode->GetBindItems())) {
      pArrayNode->JSNode()->SetContent(wsContent, wsContent, bNotify,
                                       bScriptModify, false);
    }
  }
  return true;
}

WideString CJX_Node::GetContent(bool bScriptModify) {
  return TryContent(bScriptModify, true).value_or(WideString());
}

pdfium::Optional<WideString> CJX_Node::TryContent(bool bScriptModify,
                                                  bool bProto) {
  CXFA_Node* pNode = nullptr;
  switch (GetXFANode()->GetObjectType()) {
    case XFA_ObjectType::ContainerNode:
      if (GetXFANode()->GetElementType() == XFA_Element::ExclGroup) {
        pNode = GetXFANode();
      } else {
        CXFA_Node* pValue =
            GetXFANode()->GetChild(0, XFA_Element::Value, false);
        if (!pValue)
          return {};

        CXFA_Node* pChildValue = pValue->GetNodeItem(XFA_NODEITEM_FirstChild);
        if (pChildValue && XFA_FieldIsMultiListBox(GetXFANode())) {
          pChildValue->JSNode()->SetAttribute(XFA_Attribute::ContentType,
                                              L"text/xml", false);
        }
        if (pChildValue)
          return pChildValue->JSNode()->TryContent(bScriptModify, bProto);
        return {};
      }
      break;
    case XFA_ObjectType::ContentNode: {
      CXFA_Node* pContentRawDataNode =
          GetXFANode()->GetNodeItem(XFA_NODEITEM_FirstChild);
      if (!pContentRawDataNode) {
        XFA_Element element = XFA_Element::Sharptext;
        if (GetXFANode()->GetElementType() == XFA_Element::ExData) {
          pdfium::Optional<WideString> contentType =
              TryAttribute(XFA_Attribute::ContentType, false);
          if (contentType) {
            if (*contentType == L"text/html")
              element = XFA_Element::SharpxHTML;
            else if (*contentType == L"text/xml")
              element = XFA_Element::Sharpxml;
          }
        }
        pContentRawDataNode = GetXFANode()->CreateSamePacketNode(element);
        GetXFANode()->InsertChild(pContentRawDataNode, nullptr);
      }
      return pContentRawDataNode->JSNode()->TryContent(bScriptModify, true);
    }
    case XFA_ObjectType::NodeC:
    case XFA_ObjectType::NodeV:
    case XFA_ObjectType::TextNode:
      pNode = GetXFANode();
    default:
      if (GetXFANode()->GetElementType() == XFA_Element::DataValue)
        pNode = GetXFANode();
      break;
  }
  if (pNode) {
    if (bScriptModify) {
      CFXJSE_Engine* pScriptContext = GetDocument()->GetScriptContext();
      if (pScriptContext)
        GetDocument()->GetScriptContext()->AddNodesOfRunScript(GetXFANode());
    }
    return TryCData(XFA_Attribute::Value, false);
  }
  return {};
}

void CJX_Node::SetWidgetData(std::unique_ptr<CXFA_WidgetData> data) {
  widget_data_ = std::move(data);
}

void CJX_Node::SetCalcData(std::unique_ptr<CXFA_CalcData> data) {
  calc_data_ = std::move(data);
}

std::unique_ptr<CXFA_CalcData> CJX_Node::ReleaseCalcData() {
  return std::move(calc_data_);
}

pdfium::Optional<WideString> CJX_Node::TryNamespace() {
  if (GetXFANode()->IsModelNode() ||
      GetXFANode()->GetElementType() == XFA_Element::Packet) {
    CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
    if (!pXMLNode || pXMLNode->GetType() != FX_XMLNODE_Element)
      return {};

    return {static_cast<CFX_XMLElement*>(pXMLNode)->GetNamespaceURI()};
  }

  if (GetXFANode()->GetPacketType() != XFA_PacketType::Datasets)
    return GetXFANode()->GetModelNode()->JSNode()->TryNamespace();

  CFX_XMLNode* pXMLNode = GetXFANode()->GetXMLMappingNode();
  if (!pXMLNode || pXMLNode->GetType() != FX_XMLNODE_Element)
    return {};

  if (GetXFANode()->GetElementType() == XFA_Element::DataValue &&
      GetEnum(XFA_Attribute::Contains) == XFA_AttributeEnum::MetaData) {
    WideString wsNamespace;
    bool ret = XFA_FDEExtension_ResolveNamespaceQualifier(
        static_cast<CFX_XMLElement*>(pXMLNode),
        GetCData(XFA_Attribute::QualifiedName), &wsNamespace);
    if (!ret)
      return {};
    return {wsNamespace};
  }
  return {static_cast<CFX_XMLElement*>(pXMLNode)->GetNamespaceURI()};
}

CXFA_Node* CJX_Node::GetProperty(int32_t index,
                                 XFA_Element eProperty,
                                 bool bCreateProperty) {
  if (index < 0 || index >= GetXFANode()->PropertyOccuranceCount(eProperty))
    return nullptr;

  CXFA_Node* pNode = GetXFANode()->GetChildNode();
  int32_t iCount = 0;
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eProperty) {
      iCount++;
      if (iCount > index)
        return pNode;
    }
  }
  if (!bCreateProperty)
    return nullptr;

  if (GetXFANode()->HasPropertyFlags(eProperty, XFA_PROPERTYFLAG_OneOf)) {
    pNode = GetXFANode()->GetChildNode();
    for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      if (GetXFANode()->HasPropertyFlags(pNode->GetElementType(),
                                         XFA_PROPERTYFLAG_OneOf)) {
        return nullptr;
      }
    }
  }

  CXFA_Node* pNewNode = nullptr;
  for (; iCount <= index; ++iCount) {
    pNewNode =
        GetDocument()->CreateNode(GetXFANode()->GetPacketType(), eProperty);
    if (!pNewNode)
      return nullptr;
    GetXFANode()->InsertChild(pNewNode, nullptr);
    pNewNode->SetFlag(XFA_NodeFlag_Initialized, true);
  }
  return pNewNode;
}

XFA_MAPMODULEDATA* CJX_Node::CreateMapModuleData() {
  if (!map_module_data_)
    map_module_data_ = pdfium::MakeUnique<XFA_MAPMODULEDATA>();
  return map_module_data_.get();
}

XFA_MAPMODULEDATA* CJX_Node::GetMapModuleData() const {
  return map_module_data_.get();
}

void CJX_Node::SetMapModuleValue(void* pKey, void* pValue) {
  CreateMapModuleData()->m_ValueMap[pKey] = pValue;
}

bool CJX_Node::GetMapModuleValue(void* pKey, void*& pValue) {
  for (CXFA_Node* pNode = GetXFANode(); pNode;
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

void CJX_Node::SetMapModuleString(void* pKey, const WideStringView& wsValue) {
  SetMapModuleBuffer(pKey, (void*)wsValue.unterminated_c_str(),
                     wsValue.GetLength() * sizeof(wchar_t), nullptr);
}

bool CJX_Node::GetMapModuleString(void* pKey, WideStringView& wsValue) {
  void* pValue;
  int32_t iBytes;
  if (!GetMapModuleBuffer(pKey, pValue, iBytes, true))
    return false;

  // Defensive measure: no out-of-bounds pointers even if zero length.
  int32_t iChars = iBytes / sizeof(wchar_t);
  wsValue = WideStringView(iChars ? (const wchar_t*)pValue : nullptr, iChars);
  return true;
}

void CJX_Node::SetMapModuleBuffer(void* pKey,
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

bool CJX_Node::GetMapModuleBuffer(void* pKey,
                                  void*& pValue,
                                  int32_t& iBytes,
                                  bool bProtoAlso) const {
  XFA_MAPDATABLOCK* pBuffer = nullptr;
  for (const CXFA_Node* pNode = GetXFANode(); pNode;
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

bool CJX_Node::HasMapModuleKey(void* pKey) {
  XFA_MAPMODULEDATA* pModule = GetXFANode()->JSNode()->GetMapModuleData();
  return pModule && (pdfium::ContainsKey(pModule->m_ValueMap, pKey) ||
                     pdfium::ContainsKey(pModule->m_BufferMap, pKey));
}

void CJX_Node::ClearMapModuleBuffer() {
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

void CJX_Node::RemoveMapModuleKey(void* pKey) {
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

void CJX_Node::MergeAllData(CXFA_Node* pDstModule) {
  XFA_MAPMODULEDATA* pDstModuleData =
      pDstModule->JSNode()->CreateMapModuleData();
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

void CJX_Node::MoveBufferMapData(CXFA_Node* pDstModule) {
  if (!pDstModule)
    return;

  bool bNeedMove = true;
  if (pDstModule->GetElementType() != GetXFANode()->GetElementType())
    bNeedMove = false;

  if (bNeedMove)
    pDstModule->JSNode()->SetCalcData(ReleaseCalcData());
  if (!pDstModule->IsNodeV())
    return;

  WideString wsValue = pDstModule->JSNode()->GetContent(false);
  WideString wsFormatValue(wsValue);
  CXFA_WidgetData* pWidgetData = pDstModule->GetContainerWidgetData();
  if (pWidgetData)
    wsFormatValue = pWidgetData->GetFormatDataValue(wsValue);

  pDstModule->JSNode()->SetContent(wsValue, wsFormatValue, true, true, true);
}

void CJX_Node::MoveBufferMapData(CXFA_Node* pSrcModule, CXFA_Node* pDstModule) {
  if (!pSrcModule || !pDstModule)
    return;

  CXFA_Node* pSrcChild = pSrcModule->GetNodeItem(XFA_NODEITEM_FirstChild);
  CXFA_Node* pDstChild = pDstModule->GetNodeItem(XFA_NODEITEM_FirstChild);
  while (pSrcChild && pDstChild) {
    MoveBufferMapData(pSrcChild, pDstChild);

    pSrcChild = pSrcChild->GetNodeItem(XFA_NODEITEM_NextSibling);
    pDstChild = pDstChild->GetNodeItem(XFA_NODEITEM_NextSibling);
  }
  pSrcModule->JSNode()->MoveBufferMapData(pDstModule);
}

int32_t CJX_Node::execSingleEventByName(const WideStringView& wsEventName,
                                        XFA_Element eType) {
  const XFA_ExecEventParaInfo* eventParaInfo =
      GetEventParaInfoByName(wsEventName);
  if (!eventParaInfo)
    return XFA_EVENTERROR_NotExist;

  uint32_t validFlags = eventParaInfo->m_validFlags;
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return XFA_EVENTERROR_NotExist;

  if (validFlags == 1) {
    return pNotify->ExecEventByDeepFirst(GetXFANode(),
                                         eventParaInfo->m_eventType);
  }

  if (validFlags == 2) {
    return pNotify->ExecEventByDeepFirst(
        GetXFANode(), eventParaInfo->m_eventType, false, false);
  }

  if (validFlags == 3) {
    if (eType != XFA_Element::Subform)
      return XFA_EVENTERROR_NotExist;

    return pNotify->ExecEventByDeepFirst(
        GetXFANode(), eventParaInfo->m_eventType, false, false);
  }

  if (validFlags == 4) {
    if (eType != XFA_Element::ExclGroup && eType != XFA_Element::Field)
      return XFA_EVENTERROR_NotExist;

    CXFA_Node* pParentNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
    if (pParentNode &&
        pParentNode->GetElementType() == XFA_Element::ExclGroup) {
      pNotify->ExecEventByDeepFirst(GetXFANode(), eventParaInfo->m_eventType,
                                    false, false);
    }
    return pNotify->ExecEventByDeepFirst(
        GetXFANode(), eventParaInfo->m_eventType, false, false);
  }

  if (validFlags == 5) {
    if (eType != XFA_Element::Field)
      return XFA_EVENTERROR_NotExist;

    return pNotify->ExecEventByDeepFirst(
        GetXFANode(), eventParaInfo->m_eventType, false, false);
  }

  if (validFlags == 6) {
    CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
    if (!pWidgetData)
      return XFA_EVENTERROR_NotExist;

    CXFA_Node* pUINode = pWidgetData->GetUIChild();
    if (pUINode->GetElementType() != XFA_Element::Signature)
      return XFA_EVENTERROR_NotExist;

    return pNotify->ExecEventByDeepFirst(
        GetXFANode(), eventParaInfo->m_eventType, false, false);
  }

  if (validFlags == 7) {
    CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
    if (!pWidgetData)
      return XFA_EVENTERROR_NotExist;

    CXFA_Node* pUINode = pWidgetData->GetUIChild();
    if (pUINode->GetElementType() != XFA_Element::ChoiceList ||
        pWidgetData->IsListBox()) {
      return XFA_EVENTERROR_NotExist;
    }
    return pNotify->ExecEventByDeepFirst(
        GetXFANode(), eventParaInfo->m_eventType, false, false);
  }

  return XFA_EVENTERROR_NotExist;
}

void CJX_Node::ThrowMissingPropertyException(const WideString& obj,
                                             const WideString& prop) const {
  ThrowException(L"'%s' doesn't have property '%s'.", obj.c_str(),
                 prop.c_str());
}

void CJX_Node::ThrowTooManyOccurancesException(const WideString& obj) const {
  ThrowException(
      L"The element [%s] has violated its allowable number of occurrences.",
      obj.c_str());
}

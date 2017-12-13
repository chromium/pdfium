// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_node.h"

#include <map>
#include <vector>

#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/js_resources.h"
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

enum class EventAppliesToo {
  kNone = 0,
  kAll = 1,
  kAllNonRecursive = 2,
  kSubform = 3,
  kFieldOrExclusion = 4,
  kField = 5,
  kSignature = 6,
  kChoiceList = 7
};

struct XFA_ExecEventParaInfo {
 public:
  uint32_t m_uHash;
  const wchar_t* m_lpcEventName;
  XFA_EVENTTYPE m_eventType;
  EventAppliesToo m_validFlags;
};

const XFA_ExecEventParaInfo gs_eventParaInfos[] = {
    {0x109d7ce7, L"mouseEnter", XFA_EVENT_MouseEnter, EventAppliesToo::kField},
    {0x1bfc72d9, L"preOpen", XFA_EVENT_PreOpen, EventAppliesToo::kChoiceList},
    {0x2196a452, L"initialize", XFA_EVENT_Initialize, EventAppliesToo::kAll},
    {0x27410f03, L"mouseExit", XFA_EVENT_MouseExit, EventAppliesToo::kField},
    {0x36f1c6d8, L"preSign", XFA_EVENT_PreSign, EventAppliesToo::kSignature},
    {0x4731d6ba, L"exit", XFA_EVENT_Exit, EventAppliesToo::kAllNonRecursive},
    {0x7233018a, L"validate", XFA_EVENT_Validate, EventAppliesToo::kAll},
    {0x8808385e, L"indexChange", XFA_EVENT_IndexChange,
     EventAppliesToo::kSubform},
    {0x891f4606, L"change", XFA_EVENT_Change,
     EventAppliesToo::kFieldOrExclusion},
    {0x9f693b21, L"mouseDown", XFA_EVENT_MouseDown, EventAppliesToo::kField},
    {0xcdce56b3, L"full", XFA_EVENT_Full, EventAppliesToo::kFieldOrExclusion},
    {0xd576d08e, L"mouseUp", XFA_EVENT_MouseUp, EventAppliesToo::kField},
    {0xd95657a6, L"click", XFA_EVENT_Click, EventAppliesToo::kFieldOrExclusion},
    {0xdbfbe02e, L"calculate", XFA_EVENT_Calculate, EventAppliesToo::kAll},
    {0xe25fa7b8, L"postOpen", XFA_EVENT_PostOpen, EventAppliesToo::kChoiceList},
    {0xe28dce7e, L"enter", XFA_EVENT_Enter, EventAppliesToo::kAllNonRecursive},
    {0xfd54fbb7, L"postSign", XFA_EVENT_PostSign, EventAppliesToo::kSignature},
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

const CJX_MethodSpec CJX_Node::MethodSpecs[] = {
    {"applyXSL", applyXSL_static},
    {"assignNode", assignNode_static},
    {"clone", clone_static},
    {"getAttribute", getAttribute_static},
    {"getElement", getElement_static},
    {"isPropertySpecified", isPropertySpecified_static},
    {"loadXML", loadXML_static},
    {"saveFilteredXML", saveFilteredXML_static},
    {"saveXML", saveXML_static},
    {"setAttribute", setAttribute_static},
    {"setElement", setElement_static}};

CJX_Node::CJX_Node(CXFA_Node* node) : CJX_Tree(node) {
  DefineMethods(MethodSpecs, FX_ArraySize(MethodSpecs));
}

CJX_Node::~CJX_Node() = default;

CXFA_Node* CJX_Node::GetXFANode() {
  return static_cast<CXFA_Node*>(GetXFAObject());
}

const CXFA_Node* CJX_Node::GetXFANode() const {
  return static_cast<const CXFA_Node*>(GetXFAObject());
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

CJS_Return CJX_Node::applyXSL(CJS_V8* runtime,
                              const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  // TODO(weili): check whether we need to implement this, pdfium:501.
  return CJS_Return(true);
}

CJS_Return CJX_Node::assignNode(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 3)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  // TODO(weili): check whether we need to implement this, pdfium:501.
  return CJS_Return(true);
}

CJS_Return CJX_Node::clone(CJS_V8* runtime,
                           const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  CXFA_Node* pCloneNode = GetXFANode()->Clone(runtime->ToBoolean(params[0]));
  CFXJSE_Value* value =
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pCloneNode);
  if (!value)
    return CJS_Return(runtime->NewNull());
  return CJS_Return(value->DirectGetValue().Get(runtime->GetIsolate()));
}

CJS_Return CJX_Node::getAttribute(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  WideString expression = runtime->ToWideString(params[0]);
  return CJS_Return(runtime->NewString(
      GetAttribute(expression.AsStringView()).UTF8Encode().AsStringView()));
}

CJS_Return CJX_Node::getElement(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 2)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  WideString expression = runtime->ToWideString(params[0]);
  int32_t iValue = params.size() >= 2 ? runtime->ToInt32(params[1]) : 0;

  CXFA_Node* pNode =
      GetProperty(iValue, CXFA_Node::NameToElement(expression), true);
  CFXJSE_Value* value =
      GetDocument()->GetScriptContext()->GetJSValueFromMap(pNode);
  if (!value)
    return CJS_Return(runtime->NewNull());
  return CJS_Return(value->DirectGetValue().Get(runtime->GetIsolate()));
}

CJS_Return CJX_Node::isPropertySpecified(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 3)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  WideString expression = runtime->ToWideString(params[0]);
  XFA_Attribute attr = CXFA_Node::NameToAttribute(expression.AsStringView());
  if (attr != XFA_Attribute::Unknown && HasAttribute(attr))
    return CJS_Return(runtime->NewBoolean(true));

  bool bParent = params.size() < 2 || runtime->ToBoolean(params[1]);
  int32_t iIndex = params.size() == 3 ? runtime->ToInt32(params[2]) : 0;
  XFA_Element eType = CXFA_Node::NameToElement(expression);
  bool bHas = !!GetProperty(iIndex, eType, true);
  if (!bHas && bParent && GetXFANode()->GetParent()) {
    // Also check on the parent.
    auto* jsnode = GetXFANode()->GetParent()->JSNode();
    bHas = jsnode->HasAttribute(attr) ||
           !!jsnode->GetProperty(iIndex, eType, true);
  }
  return CJS_Return(runtime->NewBoolean(bHas));
}

CJS_Return CJX_Node::loadXML(CJS_V8* runtime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 3)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  ByteString expression = runtime->ToByteString(params[0]);
  if (expression.IsEmpty())
    return CJS_Return(true);

  bool bIgnoreRoot = true;
  if (params.size() >= 2)
    bIgnoreRoot = runtime->ToBoolean(params[1]);

  bool bOverwrite = 0;
  if (params.size() >= 3)
    bOverwrite = runtime->ToBoolean(params[2]);

  auto pParser = pdfium::MakeUnique<CXFA_SimpleParser>(GetDocument());
  if (!pParser)
    return CJS_Return(true);

  CFX_XMLNode* pXMLNode = pParser->ParseXMLData(expression);
  if (!pXMLNode)
    return CJS_Return(true);

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
    return CJS_Return(true);

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

  return CJS_Return(true);
}

CJS_Return CJX_Node::saveFilteredXML(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  // TODO(weili): Check whether we need to implement this, pdfium:501.
  return CJS_Return(true);
}

CJS_Return CJX_Node::saveXML(CJS_V8* runtime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() > 1)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));
  if (params.size() == 1 && runtime->ToWideString(params[0]) != L"pretty")
    return CJS_Return(JSGetStringFromID(JSMessage::kValueError));

  // TODO(weili): Check whether we need to save pretty print XML, pdfium:501.

  WideString bsXMLHeader = L"<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  if (GetXFANode()->GetPacketType() != XFA_PacketType::Form &&
      GetXFANode()->GetPacketType() != XFA_PacketType::Datasets) {
    return CJS_Return(runtime->NewString(""));
  }

  CFX_XMLNode* pElement = nullptr;
  if (GetXFANode()->GetPacketType() == XFA_PacketType::Datasets) {
    pElement = GetXFANode()->GetXMLMappingNode();
    if (!pElement || pElement->GetType() != FX_XMLNODE_Element) {
      return CJS_Return(
          runtime->NewString(bsXMLHeader.UTF8Encode().AsStringView()));
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

  return CJS_Return(runtime->NewString(
      ByteStringView(pMemoryStream->GetBuffer(), pMemoryStream->GetSize())));
}

CJS_Return CJX_Node::setAttribute(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 2)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  WideString attributeValue = runtime->ToWideString(params[0]);
  WideString attribute = runtime->ToWideString(params[1]);
  SetAttribute(attribute.AsStringView(), attributeValue.AsStringView(), true);
  return CJS_Return(true);
}

CJS_Return CJX_Node::setElement(
    CJS_V8* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1 && params.size() != 2)
    return CJS_Return(JSGetStringFromID(JSMessage::kParamError));

  // TODO(weili): check whether we need to implement this, pdfium:501.
  return CJS_Return(true);
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

void CJX_Node::Script_ModelClass_Context(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {}

void CJX_Node::Script_ModelClass_AliasNode(CFXJSE_Value* pValue,
                                           bool bSetting,
                                           XFA_Attribute eAttribute) {}

void CJX_Node::Script_Delta_CurrentValue(CFXJSE_Value* pValue,
                                         bool bSetting,
                                         XFA_Attribute eAttribute) {}

void CJX_Node::Script_Delta_SavedValue(CFXJSE_Value* pValue,
                                       bool bSetting,
                                       XFA_Attribute eAttribute) {}

void CJX_Node::Script_Delta_Target(CFXJSE_Value* pValue,
                                   bool bSetting,
                                   XFA_Attribute eAttribute) {}

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

int32_t CJX_Node::execSingleEventByName(const WideStringView& wsEventName,
                                        XFA_Element eType) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return XFA_EVENTERROR_NotExist;

  const XFA_ExecEventParaInfo* eventParaInfo =
      GetEventParaInfoByName(wsEventName);
  if (!eventParaInfo)
    return XFA_EVENTERROR_NotExist;

  switch (eventParaInfo->m_validFlags) {
    case EventAppliesToo::kNone:
      return XFA_EVENTERROR_NotExist;
    case EventAppliesToo::kAll:
    case EventAppliesToo::kAllNonRecursive:
      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false,
          eventParaInfo->m_validFlags == EventAppliesToo::kAll);
    case EventAppliesToo::kSubform:
      if (eType != XFA_Element::Subform)
        return XFA_EVENTERROR_NotExist;

      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    case EventAppliesToo::kFieldOrExclusion: {
      if (eType != XFA_Element::ExclGroup && eType != XFA_Element::Field)
        return XFA_EVENTERROR_NotExist;

      CXFA_Node* pParentNode = GetXFANode()->GetNodeItem(XFA_NODEITEM_Parent);
      if (pParentNode &&
          pParentNode->GetElementType() == XFA_Element::ExclGroup) {
        // TODO(dsinclair): This seems like a bug, we do the same work twice?
        pNotify->ExecEventByDeepFirst(GetXFANode(), eventParaInfo->m_eventType,
                                      false, false);
      }
      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    }
    case EventAppliesToo::kField:
      if (eType != XFA_Element::Field)
        return XFA_EVENTERROR_NotExist;

      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    case EventAppliesToo::kSignature: {
      CXFA_WidgetData* pWidgetData = GetXFANode()->GetWidgetData();
      if (!pWidgetData)
        return XFA_EVENTERROR_NotExist;

      CXFA_Node* pUINode = pWidgetData->GetUIChild();
      if (pUINode->GetElementType() != XFA_Element::Signature)
        return XFA_EVENTERROR_NotExist;

      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    }
    case EventAppliesToo::kChoiceList: {
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
  }
  return XFA_EVENTERROR_NotExist;
}

void CJX_Node::ThrowMissingPropertyException(const WideString& obj,
                                             const WideString& prop) const {
  ThrowException(L"'%ls' doesn't have property '%ls'.", obj.c_str(),
                 prop.c_str());
}

void CJX_Node::ThrowTooManyOccurancesException(const WideString& obj) const {
  ThrowException(
      L"The element [%ls] has violated its allowable number of occurrences.",
      obj.c_str());
}

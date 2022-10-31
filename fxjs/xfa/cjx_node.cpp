// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cjx_node.h"

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/cfx_read_only_string_stream.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmldocument.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "fxjs/fxv8.h"
#include "fxjs/js_resources.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/v8-object.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffdoc.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_document_builder.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

enum class EventAppliesTo : uint8_t {
  kNone = 0,
  kAll = 1,
  kAllNonRecursive = 2,
  kSubform = 3,
  kFieldOrExclusion = 4,
  kField = 5,
  kSignature = 6,
  kChoiceList = 7
};

struct ExecEventParaInfo {
  uint32_t m_uHash;  // hashed as wide string.
  XFA_EVENTTYPE m_eventType;
  EventAppliesTo m_validFlags;
};

#undef PARA
#define PARA(a, b, c, d) a, c, EventAppliesTo::d
const ExecEventParaInfo kExecEventParaInfoTable[] = {
    {PARA(0x109d7ce7, "mouseEnter", XFA_EVENT_MouseEnter, kField)},
    {PARA(0x1bfc72d9, "preOpen", XFA_EVENT_PreOpen, kChoiceList)},
    {PARA(0x2196a452, "initialize", XFA_EVENT_Initialize, kAll)},
    {PARA(0x27410f03, "mouseExit", XFA_EVENT_MouseExit, kField)},
    {PARA(0x36f1c6d8, "preSign", XFA_EVENT_PreSign, kSignature)},
    {PARA(0x4731d6ba, "exit", XFA_EVENT_Exit, kAllNonRecursive)},
    {PARA(0x7233018a, "validate", XFA_EVENT_Validate, kAll)},
    {PARA(0x8808385e, "indexChange", XFA_EVENT_IndexChange, kSubform)},
    {PARA(0x891f4606, "change", XFA_EVENT_Change, kFieldOrExclusion)},
    {PARA(0x9f693b21, "mouseDown", XFA_EVENT_MouseDown, kField)},
    {PARA(0xcdce56b3, "full", XFA_EVENT_Full, kFieldOrExclusion)},
    {PARA(0xd576d08e, "mouseUp", XFA_EVENT_MouseUp, kField)},
    {PARA(0xd95657a6, "click", XFA_EVENT_Click, kFieldOrExclusion)},
    {PARA(0xdbfbe02e, "calculate", XFA_EVENT_Calculate, kAll)},
    {PARA(0xe25fa7b8, "postOpen", XFA_EVENT_PostOpen, kChoiceList)},
    {PARA(0xe28dce7e, "enter", XFA_EVENT_Enter, kAllNonRecursive)},
    {PARA(0xfd54fbb7, "postSign", XFA_EVENT_PostSign, kSignature)},
};
#undef PARA

const ExecEventParaInfo* GetExecEventParaInfoByName(
    WideStringView wsEventName) {
  if (wsEventName.IsEmpty())
    return nullptr;

  uint32_t uHash = FX_HashCode_GetW(wsEventName);
  auto* result = std::lower_bound(
      std::begin(kExecEventParaInfoTable), std::end(kExecEventParaInfoTable),
      uHash, [](const ExecEventParaInfo& iter, const uint16_t& hash) {
        return iter.m_uHash < hash;
      });
  if (result != std::end(kExecEventParaInfoTable) && result->m_uHash == uHash)
    return result;
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
  DefineMethods(MethodSpecs);
}

CJX_Node::~CJX_Node() = default;

bool CJX_Node::DynamicTypeIs(TypeTag eType) const {
  return eType == static_type__ || ParentType__::DynamicTypeIs(eType);
}

CJS_Result CJX_Node::applyXSL(CFXJSE_Engine* runtime,
                              const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  // TODO(weili): check whether we need to implement this, pdfium:501.
  return CJS_Result::Success();
}

CJS_Result CJX_Node::assignNode(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 3)
    return CJS_Result::Failure(JSMessage::kParamError);

  // TODO(weili): check whether we need to implement this, pdfium:501.
  return CJS_Result::Success();
}

CJS_Result CJX_Node::clone(CFXJSE_Engine* runtime,
                           const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  CXFA_Node* pCloneNode = GetXFANode()->Clone(runtime->ToBoolean(params[0]));
  return CJS_Result::Success(
      GetDocument()->GetScriptContext()->GetOrCreateJSBindingFromMap(
          pCloneNode));
}

CJS_Result CJX_Node::getAttribute(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  WideString expression = runtime->ToWideString(params[0]);
  return CJS_Result::Success(runtime->NewString(
      GetAttributeByString(expression.AsStringView()).ToUTF8().AsStringView()));
}

CJS_Result CJX_Node::getElement(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  WideString expression = runtime->ToWideString(params[0]);
  int32_t iValue = params.size() >= 2 ? runtime->ToInt32(params[1]) : 0;
  XFA_Element eElement = XFA_GetElementByName(expression.AsStringView());
  if (eElement == XFA_Element::Unknown)
    return CJS_Result::Success(runtime->NewNull());

  CXFA_Node* pNode = GetOrCreateProperty<CXFA_Node>(iValue, eElement);
  if (!pNode)
    return CJS_Result::Success(runtime->NewNull());

  return CJS_Result::Success(
      GetDocument()->GetScriptContext()->GetOrCreateJSBindingFromMap(pNode));
}

CJS_Result CJX_Node::isPropertySpecified(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 3)
    return CJS_Result::Failure(JSMessage::kParamError);

  WideString expression = runtime->ToWideString(params[0]);
  absl::optional<XFA_ATTRIBUTEINFO> attr =
      XFA_GetAttributeByName(expression.AsStringView());
  if (attr.has_value() && HasAttribute(attr.value().attribute))
    return CJS_Result::Success(runtime->NewBoolean(true));

  XFA_Element eType = XFA_GetElementByName(expression.AsStringView());
  if (eType == XFA_Element::Unknown)
    return CJS_Result::Success(runtime->NewBoolean(false));

  bool bParent = params.size() < 2 || runtime->ToBoolean(params[1]);
  int32_t iIndex = params.size() == 3 ? runtime->ToInt32(params[2]) : 0;
  bool bHas = !!GetOrCreateProperty<CXFA_Node>(iIndex, eType);
  if (!bHas && bParent && GetXFANode()->GetParent()) {
    // Also check on the parent.
    auto* jsnode = GetXFANode()->GetParent()->JSObject();
    bHas = jsnode->HasAttribute(attr.value().attribute) ||
           !!jsnode->GetOrCreateProperty<CXFA_Node>(iIndex, eType);
  }
  return CJS_Result::Success(runtime->NewBoolean(bHas));
}

CJS_Result CJX_Node::loadXML(CFXJSE_Engine* runtime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  if (params.empty() || params.size() > 3)
    return CJS_Result::Failure(JSMessage::kParamError);

  ByteString expression = runtime->ToByteString(params[0]);
  if (expression.IsEmpty())
    return CJS_Result::Success();

  bool bIgnoreRoot = true;
  if (params.size() >= 2)
    bIgnoreRoot = runtime->ToBoolean(params[1]);

  bool bOverwrite = false;
  if (params.size() >= 3)
    bOverwrite = runtime->ToBoolean(params[2]);

  auto stream =
      pdfium::MakeRetain<CFX_ReadOnlyStringStream>(std::move(expression));

  CFX_XMLParser parser(stream);
  std::unique_ptr<CFX_XMLDocument> xml_doc = parser.Parse();
  CXFA_DocumentBuilder builder(GetDocument());
  CFX_XMLNode* pXMLNode = builder.Build(xml_doc.get());
  if (!pXMLNode)
    return CJS_Result::Success();

  CFX_XMLDocument* top_xml_doc =
      GetXFANode()->GetDocument()->GetNotify()->GetFFDoc()->GetXMLDocument();
  top_xml_doc->AppendNodesFrom(xml_doc.get());

  if (bIgnoreRoot &&
      (pXMLNode->GetType() != CFX_XMLNode::Type::kElement ||
       XFA_RecognizeRichText(static_cast<CFX_XMLElement*>(pXMLNode)))) {
    bIgnoreRoot = false;
  }

  CXFA_Node* pFakeRoot = GetXFANode()->Clone(false);
  WideString wsContentType = GetCData(XFA_Attribute::ContentType);
  if (!wsContentType.IsEmpty()) {
    pFakeRoot->JSObject()->SetCData(XFA_Attribute::ContentType,
                                    WideString(wsContentType));
  }

  CFX_XMLNode* pFakeXMLRoot = pFakeRoot->GetXMLMappingNode();
  if (!pFakeXMLRoot) {
    CFX_XMLNode* pThisXMLRoot = GetXFANode()->GetXMLMappingNode();
    CFX_XMLNode* clone;
    if (pThisXMLRoot) {
      clone = pThisXMLRoot->Clone(top_xml_doc);
    } else {
      clone = top_xml_doc->CreateNode<CFX_XMLElement>(
          WideString::FromASCII(GetXFANode()->GetClassName()));
    }
    pFakeXMLRoot = clone;
  }

  if (bIgnoreRoot) {
    CFX_XMLNode* pXMLChild = pXMLNode->GetFirstChild();
    while (pXMLChild) {
      CFX_XMLNode* pXMLSibling = pXMLChild->GetNextSibling();
      pXMLNode->RemoveChild(pXMLChild);
      pFakeXMLRoot->AppendLastChild(pXMLChild);
      pXMLChild = pXMLSibling;
    }
  } else {
    pXMLNode->RemoveSelfIfParented();
    pFakeXMLRoot->AppendLastChild(pXMLNode);
  }

  builder.ConstructXFANode(pFakeRoot, pFakeXMLRoot);
  pFakeRoot = builder.GetRootNode();
  if (!pFakeRoot)
    return CJS_Result::Success();

  if (bOverwrite) {
    CXFA_Node* pChild = GetXFANode()->GetFirstChild();
    CXFA_Node* pNewChild = pFakeRoot->GetFirstChild();
    int32_t index = 0;
    while (pNewChild) {
      CXFA_Node* pItem = pNewChild->GetNextSibling();
      pFakeRoot->RemoveChildAndNotify(pNewChild, true);
      GetXFANode()->InsertChildAndNotify(index++, pNewChild);
      pNewChild->SetInitializedFlagAndNotify();
      pNewChild = pItem;
    }

    while (pChild) {
      CXFA_Node* pItem = pChild->GetNextSibling();
      GetXFANode()->RemoveChildAndNotify(pChild, true);
      pFakeRoot->InsertChildAndNotify(pChild, nullptr);
      pChild = pItem;
    }

    if (GetXFANode()->GetPacketType() == XFA_PacketType::Form &&
        GetXFANode()->GetElementType() == XFA_Element::ExData) {
      CFX_XMLNode* pTempXMLNode = GetXFANode()->GetXMLMappingNode();
      GetXFANode()->SetXMLMappingNode(pFakeXMLRoot);

      if (pTempXMLNode && !pTempXMLNode->GetParent())
        pFakeXMLRoot = pTempXMLNode;
      else
        pFakeXMLRoot = nullptr;
    }
    MoveBufferMapData(pFakeRoot, GetXFANode());
  } else {
    CXFA_Node* pChild = pFakeRoot->GetFirstChild();
    while (pChild) {
      CXFA_Node* pItem = pChild->GetNextSibling();
      pFakeRoot->RemoveChildAndNotify(pChild, true);
      GetXFANode()->InsertChildAndNotify(pChild, nullptr);
      pChild->SetInitializedFlagAndNotify();
      pChild = pItem;
    }
  }

  if (pFakeXMLRoot) {
    pFakeRoot->SetXMLMappingNode(std::move(pFakeXMLRoot));
  }
  pFakeRoot->SetFlag(XFA_NodeFlag::kHasRemovedChildren);

  return CJS_Result::Success();
}

CJS_Result CJX_Node::saveFilteredXML(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  // TODO(weili): Check whether we need to implement this, pdfium:501.
  return CJS_Result::Success();
}

CJS_Result CJX_Node::saveXML(CFXJSE_Engine* runtime,
                             const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() > 1)
    return CJS_Result::Failure(JSMessage::kParamError);

  if (params.size() == 1 &&
      !runtime->ToWideString(params[0]).EqualsASCII("pretty")) {
    return CJS_Result::Failure(JSMessage::kValueError);
  }

  // TODO(weili): Check whether we need to save pretty print XML, pdfium:501.

  ByteString bsXMLHeader = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
  if (GetXFANode()->GetPacketType() != XFA_PacketType::Form &&
      GetXFANode()->GetPacketType() != XFA_PacketType::Datasets) {
    return CJS_Result::Success(runtime->NewString(""));
  }

  CFX_XMLNode* pElement = nullptr;
  if (GetXFANode()->GetPacketType() == XFA_PacketType::Datasets) {
    pElement = GetXFANode()->GetXMLMappingNode();
    if (!pElement || pElement->GetType() != CFX_XMLNode::Type::kElement) {
      return CJS_Result::Success(
          runtime->NewString(bsXMLHeader.AsStringView()));
    }

    XFA_DataExporter_DealWithDataGroupNode(GetXFANode());
  }

  auto pMemoryStream = pdfium::MakeRetain<CFX_MemoryStream>();
  pMemoryStream->WriteString(bsXMLHeader.AsStringView());

  if (GetXFANode()->GetPacketType() == XFA_PacketType::Form) {
    XFA_DataExporter_RegenerateFormFile(GetXFANode(), pMemoryStream, true);
  } else {
    pElement->Save(pMemoryStream);
  }

  return CJS_Result::Success(
      runtime->NewString(ByteStringView(pMemoryStream->GetSpan())));
}

CJS_Result CJX_Node::setAttribute(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  // Note: yes, arglist is spec'd absolutely backwards from what any sane
  // person would do, namely value first, attribute second.
  WideString attributeValue = runtime->ToWideString(params[0]);
  WideString attribute = runtime->ToWideString(params[1]);

  // Pass them to our method, however, in the more usual manner.
  SetAttributeByString(attribute.AsStringView(), attributeValue);
  return CJS_Result::Success();
}

CJS_Result CJX_Node::setElement(
    CFXJSE_Engine* runtime,
    const std::vector<v8::Local<v8::Value>>& params) {
  if (params.size() != 1 && params.size() != 2)
    return CJS_Result::Failure(JSMessage::kParamError);

  // TODO(weili): check whether we need to implement this, pdfium:501.
  return CJS_Result::Success();
}

void CJX_Node::ns(v8::Isolate* pIsolate,
                  v8::Local<v8::Value>* pValue,
                  bool bSetting,
                  XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  *pValue = fxv8::NewStringHelper(
      pIsolate, TryNamespace().value_or(WideString()).ToUTF8().AsStringView());
}

void CJX_Node::model(v8::Isolate* pIsolate,
                     v8::Local<v8::Value>* pValue,
                     bool bSetting,
                     XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  CXFA_Node* pModel = GetXFANode()->GetModelNode();
  if (!pModel) {
    *pValue = fxv8::NewNullHelper(pIsolate);
    return;
  }
  *pValue =
      GetDocument()->GetScriptContext()->GetOrCreateJSBindingFromMap(pModel);
}

void CJX_Node::isContainer(v8::Isolate* pIsolate,
                           v8::Local<v8::Value>* pValue,
                           bool bSetting,
                           XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  *pValue = fxv8::NewBooleanHelper(pIsolate, GetXFANode()->IsContainerNode());
}

void CJX_Node::isNull(v8::Isolate* pIsolate,
                      v8::Local<v8::Value>* pValue,
                      bool bSetting,
                      XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }
  if (GetXFANode()->GetElementType() == XFA_Element::Subform) {
    *pValue = fxv8::NewBooleanHelper(pIsolate, false);
    return;
  }
  *pValue = fxv8::NewBooleanHelper(pIsolate, GetContent(false).IsEmpty());
}

void CJX_Node::oneOfChild(v8::Isolate* pIsolate,
                          v8::Local<v8::Value>* pValue,
                          bool bSetting,
                          XFA_Attribute eAttribute) {
  if (bSetting) {
    ThrowInvalidPropertyException(pIsolate);
    return;
  }

  std::vector<CXFA_Node*> properties =
      GetXFANode()->GetNodeListWithFilter(XFA_NodeFilter::kOneOfProperty);
  if (!properties.empty()) {
    *pValue = GetDocument()->GetScriptContext()->GetOrCreateJSBindingFromMap(
        properties.front());
  }
}

XFA_EventError CJX_Node::execSingleEventByName(WideStringView wsEventName,
                                               XFA_Element eType) {
  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return XFA_EventError::kNotExist;

  const ExecEventParaInfo* eventParaInfo =
      GetExecEventParaInfoByName(wsEventName);
  if (!eventParaInfo)
    return XFA_EventError::kNotExist;

  switch (eventParaInfo->m_validFlags) {
    case EventAppliesTo::kNone:
      return XFA_EventError::kNotExist;
    case EventAppliesTo::kAll:
    case EventAppliesTo::kAllNonRecursive:
      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false,
          eventParaInfo->m_validFlags == EventAppliesTo::kAll);
    case EventAppliesTo::kSubform:
      if (eType != XFA_Element::Subform)
        return XFA_EventError::kNotExist;

      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    case EventAppliesTo::kFieldOrExclusion: {
      if (eType != XFA_Element::ExclGroup && eType != XFA_Element::Field)
        return XFA_EventError::kNotExist;

      CXFA_Node* pParentNode = GetXFANode()->GetParent();
      if (pParentNode &&
          pParentNode->GetElementType() == XFA_Element::ExclGroup) {
        // TODO(dsinclair): This seems like a bug, we do the same work twice?
        pNotify->ExecEventByDeepFirst(GetXFANode(), eventParaInfo->m_eventType,
                                      false, false);
      }
      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    }
    case EventAppliesTo::kField:
      if (eType != XFA_Element::Field)
        return XFA_EventError::kNotExist;

      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    case EventAppliesTo::kSignature: {
      if (!GetXFANode()->IsWidgetReady())
        return XFA_EventError::kNotExist;
      if (GetXFANode()->GetUIChildNode()->GetElementType() !=
          XFA_Element::Signature) {
        return XFA_EventError::kNotExist;
      }
      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    }
    case EventAppliesTo::kChoiceList: {
      if (!GetXFANode()->IsWidgetReady())
        return XFA_EventError::kNotExist;
      if (GetXFANode()->GetUIChildNode()->GetElementType() !=
          XFA_Element::ChoiceList) {
        return XFA_EventError::kNotExist;
      }
      return pNotify->ExecEventByDeepFirst(
          GetXFANode(), eventParaInfo->m_eventType, false, false);
    }
  }
  return XFA_EventError::kNotExist;
}

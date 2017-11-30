// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_node.h"

#include <map>
#include <memory>
#include <set>
#include <utility>
#include <vector>

#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_attachnodelist.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_nodeiteratortemplate.h"
#include "xfa/fxfa/parser/cxfa_occurdata.h"
#include "xfa/fxfa/parser/cxfa_simple_parser.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfacontainernode.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

std::vector<CXFA_Node*> NodesSortedByDocumentIdx(
    const std::set<CXFA_Node*>& rgNodeSet) {
  if (rgNodeSet.empty())
    return std::vector<CXFA_Node*>();

  std::vector<CXFA_Node*> rgNodeArray;
  CXFA_Node* pCommonParent =
      (*rgNodeSet.begin())->GetNodeItem(XFA_NODEITEM_Parent);
  for (CXFA_Node* pNode = pCommonParent->GetNodeItem(XFA_NODEITEM_FirstChild);
       pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pdfium::ContainsValue(rgNodeSet, pNode))
      rgNodeArray.push_back(pNode);
  }
  return rgNodeArray;
}

using CXFA_NodeSetPair = std::pair<std::set<CXFA_Node*>, std::set<CXFA_Node*>>;
using CXFA_NodeSetPairMap =
    std::map<uint32_t, std::unique_ptr<CXFA_NodeSetPair>>;
using CXFA_NodeSetPairMapMap =
    std::map<CXFA_Node*, std::unique_ptr<CXFA_NodeSetPairMap>>;

CXFA_NodeSetPair* NodeSetPairForNode(CXFA_Node* pNode,
                                     CXFA_NodeSetPairMapMap* pMap) {
  CXFA_Node* pParentNode = pNode->GetNodeItem(XFA_NODEITEM_Parent);
  uint32_t dwNameHash = pNode->GetNameHash();
  if (!pParentNode || !dwNameHash)
    return nullptr;

  if (!(*pMap)[pParentNode])
    (*pMap)[pParentNode] = pdfium::MakeUnique<CXFA_NodeSetPairMap>();

  CXFA_NodeSetPairMap* pNodeSetPairMap = (*pMap)[pParentNode].get();
  if (!(*pNodeSetPairMap)[dwNameHash])
    (*pNodeSetPairMap)[dwNameHash] = pdfium::MakeUnique<CXFA_NodeSetPair>();

  return (*pNodeSetPairMap)[dwNameHash].get();
}

void ReorderDataNodes(const std::set<CXFA_Node*>& sSet1,
                      const std::set<CXFA_Node*>& sSet2,
                      bool bInsertBefore) {
  CXFA_NodeSetPairMapMap rgMap;
  for (CXFA_Node* pNode : sSet1) {
    CXFA_NodeSetPair* pNodeSetPair = NodeSetPairForNode(pNode, &rgMap);
    if (pNodeSetPair)
      pNodeSetPair->first.insert(pNode);
  }
  for (CXFA_Node* pNode : sSet2) {
    CXFA_NodeSetPair* pNodeSetPair = NodeSetPairForNode(pNode, &rgMap);
    if (pNodeSetPair) {
      if (pdfium::ContainsValue(pNodeSetPair->first, pNode))
        pNodeSetPair->first.erase(pNode);
      else
        pNodeSetPair->second.insert(pNode);
    }
  }
  for (const auto& iter1 : rgMap) {
    CXFA_NodeSetPairMap* pNodeSetPairMap = iter1.second.get();
    if (!pNodeSetPairMap)
      continue;

    for (const auto& iter2 : *pNodeSetPairMap) {
      CXFA_NodeSetPair* pNodeSetPair = iter2.second.get();
      if (!pNodeSetPair)
        continue;
      if (!pNodeSetPair->first.empty() && !pNodeSetPair->second.empty()) {
        std::vector<CXFA_Node*> rgNodeArray1 =
            NodesSortedByDocumentIdx(pNodeSetPair->first);
        std::vector<CXFA_Node*> rgNodeArray2 =
            NodesSortedByDocumentIdx(pNodeSetPair->second);
        CXFA_Node* pParentNode = nullptr;
        CXFA_Node* pBeforeNode = nullptr;
        if (bInsertBefore) {
          pBeforeNode = rgNodeArray2.front();
          pParentNode = pBeforeNode->GetNodeItem(XFA_NODEITEM_Parent);
        } else {
          CXFA_Node* pLastNode = rgNodeArray2.back();
          pParentNode = pLastNode->GetNodeItem(XFA_NODEITEM_Parent);
          pBeforeNode = pLastNode->GetNodeItem(XFA_NODEITEM_NextSibling);
        }
        for (auto* pCurNode : rgNodeArray1) {
          pParentNode->RemoveChild(pCurNode, true);
          pParentNode->InsertChild(pCurNode, pBeforeNode);
        }
      }
    }
    pNodeSetPairMap->clear();
  }
}

}  // namespace

// static
WideString CXFA_Node::AttributeEnumToName(XFA_AttributeEnum item) {
  return g_XFAEnumData[static_cast<int32_t>(item)].pName;
}

// static
pdfium::Optional<XFA_AttributeEnum> CXFA_Node::NameToAttributeEnum(
    const WideStringView& name) {
  if (name.IsEmpty())
    return {};

  auto* it = std::lower_bound(g_XFAEnumData, g_XFAEnumData + g_iXFAEnumCount,
                              FX_HashCode_GetW(name, false),
                              [](const XFA_AttributeEnumInfo& arg,
                                 uint32_t hash) { return arg.uHash < hash; });
  if (it != g_XFAEnumData + g_iXFAEnumCount && name == it->pName)
    return {it->eName};
  return {};
}

CXFA_Node::CXFA_Node(CXFA_Document* pDoc,
                     XFA_PacketType ePacket,
                     uint32_t validPackets,
                     XFA_ObjectType oType,
                     XFA_Element eType,
                     const PropertyData* properties,
                     const AttributeData* attributes,
                     const WideStringView& elementName)
    : CXFA_Object(pDoc,
                  oType,
                  eType,
                  elementName,
                  pdfium::MakeUnique<CJX_Node>(this)),
      m_Properties(properties),
      m_Attributes(attributes),
      m_ValidPackets(validPackets),
      m_pNext(nullptr),
      m_pChild(nullptr),
      m_pLastChild(nullptr),
      m_pParent(nullptr),
      m_pXMLNode(nullptr),
      m_ePacket(ePacket),
      m_uNodeFlags(XFA_NodeFlag_None),
      m_dwNameHash(0),
      m_pAuxNode(nullptr) {
  ASSERT(m_pDocument);
}

CXFA_Node::~CXFA_Node() {
  ASSERT(!m_pParent);

  CXFA_Node* pNode = m_pChild;
  while (pNode) {
    CXFA_Node* pNext = pNode->m_pNext;
    pNode->m_pParent = nullptr;
    delete pNode;
    pNode = pNext;
  }
  if (m_pXMLNode && IsOwnXMLNode())
    delete m_pXMLNode;
}

CXFA_Node* CXFA_Node::Clone(bool bRecursive) {
  CXFA_Node* pClone = m_pDocument->CreateNode(m_ePacket, m_elementType);
  if (!pClone)
    return nullptr;

  JSNode()->MergeAllData(pClone);
  pClone->UpdateNameHash();
  if (IsNeedSavingXMLNode()) {
    std::unique_ptr<CFX_XMLNode> pCloneXML;
    if (IsAttributeInXML()) {
      WideString wsName = JSNode()
                              ->TryAttribute(XFA_Attribute::Name, false)
                              .value_or(WideString());
      auto pCloneXMLElement = pdfium::MakeUnique<CFX_XMLElement>(wsName);
      WideString wsValue = JSNode()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty())
        pCloneXMLElement->SetTextData(WideString(wsValue));

      pCloneXML.reset(pCloneXMLElement.release());
      pClone->JSNode()->SetEnum(XFA_Attribute::Contains,
                                XFA_AttributeEnum::Unknown, false);
    } else {
      pCloneXML = m_pXMLNode->Clone();
    }
    pClone->SetXMLMappingNode(pCloneXML.release());
    pClone->SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  if (bRecursive) {
    for (CXFA_Node* pChild = GetNodeItem(XFA_NODEITEM_FirstChild); pChild;
         pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      pClone->InsertChild(pChild->Clone(bRecursive), nullptr);
    }
  }
  pClone->SetFlag(XFA_NodeFlag_Initialized, true);
  pClone->JSNode()->SetBindingNode(nullptr);
  return pClone;
}

CXFA_Node* CXFA_Node::GetNodeItem(XFA_NODEITEM eItem) const {
  switch (eItem) {
    case XFA_NODEITEM_NextSibling:
      return m_pNext;
    case XFA_NODEITEM_FirstChild:
      return m_pChild;
    case XFA_NODEITEM_Parent:
      return m_pParent;
    case XFA_NODEITEM_PrevSibling:
      if (m_pParent) {
        CXFA_Node* pSibling = m_pParent->m_pChild;
        CXFA_Node* pPrev = nullptr;
        while (pSibling && pSibling != this) {
          pPrev = pSibling;
          pSibling = pSibling->m_pNext;
        }
        return pPrev;
      }
      return nullptr;
    default:
      break;
  }
  return nullptr;
}

bool CXFA_Node::IsValidInPacket(XFA_PacketType packet) const {
  return !!(m_ValidPackets & (1 << static_cast<uint8_t>(packet)));
}

const CXFA_Node::PropertyData* CXFA_Node::GetPropertyData(
    XFA_Element property) const {
  if (m_Properties == nullptr)
    return nullptr;

  for (size_t i = 0;; ++i) {
    const PropertyData* data = m_Properties + i;
    if (data->property == XFA_Element::Unknown)
      break;
    if (data->property == property)
      return data;
  }
  return nullptr;
}

bool CXFA_Node::HasProperty(XFA_Element property) const {
  return !!GetPropertyData(property);
}

bool CXFA_Node::HasPropertyFlags(XFA_Element property, uint8_t flags) const {
  const PropertyData* data = GetPropertyData(property);
  return data && !!(data->flags & flags);
}

uint8_t CXFA_Node::PropertyOccuranceCount(XFA_Element property) const {
  const PropertyData* data = GetPropertyData(property);
  return data ? data->occurance_count : 0;
}

pdfium::Optional<XFA_Element> CXFA_Node::GetFirstPropertyWithFlag(
    uint8_t flag) {
  if (m_Properties == nullptr)
    return {};

  for (size_t i = 0;; ++i) {
    const PropertyData* data = m_Properties + i;
    if (data->property == XFA_Element::Unknown)
      break;
    if (data->flags & flag)
      return {data->property};
  }
  return {};
}

const CXFA_Node::AttributeData* CXFA_Node::GetAttributeData(
    XFA_Attribute attr) const {
  if (m_Attributes == nullptr)
    return nullptr;

  for (size_t i = 0;; ++i) {
    const AttributeData* cur_attr = &m_Attributes[i];
    if (cur_attr->attribute == XFA_Attribute::Unknown)
      break;
    if (cur_attr->attribute == attr)
      return cur_attr;
  }
  return nullptr;
}

bool CXFA_Node::HasAttribute(XFA_Attribute attr) const {
  return !!GetAttributeData(attr);
}

// Note: This Method assumes that i is a valid index ....
XFA_Attribute CXFA_Node::GetAttribute(size_t i) const {
  if (m_Attributes == nullptr)
    return XFA_Attribute::Unknown;
  return m_Attributes[i].attribute;
}

XFA_AttributeType CXFA_Node::GetAttributeType(XFA_Attribute type) const {
  const AttributeData* data = GetAttributeData(type);
  return data ? data->type : XFA_AttributeType::CData;
}

CXFA_Node* CXFA_Node::GetNodeItem(XFA_NODEITEM eItem,
                                  XFA_ObjectType eType) const {
  CXFA_Node* pNode = nullptr;
  switch (eItem) {
    case XFA_NODEITEM_NextSibling:
      pNode = m_pNext;
      while (pNode && pNode->GetObjectType() != eType)
        pNode = pNode->m_pNext;
      break;
    case XFA_NODEITEM_FirstChild:
      pNode = m_pChild;
      while (pNode && pNode->GetObjectType() != eType)
        pNode = pNode->m_pNext;
      break;
    case XFA_NODEITEM_Parent:
      pNode = m_pParent;
      while (pNode && pNode->GetObjectType() != eType)
        pNode = pNode->m_pParent;
      break;
    case XFA_NODEITEM_PrevSibling:
      if (m_pParent) {
        CXFA_Node* pSibling = m_pParent->m_pChild;
        while (pSibling && pSibling != this) {
          if (eType == pSibling->GetObjectType())
            pNode = pSibling;

          pSibling = pSibling->m_pNext;
        }
      }
      break;
    default:
      break;
  }
  return pNode;
}

std::vector<CXFA_Node*> CXFA_Node::GetNodeList(uint32_t dwTypeFilter,
                                               XFA_Element eTypeFilter) {
  if (eTypeFilter != XFA_Element::Unknown) {
    std::vector<CXFA_Node*> nodes;
    for (CXFA_Node* pChild = m_pChild; pChild; pChild = pChild->m_pNext) {
      if (pChild->GetElementType() == eTypeFilter)
        nodes.push_back(pChild);
    }
    return nodes;
  }

  if (dwTypeFilter == (XFA_NODEFILTER_Children | XFA_NODEFILTER_Properties)) {
    std::vector<CXFA_Node*> nodes;
    for (CXFA_Node* pChild = m_pChild; pChild; pChild = pChild->m_pNext)
      nodes.push_back(pChild);
    return nodes;
  }

  if (dwTypeFilter == 0)
    return std::vector<CXFA_Node*>();

  bool bFilterChildren = !!(dwTypeFilter & XFA_NODEFILTER_Children);
  bool bFilterProperties = !!(dwTypeFilter & XFA_NODEFILTER_Properties);
  bool bFilterOneOfProperties = !!(dwTypeFilter & XFA_NODEFILTER_OneOfProperty);
  std::vector<CXFA_Node*> nodes;
  for (CXFA_Node* pChild = m_pChild; pChild; pChild = pChild->m_pNext) {
    if (!HasProperty(pChild->GetElementType())) {
      if (bFilterProperties) {
        nodes.push_back(pChild);
      } else if (bFilterOneOfProperties &&
                 HasPropertyFlags(pChild->GetElementType(),
                                  XFA_PROPERTYFLAG_OneOf)) {
        nodes.push_back(pChild);
      } else if (bFilterChildren &&
                 (pChild->GetElementType() == XFA_Element::Variables ||
                  pChild->GetElementType() == XFA_Element::PageSet)) {
        nodes.push_back(pChild);
      }
    } else if (bFilterChildren) {
      nodes.push_back(pChild);
    }
  }

  if (!bFilterOneOfProperties || !nodes.empty())
    return nodes;
  if (m_Properties == nullptr)
    return nodes;

  pdfium::Optional<XFA_Element> property =
      GetFirstPropertyWithFlag(XFA_PROPERTYFLAG_DefaultOneOf);
  if (!property)
    return nodes;

  CXFA_Node* pNewNode = m_pDocument->CreateNode(GetPacketType(), *property);
  if (pNewNode) {
    InsertChild(pNewNode, nullptr);
    pNewNode->SetFlag(XFA_NodeFlag_Initialized, true);
    nodes.push_back(pNewNode);
  }
  return nodes;
}

CXFA_Node* CXFA_Node::CreateSamePacketNode(XFA_Element eType) {
  CXFA_Node* pNode = m_pDocument->CreateNode(m_ePacket, eType);
  pNode->SetFlag(XFA_NodeFlag_Initialized, true);
  return pNode;
}

CXFA_Node* CXFA_Node::CloneTemplateToForm(bool bRecursive) {
  ASSERT(m_ePacket == XFA_PacketType::Template);
  CXFA_Node* pClone =
      m_pDocument->CreateNode(XFA_PacketType::Form, m_elementType);
  if (!pClone)
    return nullptr;

  pClone->SetTemplateNode(this);
  pClone->UpdateNameHash();
  pClone->SetXMLMappingNode(GetXMLMappingNode());
  if (bRecursive) {
    for (CXFA_Node* pChild = GetNodeItem(XFA_NODEITEM_FirstChild); pChild;
         pChild = pChild->GetNodeItem(XFA_NODEITEM_NextSibling)) {
      pClone->InsertChild(pChild->CloneTemplateToForm(bRecursive), nullptr);
    }
  }
  pClone->SetFlag(XFA_NodeFlag_Initialized, true);
  return pClone;
}

CXFA_Node* CXFA_Node::GetTemplateNode() const {
  return m_pAuxNode;
}

void CXFA_Node::SetTemplateNode(CXFA_Node* pTemplateNode) {
  m_pAuxNode = pTemplateNode;
}

CXFA_Node* CXFA_Node::GetBindData() {
  ASSERT(GetPacketType() == XFA_PacketType::Form);
  return JSNode()->GetBindingNode();
}

std::vector<UnownedPtr<CXFA_Node>>* CXFA_Node::GetBindItems() {
  return JSNode()->GetBindingNodes();
}

int32_t CXFA_Node::AddBindItem(CXFA_Node* pFormNode) {
  ASSERT(pFormNode);

  if (BindsFormItems()) {
    std::vector<UnownedPtr<CXFA_Node>>* nodes = JSNode()->GetBindingNodes();
    bool found = false;
    for (auto& v : *nodes) {
      if (v.Get() == pFormNode) {
        found = true;
        break;
      }
    }
    if (!found)
      nodes->emplace_back(pFormNode);
    return pdfium::CollectionSize<int32_t>(*nodes);
  }

  CXFA_Node* pOldFormItem = JSNode()->GetBindingNode();
  if (!pOldFormItem) {
    JSNode()->SetBindingNode(pFormNode);
    return 1;
  }
  if (pOldFormItem == pFormNode)
    return 1;

  std::vector<UnownedPtr<CXFA_Node>> items;
  items.emplace_back(pOldFormItem);
  items.emplace_back(pFormNode);
  JSNode()->SetBindingNodes(std::move(items));

  m_uNodeFlags |= XFA_NodeFlag_BindFormItems;
  return 2;
}

int32_t CXFA_Node::RemoveBindItem(CXFA_Node* pFormNode) {
  if (BindsFormItems()) {
    std::vector<UnownedPtr<CXFA_Node>>* nodes = JSNode()->GetBindingNodes();

    auto it = std::find_if(nodes->begin(), nodes->end(),
                           [&pFormNode](const UnownedPtr<CXFA_Node>& node) {
                             return node.Get() == pFormNode;
                           });
    if (it != nodes->end())
      nodes->erase(it);

    if (nodes->size() == 1) {
      m_uNodeFlags &= ~XFA_NodeFlag_BindFormItems;
      return 1;
    }
    return pdfium::CollectionSize<int32_t>(*nodes);
  }

  CXFA_Node* pOldFormItem = JSNode()->GetBindingNode();
  if (pOldFormItem != pFormNode)
    return pOldFormItem ? 1 : 0;

  JSNode()->SetBindingNode(nullptr);
  return 0;
}

bool CXFA_Node::HasBindItem() {
  return GetPacketType() == XFA_PacketType::Datasets &&
         JSNode()->GetBindingNode();
}

CXFA_WidgetData* CXFA_Node::GetWidgetData() {
  return JSNode()->GetWidgetData();
}

CXFA_WidgetData* CXFA_Node::GetContainerWidgetData() {
  if (GetPacketType() != XFA_PacketType::Form)
    return nullptr;
  XFA_Element eType = GetElementType();
  if (eType == XFA_Element::ExclGroup)
    return nullptr;
  CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
  if (pParentNode && pParentNode->GetElementType() == XFA_Element::ExclGroup)
    return nullptr;

  if (eType == XFA_Element::Field) {
    CXFA_WidgetData* pFieldWidgetData = GetWidgetData();
    if (pFieldWidgetData && pFieldWidgetData->IsChoiceListMultiSelect())
      return nullptr;

    WideString wsPicture;
    if (pFieldWidgetData) {
      wsPicture =
          pFieldWidgetData->GetPictureContent(XFA_VALUEPICTURE_DataBind);
    }
    if (!wsPicture.IsEmpty())
      return pFieldWidgetData;

    CXFA_Node* pDataNode = GetBindData();
    if (!pDataNode)
      return nullptr;
    pFieldWidgetData = nullptr;
    for (const auto& pFormNode : *(pDataNode->GetBindItems())) {
      if (!pFormNode || pFormNode->HasRemovedChildren())
        continue;
      pFieldWidgetData = pFormNode->GetWidgetData();
      if (pFieldWidgetData) {
        wsPicture =
            pFieldWidgetData->GetPictureContent(XFA_VALUEPICTURE_DataBind);
      }
      if (!wsPicture.IsEmpty())
        break;
      pFieldWidgetData = nullptr;
    }
    return pFieldWidgetData;
  }

  CXFA_Node* pGrandNode =
      pParentNode ? pParentNode->GetNodeItem(XFA_NODEITEM_Parent) : nullptr;
  CXFA_Node* pValueNode =
      (pParentNode && pParentNode->GetElementType() == XFA_Element::Value)
          ? pParentNode
          : nullptr;
  if (!pValueNode) {
    pValueNode =
        (pGrandNode && pGrandNode->GetElementType() == XFA_Element::Value)
            ? pGrandNode
            : nullptr;
  }
  CXFA_Node* pParentOfValueNode =
      pValueNode ? pValueNode->GetNodeItem(XFA_NODEITEM_Parent) : nullptr;
  return pParentOfValueNode ? pParentOfValueNode->GetContainerWidgetData()
                            : nullptr;
}

bool CXFA_Node::GetLocaleName(WideString& wsLocaleName) {
  CXFA_Node* pForm = GetDocument()->GetXFAObject(XFA_HASHCODE_Form)->AsNode();
  CXFA_Node* pTopSubform = pForm->GetFirstChildByClass(XFA_Element::Subform);
  ASSERT(pTopSubform);

  CXFA_Node* pLocaleNode = this;
  do {
    pdfium::Optional<WideString> ret =
        pLocaleNode->JSNode()->TryCData(XFA_Attribute::Locale, false);
    if (ret) {
      wsLocaleName = *ret;
      return true;
    }
    pLocaleNode = pLocaleNode->GetNodeItem(XFA_NODEITEM_Parent);
  } while (pLocaleNode && pLocaleNode != pTopSubform);

  CXFA_Node* pConfig = ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Config));
  wsLocaleName = GetDocument()->GetLocalMgr()->GetConfigLocaleName(pConfig);
  if (!wsLocaleName.IsEmpty())
    return true;

  if (pTopSubform) {
    pdfium::Optional<WideString> ret =
        pTopSubform->JSNode()->TryCData(XFA_Attribute::Locale, false);
    if (ret) {
      wsLocaleName = *ret;
      return true;
    }
  }

  IFX_Locale* pLocale = GetDocument()->GetLocalMgr()->GetDefLocale();
  if (!pLocale)
    return false;

  wsLocaleName = pLocale->GetName();
  return true;
}

XFA_AttributeEnum CXFA_Node::GetIntact() {
  CXFA_Node* pKeep = GetFirstChildByClass(XFA_Element::Keep);
  XFA_AttributeEnum eLayoutType = JSNode()
                                      ->TryEnum(XFA_Attribute::Layout, true)
                                      .value_or(XFA_AttributeEnum::Position);
  if (pKeep) {
    pdfium::Optional<XFA_AttributeEnum> intact =
        pKeep->JSNode()->TryEnum(XFA_Attribute::Intact, false);
    if (intact) {
      if (*intact == XFA_AttributeEnum::None &&
          eLayoutType == XFA_AttributeEnum::Row &&
          m_pDocument->GetCurVersionMode() < XFA_VERSION_208) {
        CXFA_Node* pPreviewRow = GetNodeItem(XFA_NODEITEM_PrevSibling,
                                             XFA_ObjectType::ContainerNode);
        if (pPreviewRow &&
            pPreviewRow->JSNode()->GetEnum(XFA_Attribute::Layout) ==
                XFA_AttributeEnum::Row) {
          pdfium::Optional<XFA_AttributeEnum> value =
              pKeep->JSNode()->TryEnum(XFA_Attribute::Previous, false);
          if (value && (*value == XFA_AttributeEnum::ContentArea ||
                        *value == XFA_AttributeEnum::PageArea)) {
            return XFA_AttributeEnum::ContentArea;
          }

          CXFA_Node* pNode =
              pPreviewRow->GetFirstChildByClass(XFA_Element::Keep);
          pdfium::Optional<XFA_AttributeEnum> ret;
          if (pNode)
            ret = pNode->JSNode()->TryEnum(XFA_Attribute::Next, false);
          if (ret && (*ret == XFA_AttributeEnum::ContentArea ||
                      *ret == XFA_AttributeEnum::PageArea)) {
            return XFA_AttributeEnum::ContentArea;
          }
        }
      }
      return *intact;
    }
  }
  switch (GetElementType()) {
    case XFA_Element::Subform:
      switch (eLayoutType) {
        case XFA_AttributeEnum::Position:
        case XFA_AttributeEnum::Row:
          return XFA_AttributeEnum::ContentArea;
        case XFA_AttributeEnum::Tb:
        case XFA_AttributeEnum::Table:
        case XFA_AttributeEnum::Lr_tb:
        case XFA_AttributeEnum::Rl_tb:
          return XFA_AttributeEnum::None;
        default:
          break;
      }
      break;
    case XFA_Element::Field: {
      CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
      if (!pParentNode ||
          pParentNode->GetElementType() == XFA_Element::PageArea)
        return XFA_AttributeEnum::ContentArea;
      if (pParentNode->GetIntact() == XFA_AttributeEnum::None) {
        XFA_AttributeEnum eParLayout =
            pParentNode->JSNode()
                ->TryEnum(XFA_Attribute::Layout, true)
                .value_or(XFA_AttributeEnum::Position);
        if (eParLayout == XFA_AttributeEnum::Position ||
            eParLayout == XFA_AttributeEnum::Row ||
            eParLayout == XFA_AttributeEnum::Table) {
          return XFA_AttributeEnum::None;
        }
        XFA_VERSION version = m_pDocument->GetCurVersionMode();
        if (eParLayout == XFA_AttributeEnum::Tb && version < XFA_VERSION_208) {
          pdfium::Optional<CXFA_Measurement> measureH =
              JSNode()->TryMeasure(XFA_Attribute::H, false);
          if (measureH)
            return XFA_AttributeEnum::ContentArea;
        }
        return XFA_AttributeEnum::None;
      }
      return XFA_AttributeEnum::ContentArea;
    }
    case XFA_Element::Draw:
      return XFA_AttributeEnum::ContentArea;
    default:
      break;
  }
  return XFA_AttributeEnum::None;
}

CXFA_Node* CXFA_Node::GetDataDescriptionNode() {
  if (m_ePacket == XFA_PacketType::Datasets)
    return m_pAuxNode;
  return nullptr;
}

void CXFA_Node::SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode) {
  ASSERT(m_ePacket == XFA_PacketType::Datasets);
  m_pAuxNode = pDataDescriptionNode;
}

CXFA_Node* CXFA_Node::GetModelNode() {
  switch (GetPacketType()) {
    case XFA_PacketType::Xdp:
      return m_pDocument->GetRoot();
    case XFA_PacketType::Config:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Config));
    case XFA_PacketType::Template:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Template));
    case XFA_PacketType::Form:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Form));
    case XFA_PacketType::Datasets:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Datasets));
    case XFA_PacketType::LocaleSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_LocaleSet));
    case XFA_PacketType::ConnectionSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_ConnectionSet));
    case XFA_PacketType::SourceSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_SourceSet));
    case XFA_PacketType::Xdc:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Xdc));
    default:
      return this;
  }
}

int32_t CXFA_Node::CountChildren(XFA_Element eType, bool bOnlyChild) {
  int32_t iCount = 0;
  for (CXFA_Node* pNode = m_pChild; pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() != eType && eType != XFA_Element::Unknown)
      continue;
    if (bOnlyChild && HasProperty(pNode->GetElementType()))
      continue;
    ++iCount;
  }
  return iCount;
}

CXFA_Node* CXFA_Node::GetChild(int32_t index,
                               XFA_Element eType,
                               bool bOnlyChild) {
  ASSERT(index > -1);

  int32_t iCount = 0;
  for (CXFA_Node* pNode = m_pChild; pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() != eType && eType != XFA_Element::Unknown)
      continue;
    if (bOnlyChild && HasProperty(pNode->GetElementType()))
      continue;
    if (iCount == index)
      return pNode;

    ++iCount;
  }
  return nullptr;
}

int32_t CXFA_Node::InsertChild(int32_t index, CXFA_Node* pNode) {
  ASSERT(!pNode->m_pNext);
  pNode->m_pParent = this;
  bool ret = m_pDocument->RemovePurgeNode(pNode);
  ASSERT(ret);
  (void)ret;  // Avoid unused variable warning.

  if (!m_pChild || index == 0) {
    if (index > 0) {
      return -1;
    }
    pNode->m_pNext = m_pChild;
    m_pChild = pNode;
    index = 0;
  } else if (index < 0) {
    m_pLastChild->m_pNext = pNode;
  } else {
    CXFA_Node* pPrev = m_pChild;
    int32_t iCount = 0;
    while (++iCount != index && pPrev->m_pNext) {
      pPrev = pPrev->m_pNext;
    }
    if (index > 0 && index != iCount) {
      return -1;
    }
    pNode->m_pNext = pPrev->m_pNext;
    pPrev->m_pNext = pNode;
    index = iCount;
  }
  if (!pNode->m_pNext) {
    m_pLastChild = pNode;
  }
  ASSERT(m_pLastChild);
  ASSERT(!m_pLastChild->m_pNext);
  pNode->ClearFlag(XFA_NodeFlag_HasRemovedChildren);
  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildAdded(this);

  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    ASSERT(!pNode->m_pXMLNode->GetNodeItem(CFX_XMLNode::Parent));
    m_pXMLNode->InsertChildNode(pNode->m_pXMLNode, index);
    pNode->ClearFlag(XFA_NodeFlag_OwnXMLNode);
  }
  return index;
}

bool CXFA_Node::InsertChild(CXFA_Node* pNode, CXFA_Node* pBeforeNode) {
  if (!pNode || pNode->m_pParent ||
      (pBeforeNode && pBeforeNode->m_pParent != this)) {
    NOTREACHED();
    return false;
  }
  bool ret = m_pDocument->RemovePurgeNode(pNode);
  ASSERT(ret);
  (void)ret;  // Avoid unused variable warning.

  int32_t nIndex = -1;
  pNode->m_pParent = this;
  if (!m_pChild || pBeforeNode == m_pChild) {
    pNode->m_pNext = m_pChild;
    m_pChild = pNode;
    nIndex = 0;
  } else if (!pBeforeNode) {
    pNode->m_pNext = m_pLastChild->m_pNext;
    m_pLastChild->m_pNext = pNode;
  } else {
    nIndex = 1;
    CXFA_Node* pPrev = m_pChild;
    while (pPrev->m_pNext != pBeforeNode) {
      pPrev = pPrev->m_pNext;
      nIndex++;
    }
    pNode->m_pNext = pPrev->m_pNext;
    pPrev->m_pNext = pNode;
  }
  if (!pNode->m_pNext) {
    m_pLastChild = pNode;
  }
  ASSERT(m_pLastChild);
  ASSERT(!m_pLastChild->m_pNext);
  pNode->ClearFlag(XFA_NodeFlag_HasRemovedChildren);
  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildAdded(this);

  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    ASSERT(!pNode->m_pXMLNode->GetNodeItem(CFX_XMLNode::Parent));
    m_pXMLNode->InsertChildNode(pNode->m_pXMLNode, nIndex);
    pNode->ClearFlag(XFA_NodeFlag_OwnXMLNode);
  }
  return true;
}

CXFA_Node* CXFA_Node::Deprecated_GetPrevSibling() {
  if (!m_pParent) {
    return nullptr;
  }
  for (CXFA_Node* pSibling = m_pParent->m_pChild; pSibling;
       pSibling = pSibling->m_pNext) {
    if (pSibling->m_pNext == this) {
      return pSibling;
    }
  }
  return nullptr;
}

bool CXFA_Node::RemoveChild(CXFA_Node* pNode, bool bNotify) {
  if (!pNode || pNode->m_pParent != this) {
    NOTREACHED();
    return false;
  }
  if (m_pChild == pNode) {
    m_pChild = pNode->m_pNext;
    if (m_pLastChild == pNode) {
      m_pLastChild = pNode->m_pNext;
    }
    pNode->m_pNext = nullptr;
    pNode->m_pParent = nullptr;
  } else {
    CXFA_Node* pPrev = pNode->Deprecated_GetPrevSibling();
    pPrev->m_pNext = pNode->m_pNext;
    if (m_pLastChild == pNode) {
      m_pLastChild = pNode->m_pNext ? pNode->m_pNext : pPrev;
    }
    pNode->m_pNext = nullptr;
    pNode->m_pParent = nullptr;
  }
  ASSERT(!m_pLastChild || !m_pLastChild->m_pNext);
  OnRemoved(bNotify);
  pNode->SetFlag(XFA_NodeFlag_HasRemovedChildren, true);
  m_pDocument->AddPurgeNode(pNode);
  if (IsNeedSavingXMLNode() && pNode->m_pXMLNode) {
    if (pNode->IsAttributeInXML()) {
      ASSERT(pNode->m_pXMLNode == m_pXMLNode &&
             m_pXMLNode->GetType() == FX_XMLNODE_Element);
      if (pNode->m_pXMLNode->GetType() == FX_XMLNODE_Element) {
        CFX_XMLElement* pXMLElement =
            static_cast<CFX_XMLElement*>(pNode->m_pXMLNode);
        WideString wsAttributeName =
            pNode->JSNode()->GetCData(XFA_Attribute::QualifiedName);
        pXMLElement->RemoveAttribute(wsAttributeName.c_str());
      }

      WideString wsName = pNode->JSNode()
                              ->TryAttribute(XFA_Attribute::Name, false)
                              .value_or(WideString());
      CFX_XMLElement* pNewXMLElement = new CFX_XMLElement(wsName);
      WideString wsValue = JSNode()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty())
        pNewXMLElement->SetTextData(WideString(wsValue));

      pNode->m_pXMLNode = pNewXMLElement;
      pNode->JSNode()->SetEnum(XFA_Attribute::Contains,
                               XFA_AttributeEnum::Unknown, false);
    } else {
      m_pXMLNode->RemoveChildNode(pNode->m_pXMLNode);
    }
    pNode->SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  return true;
}

CXFA_Node* CXFA_Node::GetFirstChildByName(const WideStringView& wsName) const {
  return GetFirstChildByName(FX_HashCode_GetW(wsName, false));
}

CXFA_Node* CXFA_Node::GetFirstChildByName(uint32_t dwNameHash) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == dwNameHash) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetFirstChildByClass(XFA_Element eType) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_FirstChild); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eType) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextSameNameSibling(uint32_t dwNameHash) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetNameHash() == dwNameHash) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextSameNameSibling(
    const WideStringView& wsNodeName) const {
  return GetNextSameNameSibling(FX_HashCode_GetW(wsNodeName, false));
}

CXFA_Node* CXFA_Node::GetNextSameClassSibling(XFA_Element eType) const {
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eType) {
      return pNode;
    }
  }
  return nullptr;
}

int32_t CXFA_Node::GetNodeSameNameIndex() const {
  CFXJSE_Engine* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return -1;
  }
  return pScriptContext->GetIndexByName(const_cast<CXFA_Node*>(this));
}

int32_t CXFA_Node::GetNodeSameClassIndex() const {
  CFXJSE_Engine* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return -1;
  }
  return pScriptContext->GetIndexByClassName(const_cast<CXFA_Node*>(this));
}

void CXFA_Node::GetSOMExpression(WideString& wsSOMExpression) {
  CFXJSE_Engine* pScriptContext = m_pDocument->GetScriptContext();
  if (!pScriptContext) {
    return;
  }
  pScriptContext->GetSomExpression(this, wsSOMExpression);
}

CXFA_Node* CXFA_Node::GetInstanceMgrOfSubform() {
  CXFA_Node* pInstanceMgr = nullptr;
  if (m_ePacket == XFA_PacketType::Form) {
    CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
    if (!pParentNode || pParentNode->GetElementType() == XFA_Element::Area) {
      return pInstanceMgr;
    }
    for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_PrevSibling); pNode;
         pNode = pNode->GetNodeItem(XFA_NODEITEM_PrevSibling)) {
      XFA_Element eType = pNode->GetElementType();
      if ((eType == XFA_Element::Subform || eType == XFA_Element::SubformSet) &&
          pNode->m_dwNameHash != m_dwNameHash) {
        break;
      }
      if (eType == XFA_Element::InstanceManager) {
        WideString wsName = JSNode()->GetCData(XFA_Attribute::Name);
        WideString wsInstName = pNode->JSNode()->GetCData(XFA_Attribute::Name);
        if (wsInstName.GetLength() > 0 && wsInstName[0] == '_' &&
            wsInstName.Right(wsInstName.GetLength() - 1) == wsName) {
          pInstanceMgr = pNode;
        }
        break;
      }
    }
  }
  return pInstanceMgr;
}

CXFA_Node* CXFA_Node::GetOccurNode() {
  return GetFirstChildByClass(XFA_Element::Occur);
}

bool CXFA_Node::HasFlag(XFA_NodeFlag dwFlag) const {
  if (m_uNodeFlags & dwFlag)
    return true;
  if (dwFlag == XFA_NodeFlag_HasRemovedChildren)
    return m_pParent && m_pParent->HasFlag(dwFlag);
  return false;
}

void CXFA_Node::SetFlag(uint32_t dwFlag, bool bNotify) {
  if (dwFlag == XFA_NodeFlag_Initialized && bNotify && !IsInitialized()) {
    CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
    if (pNotify) {
      pNotify->OnNodeReady(this);
    }
  }
  m_uNodeFlags |= dwFlag;
}

void CXFA_Node::ClearFlag(uint32_t dwFlag) {
  m_uNodeFlags &= ~dwFlag;
}

void CXFA_Node::ReleaseBindingNodes() {
  // Clear any binding nodes set on our JS node as we don't necessarily destruct
  // in an order that makes sense.
  JSNode()->ReleaseBindingNodes();

  for (CXFA_Node* pNode = m_pChild; pNode; pNode = pNode->m_pNext)
    pNode->ReleaseBindingNodes();
}

bool CXFA_Node::IsAttributeInXML() {
  return JSNode()->GetEnum(XFA_Attribute::Contains) ==
         XFA_AttributeEnum::MetaData;
}

void CXFA_Node::OnRemoved(bool bNotify) {
  if (!bNotify)
    return;

  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildRemoved();
}

void CXFA_Node::UpdateNameHash() {
  WideString wsName = JSNode()->GetCData(XFA_Attribute::Name);
  m_dwNameHash = FX_HashCode_GetW(wsName.AsStringView(), false);
}

CFX_XMLNode* CXFA_Node::CreateXMLMappingNode() {
  if (!m_pXMLNode) {
    WideString wsTag(JSNode()->GetCData(XFA_Attribute::Name));
    m_pXMLNode = new CFX_XMLElement(wsTag);
    SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  return m_pXMLNode;
}

bool CXFA_Node::IsNeedSavingXMLNode() {
  return m_pXMLNode && (GetPacketType() == XFA_PacketType::Datasets ||
                        GetElementType() == XFA_Element::Xfa);
}

CXFA_Node* CXFA_Node::GetItem(int32_t iIndex) {
  int32_t iCount = 0;
  uint32_t dwNameHash = 0;
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_Element eCurType = pNode->GetElementType();
    if (eCurType == XFA_Element::InstanceManager)
      break;
    if ((eCurType != XFA_Element::Subform) &&
        (eCurType != XFA_Element::SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      WideString wsName = pNode->JSNode()->GetCData(XFA_Attribute::Name);
      WideString wsInstName = JSNode()->GetCData(XFA_Attribute::Name);
      if (wsInstName.GetLength() < 1 || wsInstName[0] != '_' ||
          wsInstName.Right(wsInstName.GetLength() - 1) != wsName) {
        return nullptr;
      }
      dwNameHash = pNode->GetNameHash();
    }
    if (dwNameHash != pNode->GetNameHash())
      break;

    iCount++;
    if (iCount > iIndex)
      return pNode;
  }
  return nullptr;
}

int32_t CXFA_Node::GetCount() {
  int32_t iCount = 0;
  uint32_t dwNameHash = 0;
  for (CXFA_Node* pNode = GetNodeItem(XFA_NODEITEM_NextSibling); pNode;
       pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    XFA_Element eCurType = pNode->GetElementType();
    if (eCurType == XFA_Element::InstanceManager)
      break;
    if ((eCurType != XFA_Element::Subform) &&
        (eCurType != XFA_Element::SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      WideString wsName = pNode->JSNode()->GetCData(XFA_Attribute::Name);
      WideString wsInstName = JSNode()->GetCData(XFA_Attribute::Name);
      if (wsInstName.GetLength() < 1 || wsInstName[0] != '_' ||
          wsInstName.Right(wsInstName.GetLength() - 1) != wsName) {
        return iCount;
      }
      dwNameHash = pNode->GetNameHash();
    }
    if (dwNameHash != pNode->GetNameHash())
      break;

    iCount++;
  }
  return iCount;
}

void CXFA_Node::InsertItem(CXFA_Node* pNewInstance,
                           int32_t iPos,
                           int32_t iCount,
                           bool bMoveDataBindingNodes) {
  if (iCount < 0)
    iCount = GetCount();
  if (iPos < 0)
    iPos = iCount;
  if (iPos == iCount) {
    CXFA_Node* pNextSibling =
        iCount > 0 ? GetItem(iCount - 1)->GetNodeItem(XFA_NODEITEM_NextSibling)
                   : GetNodeItem(XFA_NODEITEM_NextSibling);
    GetNodeItem(XFA_NODEITEM_Parent)->InsertChild(pNewInstance, pNextSibling);
    if (bMoveDataBindingNodes) {
      std::set<CXFA_Node*> sNew;
      std::set<CXFA_Node*> sAfter;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorNew(pNewInstance);
      for (CXFA_Node* pNode = sIteratorNew.GetCurrent(); pNode;
           pNode = sIteratorNew.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode)
          continue;

        sNew.insert(pDataNode);
      }
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorAfter(pNextSibling);
      for (CXFA_Node* pNode = sIteratorAfter.GetCurrent(); pNode;
           pNode = sIteratorAfter.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode)
          continue;

        sAfter.insert(pDataNode);
      }
      ReorderDataNodes(sNew, sAfter, false);
    }
  } else {
    CXFA_Node* pBeforeInstance = GetItem(iPos);
    GetNodeItem(XFA_NODEITEM_Parent)
        ->InsertChild(pNewInstance, pBeforeInstance);
    if (bMoveDataBindingNodes) {
      std::set<CXFA_Node*> sNew;
      std::set<CXFA_Node*> sBefore;
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorNew(pNewInstance);
      for (CXFA_Node* pNode = sIteratorNew.GetCurrent(); pNode;
           pNode = sIteratorNew.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode)
          continue;

        sNew.insert(pDataNode);
      }
      CXFA_NodeIteratorTemplate<CXFA_Node,
                                CXFA_TraverseStrategy_XFAContainerNode>
          sIteratorBefore(pBeforeInstance);
      for (CXFA_Node* pNode = sIteratorBefore.GetCurrent(); pNode;
           pNode = sIteratorBefore.MoveToNext()) {
        CXFA_Node* pDataNode = pNode->GetBindData();
        if (!pDataNode)
          continue;

        sBefore.insert(pDataNode);
      }
      ReorderDataNodes(sNew, sBefore, true);
    }
  }
}

void CXFA_Node::RemoveItem(CXFA_Node* pRemoveInstance,
                           bool bRemoveDataBinding) {
  GetNodeItem(XFA_NODEITEM_Parent)->RemoveChild(pRemoveInstance, true);
  if (!bRemoveDataBinding)
    return;

  CXFA_NodeIteratorTemplate<CXFA_Node, CXFA_TraverseStrategy_XFAContainerNode>
      sIterator(pRemoveInstance);
  for (CXFA_Node* pFormNode = sIterator.GetCurrent(); pFormNode;
       pFormNode = sIterator.MoveToNext()) {
    CXFA_Node* pDataNode = pFormNode->GetBindData();
    if (!pDataNode)
      continue;

    if (pDataNode->RemoveBindItem(pFormNode) == 0) {
      if (CXFA_Node* pDataParent =
              pDataNode->GetNodeItem(XFA_NODEITEM_Parent)) {
        pDataParent->RemoveChild(pDataNode, true);
      }
    }
    pFormNode->JSNode()->SetBindingNode(nullptr);
  }
}

CXFA_Node* CXFA_Node::CreateInstance(bool bDataMerge) {
  CXFA_Document* pDocument = GetDocument();
  CXFA_Node* pTemplateNode = GetTemplateNode();
  CXFA_Node* pFormParent = GetNodeItem(XFA_NODEITEM_Parent);
  CXFA_Node* pDataScope = nullptr;
  for (CXFA_Node* pRootBoundNode = pFormParent;
       pRootBoundNode && pRootBoundNode->IsContainerNode();
       pRootBoundNode = pRootBoundNode->GetNodeItem(XFA_NODEITEM_Parent)) {
    pDataScope = pRootBoundNode->GetBindData();
    if (pDataScope)
      break;
  }
  if (!pDataScope) {
    pDataScope = ToNode(pDocument->GetXFAObject(XFA_HASHCODE_Record));
    ASSERT(pDataScope);
  }
  CXFA_Node* pInstance = pDocument->DataMerge_CopyContainer(
      pTemplateNode, pFormParent, pDataScope, true, bDataMerge, true);
  if (pInstance) {
    pDocument->DataMerge_UpdateBindingRelations(pInstance);
    pFormParent->RemoveChild(pInstance, true);
  }
  return pInstance;
}

pdfium::Optional<bool> CXFA_Node::GetDefaultBoolean(XFA_Attribute attr) const {
  pdfium::Optional<void*> value =
      GetDefaultValue(attr, XFA_AttributeType::Boolean);
  if (!value)
    return {};
  return {!!*value};
}

pdfium::Optional<int32_t> CXFA_Node::GetDefaultInteger(
    XFA_Attribute attr) const {
  pdfium::Optional<void*> value =
      GetDefaultValue(attr, XFA_AttributeType::Integer);
  if (!value)
    return {};
  return {static_cast<int32_t>(reinterpret_cast<uintptr_t>(*value))};
}

pdfium::Optional<CXFA_Measurement> CXFA_Node::GetDefaultMeasurement(
    XFA_Attribute attr) const {
  pdfium::Optional<void*> value =
      GetDefaultValue(attr, XFA_AttributeType::Measure);
  if (!value)
    return {};

  WideString str = WideString(static_cast<const wchar_t*>(*value));
  return {CXFA_Measurement(str.AsStringView())};
}

pdfium::Optional<WideString> CXFA_Node::GetDefaultCData(
    XFA_Attribute attr) const {
  pdfium::Optional<void*> value =
      GetDefaultValue(attr, XFA_AttributeType::CData);
  if (!value)
    return {};

  return {WideString(static_cast<const wchar_t*>(*value))};
}

pdfium::Optional<XFA_AttributeEnum> CXFA_Node::GetDefaultEnum(
    XFA_Attribute attr) const {
  pdfium::Optional<void*> value =
      GetDefaultValue(attr, XFA_AttributeType::Enum);
  if (!value)
    return {};
  return {static_cast<XFA_AttributeEnum>(reinterpret_cast<uintptr_t>(*value))};
}

pdfium::Optional<void*> CXFA_Node::GetDefaultValue(
    XFA_Attribute attr,
    XFA_AttributeType eType) const {
  const AttributeData* data = GetAttributeData(attr);
  if (!data)
    return {};
  if (data->type == eType)
    return {data->default_value};
  return {};
}

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

#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/cfx_decimal.h"
#include "core/fxcrt/cfx_memorystream.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmltext.h"
#include "fxjs/cfxjse_engine.h"
#include "fxjs/cfxjse_value.h"
#include "fxjs/xfa/cjx_node.h"
#include "third_party/base/logging.h"
#include "third_party/base/ptr_util.h"
#include "third_party/base/stl_util.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/cxfa_ffdocview.h"
#include "xfa/fxfa/cxfa_ffnotify.h"
#include "xfa/fxfa/cxfa_ffwidget.h"
#include "xfa/fxfa/parser/cxfa_arraynodelist.h"
#include "xfa/fxfa/parser/cxfa_attachnodelist.h"
#include "xfa/fxfa/parser/cxfa_bind.h"
#include "xfa/fxfa/parser/cxfa_border.h"
#include "xfa/fxfa/parser/cxfa_calculate.h"
#include "xfa/fxfa/parser/cxfa_caption.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_event.h"
#include "xfa/fxfa/parser/cxfa_font.h"
#include "xfa/fxfa/parser/cxfa_keep.h"
#include "xfa/fxfa/parser/cxfa_layoutprocessor.h"
#include "xfa/fxfa/parser/cxfa_localevalue.h"
#include "xfa/fxfa/parser/cxfa_margin.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"
#include "xfa/fxfa/parser/cxfa_nodeiteratortemplate.h"
#include "xfa/fxfa/parser/cxfa_occur.h"
#include "xfa/fxfa/parser/cxfa_para.h"
#include "xfa/fxfa/parser/cxfa_simple_parser.h"
#include "xfa/fxfa/parser/cxfa_subform.h"
#include "xfa/fxfa/parser/cxfa_traversestrategy_xfacontainernode.h"
#include "xfa/fxfa/parser/cxfa_validate.h"
#include "xfa/fxfa/parser/cxfa_value.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

constexpr uint8_t kMaxExecuteRecursion = 2;

std::vector<CXFA_Node*> NodesSortedByDocumentIdx(
    const std::set<CXFA_Node*>& rgNodeSet) {
  if (rgNodeSet.empty())
    return std::vector<CXFA_Node*>();

  std::vector<CXFA_Node*> rgNodeArray;
  CXFA_Node* pCommonParent = (*rgNodeSet.begin())->GetParent();
  for (CXFA_Node* pNode = pCommonParent->GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
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
  CXFA_Node* pParentNode = pNode->GetParent();
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
          pParentNode = pBeforeNode->GetParent();
        } else {
          CXFA_Node* pLastNode = rgNodeArray2.back();
          pParentNode = pLastNode->GetParent();
          pBeforeNode = pLastNode->GetNextSibling();
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
Optional<XFA_AttributeEnum> CXFA_Node::NameToAttributeEnum(
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
                     const WideStringView& elementName,
                     std::unique_ptr<CJX_Object> js_node)
    : CXFA_Object(pDoc, oType, eType, elementName, std::move(js_node)),
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

CXFA_Node::CXFA_Node(CXFA_Document* pDoc,
                     XFA_PacketType ePacket,
                     uint32_t validPackets,
                     XFA_ObjectType oType,
                     XFA_Element eType,
                     const PropertyData* properties,
                     const AttributeData* attributes,
                     const WideStringView& elementName)
    : CXFA_Node(pDoc,
                ePacket,
                validPackets,
                oType,
                eType,
                properties,
                attributes,
                elementName,
                pdfium::MakeUnique<CJX_Node>(this)) {}

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

  JSObject()->MergeAllData(pClone);
  pClone->UpdateNameHash();
  if (IsNeedSavingXMLNode()) {
    std::unique_ptr<CFX_XMLNode> pCloneXML;
    if (IsAttributeInXML()) {
      WideString wsName = JSObject()
                              ->TryAttribute(XFA_Attribute::Name, false)
                              .value_or(WideString());
      auto pCloneXMLElement = pdfium::MakeUnique<CFX_XMLElement>(wsName);
      WideString wsValue = JSObject()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty())
        pCloneXMLElement->SetTextData(WideString(wsValue));

      pCloneXML.reset(pCloneXMLElement.release());
      pClone->JSObject()->SetEnum(XFA_Attribute::Contains,
                                  XFA_AttributeEnum::Unknown, false);
    } else {
      pCloneXML = m_pXMLNode->Clone();
    }
    pClone->SetXMLMappingNode(pCloneXML.release());
    pClone->SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  if (bRecursive) {
    for (CXFA_Node* pChild = GetFirstChild(); pChild;
         pChild = pChild->GetNextSibling()) {
      pClone->InsertChild(pChild->Clone(bRecursive), nullptr);
    }
  }
  pClone->SetFlag(XFA_NodeFlag_Initialized, true);
  pClone->SetBindingNode(nullptr);
  return pClone;
}

CXFA_Node* CXFA_Node::GetPrevSibling() const {
  if (!m_pParent || m_pParent->m_pChild == this)
    return nullptr;

  for (CXFA_Node* pNode = m_pParent->m_pChild; pNode; pNode = pNode->m_pNext) {
    if (pNode->m_pNext == this)
      return pNode;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextContainerSibling() const {
  CXFA_Node* pNode = m_pNext;
  while (pNode && pNode->GetObjectType() != XFA_ObjectType::ContainerNode)
    pNode = pNode->m_pNext;
  return pNode;
}

CXFA_Node* CXFA_Node::GetPrevContainerSibling() const {
  if (!m_pParent || m_pParent->m_pChild == this)
    return nullptr;

  CXFA_Node* container = nullptr;
  for (CXFA_Node* pNode = m_pParent->m_pChild; pNode; pNode = pNode->m_pNext) {
    if (pNode->GetObjectType() == XFA_ObjectType::ContainerNode)
      container = pNode;
    if (pNode->m_pNext == this)
      return container;
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetFirstContainerChild() const {
  CXFA_Node* pNode = m_pChild;
  while (pNode && pNode->GetObjectType() != XFA_ObjectType::ContainerNode)
    pNode = pNode->m_pNext;
  return pNode;
}

CXFA_Node* CXFA_Node::GetContainerParent() const {
  CXFA_Node* pNode = m_pParent;
  while (pNode && pNode->GetObjectType() != XFA_ObjectType::ContainerNode)
    pNode = pNode->m_pParent;
  return pNode;
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

Optional<XFA_Element> CXFA_Node::GetFirstPropertyWithFlag(uint8_t flag) {
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

  Optional<XFA_Element> property =
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
    for (CXFA_Node* pChild = GetFirstChild(); pChild;
         pChild = pChild->GetNextSibling()) {
      pClone->InsertChild(pChild->CloneTemplateToForm(bRecursive), nullptr);
    }
  }
  pClone->SetFlag(XFA_NodeFlag_Initialized, true);
  return pClone;
}

CXFA_Node* CXFA_Node::GetTemplateNodeIfExists() const {
  return m_pAuxNode;
}

void CXFA_Node::SetTemplateNode(CXFA_Node* pTemplateNode) {
  m_pAuxNode = pTemplateNode;
}

CXFA_Node* CXFA_Node::GetBindData() {
  ASSERT(GetPacketType() == XFA_PacketType::Form);
  return GetBindingNode();
}

std::vector<UnownedPtr<CXFA_Node>>* CXFA_Node::GetBindItems() {
  return GetBindingNodes();
}

int32_t CXFA_Node::AddBindItem(CXFA_Node* pFormNode) {
  ASSERT(pFormNode);

  if (BindsFormItems()) {
    bool found = false;
    for (auto& v : binding_nodes_) {
      if (v.Get() == pFormNode) {
        found = true;
        break;
      }
    }
    if (!found)
      binding_nodes_.emplace_back(pFormNode);
    return pdfium::CollectionSize<int32_t>(binding_nodes_);
  }

  CXFA_Node* pOldFormItem = GetBindingNode();
  if (!pOldFormItem) {
    SetBindingNode(pFormNode);
    return 1;
  }
  if (pOldFormItem == pFormNode)
    return 1;

  std::vector<UnownedPtr<CXFA_Node>> items;
  items.emplace_back(pOldFormItem);
  items.emplace_back(pFormNode);
  SetBindingNodes(std::move(items));

  m_uNodeFlags |= XFA_NodeFlag_BindFormItems;
  return 2;
}

int32_t CXFA_Node::RemoveBindItem(CXFA_Node* pFormNode) {
  if (BindsFormItems()) {
    auto it = std::find_if(binding_nodes_.begin(), binding_nodes_.end(),
                           [&pFormNode](const UnownedPtr<CXFA_Node>& node) {
                             return node.Get() == pFormNode;
                           });
    if (it != binding_nodes_.end())
      binding_nodes_.erase(it);

    if (binding_nodes_.size() == 1) {
      m_uNodeFlags &= ~XFA_NodeFlag_BindFormItems;
      return 1;
    }
    return pdfium::CollectionSize<int32_t>(binding_nodes_);
  }

  CXFA_Node* pOldFormItem = GetBindingNode();
  if (pOldFormItem != pFormNode)
    return pOldFormItem ? 1 : 0;

  SetBindingNode(nullptr);
  return 0;
}

bool CXFA_Node::HasBindItem() {
  return GetPacketType() == XFA_PacketType::Datasets && GetBindingNode();
}

CXFA_WidgetAcc* CXFA_Node::GetContainerWidgetAcc() {
  if (GetPacketType() != XFA_PacketType::Form)
    return nullptr;
  XFA_Element eType = GetElementType();
  if (eType == XFA_Element::ExclGroup)
    return nullptr;
  CXFA_Node* pParentNode = GetParent();
  if (pParentNode && pParentNode->GetElementType() == XFA_Element::ExclGroup)
    return nullptr;

  if (eType == XFA_Element::Field) {
    CXFA_WidgetAcc* pFieldWidgetAcc = GetWidgetAcc();
    if (pFieldWidgetAcc && pFieldWidgetAcc->IsChoiceListMultiSelect())
      return nullptr;

    WideString wsPicture;
    if (pFieldWidgetAcc) {
      wsPicture = pFieldWidgetAcc->GetPictureContent(XFA_VALUEPICTURE_DataBind);
    }
    if (!wsPicture.IsEmpty())
      return pFieldWidgetAcc;

    CXFA_Node* pDataNode = GetBindData();
    if (!pDataNode)
      return nullptr;
    pFieldWidgetAcc = nullptr;
    for (const auto& pFormNode : *(pDataNode->GetBindItems())) {
      if (!pFormNode || pFormNode->HasRemovedChildren())
        continue;
      pFieldWidgetAcc = pFormNode->GetWidgetAcc();
      if (pFieldWidgetAcc) {
        wsPicture =
            pFieldWidgetAcc->GetPictureContent(XFA_VALUEPICTURE_DataBind);
      }
      if (!wsPicture.IsEmpty())
        break;
      pFieldWidgetAcc = nullptr;
    }
    return pFieldWidgetAcc;
  }

  CXFA_Node* pGrandNode = pParentNode ? pParentNode->GetParent() : nullptr;
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
      pValueNode ? pValueNode->GetParent() : nullptr;
  return pParentOfValueNode ? pParentOfValueNode->GetContainerWidgetAcc()
                            : nullptr;
}

IFX_Locale* CXFA_Node::GetLocale() {
  Optional<WideString> localeName = GetLocaleName();
  if (!localeName)
    return nullptr;
  if (localeName.value() == L"ambient")
    return GetDocument()->GetLocalMgr()->GetDefLocale();
  return GetDocument()->GetLocalMgr()->GetLocaleByName(localeName.value());
}

Optional<WideString> CXFA_Node::GetLocaleName() {
  CXFA_Node* pForm = GetDocument()->GetXFAObject(XFA_HASHCODE_Form)->AsNode();
  CXFA_Subform* pTopSubform =
      pForm->GetFirstChildByClass<CXFA_Subform>(XFA_Element::Subform);
  ASSERT(pTopSubform);

  CXFA_Node* pLocaleNode = this;
  do {
    Optional<WideString> localeName =
        pLocaleNode->JSObject()->TryCData(XFA_Attribute::Locale, false);
    if (localeName)
      return localeName;

    pLocaleNode = pLocaleNode->GetParent();
  } while (pLocaleNode && pLocaleNode != pTopSubform);

  CXFA_Node* pConfig = ToNode(GetDocument()->GetXFAObject(XFA_HASHCODE_Config));
  Optional<WideString> localeName = {
      WideString(GetDocument()->GetLocalMgr()->GetConfigLocaleName(pConfig))};
  if (localeName && !localeName->IsEmpty())
    return localeName;

  if (pTopSubform) {
    localeName =
        pTopSubform->JSObject()->TryCData(XFA_Attribute::Locale, false);
    if (localeName)
      return localeName;
  }

  IFX_Locale* pLocale = GetDocument()->GetLocalMgr()->GetDefLocale();
  if (!pLocale)
    return {};

  return {pLocale->GetName()};
}

XFA_AttributeEnum CXFA_Node::GetIntact() {
  CXFA_Keep* pKeep = GetFirstChildByClass<CXFA_Keep>(XFA_Element::Keep);
  XFA_AttributeEnum eLayoutType = JSObject()
                                      ->TryEnum(XFA_Attribute::Layout, true)
                                      .value_or(XFA_AttributeEnum::Position);
  if (pKeep) {
    Optional<XFA_AttributeEnum> intact =
        pKeep->JSObject()->TryEnum(XFA_Attribute::Intact, false);
    if (intact) {
      if (*intact == XFA_AttributeEnum::None &&
          eLayoutType == XFA_AttributeEnum::Row &&
          m_pDocument->GetCurVersionMode() < XFA_VERSION_208) {
        CXFA_Node* pPreviewRow = GetPrevContainerSibling();
        if (pPreviewRow &&
            pPreviewRow->JSObject()->GetEnum(XFA_Attribute::Layout) ==
                XFA_AttributeEnum::Row) {
          Optional<XFA_AttributeEnum> value =
              pKeep->JSObject()->TryEnum(XFA_Attribute::Previous, false);
          if (value && (*value == XFA_AttributeEnum::ContentArea ||
                        *value == XFA_AttributeEnum::PageArea)) {
            return XFA_AttributeEnum::ContentArea;
          }

          CXFA_Keep* pNode =
              pPreviewRow->GetFirstChildByClass<CXFA_Keep>(XFA_Element::Keep);
          Optional<XFA_AttributeEnum> ret;
          if (pNode)
            ret = pNode->JSObject()->TryEnum(XFA_Attribute::Next, false);
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
        default:
          return XFA_AttributeEnum::None;
      }
    case XFA_Element::Field: {
      CXFA_Node* parent = GetParent();
      if (!parent || parent->GetElementType() == XFA_Element::PageArea)
        return XFA_AttributeEnum::ContentArea;
      if (parent->GetIntact() != XFA_AttributeEnum::None)
        return XFA_AttributeEnum::ContentArea;

      XFA_AttributeEnum eParLayout = parent->JSObject()
                                         ->TryEnum(XFA_Attribute::Layout, true)
                                         .value_or(XFA_AttributeEnum::Position);
      if (eParLayout == XFA_AttributeEnum::Position ||
          eParLayout == XFA_AttributeEnum::Row ||
          eParLayout == XFA_AttributeEnum::Table) {
        return XFA_AttributeEnum::None;
      }

      XFA_VERSION version = m_pDocument->GetCurVersionMode();
      if (eParLayout == XFA_AttributeEnum::Tb && version < XFA_VERSION_208) {
        Optional<CXFA_Measurement> measureH =
            JSObject()->TryMeasure(XFA_Attribute::H, false);
        if (measureH)
          return XFA_AttributeEnum::ContentArea;
      }
      return XFA_AttributeEnum::None;
    }
    case XFA_Element::Draw:
      return XFA_AttributeEnum::ContentArea;
    default:
      return XFA_AttributeEnum::None;
  }
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

size_t CXFA_Node::CountChildren(XFA_Element eType, bool bOnlyChild) {
  size_t count = 0;
  for (CXFA_Node* pNode = m_pChild; pNode; pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != eType && eType != XFA_Element::Unknown)
      continue;
    if (bOnlyChild && HasProperty(pNode->GetElementType()))
      continue;
    ++count;
  }
  return count;
}

CXFA_Node* CXFA_Node::GetChildInternal(size_t index,
                                       XFA_Element eType,
                                       bool bOnlyChild) {
  size_t count = 0;
  for (CXFA_Node* pNode = m_pChild; pNode; pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() != eType && eType != XFA_Element::Unknown)
      continue;
    if (bOnlyChild && HasProperty(pNode->GetElementType()))
      continue;
    if (count == index)
      return pNode;

    ++count;
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
            pNode->JSObject()->GetCData(XFA_Attribute::QualifiedName);
        pXMLElement->RemoveAttribute(wsAttributeName.c_str());
      }

      WideString wsName = pNode->JSObject()
                              ->TryAttribute(XFA_Attribute::Name, false)
                              .value_or(WideString());
      CFX_XMLElement* pNewXMLElement = new CFX_XMLElement(wsName);
      WideString wsValue = JSObject()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty())
        pNewXMLElement->SetTextData(WideString(wsValue));

      pNode->m_pXMLNode = pNewXMLElement;
      pNode->JSObject()->SetEnum(XFA_Attribute::Contains,
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
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetNameHash() == dwNameHash) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetFirstChildByClassInternal(XFA_Element eType) const {
  for (CXFA_Node* pNode = GetFirstChild(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetElementType() == eType) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextSameNameSibling(uint32_t dwNameHash) const {
  for (CXFA_Node* pNode = GetNextSibling(); pNode;
       pNode = pNode->GetNextSibling()) {
    if (pNode->GetNameHash() == dwNameHash) {
      return pNode;
    }
  }
  return nullptr;
}

CXFA_Node* CXFA_Node::GetNextSameNameSiblingInternal(
    const WideStringView& wsNodeName) const {
  return GetNextSameNameSibling(FX_HashCode_GetW(wsNodeName, false));
}

CXFA_Node* CXFA_Node::GetNextSameClassSiblingInternal(XFA_Element eType) const {
  for (CXFA_Node* pNode = GetNextSibling(); pNode;
       pNode = pNode->GetNextSibling()) {
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

CXFA_Node* CXFA_Node::GetInstanceMgrOfSubform() {
  CXFA_Node* pInstanceMgr = nullptr;
  if (m_ePacket == XFA_PacketType::Form) {
    CXFA_Node* pParentNode = GetParent();
    if (!pParentNode || pParentNode->GetElementType() == XFA_Element::Area) {
      return pInstanceMgr;
    }
    for (CXFA_Node* pNode = GetPrevSibling(); pNode;
         pNode = pNode->GetPrevSibling()) {
      XFA_Element eType = pNode->GetElementType();
      if ((eType == XFA_Element::Subform || eType == XFA_Element::SubformSet) &&
          pNode->m_dwNameHash != m_dwNameHash) {
        break;
      }
      if (eType == XFA_Element::InstanceManager) {
        WideString wsName = JSObject()->GetCData(XFA_Attribute::Name);
        WideString wsInstName =
            pNode->JSObject()->GetCData(XFA_Attribute::Name);
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

CXFA_Occur* CXFA_Node::GetOccurIfExists() {
  return GetFirstChildByClass<CXFA_Occur>(XFA_Element::Occur);
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
  // Clear any binding nodes as we don't necessarily destruct in an order that
  // makes sense.
  for (auto& node : binding_nodes_)
    node.Release();

  for (CXFA_Node* pNode = m_pChild; pNode; pNode = pNode->m_pNext)
    pNode->ReleaseBindingNodes();
}

bool CXFA_Node::IsAttributeInXML() {
  return JSObject()->GetEnum(XFA_Attribute::Contains) ==
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
  WideString wsName = JSObject()->GetCData(XFA_Attribute::Name);
  m_dwNameHash = FX_HashCode_GetW(wsName.AsStringView(), false);
}

CFX_XMLNode* CXFA_Node::CreateXMLMappingNode() {
  if (!m_pXMLNode) {
    WideString wsTag(JSObject()->GetCData(XFA_Attribute::Name));
    m_pXMLNode = new CFX_XMLElement(wsTag);
    SetFlag(XFA_NodeFlag_OwnXMLNode, false);
  }
  return m_pXMLNode;
}

bool CXFA_Node::IsNeedSavingXMLNode() {
  return m_pXMLNode && (GetPacketType() == XFA_PacketType::Datasets ||
                        GetElementType() == XFA_Element::Xfa);
}

CXFA_Node* CXFA_Node::GetItemIfExists(int32_t iIndex) {
  int32_t iCount = 0;
  uint32_t dwNameHash = 0;
  for (CXFA_Node* pNode = GetNextSibling(); pNode;
       pNode = pNode->GetNextSibling()) {
    XFA_Element eCurType = pNode->GetElementType();
    if (eCurType == XFA_Element::InstanceManager)
      break;
    if ((eCurType != XFA_Element::Subform) &&
        (eCurType != XFA_Element::SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      WideString wsName = pNode->JSObject()->GetCData(XFA_Attribute::Name);
      WideString wsInstName = JSObject()->GetCData(XFA_Attribute::Name);
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
  for (CXFA_Node* pNode = GetNextSibling(); pNode;
       pNode = pNode->GetNextSibling()) {
    XFA_Element eCurType = pNode->GetElementType();
    if (eCurType == XFA_Element::InstanceManager)
      break;
    if ((eCurType != XFA_Element::Subform) &&
        (eCurType != XFA_Element::SubformSet)) {
      continue;
    }
    if (iCount == 0) {
      WideString wsName = pNode->JSObject()->GetCData(XFA_Attribute::Name);
      WideString wsInstName = JSObject()->GetCData(XFA_Attribute::Name);
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
    CXFA_Node* item = GetItemIfExists(iCount - 1);
    if (!item)
      return;

    CXFA_Node* pNextSibling =
        iCount > 0 ? item->GetNextSibling() : GetNextSibling();
    GetParent()->InsertChild(pNewInstance, pNextSibling);
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
    CXFA_Node* pBeforeInstance = GetItemIfExists(iPos);
    if (!pBeforeInstance) {
      // TODO(dsinclair): What should happen here?
      return;
    }

    GetParent()->InsertChild(pNewInstance, pBeforeInstance);
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
  GetParent()->RemoveChild(pRemoveInstance, true);
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
      if (CXFA_Node* pDataParent = pDataNode->GetParent()) {
        pDataParent->RemoveChild(pDataNode, true);
      }
    }
    pFormNode->SetBindingNode(nullptr);
  }
}

CXFA_Node* CXFA_Node::CreateInstanceIfPossible(bool bDataMerge) {
  CXFA_Document* pDocument = GetDocument();
  CXFA_Node* pTemplateNode = GetTemplateNodeIfExists();
  if (!pTemplateNode)
    return nullptr;

  CXFA_Node* pFormParent = GetParent();
  CXFA_Node* pDataScope = nullptr;
  for (CXFA_Node* pRootBoundNode = pFormParent;
       pRootBoundNode && pRootBoundNode->IsContainerNode();
       pRootBoundNode = pRootBoundNode->GetParent()) {
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

Optional<bool> CXFA_Node::GetDefaultBoolean(XFA_Attribute attr) const {
  Optional<void*> value = GetDefaultValue(attr, XFA_AttributeType::Boolean);
  if (!value)
    return {};
  return {!!*value};
}

Optional<int32_t> CXFA_Node::GetDefaultInteger(XFA_Attribute attr) const {
  Optional<void*> value = GetDefaultValue(attr, XFA_AttributeType::Integer);
  if (!value)
    return {};
  return {static_cast<int32_t>(reinterpret_cast<uintptr_t>(*value))};
}

Optional<CXFA_Measurement> CXFA_Node::GetDefaultMeasurement(
    XFA_Attribute attr) const {
  Optional<void*> value = GetDefaultValue(attr, XFA_AttributeType::Measure);
  if (!value)
    return {};

  WideString str = WideString(static_cast<const wchar_t*>(*value));
  return {CXFA_Measurement(str.AsStringView())};
}

Optional<WideString> CXFA_Node::GetDefaultCData(XFA_Attribute attr) const {
  Optional<void*> value = GetDefaultValue(attr, XFA_AttributeType::CData);
  if (!value)
    return {};

  return {WideString(static_cast<const wchar_t*>(*value))};
}

Optional<XFA_AttributeEnum> CXFA_Node::GetDefaultEnum(
    XFA_Attribute attr) const {
  Optional<void*> value = GetDefaultValue(attr, XFA_AttributeType::Enum);
  if (!value)
    return {};
  return {static_cast<XFA_AttributeEnum>(reinterpret_cast<uintptr_t>(*value))};
}

Optional<void*> CXFA_Node::GetDefaultValue(XFA_Attribute attr,
                                           XFA_AttributeType eType) const {
  const AttributeData* data = GetAttributeData(attr);
  if (!data)
    return {};
  if (data->type == eType)
    return {data->default_value};
  return {};
}

void CXFA_Node::SendAttributeChangeMessage(XFA_Attribute eAttribute,
                                           bool bScriptModify) {
  CXFA_LayoutProcessor* pLayoutPro = GetDocument()->GetLayoutProcessor();
  if (!pLayoutPro)
    return;

  CXFA_FFNotify* pNotify = GetDocument()->GetNotify();
  if (!pNotify)
    return;

  if (GetPacketType() != XFA_PacketType::Form) {
    pNotify->OnValueChanged(this, eAttribute, this, this);
    return;
  }

  bool bNeedFindContainer = false;
  switch (GetElementType()) {
    case XFA_Element::Caption:
      bNeedFindContainer = true;
      pNotify->OnValueChanged(this, eAttribute, this, GetParent());
      break;
    case XFA_Element::Font:
    case XFA_Element::Para: {
      bNeedFindContainer = true;
      CXFA_Node* pParentNode = GetParent();
      if (pParentNode->GetElementType() == XFA_Element::Caption) {
        pNotify->OnValueChanged(this, eAttribute, pParentNode,
                                pParentNode->GetParent());
      } else {
        pNotify->OnValueChanged(this, eAttribute, this, pParentNode);
      }
      break;
    }
    case XFA_Element::Margin: {
      bNeedFindContainer = true;
      CXFA_Node* pParentNode = GetParent();
      XFA_Element eParentType = pParentNode->GetElementType();
      if (pParentNode->IsContainerNode()) {
        pNotify->OnValueChanged(this, eAttribute, this, pParentNode);
      } else if (eParentType == XFA_Element::Caption) {
        pNotify->OnValueChanged(this, eAttribute, pParentNode,
                                pParentNode->GetParent());
      } else {
        CXFA_Node* pNode = pParentNode->GetParent();
        if (pNode && pNode->GetElementType() == XFA_Element::Ui) {
          pNotify->OnValueChanged(this, eAttribute, pNode, pNode->GetParent());
        }
      }
      break;
    }
    case XFA_Element::Comb: {
      CXFA_Node* pEditNode = GetParent();
      XFA_Element eUIType = pEditNode->GetElementType();
      if (pEditNode && (eUIType == XFA_Element::DateTimeEdit ||
                        eUIType == XFA_Element::NumericEdit ||
                        eUIType == XFA_Element::TextEdit)) {
        CXFA_Node* pUINode = pEditNode->GetParent();
        if (pUINode) {
          pNotify->OnValueChanged(this, eAttribute, pUINode,
                                  pUINode->GetParent());
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
      CXFA_Node* pUINode = GetParent();
      if (pUINode) {
        pNotify->OnValueChanged(this, eAttribute, pUINode,
                                pUINode->GetParent());
      }
      break;
    }
    case XFA_Element::CheckButton: {
      bNeedFindContainer = true;
      CXFA_Node* pUINode = GetParent();
      if (pUINode) {
        pNotify->OnValueChanged(this, eAttribute, pUINode,
                                pUINode->GetParent());
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
      pLayoutPro->AddChangedContainer(this);
      pNotify->OnValueChanged(this, eAttribute, this, this);
      break;
    case XFA_Element::Sharptext:
    case XFA_Element::Sharpxml:
    case XFA_Element::SharpxHTML: {
      CXFA_Node* pTextNode = GetParent();
      if (!pTextNode)
        return;

      CXFA_Node* pValueNode = pTextNode->GetParent();
      if (!pValueNode)
        return;

      XFA_Element eType = pValueNode->GetElementType();
      if (eType == XFA_Element::Value) {
        bNeedFindContainer = true;
        CXFA_Node* pNode = pValueNode->GetParent();
        if (pNode && pNode->IsContainerNode()) {
          if (bScriptModify)
            pValueNode = pNode;

          pNotify->OnValueChanged(this, eAttribute, pValueNode, pNode);
        } else {
          pNotify->OnValueChanged(this, eAttribute, pNode, pNode->GetParent());
        }
      } else {
        if (eType == XFA_Element::Items) {
          CXFA_Node* pNode = pValueNode->GetParent();
          if (pNode && pNode->IsContainerNode()) {
            pNotify->OnValueChanged(this, eAttribute, pValueNode, pNode);
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

  CXFA_Node* pParent = this;
  while (pParent && !pParent->IsContainerNode())
    pParent = pParent->GetParent();

  if (pParent)
    pLayoutPro->AddChangedContainer(pParent);
}

void CXFA_Node::SyncValue(const WideString& wsValue, bool bNotify) {
  WideString wsFormatValue = wsValue;
  CXFA_WidgetAcc* pContainerWidgetAcc = GetContainerWidgetAcc();
  if (pContainerWidgetAcc)
    wsFormatValue = pContainerWidgetAcc->GetFormatDataValue(wsValue);

  JSObject()->SetContent(wsValue, wsFormatValue, bNotify, false, true);
}

WideString CXFA_Node::GetRawValue() {
  return JSObject()->GetContent(false);
}

int32_t CXFA_Node::GetRotate() {
  Optional<int32_t> degrees =
      JSObject()->TryInteger(XFA_Attribute::Rotate, false);
  return degrees ? XFA_MapRotation(*degrees) / 90 * 90 : 0;
}

CXFA_Border* CXFA_Node::GetBorderIfExists() const {
  return JSObject()->GetProperty<CXFA_Border>(0, XFA_Element::Border);
}

CXFA_Border* CXFA_Node::GetOrCreateBorderIfPossible() {
  return JSObject()->GetOrCreateProperty<CXFA_Border>(0, XFA_Element::Border);
}

CXFA_Caption* CXFA_Node::GetCaptionIfExists() const {
  return JSObject()->GetProperty<CXFA_Caption>(0, XFA_Element::Caption);
}

CXFA_Font* CXFA_Node::GetOrCreateFontIfPossible() {
  return JSObject()->GetOrCreateProperty<CXFA_Font>(0, XFA_Element::Font);
}

CXFA_Font* CXFA_Node::GetFontIfExists() const {
  return JSObject()->GetProperty<CXFA_Font>(0, XFA_Element::Font);
}

float CXFA_Node::GetFontSize() const {
  CXFA_Font* font = GetFontIfExists();
  float fFontSize = font ? font->GetFontSize() : 10.0f;
  return fFontSize < 0.1f ? 10.0f : fFontSize;
}

float CXFA_Node::GetLineHeight() const {
  float fLineHeight = 0;
  CXFA_Para* para = GetParaIfExists();
  if (para)
    fLineHeight = para->GetLineHeight();

  if (fLineHeight < 1)
    fLineHeight = GetFontSize() * 1.2f;
  return fLineHeight;
}

FX_ARGB CXFA_Node::GetTextColor() const {
  CXFA_Font* font = GetFontIfExists();
  return font ? font->GetColor() : 0xFF000000;
}

CXFA_Margin* CXFA_Node::GetMarginIfExists() const {
  return JSObject()->GetProperty<CXFA_Margin>(0, XFA_Element::Margin);
}

CXFA_Para* CXFA_Node::GetParaIfExists() const {
  return JSObject()->GetProperty<CXFA_Para>(0, XFA_Element::Para);
}

bool CXFA_Node::IsOpenAccess() {
  for (auto* pNode = this; pNode; pNode = pNode->GetContainerParent()) {
    XFA_AttributeEnum iAcc = pNode->JSObject()->GetEnum(XFA_Attribute::Access);
    if (iAcc != XFA_AttributeEnum::Open)
      return false;
  }
  return true;
}

CXFA_Value* CXFA_Node::GetDefaultValueIfExists() {
  CXFA_Node* pTemNode = GetTemplateNodeIfExists();
  return pTemNode ? pTemNode->JSObject()->GetProperty<CXFA_Value>(
                        0, XFA_Element::Value)
                  : nullptr;
}

CXFA_Value* CXFA_Node::GetFormValueIfExists() const {
  return JSObject()->GetProperty<CXFA_Value>(0, XFA_Element::Value);
}

CXFA_Calculate* CXFA_Node::GetCalculateIfExists() const {
  return JSObject()->GetProperty<CXFA_Calculate>(0, XFA_Element::Calculate);
}

CXFA_Validate* CXFA_Node::GetValidateIfExists() const {
  return JSObject()->GetProperty<CXFA_Validate>(0, XFA_Element::Validate);
}

CXFA_Validate* CXFA_Node::GetOrCreateValidateIfPossible() {
  return JSObject()->GetOrCreateProperty<CXFA_Validate>(0,
                                                        XFA_Element::Validate);
}

CXFA_Bind* CXFA_Node::GetBindIfExists() const {
  return JSObject()->GetProperty<CXFA_Bind>(0, XFA_Element::Bind);
}

Optional<float> CXFA_Node::TryWidth() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::W);
}

Optional<float> CXFA_Node::TryHeight() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::H);
}

Optional<float> CXFA_Node::TryMinWidth() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::MinW);
}

Optional<float> CXFA_Node::TryMinHeight() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::MinH);
}

Optional<float> CXFA_Node::TryMaxWidth() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::MaxW);
}

Optional<float> CXFA_Node::TryMaxHeight() {
  return JSObject()->TryMeasureAsFloat(XFA_Attribute::MaxH);
}

CXFA_Node* CXFA_Node::GetExclGroupIfExists() {
  CXFA_Node* pExcl = GetParent();
  if (!pExcl || pExcl->GetElementType() != XFA_Element::ExclGroup)
    return nullptr;
  return pExcl;
}

int32_t CXFA_Node::ProcessEvent(CXFA_FFDocView* docView,
                                XFA_AttributeEnum iActivity,
                                CXFA_EventParam* pEventParam) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EVENTERROR_NotExist;

  std::vector<CXFA_Event*> eventArray = GetWidgetAcc()->GetEventByActivity(
      iActivity, pEventParam->m_bIsFormReady);
  bool first = true;
  int32_t iRet = XFA_EVENTERROR_NotExist;
  for (CXFA_Event* event : eventArray) {
    int32_t result = ProcessEvent(docView, event, pEventParam);
    if (first || result == XFA_EVENTERROR_Success)
      iRet = result;
    first = false;
  }
  return iRet;
}

int32_t CXFA_Node::ProcessEvent(CXFA_FFDocView* docView,
                                CXFA_Event* event,
                                CXFA_EventParam* pEventParam) {
  if (!event)
    return XFA_EVENTERROR_NotExist;

  switch (event->GetEventType()) {
    case XFA_Element::Execute:
      break;
    case XFA_Element::Script:
      return ExecuteScript(docView, event->GetScriptIfExists(), pEventParam);
    case XFA_Element::SignData:
      break;
    case XFA_Element::Submit: {
      CXFA_Submit* submit = event->GetSubmitIfExists();
      if (!submit)
        return XFA_EVENTERROR_NotExist;
      return docView->GetDoc()->GetDocEnvironment()->Submit(docView->GetDoc(),
                                                            submit);
    }
    default:
      break;
  }
  return XFA_EVENTERROR_NotExist;
}

int32_t CXFA_Node::ProcessCalculate(CXFA_FFDocView* docView) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EVENTERROR_NotExist;

  CXFA_Calculate* calc = GetCalculateIfExists();
  if (!calc)
    return XFA_EVENTERROR_NotExist;
  if (IsUserInteractive())
    return XFA_EVENTERROR_Disabled;

  CXFA_EventParam EventParam;
  EventParam.m_eType = XFA_EVENT_Calculate;
  int32_t iRet = ExecuteScript(docView, calc->GetScriptIfExists(), &EventParam);
  if (iRet != XFA_EVENTERROR_Success)
    return iRet;

  if (GetRawValue() != EventParam.m_wsResult) {
    GetWidgetAcc()->SetValue(XFA_VALUEPICTURE_Raw, EventParam.m_wsResult);
    GetWidgetAcc()->UpdateUIDisplay(docView, nullptr);
  }
  return XFA_EVENTERROR_Success;
}

void CXFA_Node::ProcessScriptTestValidate(CXFA_FFDocView* docView,
                                          CXFA_Validate* validate,
                                          int32_t iRet,
                                          bool bRetValue,
                                          bool bVersionFlag) {
  if (iRet != XFA_EVENTERROR_Success)
    return;
  if (bRetValue)
    return;

  IXFA_AppProvider* pAppProvider =
      docView->GetDoc()->GetApp()->GetAppProvider();
  if (!pAppProvider)
    return;

  WideString wsTitle = pAppProvider->GetAppTitle();
  WideString wsScriptMsg = validate->GetScriptMessageText();
  if (validate->GetScriptTest() == XFA_AttributeEnum::Warning) {
    if (IsUserInteractive())
      return;
    if (wsScriptMsg.IsEmpty())
      wsScriptMsg = GetValidateMessage(false, bVersionFlag);

    if (bVersionFlag) {
      pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Warning, XFA_MB_OK);
      return;
    }
    if (pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Warning,
                             XFA_MB_YesNo) == XFA_IDYes) {
      SetFlag(XFA_NodeFlag_UserInteractive, false);
    }
    return;
  }

  if (wsScriptMsg.IsEmpty())
    wsScriptMsg = GetValidateMessage(true, bVersionFlag);
  pAppProvider->MsgBox(wsScriptMsg, wsTitle, XFA_MBICON_Error, XFA_MB_OK);
}

int32_t CXFA_Node::ProcessFormatTestValidate(CXFA_FFDocView* docView,
                                             CXFA_Validate* validate,
                                             bool bVersionFlag) {
  WideString wsRawValue = GetRawValue();
  if (!wsRawValue.IsEmpty()) {
    WideString wsPicture = validate->GetPicture();
    if (wsPicture.IsEmpty())
      return XFA_EVENTERROR_NotExist;

    IFX_Locale* pLocale = GetLocale();
    if (!pLocale)
      return XFA_EVENTERROR_NotExist;

    CXFA_LocaleValue lcValue = XFA_GetLocaleValue(this);
    if (!lcValue.ValidateValue(lcValue.GetValue(), wsPicture, pLocale,
                               nullptr)) {
      IXFA_AppProvider* pAppProvider =
          docView->GetDoc()->GetApp()->GetAppProvider();
      if (!pAppProvider)
        return XFA_EVENTERROR_NotExist;

      WideString wsFormatMsg = validate->GetFormatMessageText();
      WideString wsTitle = pAppProvider->GetAppTitle();
      if (validate->GetFormatTest() == XFA_AttributeEnum::Error) {
        if (wsFormatMsg.IsEmpty())
          wsFormatMsg = GetValidateMessage(true, bVersionFlag);
        pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Error, XFA_MB_OK);
        return XFA_EVENTERROR_Success;
      }
      if (IsUserInteractive())
        return XFA_EVENTERROR_NotExist;
      if (wsFormatMsg.IsEmpty())
        wsFormatMsg = GetValidateMessage(false, bVersionFlag);

      if (bVersionFlag) {
        pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Warning,
                             XFA_MB_OK);
        return XFA_EVENTERROR_Success;
      }
      if (pAppProvider->MsgBox(wsFormatMsg, wsTitle, XFA_MBICON_Warning,
                               XFA_MB_YesNo) == XFA_IDYes) {
        SetFlag(XFA_NodeFlag_UserInteractive, false);
      }
      return XFA_EVENTERROR_Success;
    }
  }
  return XFA_EVENTERROR_NotExist;
}

int32_t CXFA_Node::ProcessNullTestValidate(CXFA_FFDocView* docView,
                                           CXFA_Validate* validate,
                                           int32_t iFlags,
                                           bool bVersionFlag) {
  if (!GetWidgetAcc()->GetValue(XFA_VALUEPICTURE_Raw).IsEmpty())
    return XFA_EVENTERROR_Success;
  if (GetWidgetAcc()->IsNull() && GetWidgetAcc()->IsPreNull())
    return XFA_EVENTERROR_Success;

  XFA_AttributeEnum eNullTest = validate->GetNullTest();
  WideString wsNullMsg = validate->GetNullMessageText();
  if (iFlags & 0x01) {
    int32_t iRet = XFA_EVENTERROR_Success;
    if (eNullTest != XFA_AttributeEnum::Disabled)
      iRet = XFA_EVENTERROR_Error;

    if (!wsNullMsg.IsEmpty()) {
      if (eNullTest != XFA_AttributeEnum::Disabled) {
        docView->m_arrNullTestMsg.push_back(wsNullMsg);
        return XFA_EVENTERROR_Error;
      }
      return XFA_EVENTERROR_Success;
    }
    return iRet;
  }
  if (wsNullMsg.IsEmpty() && bVersionFlag &&
      eNullTest != XFA_AttributeEnum::Disabled) {
    return XFA_EVENTERROR_Error;
  }
  IXFA_AppProvider* pAppProvider =
      docView->GetDoc()->GetApp()->GetAppProvider();
  if (!pAppProvider)
    return XFA_EVENTERROR_NotExist;

  WideString wsCaptionName;
  WideString wsTitle = pAppProvider->GetAppTitle();
  switch (eNullTest) {
    case XFA_AttributeEnum::Error: {
      if (wsNullMsg.IsEmpty()) {
        wsCaptionName = GetValidateCaptionName(bVersionFlag);
        wsNullMsg =
            WideString::Format(L"%ls cannot be blank.", wsCaptionName.c_str());
      }
      pAppProvider->MsgBox(wsNullMsg, wsTitle, XFA_MBICON_Status, XFA_MB_OK);
      return XFA_EVENTERROR_Error;
    }
    case XFA_AttributeEnum::Warning: {
      if (IsUserInteractive())
        return true;

      if (wsNullMsg.IsEmpty()) {
        wsCaptionName = GetValidateCaptionName(bVersionFlag);
        wsNullMsg = WideString::Format(
            L"%ls cannot be blank. To ignore validations for %ls, click "
            L"Ignore.",
            wsCaptionName.c_str(), wsCaptionName.c_str());
      }
      if (pAppProvider->MsgBox(wsNullMsg, wsTitle, XFA_MBICON_Warning,
                               XFA_MB_YesNo) == XFA_IDYes) {
        SetFlag(XFA_NodeFlag_UserInteractive, false);
      }
      return XFA_EVENTERROR_Error;
    }
    case XFA_AttributeEnum::Disabled:
    default:
      break;
  }
  return XFA_EVENTERROR_Success;
}

int32_t CXFA_Node::ProcessValidate(CXFA_FFDocView* docView, int32_t iFlags) {
  if (GetElementType() == XFA_Element::Draw)
    return XFA_EVENTERROR_NotExist;

  CXFA_Validate* validate = GetValidateIfExists();
  if (!validate)
    return XFA_EVENTERROR_NotExist;

  bool bInitDoc = validate->NeedsInitApp();
  bool bStatus = docView->GetLayoutStatus() < XFA_DOCVIEW_LAYOUTSTATUS_End;
  int32_t iFormat = 0;
  int32_t iRet = XFA_EVENTERROR_NotExist;
  CXFA_Script* script = validate->GetScriptIfExists();
  bool bRet = false;
  bool hasBoolResult = (bInitDoc || bStatus) && GetRawValue().IsEmpty();
  if (script) {
    CXFA_EventParam eParam;
    eParam.m_eType = XFA_EVENT_Validate;
    eParam.m_pTarget = GetWidgetAcc();
    std::tie(iRet, bRet) = ExecuteBoolScript(docView, script, &eParam);
  }

  XFA_VERSION version = docView->GetDoc()->GetXFADoc()->GetCurVersionMode();
  bool bVersionFlag = false;
  if (version < XFA_VERSION_208)
    bVersionFlag = true;

  if (bInitDoc) {
    validate->ClearFlag(XFA_NodeFlag_NeedsInitApp);
  } else {
    iFormat = ProcessFormatTestValidate(docView, validate, bVersionFlag);
    if (!bVersionFlag) {
      bVersionFlag =
          docView->GetDoc()->GetXFADoc()->HasFlag(XFA_DOCFLAG_Scripting);
    }

    iRet |= ProcessNullTestValidate(docView, validate, iFlags, bVersionFlag);
  }

  if (iFormat != XFA_EVENTERROR_Success && hasBoolResult)
    ProcessScriptTestValidate(docView, validate, iRet, bRet, bVersionFlag);

  return iRet | iFormat;
}

WideString CXFA_Node::GetValidateCaptionName(bool bVersionFlag) {
  WideString wsCaptionName;

  if (!bVersionFlag) {
    CXFA_Caption* caption = GetCaptionIfExists();
    if (caption) {
      CXFA_Value* capValue = caption->GetValueIfExists();
      if (capValue) {
        CXFA_Text* captionText = capValue->GetTextIfExists();
        if (captionText)
          wsCaptionName = captionText->GetContent();
      }
    }
  }
  if (!wsCaptionName.IsEmpty())
    return wsCaptionName;
  return JSObject()->GetCData(XFA_Attribute::Name);
}

WideString CXFA_Node::GetValidateMessage(bool bError, bool bVersionFlag) {
  WideString wsCaptionName = GetValidateCaptionName(bVersionFlag);
  if (bVersionFlag)
    return WideString::Format(L"%ls validation failed", wsCaptionName.c_str());
  if (bError) {
    return WideString::Format(L"The value you entered for %ls is invalid.",
                              wsCaptionName.c_str());
  }
  return WideString::Format(
      L"The value you entered for %ls is invalid. To ignore "
      L"validations for %ls, click Ignore.",
      wsCaptionName.c_str(), wsCaptionName.c_str());
}

int32_t CXFA_Node::ExecuteScript(CXFA_FFDocView* docView,
                                 CXFA_Script* script,
                                 CXFA_EventParam* pEventParam) {
  bool bRet;
  int32_t iRet;
  std::tie(iRet, bRet) = ExecuteBoolScript(docView, script, pEventParam);
  return iRet;
}

std::pair<int32_t, bool> CXFA_Node::ExecuteBoolScript(
    CXFA_FFDocView* docView,
    CXFA_Script* script,
    CXFA_EventParam* pEventParam) {
  if (m_ExecuteRecursionDepth > kMaxExecuteRecursion)
    return {XFA_EVENTERROR_Success, false};

  ASSERT(pEventParam);
  if (!script)
    return {XFA_EVENTERROR_NotExist, false};
  if (script->GetRunAt() == XFA_AttributeEnum::Server)
    return {XFA_EVENTERROR_Disabled, false};

  WideString wsExpression = script->GetExpression();
  if (wsExpression.IsEmpty())
    return {XFA_EVENTERROR_NotExist, false};

  CXFA_Script::Type eScriptType = script->GetContentType();
  if (eScriptType == CXFA_Script::Type::Unknown)
    return {XFA_EVENTERROR_Success, false};

  CXFA_FFDoc* pDoc = docView->GetDoc();
  CFXJSE_Engine* pContext = pDoc->GetXFADoc()->GetScriptContext();
  pContext->SetEventParam(*pEventParam);
  pContext->SetRunAtType(script->GetRunAt());

  std::vector<CXFA_Node*> refNodes;
  if (pEventParam->m_eType == XFA_EVENT_InitCalculate ||
      pEventParam->m_eType == XFA_EVENT_Calculate) {
    pContext->SetNodesOfRunScript(&refNodes);
  }

  auto pTmpRetValue = pdfium::MakeUnique<CFXJSE_Value>(pContext->GetIsolate());
  bool bRet = false;
  {
    AutoRestorer<uint8_t> restorer(&m_ExecuteRecursionDepth);
    ++m_ExecuteRecursionDepth;
    bRet = pContext->RunScript(eScriptType, wsExpression.AsStringView(),
                               pTmpRetValue.get(), this);
  }

  int32_t iRet = XFA_EVENTERROR_Error;
  if (bRet) {
    iRet = XFA_EVENTERROR_Success;
    if (pEventParam->m_eType == XFA_EVENT_Calculate ||
        pEventParam->m_eType == XFA_EVENT_InitCalculate) {
      if (!pTmpRetValue->IsUndefined()) {
        if (!pTmpRetValue->IsNull())
          pEventParam->m_wsResult = pTmpRetValue->ToWideString();

        iRet = XFA_EVENTERROR_Success;
      } else {
        iRet = XFA_EVENTERROR_Error;
      }
      if (pEventParam->m_eType == XFA_EVENT_InitCalculate) {
        if ((iRet == XFA_EVENTERROR_Success) &&
            (GetRawValue() != pEventParam->m_wsResult)) {
          GetWidgetAcc()->SetValue(XFA_VALUEPICTURE_Raw,
                                   pEventParam->m_wsResult);
          docView->AddValidateWidget(GetWidgetAcc());
        }
      }
      for (CXFA_Node* pRefNode : refNodes) {
        if (pRefNode == this)
          continue;

        CXFA_CalcData* pGlobalData = pRefNode->JSObject()->GetCalcData();
        if (!pGlobalData) {
          pRefNode->JSObject()->SetCalcData(
              pdfium::MakeUnique<CXFA_CalcData>());
          pGlobalData = pRefNode->JSObject()->GetCalcData();
        }
        if (!pdfium::ContainsValue(pGlobalData->m_Globals, this))
          pGlobalData->m_Globals.push_back(this);
      }
    }
  }
  pContext->SetNodesOfRunScript(nullptr);

  return {iRet, pTmpRetValue->IsBoolean() ? pTmpRetValue->ToBoolean() : false};
}

WideString CXFA_Node::GetBarcodeType() {
  CXFA_Node* pUIChild = GetWidgetAcc()->GetUIChild();
  return pUIChild
             ? WideString(pUIChild->JSObject()->GetCData(XFA_Attribute::Type))
             : WideString();
}

Optional<BC_CHAR_ENCODING> CXFA_Node::GetBarcodeAttribute_CharEncoding() {
  Optional<WideString> wsCharEncoding =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryCData(
          XFA_Attribute::CharEncoding, true);
  if (!wsCharEncoding)
    return {};
  if (wsCharEncoding->CompareNoCase(L"UTF-16"))
    return {CHAR_ENCODING_UNICODE};
  if (wsCharEncoding->CompareNoCase(L"UTF-8"))
    return {CHAR_ENCODING_UTF8};
  return {};
}

Optional<bool> CXFA_Node::GetBarcodeAttribute_Checksum() {
  Optional<XFA_AttributeEnum> checksum =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryEnum(XFA_Attribute::Checksum,
                                                        true);
  if (!checksum)
    return {};

  switch (*checksum) {
    case XFA_AttributeEnum::None:
      return {false};
    case XFA_AttributeEnum::Auto:
      return {true};
    case XFA_AttributeEnum::Checksum_1mod10:
    case XFA_AttributeEnum::Checksum_1mod10_1mod11:
    case XFA_AttributeEnum::Checksum_2mod10:
    default:
      break;
  }
  return {};
}

Optional<int32_t> CXFA_Node::GetBarcodeAttribute_DataLength() {
  Optional<WideString> wsDataLength =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryCData(
          XFA_Attribute::DataLength, true);
  if (!wsDataLength)
    return {};

  return {FXSYS_wtoi(wsDataLength->c_str())};
}

Optional<char> CXFA_Node::GetBarcodeAttribute_StartChar() {
  Optional<WideString> wsStartEndChar =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryCData(
          XFA_Attribute::StartChar, true);
  if (!wsStartEndChar || wsStartEndChar->IsEmpty())
    return {};

  return {static_cast<char>((*wsStartEndChar)[0])};
}

Optional<char> CXFA_Node::GetBarcodeAttribute_EndChar() {
  Optional<WideString> wsStartEndChar =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryCData(XFA_Attribute::EndChar,
                                                         true);
  if (!wsStartEndChar || wsStartEndChar->IsEmpty())
    return {};

  return {static_cast<char>((*wsStartEndChar)[0])};
}

Optional<int32_t> CXFA_Node::GetBarcodeAttribute_ECLevel() {
  Optional<WideString> wsECLevel =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryCData(
          XFA_Attribute::ErrorCorrectionLevel, true);
  if (!wsECLevel)
    return {};
  return {FXSYS_wtoi(wsECLevel->c_str())};
}

Optional<int32_t> CXFA_Node::GetBarcodeAttribute_ModuleWidth() {
  Optional<CXFA_Measurement> moduleWidthHeight =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryMeasure(
          XFA_Attribute::ModuleWidth, true);
  if (!moduleWidthHeight)
    return {};

  return {static_cast<int32_t>(moduleWidthHeight->ToUnit(XFA_Unit::Pt))};
}

Optional<int32_t> CXFA_Node::GetBarcodeAttribute_ModuleHeight() {
  Optional<CXFA_Measurement> moduleWidthHeight =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryMeasure(
          XFA_Attribute::ModuleHeight, true);
  if (!moduleWidthHeight)
    return {};

  return {static_cast<int32_t>(moduleWidthHeight->ToUnit(XFA_Unit::Pt))};
}

Optional<bool> CXFA_Node::GetBarcodeAttribute_PrintChecksum() {
  return GetWidgetAcc()->GetUIChild()->JSObject()->TryBoolean(
      XFA_Attribute::PrintCheckDigit, true);
}

Optional<BC_TEXT_LOC> CXFA_Node::GetBarcodeAttribute_TextLocation() {
  Optional<XFA_AttributeEnum> textLocation =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryEnum(
          XFA_Attribute::TextLocation, true);
  if (!textLocation)
    return {};

  switch (*textLocation) {
    case XFA_AttributeEnum::None:
      return {BC_TEXT_LOC_NONE};
    case XFA_AttributeEnum::Above:
      return {BC_TEXT_LOC_ABOVE};
    case XFA_AttributeEnum::Below:
      return {BC_TEXT_LOC_BELOW};
    case XFA_AttributeEnum::AboveEmbedded:
      return {BC_TEXT_LOC_ABOVEEMBED};
    case XFA_AttributeEnum::BelowEmbedded:
      return {BC_TEXT_LOC_BELOWEMBED};
    default:
      break;
  }
  return {};
}

Optional<bool> CXFA_Node::GetBarcodeAttribute_Truncate() {
  return GetWidgetAcc()->GetUIChild()->JSObject()->TryBoolean(
      XFA_Attribute::Truncate, true);
}

Optional<int8_t> CXFA_Node::GetBarcodeAttribute_WideNarrowRatio() {
  Optional<WideString> wsWideNarrowRatio =
      GetWidgetAcc()->GetUIChild()->JSObject()->TryCData(
          XFA_Attribute::WideNarrowRatio, true);
  if (!wsWideNarrowRatio)
    return {};

  Optional<size_t> ptPos = wsWideNarrowRatio->Find(':');
  if (!ptPos)
    return {static_cast<int8_t>(FXSYS_wtoi(wsWideNarrowRatio->c_str()))};

  int32_t fB = FXSYS_wtoi(
      wsWideNarrowRatio->Right(wsWideNarrowRatio->GetLength() - (*ptPos + 1))
          .c_str());
  if (!fB)
    return {0};

  int32_t fA = FXSYS_wtoi(wsWideNarrowRatio->Left(*ptPos).c_str());
  float result = static_cast<float>(fA) / static_cast<float>(fB);
  return {static_cast<int8_t>(result)};
}

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

void XFA_DataNodeDeleteBindItem(void* pData) {
  delete static_cast<std::vector<CXFA_Node*>*>(pData);
}

XFA_MAPDATABLOCKCALLBACKINFO deleteBindItemCallBack = {
    XFA_DataNodeDeleteBindItem, nullptr};

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

const XFA_ATTRIBUTEENUMINFO* GetAttributeEnumByID(XFA_ATTRIBUTEENUM eName) {
  return g_XFAEnumData + eName;
}

// static
std::unique_ptr<CXFA_Node> CXFA_Node::Create(CXFA_Document* doc,
                                             XFA_XDPPACKET packet,
                                             const XFA_ELEMENTINFO* pElement) {
  return std::unique_ptr<CXFA_Node>(new CXFA_Node(
      doc, packet, pElement->eObjectType, pElement->eName, pElement->pName));
}

CXFA_Node::CXFA_Node(CXFA_Document* pDoc,
                     uint16_t ePacket,
                     XFA_ObjectType oType,
                     XFA_Element eType,
                     const WideStringView& elementName)
    : CXFA_Object(pDoc,
                  oType,
                  eType,
                  elementName,
                  pdfium::MakeUnique<CJX_Node>(this)),
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
      WideString wsName;
      JSNode()->GetAttribute(XFA_Attribute::Name, wsName, false);
      auto pCloneXMLElement = pdfium::MakeUnique<CFX_XMLElement>(wsName);
      WideString wsValue = JSNode()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty())
        pCloneXMLElement->SetTextData(WideString(wsValue));

      pCloneXML.reset(pCloneXMLElement.release());
      pClone->JSNode()->SetEnum(XFA_Attribute::Contains,
                                XFA_ATTRIBUTEENUM_Unknown, false);
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
  pClone->JSNode()->SetObject(XFA_Attribute::BindingNode, nullptr, nullptr);
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
  std::vector<CXFA_Node*> nodes;
  if (eTypeFilter != XFA_Element::Unknown) {
    for (CXFA_Node* pChild = m_pChild; pChild; pChild = pChild->m_pNext) {
      if (pChild->GetElementType() == eTypeFilter)
        nodes.push_back(pChild);
    }
  } else if (dwTypeFilter ==
             (XFA_NODEFILTER_Children | XFA_NODEFILTER_Properties)) {
    for (CXFA_Node* pChild = m_pChild; pChild; pChild = pChild->m_pNext)
      nodes.push_back(pChild);
  } else if (dwTypeFilter != 0) {
    bool bFilterChildren = !!(dwTypeFilter & XFA_NODEFILTER_Children);
    bool bFilterProperties = !!(dwTypeFilter & XFA_NODEFILTER_Properties);
    bool bFilterOneOfProperties =
        !!(dwTypeFilter & XFA_NODEFILTER_OneOfProperty);
    CXFA_Node* pChild = m_pChild;
    while (pChild) {
      const XFA_PROPERTY* pProperty = XFA_GetPropertyOfElement(
          GetElementType(), pChild->GetElementType(), XFA_XDPPACKET_UNKNOWN);
      if (pProperty) {
        if (bFilterProperties) {
          nodes.push_back(pChild);
        } else if (bFilterOneOfProperties &&
                   (pProperty->uFlags & XFA_PROPERTYFLAG_OneOf)) {
          nodes.push_back(pChild);
        } else if (bFilterChildren &&
                   (pChild->GetElementType() == XFA_Element::Variables ||
                    pChild->GetElementType() == XFA_Element::PageSet)) {
          nodes.push_back(pChild);
        }
      } else if (bFilterChildren) {
        nodes.push_back(pChild);
      }
      pChild = pChild->m_pNext;
    }
    if (bFilterOneOfProperties && nodes.empty()) {
      int32_t iProperties = 0;
      const XFA_PROPERTY* pProperty =
          XFA_GetElementProperties(GetElementType(), iProperties);
      if (!pProperty || iProperties < 1)
        return nodes;
      for (int32_t i = 0; i < iProperties; i++) {
        if (pProperty[i].uFlags & XFA_PROPERTYFLAG_DefaultOneOf) {
          const XFA_PACKETINFO* pPacket = XFA_GetPacketByID(GetPacketID());
          CXFA_Node* pNewNode =
              m_pDocument->CreateNode(pPacket, pProperty[i].eName);
          if (!pNewNode)
            break;
          InsertChild(pNewNode, nullptr);
          pNewNode->SetFlag(XFA_NodeFlag_Initialized, true);
          nodes.push_back(pNewNode);
          break;
        }
      }
    }
  }
  return nodes;
}

CXFA_Node* CXFA_Node::CreateSamePacketNode(XFA_Element eType) {
  CXFA_Node* pNode = m_pDocument->CreateNode(m_ePacket, eType);
  pNode->SetFlag(XFA_NodeFlag_Initialized, true);
  return pNode;
}

CXFA_Node* CXFA_Node::CloneTemplateToForm(bool bRecursive) {
  ASSERT(m_ePacket == XFA_XDPPACKET_Template);
  CXFA_Node* pClone =
      m_pDocument->CreateNode(XFA_XDPPACKET_Form, m_elementType);
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
  ASSERT(GetPacketID() == XFA_XDPPACKET_Form);
  return static_cast<CXFA_Node*>(
      JSNode()->GetObject(XFA_Attribute::BindingNode));
}

std::vector<CXFA_Node*> CXFA_Node::GetBindItems() {
  if (BindsFormItems()) {
    void* pBinding = nullptr;
    JSNode()->TryObject(XFA_Attribute::BindingNode, pBinding);
    return *static_cast<std::vector<CXFA_Node*>*>(pBinding);
  }
  std::vector<CXFA_Node*> result;
  CXFA_Node* pFormNode =
      static_cast<CXFA_Node*>(JSNode()->GetObject(XFA_Attribute::BindingNode));
  if (pFormNode)
    result.push_back(pFormNode);
  return result;
}

int32_t CXFA_Node::AddBindItem(CXFA_Node* pFormNode) {
  ASSERT(pFormNode);
  if (BindsFormItems()) {
    void* pBinding = nullptr;
    JSNode()->TryObject(XFA_Attribute::BindingNode, pBinding);
    auto* pItems = static_cast<std::vector<CXFA_Node*>*>(pBinding);
    if (!pdfium::ContainsValue(*pItems, pFormNode))
      pItems->push_back(pFormNode);
    return pdfium::CollectionSize<int32_t>(*pItems);
  }
  CXFA_Node* pOldFormItem =
      static_cast<CXFA_Node*>(JSNode()->GetObject(XFA_Attribute::BindingNode));
  if (!pOldFormItem) {
    JSNode()->SetObject(XFA_Attribute::BindingNode, pFormNode, nullptr);
    return 1;
  }
  if (pOldFormItem == pFormNode)
    return 1;

  std::vector<CXFA_Node*>* pItems = new std::vector<CXFA_Node*>;
  JSNode()->SetObject(XFA_Attribute::BindingNode, pItems,
                      &deleteBindItemCallBack);
  pItems->push_back(pOldFormItem);
  pItems->push_back(pFormNode);
  m_uNodeFlags |= XFA_NodeFlag_BindFormItems;
  return 2;
}

int32_t CXFA_Node::RemoveBindItem(CXFA_Node* pFormNode) {
  if (BindsFormItems()) {
    void* pBinding = nullptr;
    JSNode()->TryObject(XFA_Attribute::BindingNode, pBinding);
    auto* pItems = static_cast<std::vector<CXFA_Node*>*>(pBinding);
    auto iter = std::find(pItems->begin(), pItems->end(), pFormNode);
    if (iter != pItems->end()) {
      *iter = pItems->back();
      pItems->pop_back();
      if (pItems->size() == 1) {
        JSNode()->SetObject(XFA_Attribute::BindingNode, (*pItems)[0],
                            nullptr);  // Invalidates pItems.
        m_uNodeFlags &= ~XFA_NodeFlag_BindFormItems;
        return 1;
      }
    }
    return pdfium::CollectionSize<int32_t>(*pItems);
  }
  CXFA_Node* pOldFormItem =
      static_cast<CXFA_Node*>(JSNode()->GetObject(XFA_Attribute::BindingNode));
  if (pOldFormItem != pFormNode)
    return pOldFormItem ? 1 : 0;

  JSNode()->SetObject(XFA_Attribute::BindingNode, nullptr, nullptr);
  return 0;
}

bool CXFA_Node::HasBindItem() {
  return GetPacketID() == XFA_XDPPACKET_Datasets &&
         JSNode()->GetObject(XFA_Attribute::BindingNode);
}

CXFA_WidgetData* CXFA_Node::GetWidgetData() {
  return (CXFA_WidgetData*)JSNode()->GetObject(XFA_Attribute::WidgetData);
}

CXFA_WidgetData* CXFA_Node::GetContainerWidgetData() {
  if (GetPacketID() != XFA_XDPPACKET_Form)
    return nullptr;
  XFA_Element eType = GetElementType();
  if (eType == XFA_Element::ExclGroup)
    return nullptr;
  CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
  if (pParentNode && pParentNode->GetElementType() == XFA_Element::ExclGroup)
    return nullptr;

  if (eType == XFA_Element::Field) {
    CXFA_WidgetData* pFieldWidgetData = GetWidgetData();
    if (pFieldWidgetData &&
        pFieldWidgetData->GetChoiceListOpen() ==
            XFA_ATTRIBUTEENUM_MultiSelect) {
      return nullptr;
    } else {
      WideString wsPicture;
      if (pFieldWidgetData) {
        pFieldWidgetData->GetPictureContent(wsPicture,
                                            XFA_VALUEPICTURE_DataBind);
      }
      if (!wsPicture.IsEmpty())
        return pFieldWidgetData;
      CXFA_Node* pDataNode = GetBindData();
      if (!pDataNode)
        return nullptr;
      pFieldWidgetData = nullptr;
      for (CXFA_Node* pFormNode : pDataNode->GetBindItems()) {
        if (!pFormNode || pFormNode->HasRemovedChildren())
          continue;
        pFieldWidgetData = pFormNode->GetWidgetData();
        if (pFieldWidgetData) {
          pFieldWidgetData->GetPictureContent(wsPicture,
                                              XFA_VALUEPICTURE_DataBind);
        }
        if (!wsPicture.IsEmpty())
          break;
        pFieldWidgetData = nullptr;
      }
      return pFieldWidgetData;
    }
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

XFA_ATTRIBUTEENUM CXFA_Node::GetIntact() {
  CXFA_Node* pKeep = GetFirstChildByClass(XFA_Element::Keep);
  XFA_ATTRIBUTEENUM eLayoutType = JSNode()->GetEnum(XFA_Attribute::Layout);
  if (pKeep) {
    pdfium::Optional<XFA_ATTRIBUTEENUM> intact =
        pKeep->JSNode()->TryEnum(XFA_Attribute::Intact, false);
    if (intact) {
      if (*intact == XFA_ATTRIBUTEENUM_None &&
          eLayoutType == XFA_ATTRIBUTEENUM_Row &&
          m_pDocument->GetCurVersionMode() < XFA_VERSION_208) {
        CXFA_Node* pPreviewRow = GetNodeItem(XFA_NODEITEM_PrevSibling,
                                             XFA_ObjectType::ContainerNode);
        if (pPreviewRow &&
            pPreviewRow->JSNode()->GetEnum(XFA_Attribute::Layout) ==
                XFA_ATTRIBUTEENUM_Row) {
          pdfium::Optional<XFA_ATTRIBUTEENUM> value =
              pKeep->JSNode()->TryEnum(XFA_Attribute::Previous, false);
          if (value && (*value == XFA_ATTRIBUTEENUM_ContentArea ||
                        *value == XFA_ATTRIBUTEENUM_PageArea)) {
            return XFA_ATTRIBUTEENUM_ContentArea;
          }

          CXFA_Node* pNode =
              pPreviewRow->GetFirstChildByClass(XFA_Element::Keep);
          pdfium::Optional<XFA_ATTRIBUTEENUM> ret;
          if (pNode)
            ret = pNode->JSNode()->TryEnum(XFA_Attribute::Next, false);
          if (ret && (*ret == XFA_ATTRIBUTEENUM_ContentArea ||
                      *ret == XFA_ATTRIBUTEENUM_PageArea)) {
            return XFA_ATTRIBUTEENUM_ContentArea;
          }
        }
      }
      return *intact;
    }
  }
  switch (GetElementType()) {
    case XFA_Element::Subform:
      switch (eLayoutType) {
        case XFA_ATTRIBUTEENUM_Position:
        case XFA_ATTRIBUTEENUM_Row:
          return XFA_ATTRIBUTEENUM_ContentArea;
        case XFA_ATTRIBUTEENUM_Tb:
        case XFA_ATTRIBUTEENUM_Table:
        case XFA_ATTRIBUTEENUM_Lr_tb:
        case XFA_ATTRIBUTEENUM_Rl_tb:
          return XFA_ATTRIBUTEENUM_None;
        default:
          break;
      }
      break;
    case XFA_Element::Field: {
      CXFA_Node* pParentNode = GetNodeItem(XFA_NODEITEM_Parent);
      if (!pParentNode ||
          pParentNode->GetElementType() == XFA_Element::PageArea)
        return XFA_ATTRIBUTEENUM_ContentArea;
      if (pParentNode->GetIntact() == XFA_ATTRIBUTEENUM_None) {
        XFA_ATTRIBUTEENUM eParLayout =
            pParentNode->JSNode()->GetEnum(XFA_Attribute::Layout);
        if (eParLayout == XFA_ATTRIBUTEENUM_Position ||
            eParLayout == XFA_ATTRIBUTEENUM_Row ||
            eParLayout == XFA_ATTRIBUTEENUM_Table) {
          return XFA_ATTRIBUTEENUM_None;
        }
        XFA_VERSION version = m_pDocument->GetCurVersionMode();
        if (eParLayout == XFA_ATTRIBUTEENUM_Tb && version < XFA_VERSION_208) {
          CXFA_Measurement measureH;
          if (JSNode()->TryMeasure(XFA_Attribute::H, measureH, false))
            return XFA_ATTRIBUTEENUM_ContentArea;
        }
        return XFA_ATTRIBUTEENUM_None;
      }
      return XFA_ATTRIBUTEENUM_ContentArea;
    }
    case XFA_Element::Draw:
      return XFA_ATTRIBUTEENUM_ContentArea;
    default:
      break;
  }
  return XFA_ATTRIBUTEENUM_None;
}

CXFA_Node* CXFA_Node::GetDataDescriptionNode() {
  if (m_ePacket == XFA_XDPPACKET_Datasets)
    return m_pAuxNode;
  return nullptr;
}

void CXFA_Node::SetDataDescriptionNode(CXFA_Node* pDataDescriptionNode) {
  ASSERT(m_ePacket == XFA_XDPPACKET_Datasets);
  m_pAuxNode = pDataDescriptionNode;
}

CXFA_Node* CXFA_Node::GetModelNode() {
  switch (GetPacketID()) {
    case XFA_XDPPACKET_XDP:
      return m_pDocument->GetRoot();
    case XFA_XDPPACKET_Config:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Config));
    case XFA_XDPPACKET_Template:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Template));
    case XFA_XDPPACKET_Form:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Form));
    case XFA_XDPPACKET_Datasets:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Datasets));
    case XFA_XDPPACKET_LocaleSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_LocaleSet));
    case XFA_XDPPACKET_ConnectionSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_ConnectionSet));
    case XFA_XDPPACKET_SourceSet:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_SourceSet));
    case XFA_XDPPACKET_Xdc:
      return ToNode(m_pDocument->GetXFAObject(XFA_HASHCODE_Xdc));
    default:
      return this;
  }
}

int32_t CXFA_Node::CountChildren(XFA_Element eType, bool bOnlyChild) {
  CXFA_Node* pNode = m_pChild;
  int32_t iCount = 0;
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eType || eType == XFA_Element::Unknown) {
      if (bOnlyChild) {
        const XFA_PROPERTY* pProperty = XFA_GetPropertyOfElement(
            GetElementType(), pNode->GetElementType(), XFA_XDPPACKET_UNKNOWN);
        if (pProperty) {
          continue;
        }
      }
      iCount++;
    }
  }
  return iCount;
}

CXFA_Node* CXFA_Node::GetChild(int32_t index,
                               XFA_Element eType,
                               bool bOnlyChild) {
  ASSERT(index > -1);
  CXFA_Node* pNode = m_pChild;
  int32_t iCount = 0;
  for (; pNode; pNode = pNode->GetNodeItem(XFA_NODEITEM_NextSibling)) {
    if (pNode->GetElementType() == eType || eType == XFA_Element::Unknown) {
      if (bOnlyChild) {
        const XFA_PROPERTY* pProperty = XFA_GetPropertyOfElement(
            GetElementType(), pNode->GetElementType(), XFA_XDPPACKET_UNKNOWN);
        if (pProperty) {
          continue;
        }
      }
      iCount++;
      if (iCount > index) {
        return pNode;
      }
    }
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
      WideString wsName;
      pNode->JSNode()->GetAttribute(XFA_Attribute::Name, wsName, false);
      CFX_XMLElement* pNewXMLElement = new CFX_XMLElement(wsName);
      WideString wsValue = JSNode()->GetCData(XFA_Attribute::Value);
      if (!wsValue.IsEmpty())
        pNewXMLElement->SetTextData(WideString(wsValue));

      pNode->m_pXMLNode = pNewXMLElement;
      pNode->JSNode()->SetEnum(XFA_Attribute::Contains,
                               XFA_ATTRIBUTEENUM_Unknown, false);
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
  if (m_ePacket == XFA_XDPPACKET_Form) {
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

bool CXFA_Node::IsAttributeInXML() {
  return JSNode()->GetEnum(XFA_Attribute::Contains) ==
         XFA_ATTRIBUTEENUM_MetaData;
}

void CXFA_Node::OnRemoved(bool bNotify) {
  if (!bNotify)
    return;

  CXFA_FFNotify* pNotify = m_pDocument->GetNotify();
  if (pNotify)
    pNotify->OnChildRemoved();
}

void CXFA_Node::UpdateNameHash() {
  const XFA_NOTSUREATTRIBUTE* pNotsure =
      XFA_GetNotsureAttribute(GetElementType(), XFA_Attribute::Name);
  WideString wsName;
  if (!pNotsure || pNotsure->eType == XFA_AttributeType::CData) {
    wsName = JSNode()->GetCData(XFA_Attribute::Name);
    m_dwNameHash = FX_HashCode_GetW(wsName.AsStringView(), false);
  } else if (pNotsure->eType == XFA_AttributeType::Enum) {
    wsName =
        GetAttributeEnumByID(JSNode()->GetEnum(XFA_Attribute::Name))->pName;
    m_dwNameHash = FX_HashCode_GetW(wsName.AsStringView(), false);
  }
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
  return m_pXMLNode && (GetPacketID() == XFA_XDPPACKET_Datasets ||
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
    pFormNode->JSNode()->SetObject(XFA_Attribute::BindingNode, nullptr,
                                   nullptr);
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
  void* pValue = nullptr;
  if (!XFA_GetAttributeDefaultValue(pValue, GetElementType(), attr,
                                    XFA_AttributeType::Boolean,
                                    GetPacketID())) {
    return {};
  }
  return {!!pValue};
}

pdfium::Optional<int32_t> CXFA_Node::GetDefaultInteger(
    XFA_Attribute attr) const {
  void* pValue = nullptr;
  if (!XFA_GetAttributeDefaultValue(pValue, GetElementType(), attr,
                                    XFA_AttributeType::Integer,
                                    GetPacketID())) {
    return {};
  }
  return {static_cast<int32_t>(reinterpret_cast<uintptr_t>(pValue))};
}

pdfium::Optional<CXFA_Measurement> CXFA_Node::GetDefaultMeasurement(
    XFA_Attribute attr) const {
  void* pValue = nullptr;
  if (!XFA_GetAttributeDefaultValue(pValue, GetElementType(), attr,
                                    XFA_AttributeType::Measure,
                                    GetPacketID())) {
    return {};
  }
  CXFA_Measurement measure;
  memcpy(&measure, pValue, sizeof(measure));
  return {measure};
}

pdfium::Optional<WideString> CXFA_Node::GetDefaultCData(
    XFA_Attribute attr) const {
  void* pValue = nullptr;
  if (!XFA_GetAttributeDefaultValue(pValue, GetElementType(), attr,
                                    XFA_AttributeType::CData, GetPacketID())) {
    return {};
  }
  WideStringView view((const wchar_t*)pValue);
  return {WideString(view)};
}

pdfium::Optional<XFA_ATTRIBUTEENUM> CXFA_Node::GetDefaultEnum(
    XFA_Attribute attr) const {
  void* pValue = nullptr;
  if (!XFA_GetAttributeDefaultValue(pValue, GetElementType(), attr,
                                    XFA_AttributeType::Enum, GetPacketID())) {
    return {};
  }
  return {static_cast<XFA_ATTRIBUTEENUM>(reinterpret_cast<uintptr_t>(pValue))};
}

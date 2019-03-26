// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/parser/cxfa_nodehelper.h"

#include <utility>

#include "core/fxcrt/fx_extension.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"
#include "xfa/fxfa/parser/xfa_utils.h"

namespace {

CXFA_Node* FindFirstSiblingNamedInList(CXFA_Node* parent,
                                       uint32_t dwNameHash,
                                       uint32_t dwFilter);
CXFA_Node* FindFirstSiblingOfClassInList(CXFA_Node* parent,
                                         XFA_Element element,
                                         uint32_t dwFilter);

CXFA_Node* FindFirstSiblingNamed(CXFA_Node* parent, uint32_t dwNameHash) {
  CXFA_Node* result = FindFirstSiblingNamedInList(parent, dwNameHash,
                                                  XFA_NODEFILTER_Properties);
  if (result)
    return result;

  return FindFirstSiblingNamedInList(parent, dwNameHash,
                                     XFA_NODEFILTER_Children);
}

CXFA_Node* FindFirstSiblingNamedInList(CXFA_Node* parent,
                                       uint32_t dwNameHash,
                                       uint32_t dwFilter) {
  for (CXFA_Node* child : parent->GetNodeList(dwFilter, XFA_Element::Unknown)) {
    if (child->GetNameHash() == dwNameHash)
      return child;

    CXFA_Node* result = FindFirstSiblingNamed(child, dwNameHash);
    if (result)
      return result;
  }
  return nullptr;
}

CXFA_Node* FindFirstSiblingOfClass(CXFA_Node* parent, XFA_Element element) {
  CXFA_Node* result =
      FindFirstSiblingOfClassInList(parent, element, XFA_NODEFILTER_Properties);
  if (result)
    return result;

  return FindFirstSiblingOfClassInList(parent, element,
                                       XFA_NODEFILTER_Children);
}

CXFA_Node* FindFirstSiblingOfClassInList(CXFA_Node* parent,
                                         XFA_Element element,
                                         uint32_t dwFilter) {
  for (CXFA_Node* child : parent->GetNodeList(dwFilter, XFA_Element::Unknown)) {
    if (child->GetElementType() == element)
      return child;

    CXFA_Node* result = FindFirstSiblingOfClass(child, element);
    if (result)
      return result;
  }
  return nullptr;
}

void TraverseSiblings(CXFA_Node* parent,
                      uint32_t dwNameHash,
                      std::vector<CXFA_Node*>* pSiblings,
                      XFA_LOGIC_TYPE eLogicType,
                      bool bIsClassName,
                      bool bIsFindProperty) {
  ASSERT(parent);
  ASSERT(pSiblings);

  if (bIsFindProperty) {
    for (CXFA_Node* child :
         parent->GetNodeList(XFA_NODEFILTER_Properties, XFA_Element::Unknown)) {
      if (bIsClassName) {
        if (child->GetClassHashCode() == dwNameHash)
          pSiblings->push_back(child);
      } else {
        if (child->GetNameHash() == dwNameHash) {
          if (child->GetElementType() != XFA_Element::PageSet &&
              child->GetElementType() != XFA_Element::Extras &&
              child->GetElementType() != XFA_Element::Items) {
            pSiblings->push_back(child);
          }
        }
      }
      if (child->IsUnnamed() &&
          child->GetElementType() == XFA_Element::PageSet) {
        TraverseSiblings(child, dwNameHash, pSiblings, eLogicType, bIsClassName,
                         false);
      }
    }
    if (!pSiblings->empty())
      return;
  }
  for (CXFA_Node* child :
       parent->GetNodeList(XFA_NODEFILTER_Children, XFA_Element::Unknown)) {
    if (child->GetElementType() == XFA_Element::Variables)
      continue;

    if (bIsClassName) {
      if (child->GetClassHashCode() == dwNameHash)
        pSiblings->push_back(child);
    } else {
      if (child->GetNameHash() == dwNameHash)
        pSiblings->push_back(child);
    }
    if (eLogicType == XFA_LOGIC_NoTransparent)
      continue;

    if (CXFA_NodeHelper::NodeIsTransparent(child) &&
        child->GetElementType() != XFA_Element::PageSet) {
      TraverseSiblings(child, dwNameHash, pSiblings, eLogicType, bIsClassName,
                       false);
    }
  }
  return;
}

WideString GetNameExpressionSinglePath(CXFA_Node* refNode) {
  WideString ws;
  bool bIsProperty = CXFA_NodeHelper::NodeIsProperty(refNode);
  if (refNode->IsUnnamed() ||
      (bIsProperty && refNode->GetElementType() != XFA_Element::PageSet)) {
    ws = WideString::FromASCII(refNode->GetClassName());
    return WideString::Format(
        L"#%ls[%zu]", ws.c_str(),
        CXFA_NodeHelper::GetIndex(refNode, XFA_LOGIC_Transparent, bIsProperty,
                                  true));
  }
  ws = refNode->JSObject()->GetCData(XFA_Attribute::Name);
  ws.Replace(L".", L"\\.");
  return WideString::Format(
      L"%ls[%zu]", ws.c_str(),
      CXFA_NodeHelper::GetIndex(refNode, XFA_LOGIC_Transparent, bIsProperty,
                                false));
}

CXFA_Node* GetTransparentParent(CXFA_Node* pNode) {
  CXFA_Node* parent;
  CXFA_Node* node = pNode;
  while (true) {
    parent = node ? node->GetParent() : nullptr;
    if (!parent)
      return nullptr;

    XFA_Element parentType = parent->GetElementType();
    if ((!parent->IsUnnamed() && parentType != XFA_Element::SubformSet) ||
        parentType == XFA_Element::Variables) {
      break;
    }
    node = parent;
  }
  return parent;
}

}  // namespace

CXFA_NodeHelper::CXFA_NodeHelper() = default;

CXFA_NodeHelper::~CXFA_NodeHelper() = default;

// static
CXFA_Node* CXFA_NodeHelper::GetOneChildNamed(CXFA_Node* parent,
                                             WideStringView wsName) {
  if (!parent)
    return nullptr;
  return FindFirstSiblingNamed(parent, FX_HashCode_GetW(wsName, false));
}

// static
CXFA_Node* CXFA_NodeHelper::GetOneChildOfClass(CXFA_Node* parent,
                                               WideStringView wsClass) {
  if (!parent)
    return nullptr;

  XFA_Element element = XFA_GetElementByName(wsClass);
  if (element == XFA_Element::Unknown)
    return nullptr;

  return FindFirstSiblingOfClass(parent, element);
}

// static
std::vector<CXFA_Node*> CXFA_NodeHelper::GetSiblings(CXFA_Node* pNode,
                                                     XFA_LOGIC_TYPE eLogicType,
                                                     bool bIsClassName) {
  std::vector<CXFA_Node*> siblings;
  CXFA_Node* parent = pNode ? pNode->GetParent() : nullptr;
  if (!parent)
    return siblings;
  if (!parent->HasProperty(pNode->GetElementType()) &&
      eLogicType == XFA_LOGIC_Transparent) {
    parent = GetTransparentParent(pNode);
    if (!parent)
      return siblings;
  }

  uint32_t dwNameHash =
      bIsClassName ? pNode->GetClassHashCode() : pNode->GetNameHash();
  TraverseSiblings(parent, dwNameHash, &siblings, eLogicType, bIsClassName,
                   true);
  return siblings;
}

// static
size_t CXFA_NodeHelper::GetIndex(CXFA_Node* pNode,
                                 XFA_LOGIC_TYPE eLogicType,
                                 bool bIsProperty,
                                 bool bIsClassIndex) {
  CXFA_Node* parent = pNode ? pNode->GetParent() : nullptr;
  if (!parent)
    return 0;

  if (!bIsProperty && eLogicType == XFA_LOGIC_Transparent) {
    parent = GetTransparentParent(pNode);
    if (!parent)
      return 0;
  }
  uint32_t dwHashName =
      bIsClassIndex ? pNode->GetClassHashCode() : pNode->GetNameHash();
  std::vector<CXFA_Node*> siblings;
  TraverseSiblings(parent, dwHashName, &siblings, eLogicType, bIsClassIndex,
                   true);
  for (size_t i = 0; i < siblings.size(); ++i) {
    if (siblings[i] == pNode)
      return i;
  }
  return 0;
}

// static
WideString CXFA_NodeHelper::GetNameExpression(CXFA_Node* refNode) {
  WideString wsName = GetNameExpressionSinglePath(refNode);
  CXFA_Node* parent = refNode ? refNode->GetParent() : nullptr;
  while (parent) {
    WideString wsParent = GetNameExpressionSinglePath(parent);
    wsParent += L".";
    wsParent += wsName;
    wsName = std::move(wsParent);
    parent = parent->GetParent();
  }
  return wsName;
}

// static
bool CXFA_NodeHelper::NodeIsTransparent(CXFA_Node* refNode) {
  if (!refNode)
    return false;

  XFA_Element refNodeType = refNode->GetElementType();
  return (refNode->IsUnnamed() && refNode->IsContainerNode()) ||
         refNodeType == XFA_Element::SubformSet ||
         refNodeType == XFA_Element::Area || refNodeType == XFA_Element::Proto;
}

bool CXFA_NodeHelper::CreateNodeForCondition(const WideString& wsCondition) {
  size_t szLen = wsCondition.GetLength();
  WideString wsIndex(L"0");
  bool bAll = false;
  if (szLen == 0) {
    m_iCreateFlag = XFA_ResolveNode_RSType_CreateNodeOne;
    return false;
  }
  if (wsCondition[0] != '[')
    return false;

  size_t i = 1;
  for (; i < szLen; ++i) {
    wchar_t ch = wsCondition[i];
    if (ch == ' ')
      continue;

    if (ch == '*')
      bAll = true;
    break;
  }
  if (bAll) {
    wsIndex = L"1";
    m_iCreateFlag = XFA_ResolveNode_RSType_CreateNodeAll;
  } else {
    m_iCreateFlag = XFA_ResolveNode_RSType_CreateNodeOne;
    wsIndex = wsCondition.Mid(i, szLen - 1 - i);
  }
  int32_t iCount = wsIndex.GetInteger();
  if (iCount < 0)
    return false;

  m_iCreateCount = iCount;
  return true;
}

bool CXFA_NodeHelper::CreateNode(const WideString& wsName,
                                 const WideString& wsCondition,
                                 bool bLastNode,
                                 CFXJSE_Engine* pScriptContext) {
  if (!m_pCreateParent)
    return false;

  WideStringView wsNameView = wsName.AsStringView();
  bool bIsClassName = false;
  bool bResult = false;
  if (!wsNameView.IsEmpty() && wsNameView[0] == '!') {
    wsNameView = wsNameView.Right(wsNameView.GetLength() - 1);
    m_pCreateParent = ToNode(
        pScriptContext->GetDocument()->GetXFAObject(XFA_HASHCODE_Datasets));
  }
  if (!wsNameView.IsEmpty() && wsNameView[0] == '#') {
    bIsClassName = true;
    wsNameView = wsNameView.Right(wsNameView.GetLength() - 1);
  }
  if (wsNameView.IsEmpty())
    return false;

  if (m_iCreateCount == 0)
    CreateNodeForCondition(wsCondition);

  if (bIsClassName) {
    XFA_Element eType = XFA_GetElementByName(wsNameView);
    if (eType == XFA_Element::Unknown)
      return false;

    for (size_t i = 0; i < m_iCreateCount; ++i) {
      CXFA_Node* pNewNode = m_pCreateParent->CreateSamePacketNode(eType);
      if (pNewNode) {
        m_pCreateParent->InsertChild(pNewNode, nullptr);
        if (i == m_iCreateCount - 1) {
          m_pCreateParent = pNewNode;
        }
        bResult = true;
      }
    }
  } else {
    XFA_Element eClassType = XFA_Element::DataGroup;
    if (bLastNode) {
      eClassType = m_eLastCreateType;
    }
    for (size_t i = 0; i < m_iCreateCount; ++i) {
      CXFA_Node* pNewNode = m_pCreateParent->CreateSamePacketNode(eClassType);
      if (pNewNode) {
        pNewNode->JSObject()->SetAttribute(XFA_Attribute::Name, wsNameView,
                                           false);
        pNewNode->CreateXMLMappingNode();
        m_pCreateParent->InsertChild(pNewNode, nullptr);
        if (i == m_iCreateCount - 1) {
          m_pCreateParent = pNewNode;
        }
        bResult = true;
      }
    }
  }
  if (!bResult)
    m_pCreateParent = nullptr;

  return bResult;
}

void CXFA_NodeHelper::SetCreateNodeType(CXFA_Node* refNode) {
  if (!refNode)
    return;

  if (refNode->GetElementType() == XFA_Element::Subform) {
    m_eLastCreateType = XFA_Element::DataGroup;
  } else if (refNode->GetElementType() == XFA_Element::Field) {
    m_eLastCreateType = XFA_FieldIsMultiListBox(refNode)
                            ? XFA_Element::DataGroup
                            : XFA_Element::DataValue;
  } else if (refNode->GetElementType() == XFA_Element::ExclGroup) {
    m_eLastCreateType = XFA_Element::DataValue;
  }
}

// static
bool CXFA_NodeHelper::NodeIsProperty(CXFA_Node* refNode) {
  CXFA_Node* parent = refNode ? refNode->GetParent() : nullptr;
  return parent && parent->HasProperty(refNode->GetElementType());
}

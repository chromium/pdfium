// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_nodehelper.h"

#include "core/fxcrt/fx_extension.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "fxjs/xfa/cjx_object.h"
#include "xfa/fxfa/parser/cxfa_document.h"
#include "xfa/fxfa/parser/cxfa_localemgr.h"
#include "xfa/fxfa/parser/cxfa_node.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"
#include "xfa/fxfa/parser/xfa_utils.h"

CFXJSE_NodeHelper::CFXJSE_NodeHelper() = default;

CFXJSE_NodeHelper::~CFXJSE_NodeHelper() = default;

bool CFXJSE_NodeHelper::CreateNodeForCondition(const WideString& wsCondition) {
  size_t szLen = wsCondition.GetLength();
  WideString wsIndex(L"0");
  bool bAll = false;
  if (szLen == 0) {
    m_iCreateFlag = CFXJSE_Engine::ResolveResult::Type::kCreateNodeOne;
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
    m_iCreateFlag = CFXJSE_Engine::ResolveResult::Type::kCreateNodeAll;
  } else {
    m_iCreateFlag = CFXJSE_Engine::ResolveResult::Type::kCreateNodeOne;
    wsIndex = wsCondition.Substr(i, szLen - 1 - i);
  }
  int32_t iCount = wsIndex.GetInteger();
  if (iCount < 0)
    return false;

  m_iCreateCount = iCount;
  return true;
}

bool CFXJSE_NodeHelper::CreateNode(const WideString& wsName,
                                   const WideString& wsCondition,
                                   bool bLastNode,
                                   CFXJSE_Engine* pScriptContext) {
  if (!m_pCreateParent)
    return false;

  WideStringView wsNameView = wsName.AsStringView();
  bool bIsClassName = false;
  bool bResult = false;
  if (!wsNameView.IsEmpty() && wsNameView[0] == '!') {
    wsNameView = wsNameView.Last(wsNameView.GetLength() - 1);
    m_pCreateParent = ToNode(
        pScriptContext->GetDocument()->GetXFAObject(XFA_HASHCODE_Datasets));
  }
  if (!wsNameView.IsEmpty() && wsNameView[0] == '#') {
    bIsClassName = true;
    wsNameView = wsNameView.Last(wsNameView.GetLength() - 1);
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
        m_pCreateParent->InsertChildAndNotify(pNewNode, nullptr);
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
        pNewNode->JSObject()->SetAttributeByEnum(XFA_Attribute::Name,
                                                 WideString(wsNameView), false);
        pNewNode->CreateXMLMappingNode();
        m_pCreateParent->InsertChildAndNotify(pNewNode, nullptr);
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

void CFXJSE_NodeHelper::SetCreateNodeType(CXFA_Node* refNode) {
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

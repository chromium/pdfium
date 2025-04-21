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
  if (szLen == 0) {
    create_flag_ = CFXJSE_Engine::ResolveResult::Type::kCreateNodeOne;
    return false;
  }
  if (wsCondition[0] != '[') {
    return false;
  }
  size_t i = 1;
  for (; i < szLen; ++i) {
    wchar_t ch = wsCondition[i];
    if (ch == '*') {
      create_flag_ = CFXJSE_Engine::ResolveResult::Type::kCreateNodeAll;
      create_count_ = 1;
      return true;
    }
    if (ch != ' ') {
      break;
    }
  }
  create_flag_ = CFXJSE_Engine::ResolveResult::Type::kCreateNodeOne;
  int32_t iCount = wsCondition.Substr(i, szLen - 1 - i).GetInteger();
  if (iCount < 0) {
    return false;
  }
  create_count_ = iCount;
  return true;
}

bool CFXJSE_NodeHelper::CreateNode(const WideString& wsName,
                                   const WideString& wsCondition,
                                   bool bLastNode,
                                   CFXJSE_Engine* pScriptContext) {
  if (!create_parent_) {
    return false;
  }

  WideStringView wsNameView = wsName.AsStringView();
  bool bIsClassName = false;
  bool bResult = false;
  if (!wsNameView.IsEmpty() && wsNameView[0] == '!') {
    wsNameView = wsNameView.Last(wsNameView.GetLength() - 1);
    create_parent_ = ToNode(
        pScriptContext->GetDocument()->GetXFAObject(XFA_HASHCODE_Datasets));
  }
  if (!wsNameView.IsEmpty() && wsNameView[0] == '#') {
    bIsClassName = true;
    wsNameView = wsNameView.Last(wsNameView.GetLength() - 1);
  }
  if (wsNameView.IsEmpty()) {
    return false;
  }

  if (create_count_ == 0) {
    CreateNodeForCondition(wsCondition);
  }

  if (bIsClassName) {
    XFA_Element eType = XFA_GetElementByName(wsNameView);
    if (eType == XFA_Element::Unknown) {
      return false;
    }

    for (size_t i = 0; i < create_count_; ++i) {
      CXFA_Node* pNewNode = create_parent_->CreateSamePacketNode(eType);
      if (pNewNode) {
        create_parent_->InsertChildAndNotify(pNewNode, nullptr);
        if (i == create_count_ - 1) {
          create_parent_ = pNewNode;
        }
        bResult = true;
      }
    }
  } else {
    XFA_Element eClassType = XFA_Element::DataGroup;
    if (bLastNode) {
      eClassType = last_create_type_;
    }
    for (size_t i = 0; i < create_count_; ++i) {
      CXFA_Node* pNewNode = create_parent_->CreateSamePacketNode(eClassType);
      if (pNewNode) {
        pNewNode->JSObject()->SetAttributeByEnum(XFA_Attribute::Name,
                                                 WideString(wsNameView), false);
        pNewNode->CreateXMLMappingNode();
        create_parent_->InsertChildAndNotify(pNewNode, nullptr);
        if (i == create_count_ - 1) {
          create_parent_ = pNewNode;
        }
        bResult = true;
      }
    }
  }
  if (!bResult) {
    create_parent_ = nullptr;
  }

  return bResult;
}

void CFXJSE_NodeHelper::SetCreateNodeType(CXFA_Node* refNode) {
  if (!refNode) {
    return;
  }

  if (refNode->GetElementType() == XFA_Element::Subform) {
    last_create_type_ = XFA_Element::DataGroup;
  } else if (refNode->GetElementType() == XFA_Element::Field) {
    last_create_type_ = XFA_FieldIsMultiListBox(refNode)
                            ? XFA_Element::DataGroup
                            : XFA_Element::DataValue;
  } else if (refNode->GetElementType() == XFA_Element::ExclGroup) {
    last_create_type_ = XFA_Element::DataValue;
  }
}

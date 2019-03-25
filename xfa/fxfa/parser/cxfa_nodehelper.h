// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_NODEHELPER_H_
#define XFA_FXFA_PARSER_CXFA_NODEHELPER_H_

#include <vector>

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"

class CFXJSE_Engine;
class CXFA_Node;

enum XFA_LOGIC_TYPE {
  XFA_LOGIC_NoTransparent,
  XFA_LOGIC_Transparent,
};

class CXFA_NodeHelper {
 public:
  CXFA_NodeHelper();
  ~CXFA_NodeHelper();

  static CXFA_Node* GetOneChildNamed(CXFA_Node* parent, WideStringView wsName);
  static CXFA_Node* GetOneChildOfClass(CXFA_Node* parent,
                                       WideStringView wsClass);

  static CXFA_Node* GetParent(CXFA_Node* pNode, XFA_LOGIC_TYPE eLogicType);

  static std::vector<CXFA_Node*> GetSiblings(CXFA_Node* pNode,
                                             XFA_LOGIC_TYPE eLogicType,
                                             bool bIsClassName);
  static size_t GetIndex(CXFA_Node* pNode,
                         XFA_LOGIC_TYPE eLogicType,
                         bool bIsProperty,
                         bool bIsClassIndex);
  static WideString GetNameExpression(CXFA_Node* refNode);
  static bool NodeIsTransparent(CXFA_Node* refNode);
  static bool NodeIsProperty(CXFA_Node* refNode);

  bool CreateNode(const WideString& wsName,
                  const WideString& wsCondition,
                  bool bLastNode,
                  CFXJSE_Engine* pScriptContext);
  bool CreateNodeForCondition(const WideString& wsCondition);
  void SetCreateNodeType(CXFA_Node* refNode);

  XFA_Element m_eLastCreateType = XFA_Element::DataValue;
  XFA_ResolveNode_RSType m_iCreateFlag = XFA_ResolveNode_RSType_CreateNodeOne;
  size_t m_iCreateCount = 0;
  int32_t m_iCurAllStart = -1;
  UnownedPtr<CXFA_Node> m_pCreateParent;
  UnownedPtr<CXFA_Node> m_pAllStartParent;
};

#endif  // XFA_FXFA_PARSER_CXFA_NODEHELPER_H_

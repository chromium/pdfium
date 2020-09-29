// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_NODEHELPER_H_
#define FXJS_XFA_CFXJSE_NODEHELPER_H_

#include "core/fxcrt/fx_string.h"
#include "v8/include/cppgc/persistent.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/xfa_resolvenode_rs.h"

class CFXJSE_Engine;
class CXFA_Node;

class CFXJSE_NodeHelper {
 public:
  CFXJSE_NodeHelper();
  ~CFXJSE_NodeHelper();

  bool CreateNode(const WideString& wsName,
                  const WideString& wsCondition,
                  bool bLastNode,
                  CFXJSE_Engine* pScriptContext);
  bool CreateNodeForCondition(const WideString& wsCondition);
  void SetCreateNodeType(CXFA_Node* refNode);

  XFA_Element m_eLastCreateType = XFA_Element::DataValue;
  XFA_ResolveNodeRS::Type m_iCreateFlag =
      XFA_ResolveNodeRS::Type::kCreateNodeOne;
  size_t m_iCreateCount = 0;
  int32_t m_iCurAllStart = -1;
  cppgc::Persistent<CXFA_Node> m_pCreateParent;
  cppgc::Persistent<CXFA_Node> m_pAllStartParent;
};

#endif  // FXJS_XFA_CFXJSE_NODEHELPER_H_

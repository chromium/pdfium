// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_THISPROXY_H_
#define XFA_FXFA_PARSER_CXFA_THISPROXY_H_

#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/parser/cxfa_object.h"

class CXFA_Node;

class CXFA_ThisProxy final : public CXFA_Object {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_ThisProxy() override;

  void Trace(cppgc::Visitor* visitor) const override;

  CXFA_Node* GetThisNode() const { return m_pThisNode.Get(); }
  CXFA_Node* GetScriptNode() const { return m_pScriptNode.Get(); }

 private:
  CXFA_ThisProxy(CXFA_Node* pThisNode, CXFA_Node* pScriptNode);

  cppgc::Member<CXFA_Node> m_pThisNode;
  cppgc::Member<CXFA_Node> m_pScriptNode;
};

#endif  // XFA_FXFA_PARSER_CXFA_THISPROXY_H_

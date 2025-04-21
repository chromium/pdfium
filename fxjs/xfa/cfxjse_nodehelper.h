// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_NODEHELPER_H_
#define FXJS_XFA_CFXJSE_NODEHELPER_H_

#include "core/fxcrt/widestring.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/cppgc/persistent.h"
#include "xfa/fxfa/fxfa_basic.h"

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

  XFA_Element last_create_type_ = XFA_Element::DataValue;
  CFXJSE_Engine::ResolveResult::Type create_flag_ =
      CFXJSE_Engine::ResolveResult::Type::kCreateNodeOne;
  size_t create_count_ = 0;
  int32_t cur_all_start_ = -1;
  cppgc::Persistent<CXFA_Node> create_parent_;
  cppgc::Persistent<CXFA_Node> all_start_parent_;
};

#endif  // FXJS_XFA_CFXJSE_NODEHELPER_H_

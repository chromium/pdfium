// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_RESOLVEPROCESSOR_H_
#define FXJS_XFA_CFXJSE_RESOLVEPROCESSOR_H_

#include <memory>

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "fxjs/xfa/cfxjse_engine.h"
#include "v8/include/cppgc/macros.h"
#include "xfa/fxfa/fxfa_basic.h"
#include "xfa/fxfa/parser/xfa_basic_data.h"

class CFXJSE_NodeHelper;

class CFXJSE_ResolveProcessor {
 public:
  class NodeData {
    CPPGC_STACK_ALLOCATED();  // Allows Raw/Unowned pointers.

   public:
    NodeData();
    ~NodeData();

    UnownedPtr<CXFA_Object> cur_object_;  // Ok, stack-only.
    WideString name_;
    WideString condition_;
    XFA_HashCode hash_name_ = XFA_HASHCODE_None;
    int32_t level_ = 0;
    Mask<XFA_ResolveFlag> styles_ = XFA_ResolveFlag::kChildren;
    CFXJSE_Engine::ResolveResult result_;
  };

  CFXJSE_ResolveProcessor(CFXJSE_Engine* pEngine, CFXJSE_NodeHelper* pHelper);
  ~CFXJSE_ResolveProcessor();

  bool Resolve(v8::Isolate* pIsolate, NodeData& rnd);
  int32_t GetFilter(WideStringView wsExpression, int32_t nStart, NodeData& rnd);
  int32_t IndexForDataBind(const WideString& wsNextCondition, int32_t iCount);
  void SetCurStart(int32_t start) { cur_start_ = start; }

 private:
  bool ResolveForAttributeRs(CXFA_Object* curNode,
                             CFXJSE_Engine::ResolveResult* rnd,
                             WideStringView strAttr);
  bool ResolveAnyChild(v8::Isolate* pIsolate, NodeData& rnd);
  bool ResolveDollar(v8::Isolate* pIsolate, NodeData& rnd);
  bool ResolveExcalmatory(v8::Isolate* pIsolate, NodeData& rnd);
  bool ResolveNumberSign(v8::Isolate* pIsolate, NodeData& rnd);
  bool ResolveAsterisk(NodeData& rnd);
  bool ResolveNormal(v8::Isolate* pIsolate, NodeData& rnd);
  void SetStylesForChild(Mask<XFA_ResolveFlag> dwParentStyles, NodeData& rnd);

  void ConditionArray(size_t iCurIndex,
                      WideString wsCondition,
                      size_t iFoundCount,
                      NodeData* pRnd);
  void FilterCondition(v8::Isolate* pIsolate,
                       WideString wsCondition,
                       NodeData* pRnd);
  void DoPredicateFilter(v8::Isolate* pIsolate,
                         WideString wsCondition,
                         size_t iFoundCount,
                         NodeData* pRnd);

  int32_t cur_start_ = 0;
  UnownedPtr<CFXJSE_Engine> const engine_;
  UnownedPtr<CFXJSE_NodeHelper> const node_helper_;
};

#endif  // FXJS_XFA_CFXJSE_RESOLVEPROCESSOR_H_

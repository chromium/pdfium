// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_NODE_H_
#define FXJS_XFA_CJX_NODE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_tree.h"
#include "xfa/fxfa/fxfa_basic.h"

class CXFA_Node;

class CJX_Node : public CJX_Tree {
 public:
  explicit CJX_Node(CXFA_Node* node);
  ~CJX_Node() override;

  JSE_METHOD(applyXSL, CJX_Node);
  JSE_METHOD(assignNode, CJX_Node);
  JSE_METHOD(clone, CJX_Node);
  JSE_METHOD(getAttribute, CJX_Node);
  JSE_METHOD(getElement, CJX_Node);
  JSE_METHOD(isPropertySpecified, CJX_Node);
  JSE_METHOD(loadXML, CJX_Node);
  JSE_METHOD(saveFilteredXML, CJX_Node);
  JSE_METHOD(saveXML, CJX_Node);
  JSE_METHOD(setAttribute, CJX_Node);
  JSE_METHOD(setElement, CJX_Node);

  JSE_PROP(id);
  JSE_PROP(isContainer);
  JSE_PROP(isNull);
  JSE_PROP(model);
  JSE_PROP(ns);
  JSE_PROP(oneOfChild);

  CXFA_Node* GetXFANode() const;

 protected:
  int32_t execSingleEventByName(const WideStringView& wsEventName,
                                XFA_Element eType);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_NODE_H_

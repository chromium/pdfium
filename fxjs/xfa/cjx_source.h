// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_SOURCE_H_
#define FXJS_XFA_CJX_SOURCE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Source;

class CJX_Source final : public CJX_Node {
 public:
  explicit CJX_Source(CXFA_Source* src);
  ~CJX_Source() override;

  JSE_METHOD(addNew, CJX_Source);
  JSE_METHOD(cancel, CJX_Source);
  JSE_METHOD(cancelBatch, CJX_Source);
  JSE_METHOD(close, CJX_Source);
  JSE_METHOD(deleteItem /*delete*/, CJX_Source);
  JSE_METHOD(first, CJX_Source);
  JSE_METHOD(hasDataChanged, CJX_Source);
  JSE_METHOD(isBOF, CJX_Source);
  JSE_METHOD(isEOF, CJX_Source);
  JSE_METHOD(last, CJX_Source);
  JSE_METHOD(next, CJX_Source);
  JSE_METHOD(open, CJX_Source);
  JSE_METHOD(previous, CJX_Source);
  JSE_METHOD(requery, CJX_Source);
  JSE_METHOD(resync, CJX_Source);
  JSE_METHOD(update, CJX_Source);
  JSE_METHOD(updateBatch, CJX_Source);

  JSE_PROP(db);
  JSE_PROP(use);
  JSE_PROP(usehref);

 private:
  static const CJX_MethodSpec MethodSpecs[];
};

#endif  // FXJS_XFA_CJX_SOURCE_H_

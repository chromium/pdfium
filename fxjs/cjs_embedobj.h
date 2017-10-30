// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_EMBEDOBJ_H_
#define FXJS_CJS_EMBEDOBJ_H_

#include "core/fxcrt/unowned_ptr.h"

class CJS_Object;

class CJS_EmbedObj {
 public:
  explicit CJS_EmbedObj(CJS_Object* pJSObject);
  virtual ~CJS_EmbedObj();

  CJS_Object* GetJSObject() const { return m_pJSObject.Get(); }

 protected:
  UnownedPtr<CJS_Object> const m_pJSObject;
};

#endif  // FXJS_CJS_EMBEDOBJ_H_

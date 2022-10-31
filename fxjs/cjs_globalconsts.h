// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_GLOBALCONSTS_H_
#define FXJS_CJS_GLOBALCONSTS_H_

#include "fxjs/cjs_object.h"

class CJS_GlobalConsts final : public CJS_Object {
 public:
  static void DefineJSObjects(CJS_Runtime* pRuntime);
};

#endif  // FXJS_CJS_GLOBALCONSTS_H_

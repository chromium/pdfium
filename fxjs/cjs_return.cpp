// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/cjs_return.h"

CJS_Return::CJS_Return() {}

CJS_Return::CJS_Return(v8::Local<v8::Value> ret) : return_(ret) {}

CJS_Return::CJS_Return(const WideString& err) : error_(err) {}

CJS_Return::CJS_Return(JSMessage id) : CJS_Return(JSGetStringFromID(id)) {}

CJS_Return::CJS_Return(const CJS_Return&) = default;

CJS_Return::~CJS_Return() = default;

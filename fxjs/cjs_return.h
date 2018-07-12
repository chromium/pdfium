// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_RETURN_H_
#define FXJS_CJS_RETURN_H_

#include "fxjs/cfxjs_engine.h"
#include "fxjs/js_resources.h"
#include "third_party/base/optional.h"

class CJS_Return {
 public:
  CJS_Return();                               // Successful but empty return.
  explicit CJS_Return(v8::Local<v8::Value>);  // Successful return with value.
  explicit CJS_Return(const WideString&);     // Error with custom message.
  explicit CJS_Return(JSMessage id);          // Error with stock message.
  CJS_Return(const CJS_Return&);
  ~CJS_Return();

  bool HasError() const { return error_.has_value(); }
  WideString Error() const { return error_.value(); }

  bool HasReturn() const { return !return_.IsEmpty(); }
  v8::Local<v8::Value> Return() const { return return_; }

 private:
  Optional<WideString> error_;
  v8::Local<v8::Value> return_;
};

#endif  // FXJS_CJS_RETURN_H_

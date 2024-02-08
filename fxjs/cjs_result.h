// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_CJS_RESULT_H_
#define FXJS_CJS_RESULT_H_

#include <optional>

#include "fxjs/js_resources.h"
#include "v8/include/v8-forward.h"

class CJS_Result {
 public:
  // Wrap constructors with static methods so we can apply [[nodiscard]],
  // otherwise we can't catch places where someone mistakenly writes:
  //
  //     if (error)
  //       CJS_Result(JS_ERROR_CODE);
  //
  // instead of
  //
  //     if (error)
  //       return CJS_Result(JS_ERROR_CODE);
  //
  [[nodiscard]] static CJS_Result Success() { return CJS_Result(); }
  [[nodiscard]] static CJS_Result Success(v8::Local<v8::Value> value) {
    return CJS_Result(value);
  }
  [[nodiscard]] static CJS_Result Failure(const WideString& str) {
    return CJS_Result(str);
  }
  [[nodiscard]] static CJS_Result Failure(JSMessage id) {
    return CJS_Result(id);
  }

  CJS_Result(const CJS_Result&);
  ~CJS_Result();

  bool HasError() const { return error_.has_value(); }
  const WideString& Error() const { return error_.value(); }

  bool HasReturn() const { return !return_.IsEmpty(); }
  v8::Local<v8::Value> Return() const { return return_; }

 private:
  CJS_Result();                               // Successful but empty return.
  explicit CJS_Result(v8::Local<v8::Value>);  // Successful return with value.
  explicit CJS_Result(const WideString&);     // Error with custom message.
  explicit CJS_Result(JSMessage id);          // Error with stock message.

  std::optional<WideString> error_;
  v8::Local<v8::Value> return_;
};

#endif  // FXJS_CJS_RESULT_H_

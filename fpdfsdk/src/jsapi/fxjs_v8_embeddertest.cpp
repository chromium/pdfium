// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/js_embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

namespace {

const wchar_t kScript[] = L"fred = 7";

}  // namespace

class FXJSV8EmbedderTest : public JSEmbedderTest {};

TEST_F(FXJSV8EmbedderTest, Getters) {
  v8::Isolate::Scope isolate_scope(isolate());
#ifdef PDF_ENABLE_XFA
  v8::Locker locker(isolate());
#endif  // PDF_ENABLE_XFA
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(GetV8Context());

  FXJSErr error;
  int sts = FXJS_Execute(isolate(), nullptr, kScript, &error);
  EXPECT_EQ(0, sts);

  v8::Local<v8::Object> This = FXJS_GetThisObj(isolate());
  v8::Local<v8::Value> fred = FXJS_GetObjectElement(isolate(), This, L"fred");
  EXPECT_TRUE(fred->IsNumber());
}

// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fxjs/cfxjs_engine.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/js_embedder_test.h"

namespace {

const double kExpected0 = 6.0;
const double kExpected1 = 7.0;
const double kExpected2 = 8.0;

const wchar_t kScript0[] = L"fred = 6";
const wchar_t kScript1[] = L"fred = 7";
const wchar_t kScript2[] = L"fred = 8";

}  // namespace

class CFXJSEngineEmbedderTest : public JSEmbedderTest {
 public:
  void ExecuteInCurrentContext(const WideString& script) {
    auto* current_engine =
        CFXJS_Engine::EngineFromIsolateCurrentContext(isolate());
    FXJSErr error;
    int sts = current_engine->Execute(script, &error);
    EXPECT_EQ(0, sts);
  }
  void CheckAssignmentInCurrentContext(double expected) {
    auto* current_engine =
        CFXJS_Engine::EngineFromIsolateCurrentContext(isolate());
    v8::Local<v8::Object> This = current_engine->GetThisObj();
    v8::Local<v8::Value> fred =
        current_engine->GetObjectProperty(This, L"fred");
    EXPECT_TRUE(fred->IsNumber());
    EXPECT_EQ(expected, current_engine->ToDouble(fred));
  }
};

TEST_F(CFXJSEngineEmbedderTest, Getters) {
  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());
  v8::Context::Scope context_scope(GetV8Context());

  ExecuteInCurrentContext(WideString(kScript1));
  CheckAssignmentInCurrentContext(kExpected1);
}

TEST_F(CFXJSEngineEmbedderTest, MultipleEngines) {
  v8::Isolate::Scope isolate_scope(isolate());
  v8::HandleScope handle_scope(isolate());

  CFXJS_Engine engine1(isolate());
  engine1.InitializeEngine();

  CFXJS_Engine engine2(isolate());
  engine2.InitializeEngine();

  v8::Local<v8::Context> context1 = engine1.GetV8Context();
  v8::Local<v8::Context> context2 = engine2.GetV8Context();

  v8::Context::Scope context_scope(GetV8Context());
  ExecuteInCurrentContext(WideString(kScript0));
  CheckAssignmentInCurrentContext(kExpected0);

  {
    v8::Context::Scope context_scope1(context1);
    ExecuteInCurrentContext(WideString(kScript1));
    CheckAssignmentInCurrentContext(kExpected1);
  }
  {
    v8::Context::Scope context_scope2(context2);
    ExecuteInCurrentContext(WideString(kScript2));
    CheckAssignmentInCurrentContext(kExpected2);
  }

  CheckAssignmentInCurrentContext(kExpected0);

  {
    v8::Context::Scope context_scope1(context1);
    CheckAssignmentInCurrentContext(kExpected1);
    {
      v8::Context::Scope context_scope2(context2);
      CheckAssignmentInCurrentContext(kExpected2);
    }
    CheckAssignmentInCurrentContext(kExpected1);
  }
  {
    v8::Context::Scope context_scope2(context2);
    CheckAssignmentInCurrentContext(kExpected2);
    {
      v8::Context::Scope context_scope1(context1);
      CheckAssignmentInCurrentContext(kExpected1);
    }
    CheckAssignmentInCurrentContext(kExpected2);
  }

  CheckAssignmentInCurrentContext(kExpected0);

  engine1.ReleaseEngine();
  engine2.ReleaseEngine();
}

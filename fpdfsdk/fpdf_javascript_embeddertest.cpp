// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "core/fxcrt/fx_memory.h"
#include "public/fpdf_javascript.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/fx_string_testhelpers.h"
#include "testing/utils/hash.h"

class FPDFJavaScriptEmbedderTest : public EmbedderTest {};

TEST_F(FPDFJavaScriptEmbedderTest, CountJS) {
  // Open a file with JS.
  ASSERT_TRUE(OpenDocument("bug_679649.pdf"));
  EXPECT_EQ(1, FPDFDoc_GetJavaScriptActionCount(document()));
}

TEST_F(FPDFJavaScriptEmbedderTest, CountNoJS) {
  // Open a file without JS.
  ASSERT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(0, FPDFDoc_GetJavaScriptActionCount(document()));

  // Provide no document.
  EXPECT_EQ(-1, FPDFDoc_GetJavaScriptActionCount(nullptr));
}

TEST_F(FPDFJavaScriptEmbedderTest, GetJS) {
  ASSERT_TRUE(OpenDocument("js.pdf"));
  EXPECT_EQ(6, FPDFDoc_GetJavaScriptActionCount(document()));

  ScopedFPDFJavaScriptAction js;
  js.reset(FPDFDoc_GetJavaScriptAction(document(), -1));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(document(), 6));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(nullptr, -1));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(nullptr, 0));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(nullptr, 1));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(nullptr, 2));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(nullptr, 5));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(nullptr, 6));
  EXPECT_FALSE(js);

  js.reset(FPDFDoc_GetJavaScriptAction(document(), 0));
  EXPECT_TRUE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(document(), 1));
  EXPECT_TRUE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(document(), 2));
  EXPECT_TRUE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(document(), 3));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(document(), 4));
  EXPECT_FALSE(js);
  js.reset(FPDFDoc_GetJavaScriptAction(document(), 5));
  EXPECT_FALSE(js);
}

TEST_F(FPDFJavaScriptEmbedderTest, GetJSName) {
  ASSERT_TRUE(OpenDocument("bug_679649.pdf"));
  ScopedFPDFJavaScriptAction js(FPDFDoc_GetJavaScriptAction(document(), 0));
  ASSERT_TRUE(js);

  {
    FPDF_WCHAR buf[10];
    EXPECT_EQ(0u, FPDFJavaScriptAction_GetName(nullptr, nullptr, 0));
    EXPECT_EQ(0u, FPDFJavaScriptAction_GetName(nullptr, buf, 0));
    EXPECT_EQ(0u, FPDFJavaScriptAction_GetName(nullptr, buf, sizeof(buf)));
  }

  constexpr size_t kExpectedLength = 22;
  ASSERT_EQ(kExpectedLength,
            FPDFJavaScriptAction_GetName(js.get(), nullptr, 0));

  // Check that the name not returned if the buffer is too small.
  // The result buffer should be overwritten with an empty string.
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(kExpectedLength);
  // Write in the buffer to verify it's not overwritten.
  memcpy(buf.data(), "abcdefgh", 8);
  EXPECT_EQ(kExpectedLength, FPDFJavaScriptAction_GetName(js.get(), buf.data(),
                                                          kExpectedLength - 1));
  EXPECT_EQ(0, memcmp(buf.data(), "abcdefgh", 8));

  EXPECT_EQ(kExpectedLength, FPDFJavaScriptAction_GetName(js.get(), buf.data(),
                                                          kExpectedLength));
  EXPECT_EQ(L"startDelay", GetPlatformWString(buf.data()));
}

TEST_F(FPDFJavaScriptEmbedderTest, GetJSScript) {
  ASSERT_TRUE(OpenDocument("bug_679649.pdf"));
  ScopedFPDFJavaScriptAction js(FPDFDoc_GetJavaScriptAction(document(), 0));
  ASSERT_TRUE(js);

  {
    FPDF_WCHAR buf[10];
    EXPECT_EQ(0u, FPDFJavaScriptAction_GetScript(nullptr, nullptr, 0));
    EXPECT_EQ(0u, FPDFJavaScriptAction_GetScript(nullptr, buf, 0));
    EXPECT_EQ(0u, FPDFJavaScriptAction_GetScript(nullptr, buf, sizeof(buf)));
  }

  constexpr size_t kExpectedLength = 218;
  ASSERT_EQ(kExpectedLength,
            FPDFJavaScriptAction_GetScript(js.get(), nullptr, 0));

  // Check that the string value of an AP is not returned if the buffer is too
  // small. The result buffer should be overwritten with an empty string.
  std::vector<FPDF_WCHAR> buf = GetFPDFWideStringBuffer(kExpectedLength);
  // Write in the buffer to verify it's not overwritten.
  memcpy(buf.data(), "abcdefgh", 8);
  EXPECT_EQ(kExpectedLength, FPDFJavaScriptAction_GetScript(
                                 js.get(), buf.data(), kExpectedLength - 1));
  EXPECT_EQ(0, memcmp(buf.data(), "abcdefgh", 8));

  static const wchar_t kExpectedScript[] =
      L"function ping() {\n  app.alert(\"ping\");\n}\n"
      L"var timer = app.setTimeOut(\"ping()\", 100);\napp.clearTimeOut(timer);";
  EXPECT_EQ(kExpectedLength, FPDFJavaScriptAction_GetScript(
                                 js.get(), buf.data(), kExpectedLength));
  EXPECT_EQ(kExpectedScript, GetPlatformWString(buf.data()));
}

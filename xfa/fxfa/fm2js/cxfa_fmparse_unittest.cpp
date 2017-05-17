// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/fm2js/cxfa_fmparse.h"

#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

TEST(CXFA_FMParseTest, Empty) {
  CXFA_FMErrorInfo errorInfo;
  auto parser = pdfium::MakeUnique<CXFA_FMParse>(L"", &errorInfo);
  std::unique_ptr<CXFA_FMFunctionDefinition> ast = parser->Parse();
  ASSERT(ast != nullptr);
  EXPECT_TRUE(errorInfo.message.IsEmpty());

  CFX_WideTextBuf buf;
  EXPECT_TRUE(ast->ToJavaScript(buf));
  // TODO(dsinclair): This is a little weird .....
  EXPECT_EQ(L"// comments only", buf.AsStringC());
}

TEST(CXFA_FMParseTest, CommentOnlyIsError) {
  CXFA_FMErrorInfo errorInfo;
  auto parser = pdfium::MakeUnique<CXFA_FMParse>(L"; Just comment", &errorInfo);
  std::unique_ptr<CXFA_FMFunctionDefinition> ast = parser->Parse();
  ASSERT(ast != nullptr);
  // TODO(dsinclair): This isn't allowed per the spec.
  EXPECT_TRUE(errorInfo.message.IsEmpty());
  // EXPECT_FALSE(errorInfo.message.IsEmpty());

  CFX_WideTextBuf buf;
  EXPECT_TRUE(ast->ToJavaScript(buf));
  EXPECT_EQ(L"// comments only", buf.AsStringC());
}

TEST(CXFA_FMParseTest, CommentThenValue) {
  CXFA_FMErrorInfo errorInfo;
  auto parser =
      pdfium::MakeUnique<CXFA_FMParse>(L"; Just comment\n12", &errorInfo);
  std::unique_ptr<CXFA_FMFunctionDefinition> ast = parser->Parse();
  ASSERT(ast != nullptr);
  EXPECT_TRUE(errorInfo.message.IsEmpty());

  CFX_WideTextBuf buf;
  EXPECT_TRUE(ast->ToJavaScript(buf));
  EXPECT_EQ(L"(\n"
      L"function ()\n"
      L"{\n"
        L"var foxit_xfa_formcalc_runtime_func_return_value = null;\n"
        L"foxit_xfa_formcalc_runtime_func_return_value = 12;\n"
        L"return foxit_xfa_formcalc_runtime.get_fm_value("
          L"foxit_xfa_formcalc_runtime_func_return_value);\n"
      L"}\n"
      L").call(this);\n", buf.AsStringC());
}

TEST(CXFA_FMParseTest, Parse) {
  const wchar_t input[] = L"$ = Avg (-3, 5, -6, 12, -13);\n"
      L"$ = Avg (Table2..Row[*].Cell1);\n"
      L"\n"
      L"if ($ ne -1)then\n"
      L"  border.fill.color.value = \"255,64,64\";\n"
      L"else\n"
      L"  border.fill.color.value = \"20,170,13\";\n"
      L"endif\n"
      L"\n"
      L"$";

  CXFA_FMErrorInfo errorInfo;
  auto parser = pdfium::MakeUnique<CXFA_FMParse>(input, &errorInfo);
  std::unique_ptr<CXFA_FMFunctionDefinition> ast = parser->Parse();
  ASSERT(ast != nullptr);
  EXPECT_TRUE(errorInfo.message.IsEmpty());

  CFX_WideTextBuf buf;
  EXPECT_TRUE(ast->ToJavaScript(buf));
  EXPECT_EQ(
      L"(\nfunction ()\n{\n"
      L"var foxit_xfa_formcalc_runtime_func_return_value = null;\n"
      L"if (foxit_xfa_formcalc_runtime.is_fm_object(this))\n{\n"
        L"foxit_xfa_formcalc_runtime.assign_value_operator(this, "
          L"foxit_xfa_formcalc_runtime.Avg("
            L"foxit_xfa_formcalc_runtime.negative_operator(3), 5, "
              L"foxit_xfa_formcalc_runtime.negative_operator(6), 12, "
                L"foxit_xfa_formcalc_runtime.negative_operator(13)));\n"
      L"}\n"
      L"if (foxit_xfa_formcalc_runtime.is_fm_object(this))\n{\n"
        L"foxit_xfa_formcalc_runtime.assign_value_operator(this, "
          L"foxit_xfa_formcalc_runtime.Avg("
            L"foxit_xfa_formcalc_runtime.dot_accessor("
              L"foxit_xfa_formcalc_runtime.dotdot_accessor(Table2, \"Table2\", "
                L"\"Row\", 1), \"\", \"Cell1\", 0, 0)));\n"
      L"}\n"
      L"if (foxit_xfa_formcalc_runtime.get_fm_value("
        L"foxit_xfa_formcalc_runtime.notequality_operator(this, "
          L"foxit_xfa_formcalc_runtime.negative_operator(1))))\n{\n"
            L"if (foxit_xfa_formcalc_runtime.is_fm_object("
              L"foxit_xfa_formcalc_runtime.dot_accessor("
                L"foxit_xfa_formcalc_runtime.dot_accessor("
                  L"foxit_xfa_formcalc_runtime.dot_accessor(border, "
                    L"\"border\", \"fill\", 0, 0), \"\", \"color\", 0, 0), "
                    L"\"\", \"value\", 0, 0)))\n{\n"
                      L"foxit_xfa_formcalc_runtime.assign_value_operator("
                        L"foxit_xfa_formcalc_runtime.dot_accessor("
                          L"foxit_xfa_formcalc_runtime.dot_accessor("
                            L"foxit_xfa_formcalc_runtime.dot_accessor("
                              L"border, \"border\", \"fill\", 0, 0), \"\", "
                              L"\"color\", 0, 0), \"\", \"value\", 0, 0), "
                              L"\"255,64,64\");\n"
            L"}\n"
      L"}\nelse\n{\n"
        L"if (foxit_xfa_formcalc_runtime.is_fm_object("
          L"foxit_xfa_formcalc_runtime.dot_accessor("
            L"foxit_xfa_formcalc_runtime.dot_accessor("
              L"foxit_xfa_formcalc_runtime.dot_accessor("
                L"border, \"border\", \"fill\", 0, 0), \"\", \"color\", "
                L"0, 0), \"\", \"value\", 0, 0)))\n{\n"
                  L"foxit_xfa_formcalc_runtime.assign_value_operator("
                    L"foxit_xfa_formcalc_runtime.dot_accessor("
                      L"foxit_xfa_formcalc_runtime.dot_accessor("
                        L"foxit_xfa_formcalc_runtime.dot_accessor("
                          L"border, \"border\", \"fill\", 0, 0), \"\", "
                          L"\"color\", 0, 0), \"\", \"value\", 0, 0), "
                          L"\"20,170,13\");\n"
        L"}\n"
      L"}\n"
      L"foxit_xfa_formcalc_runtime_func_return_value = this;\n"
      L"return foxit_xfa_formcalc_runtime.get_fm_value("
        L"foxit_xfa_formcalc_runtime_func_return_value);\n"
      L"}\n).call(this);\n", buf.AsStringC());
}

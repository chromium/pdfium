// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <limits>

#include "core/fpdfapi/page/cpdf_psengine.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "third_party/base/cxx17_backports.h"

namespace {

float DoOperator0(CPDF_PSEngine* engine, PDF_PSOP op) {
  EXPECT_EQ(0u, engine->GetStackSize());
  engine->DoOperator(op);
  float ret = engine->Pop();
  EXPECT_EQ(0u, engine->GetStackSize());
  return ret;
}

float DoOperator1(CPDF_PSEngine* engine, float v1, PDF_PSOP op) {
  EXPECT_EQ(0u, engine->GetStackSize());
  engine->Push(v1);
  engine->DoOperator(op);
  float ret = engine->Pop();
  EXPECT_EQ(0u, engine->GetStackSize());
  return ret;
}

float DoOperator2(CPDF_PSEngine* engine, float v1, float v2, PDF_PSOP op) {
  EXPECT_EQ(0u, engine->GetStackSize());
  engine->Push(v1);
  engine->Push(v2);
  engine->DoOperator(op);
  float ret = engine->Pop();
  EXPECT_EQ(0u, engine->GetStackSize());
  return ret;
}

}  // namespace

TEST(CPDF_PSProc, AddOperator) {
  static const struct {
    const char* name;
    PDF_PSOP op;
  } kTestData[] = {
      {"add", PSOP_ADD},         {"sub", PSOP_SUB},
      {"mul", PSOP_MUL},         {"div", PSOP_DIV},
      {"idiv", PSOP_IDIV},       {"mod", PSOP_MOD},
      {"neg", PSOP_NEG},         {"abs", PSOP_ABS},
      {"ceiling", PSOP_CEILING}, {"floor", PSOP_FLOOR},
      {"round", PSOP_ROUND},     {"truncate", PSOP_TRUNCATE},
      {"sqrt", PSOP_SQRT},       {"sin", PSOP_SIN},
      {"cos", PSOP_COS},         {"atan", PSOP_ATAN},
      {"exp", PSOP_EXP},         {"ln", PSOP_LN},
      {"log", PSOP_LOG},         {"cvi", PSOP_CVI},
      {"cvr", PSOP_CVR},         {"eq", PSOP_EQ},
      {"ne", PSOP_NE},           {"gt", PSOP_GT},
      {"ge", PSOP_GE},           {"lt", PSOP_LT},
      {"le", PSOP_LE},           {"and", PSOP_AND},
      {"or", PSOP_OR},           {"xor", PSOP_XOR},
      {"not", PSOP_NOT},         {"bitshift", PSOP_BITSHIFT},
      {"true", PSOP_TRUE},       {"false", PSOP_FALSE},
      {"if", PSOP_IF},           {"ifelse", PSOP_IFELSE},
      {"pop", PSOP_POP},         {"exch", PSOP_EXCH},
      {"dup", PSOP_DUP},         {"copy", PSOP_COPY},
      {"index", PSOP_INDEX},     {"roll", PSOP_ROLL},
      {"55", PSOP_CONST},        {"123.4", PSOP_CONST},
      {"-5", PSOP_CONST},        {"invalid", PSOP_CONST},
  };

  CPDF_PSProc proc;
  EXPECT_EQ(0U, proc.num_operators());
  for (size_t i = 0; i < pdfium::size(kTestData); ++i) {
    ByteStringView word(kTestData[i].name);
    proc.AddOperatorForTesting(word);
    ASSERT_EQ(i + 1, proc.num_operators());
    const std::unique_ptr<CPDF_PSOP>& new_psop = proc.last_operator();
    ASSERT_TRUE(new_psop);
    PDF_PSOP new_op = new_psop->GetOp();
    EXPECT_EQ(kTestData[i].op, new_op);
    if (new_op == PSOP_CONST) {
      float fv = new_psop->GetFloatValue();
      if (word == "invalid")
        EXPECT_FLOAT_EQ(0, fv);
      else
        EXPECT_EQ(word, ByteString::FormatFloat(fv));
    }
  }
}

TEST(CPDF_PSEngine, Basic) {
  CPDF_PSEngine engine;

  EXPECT_FLOAT_EQ(300.0f, DoOperator2(&engine, 100, 200, PSOP_ADD));
  EXPECT_FLOAT_EQ(-50.0f, DoOperator2(&engine, 100, 150, PSOP_SUB));
  EXPECT_FLOAT_EQ(600.0f, DoOperator2(&engine, 5, 120, PSOP_MUL));
  EXPECT_FLOAT_EQ(1.5f, DoOperator2(&engine, 15, 10, PSOP_DIV));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 15, 10, PSOP_IDIV));
  EXPECT_FLOAT_EQ(5.0f, DoOperator2(&engine, 15, 10, PSOP_MOD));
  EXPECT_FLOAT_EQ(5.0f, DoOperator1(&engine, -5, PSOP_NEG));
  EXPECT_FLOAT_EQ(5.0f, DoOperator1(&engine, -5, PSOP_ABS));
}

TEST(CPDF_PSEngine, IDivByZero) {
  CPDF_PSEngine engine;

  // Integer divide by zero is defined as resulting in 0.
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 100, 0.0, PSOP_IDIV));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 100, 0.0, PSOP_MOD));
}

TEST(CPDF_PSEngine, Ceiling) {
  CPDF_PSEngine engine;

  // Smallest positive float value.
  float min_float = std::numeric_limits<float>::min();
  // Largest positive float value.
  float max_float = std::numeric_limits<float>::max();
  EXPECT_FLOAT_EQ(1.0f, DoOperator1(&engine, min_float, PSOP_CEILING));
  EXPECT_FLOAT_EQ(max_float, DoOperator1(&engine, max_float, PSOP_CEILING));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, -min_float, PSOP_CEILING));
  EXPECT_FLOAT_EQ(-max_float, DoOperator1(&engine, -max_float, PSOP_CEILING));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, -0.9f, PSOP_CEILING));
  EXPECT_FLOAT_EQ(1.0f, DoOperator1(&engine, 0.0000000001f, PSOP_CEILING));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0f, PSOP_CEILING));
  EXPECT_FLOAT_EQ(3.0f, DoOperator1(&engine, 2.3f, PSOP_CEILING));
  EXPECT_FLOAT_EQ(4.0f, DoOperator1(&engine, 3.8f, PSOP_CEILING));
  EXPECT_FLOAT_EQ(6.0f, DoOperator1(&engine, 5.5f, PSOP_CEILING));
  EXPECT_FLOAT_EQ(-2.0f, DoOperator1(&engine, -2.3f, PSOP_CEILING));
  EXPECT_FLOAT_EQ(-3.0f, DoOperator1(&engine, -3.8f, PSOP_CEILING));
  EXPECT_FLOAT_EQ(-5.0f, DoOperator1(&engine, -5.5f, PSOP_CEILING));
}

TEST(CPDF_PSEngine, Floor) {
  CPDF_PSEngine engine;

  // Smallest positive float value.
  float min_float = std::numeric_limits<float>::min();
  // Largest positive float value.
  float max_float = std::numeric_limits<float>::max();
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, min_float, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(max_float, DoOperator1(&engine, max_float, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(-1.0f, DoOperator1(&engine, -min_float, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(-max_float, DoOperator1(&engine, -max_float, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(5.0f, DoOperator1(&engine, 5.9f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(-4.0f, DoOperator1(&engine, -4.0000000001f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(-1.0f, DoOperator1(&engine, -0.9f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0000000001f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(2.0f, DoOperator1(&engine, 2.3f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(3.0f, DoOperator1(&engine, 3.8f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(5.0f, DoOperator1(&engine, 5.5f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(-3.0f, DoOperator1(&engine, -2.3f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(-4.0f, DoOperator1(&engine, -3.8f, PSOP_FLOOR));
  EXPECT_FLOAT_EQ(-6.0f, DoOperator1(&engine, -5.5f, PSOP_FLOOR));
}

TEST(CPDF_PSEngine, Round) {
  CPDF_PSEngine engine;

  EXPECT_FLOAT_EQ(6.0f, DoOperator1(&engine, 5.9f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(-4.0f, DoOperator1(&engine, -4.0000000001f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(-1.0f, DoOperator1(&engine, -0.9f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0000000001f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0f, PSOP_ROUND));
  // Smallest positive float value.
  float min_float = std::numeric_limits<float>::min();
  // Largest positive float value.
  float max_float = std::numeric_limits<float>::max();
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, min_float, PSOP_ROUND));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, -min_float, PSOP_ROUND));
  EXPECT_FLOAT_EQ(max_float, DoOperator1(&engine, max_float, PSOP_ROUND));
  EXPECT_FLOAT_EQ(-max_float, DoOperator1(&engine, -max_float, PSOP_ROUND));
  EXPECT_FLOAT_EQ(2.0f, DoOperator1(&engine, 2.3f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(4.0f, DoOperator1(&engine, 3.8f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(6.0f, DoOperator1(&engine, 5.5f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(-2.0f, DoOperator1(&engine, -2.3f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(-4.0f, DoOperator1(&engine, -3.8f, PSOP_ROUND));
  EXPECT_FLOAT_EQ(-5.0f, DoOperator1(&engine, -5.5f, PSOP_ROUND));
}

TEST(CPDF_PSEngine, Truncate) {
  CPDF_PSEngine engine;

  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, -0.9f, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0000000001f, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 0.0f, PSOP_TRUNCATE));
  // Smallest positive float value.
  float min_float = std::numeric_limits<float>::min();
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, min_float, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, -min_float, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(2.0f, DoOperator1(&engine, 2.3f, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(3.0f, DoOperator1(&engine, 3.8f, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(5.0f, DoOperator1(&engine, 5.5f, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(-2.0f, DoOperator1(&engine, -2.3f, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(-3.0f, DoOperator1(&engine, -3.8f, PSOP_TRUNCATE));
  EXPECT_FLOAT_EQ(-5.0f, DoOperator1(&engine, -5.5f, PSOP_TRUNCATE));

  // Truncate does not behave according to the PostScript Language Reference for
  // values beyond the range of integers. This seems to match Acrobat's
  // behavior. See https://crbug.com/pdfium/1314.
  float max_int = static_cast<float>(std::numeric_limits<int>::max());
  EXPECT_FLOAT_EQ(-max_int,
                  DoOperator1(&engine, max_int * -1.5f, PSOP_TRUNCATE));
}

TEST(CPDF_PSEngine, Comparisons) {
  CPDF_PSEngine engine;

  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_EQ));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_EQ));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 255.0f, 1.0f, PSOP_EQ));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, -1.0f, 0.0f, PSOP_EQ));

  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_NE));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_NE));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 255.0f, 1.0f, PSOP_NE));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, -1.0f, 0.0f, PSOP_NE));

  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_GT));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_GT));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 255.0f, 1.0f, PSOP_GT));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, -1.0f, 0.0f, PSOP_GT));

  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_GE));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_GE));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 255.0f, 1.0f, PSOP_GE));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, -1.0f, 0.0f, PSOP_GE));

  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_LT));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_LT));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 255.0f, 1.0f, PSOP_LT));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, -1.0f, 0.0f, PSOP_LT));

  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_LE));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_LE));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 255.0f, 1.0f, PSOP_LE));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, -1.0f, 0.0f, PSOP_LE));
}

TEST(CPDF_PSEngine, Logic) {
  CPDF_PSEngine engine;

  EXPECT_FLOAT_EQ(1.0f, DoOperator0(&engine, PSOP_TRUE));
  EXPECT_FLOAT_EQ(0.0f, DoOperator0(&engine, PSOP_FALSE));

  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_AND));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_AND));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 1.0f, 0.0f, PSOP_AND));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 1.0f, 1.0f, PSOP_AND));

  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_OR));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_OR));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 1.0f, 0.0f, PSOP_OR));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 1.0f, 1.0f, PSOP_OR));

  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 0.0f, 0.0f, PSOP_XOR));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 0.0f, 1.0f, PSOP_XOR));
  EXPECT_FLOAT_EQ(1.0f, DoOperator2(&engine, 1.0f, 0.0f, PSOP_XOR));
  EXPECT_FLOAT_EQ(0.0f, DoOperator2(&engine, 1.0f, 1.0f, PSOP_XOR));

  EXPECT_FLOAT_EQ(1.0f, DoOperator1(&engine, 0.0f, PSOP_NOT));
  EXPECT_FLOAT_EQ(0.0f, DoOperator1(&engine, 1.0f, PSOP_NOT));
}

TEST(CPDF_PSEngine, MathFunctions) {
  CPDF_PSEngine engine;

  EXPECT_FLOAT_EQ(1.4142135f, DoOperator1(&engine, 2.0f, PSOP_SQRT));
  EXPECT_FLOAT_EQ(0.8660254f, DoOperator1(&engine, 60.0f, PSOP_SIN));
  EXPECT_FLOAT_EQ(0.5f, DoOperator1(&engine, 60.0f, PSOP_COS));
  EXPECT_FLOAT_EQ(45.0f, DoOperator2(&engine, 1.0f, 1.0f, PSOP_ATAN));
  EXPECT_FLOAT_EQ(1000.0f, DoOperator2(&engine, 10.0f, 3.0f, PSOP_EXP));
  EXPECT_FLOAT_EQ(3.0f, DoOperator1(&engine, 1000.0f, PSOP_LOG));
  EXPECT_FLOAT_EQ(2.302585f, DoOperator1(&engine, 10.0f, PSOP_LN));
}

// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/page/cpdf_function.h"

#include "core/fpdfapi/parser/cpdf_array.h"
#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_number.h"
#include "core/fxcrt/retain_ptr.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CPDFFunction, BadFunctionType) {
  auto pDict = pdfium::MakeRetain<CPDF_Dictionary>();
  pDict->SetNewFor<CPDF_Number>("FunctionType", -2);
  EXPECT_FALSE(CPDF_Function::Load(pDict));

  pDict->SetNewFor<CPDF_Number>("FunctionType", 5);
  EXPECT_FALSE(CPDF_Function::Load(pDict));
}

TEST(CPDFFunction, NoDomain) {
  auto pDict = pdfium::MakeRetain<CPDF_Dictionary>();
  pDict->SetNewFor<CPDF_Number>("FunctionType", 0);
  EXPECT_FALSE(CPDF_Function::Load(pDict));
}

TEST(CPDFFunction, EmptyDomain) {
  auto pDict = pdfium::MakeRetain<CPDF_Dictionary>();
  pDict->SetNewFor<CPDF_Number>("FunctionType", 0);
  pDict->SetNewFor<CPDF_Array>("Domain");
  EXPECT_FALSE(CPDF_Function::Load(pDict));
}

TEST(CPDFFunction, NoRange) {
  auto pDict = pdfium::MakeRetain<CPDF_Dictionary>();
  pDict->SetNewFor<CPDF_Number>("FunctionType", 0);

  auto pArray = pDict->SetNewFor<CPDF_Array>("Domain");
  pArray->AppendNew<CPDF_Number>(0);
  pArray->AppendNew<CPDF_Number>(10);
  EXPECT_FALSE(CPDF_Function::Load(pDict));
}

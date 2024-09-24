// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/formfiller/cffl_combobox.h"

#include "fpdfsdk/pwl/cpwl_combo_box_embeddertest.h"
#include "testing/gtest/include/gtest/gtest.h"

class CFFLComboBoxEmbedderTest : public CPWLComboBoxEmbedderTest {};

TEST_F(CFFLComboBoxEmbedderTest, GetActionData) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());
  {
    CFFL_FieldAction result;
    GetCFFLFormField()->GetActionData(GetPageView(), CPDF_AAction::kKeyStroke,
                                      result);
    EXPECT_EQ(L"Banana", result.sValue);
    EXPECT_EQ(L"Banana", result.sChangeEx);
  }
  {
    CFFL_FieldAction result;
    GetCFFLFormField()->GetActionData(GetPageView(), CPDF_AAction::kValidate,
                                      result);
    EXPECT_EQ(L"Banana", result.sValue);
    EXPECT_EQ(L"", result.sChangeEx);
  }
  {
    CFFL_FieldAction result;
    GetCFFLFormField()->GetActionData(GetPageView(), CPDF_AAction::kGetFocus,
                                      result);
    EXPECT_EQ(L"Banana", result.sValue);
    EXPECT_EQ(L"", result.sChangeEx);
  }
}

TEST_F(CFFLComboBoxEmbedderTest, SetActionData) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());
  CFFL_FieldAction input_fa;
  input_fa.nSelStart = 2;
  input_fa.nSelEnd = 4;
  input_fa.sChange = L"Hamster";
  GetCFFLFormField()->SetActionData(GetPageView(), CPDF_AAction::kKeyStroke,
                                    input_fa);

  CFFL_FieldAction output_fa;
  GetCFFLFormField()->GetActionData(GetPageView(), CPDF_AAction::kKeyStroke,
                                    output_fa);
  EXPECT_EQ(L"BaHamsterna", output_fa.sValue);
}

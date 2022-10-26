// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/formfiller/cffl_combobox.h"

#include "fpdfsdk/pwl/cpwl_combo_box_embeddertest.h"
#include "testing/gtest/include/gtest/gtest.h"

class CFFLComboBoxEmbedderTest : public CPWLComboBoxEmbedderTest {};

TEST_F(CFFLComboBoxEmbedderTest, GetActionData) {
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

// Copyright 2022 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/formfiller/cffl_combobox.h"

#include "fpdfsdk/pwl/cpwl_combo_box_embeddertest.h"
#include "testing/gtest/include/gtest/gtest.h"

class CFFLComboBoxEmbedderTest : public CPWLComboBoxEmbedderTest {};

TEST_F(CFFLComboBoxEmbedderTest, ExportText) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());
  auto* pComboBox = static_cast<CFFL_ComboBox*>(GetCFFLFormField());
  ASSERT_TRUE(pComboBox);
  EXPECT_EQ(L"Banana", pComboBox->GetSelectExportText());
}

// Copyright 2022 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef FPDFSDK_PWL_CPWL_COMBO_BOX_EMBEDDERTEST_H_
#define FPDFSDK_PWL_CPWL_COMBO_BOX_EMBEDDERTEST_H_

#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CFFL_FormField;
class CPDFSDK_FormFillEnvironment;
class CPDFSDK_PageView;
class CPDFSDK_Widget;
class CPWL_ComboBox;

class CPWLComboBoxEmbedderTest : public EmbedderTest {
 protected:
  void SetUp() override;

  ScopedPage CreateAndInitializeFormComboboxPDF();
  void FormFillerAndWindowSetup(CPDFSDK_Widget* pAnnotCombobox);
  void TypeTextIntoTextField(int num_chars);
  CPWL_ComboBox* GetCPWLComboBox() const { return combo_box_; }
  CFFL_FormField* GetCFFLFormField() const { return form_field_; }
  CPDFSDK_Widget* GetCPDFSDKAnnotNormal() const { return annot_normal_; }
  CPDFSDK_Widget* GetCPDFSDKAnnotUserEditable() const {
    return annot_editable_;
  }
  CPDFSDK_FormFillEnvironment* GetCPDFSDKFormFillEnv() const {
    return form_fill_env_;
  }
  CPDFSDK_PageView* GetPageView() const { return page_view_; }

 private:
  CPWL_ComboBox* combo_box_ = nullptr;
  CFFL_FormField* form_field_ = nullptr;
  CPDFSDK_Widget* annot_normal_ = nullptr;
  CPDFSDK_Widget* annot_editable_ = nullptr;
  CPDFSDK_FormFillEnvironment* form_fill_env_ = nullptr;
  CPDFSDK_PageView* page_view_ = nullptr;
};

#endif  // FPDFSDK_PWL_CPWL_COMBO_BOX_EMBEDDERTEST_H_

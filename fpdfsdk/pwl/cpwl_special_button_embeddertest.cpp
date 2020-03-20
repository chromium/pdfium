// Copyright 2020 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/pwl/cpwl_special_button.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "testing/embedder_test.h"

class CPWLSpecialButtonEmbedderTest : public EmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    CreateAndInitializeFormPDF();
  }

  void TearDown() override {
    UnloadPage(page_);
    EmbedderTest::TearDown();
  }

  void CreateAndInitializeFormPDF() {
    ASSERT_TRUE(OpenDocument("click_form.pdf"));

    page_ = LoadPage(0);
    ASSERT_TRUE(page_);

    formfill_env_ = CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    CPDFSDK_AnnotIterator it(formfill_env_->GetPageViewAtIndex(0),
                             CPDF_Annot::Subtype::WIDGET);

    // Read only check box.
    annot_readonly_checkbox_ = it.GetFirstAnnot();
    ASSERT_TRUE(annot_readonly_checkbox_);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET,
              annot_readonly_checkbox_->GetAnnotSubtype());

    // Check box.
    annot_checkbox_ = it.GetNextAnnot(annot_readonly_checkbox_);
    ASSERT_TRUE(annot_checkbox_);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET, annot_checkbox_->GetAnnotSubtype());

    // Read only radio button.
    annot_readonly_radiobutton_ = it.GetNextAnnot(annot_checkbox_);
    ASSERT_TRUE(annot_readonly_radiobutton_);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET,
              annot_readonly_radiobutton_->GetAnnotSubtype());

    // Tabbing three times from read only radio button to unselected normal
    // radio button.
    annot_radiobutton_ = annot_readonly_radiobutton_;
    ASSERT_TRUE(annot_radiobutton_);
    for (int i = 0; i < 3; i++) {
      annot_radiobutton_ = it.GetNextAnnot(annot_radiobutton_);
      ASSERT_TRUE(annot_radiobutton_);
    }

    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET,
              annot_radiobutton_->GetAnnotSubtype());
  }

  void FormFillerAndWindowSetup(CPDFSDK_Annot* annot) {
    CFFL_InteractiveFormFiller* interactive_formfiller =
        formfill_env_->GetInteractiveFormFiller();
    {
      ObservedPtr<CPDFSDK_Annot> observed(annot);
      EXPECT_TRUE(interactive_formfiller->OnSetFocus(&observed, 0));
    }

    form_filler_ = interactive_formfiller->GetFormFillerForTesting(annot);
    ASSERT_TRUE(form_filler_);

    window_ =
        form_filler_->GetPWLWindow(formfill_env_->GetPageViewAtIndex(0), true);
    ASSERT_TRUE(window_);
  }

  CPDFSDK_Annot* GetCPDFSDKAnnotCheckBox() const { return annot_checkbox_; }
  CPDFSDK_Annot* GetCPDFSDKAnnotReadOnlyCheckBox() const {
    return annot_readonly_checkbox_;
  }
  CPDFSDK_Annot* GetCPDFSDKAnnotRadioButton() const {
    return annot_radiobutton_;
  }
  CPDFSDK_Annot* GetCPDFSDKAnnotReadOnlyRadioButton() const {
    return annot_readonly_radiobutton_;
  }
  CPDFSDK_FormFillEnvironment* GetCPDFSDKFormFillEnv() const {
    return formfill_env_;
  }
  CPWL_Wnd* GetWindow() const { return window_; }

 private:
  FPDF_PAGE page_;
  CFFL_FormFiller* form_filler_;
  CPDFSDK_Annot* annot_checkbox_;
  CPDFSDK_Annot* annot_readonly_checkbox_;
  CPDFSDK_Annot* annot_radiobutton_;
  CPDFSDK_Annot* annot_readonly_radiobutton_;
  CPDFSDK_FormFillEnvironment* formfill_env_;
  CPWL_Wnd* window_;
};

TEST_F(CPWLSpecialButtonEmbedderTest, EnterOnReadOnlyCheckBox) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotReadOnlyCheckBox());
  CPWL_CheckBox* check_box = static_cast<CPWL_CheckBox*>(GetWindow());
  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKAnnotReadOnlyCheckBox(), '\r', 0));
  // The check box is checked by default. Since it is a read only checkbox,
  // clicking Enter shouldn't change its state.
  // TODO(http://crbug.com/pdfium/1431) : Change this to EXPECT_TRUE
  // as part of the fix.
  EXPECT_FALSE(check_box->IsChecked());
}

TEST_F(CPWLSpecialButtonEmbedderTest, EnterOnCheckBox) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCheckBox());
  CPWL_CheckBox* check_box = static_cast<CPWL_CheckBox*>(GetWindow());
  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKAnnotCheckBox(), '\r', 0));
  EXPECT_TRUE(check_box->IsChecked());

  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKAnnotCheckBox(), '\r', 0));
  EXPECT_FALSE(check_box->IsChecked());
}

TEST_F(CPWLSpecialButtonEmbedderTest, EnterOnReadOnlyRadioButton) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotReadOnlyRadioButton());
  CPWL_RadioButton* radio_button = static_cast<CPWL_RadioButton*>(GetWindow());
  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKAnnotReadOnlyRadioButton(), '\r', 0));
  // TODO(http://crbug.com/pdfium/1431) : Change this to EXPECT_FALSE
  // as part of the fix.
  EXPECT_TRUE(radio_button->IsChecked());
}

TEST_F(CPWLSpecialButtonEmbedderTest, EnterOnRadioButton) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotRadioButton());
  CPWL_RadioButton* radio_button = static_cast<CPWL_RadioButton*>(GetWindow());
  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKAnnotRadioButton(), '\r', 0));
  EXPECT_TRUE(radio_button->IsChecked());
}

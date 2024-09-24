// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "fpdfsdk/pwl/cpwl_special_button.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "testing/embedder_test.h"

class CPWLSpecialButtonEmbedderTest : public EmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    ASSERT_TRUE(OpenDocument("click_form.pdf"));
  }

  void TearDown() override {
    EmbedderTest::TearDown();
  }

  void CreateAndInitializeFormPDF() {
    formfill_env_ = CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    CPDFSDK_AnnotIterator it(formfill_env_->GetPageViewAtIndex(0),
                             {CPDF_Annot::Subtype::WIDGET});

    // Read only check box.
    widget_readonly_checkbox_ = ToCPDFSDKWidget(it.GetFirstAnnot());
    ASSERT_TRUE(widget_readonly_checkbox_);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET,
              widget_readonly_checkbox_->GetAnnotSubtype());

    // Check box.
    widget_checkbox_ =
        ToCPDFSDKWidget(it.GetNextAnnot(widget_readonly_checkbox_));
    ASSERT_TRUE(widget_checkbox_);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET, widget_checkbox_->GetAnnotSubtype());

    // Read only radio button.
    widget_readonly_radiobutton_ =
        ToCPDFSDKWidget(it.GetNextAnnot(widget_checkbox_));
    ASSERT_TRUE(widget_readonly_radiobutton_);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET,
              widget_readonly_radiobutton_->GetAnnotSubtype());

    // Tabbing three times from read only radio button to unselected normal
    // radio button.
    widget_radiobutton_ = widget_readonly_radiobutton_;
    ASSERT_TRUE(widget_radiobutton_);
    for (int i = 0; i < 3; i++) {
      widget_radiobutton_ =
          ToCPDFSDKWidget(it.GetNextAnnot(widget_radiobutton_));
      ASSERT_TRUE(widget_radiobutton_);
    }

    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET,
              widget_radiobutton_->GetAnnotSubtype());
  }

  void FormFillerAndWindowSetup(CPDFSDK_Widget* widget) {
    CFFL_InteractiveFormFiller* interactive_formfiller =
        formfill_env_->GetInteractiveFormFiller();
    {
      ObservedPtr<CPDFSDK_Widget> observed(widget);
      EXPECT_TRUE(interactive_formfiller->OnSetFocus(observed, {}));
    }

    form_filler_ = interactive_formfiller->GetFormFieldForTesting(widget);
    ASSERT_TRUE(form_filler_);

    window_ = form_filler_->CreateOrUpdatePWLWindow(
        formfill_env_->GetPageViewAtIndex(0));
    ASSERT_TRUE(window_);
  }

  CPDFSDK_Widget* GetCPDFSDKWidgetCheckBox() const { return widget_checkbox_; }
  CPDFSDK_Widget* GetCPDFSDKWidgetReadOnlyCheckBox() const {
    return widget_readonly_checkbox_;
  }
  CPDFSDK_Widget* GetCPDFSDKWidgetRadioButton() const {
    return widget_radiobutton_;
  }
  CPDFSDK_Widget* GetCPDFSDKWidgetReadOnlyRadioButton() const {
    return widget_readonly_radiobutton_;
  }
  CPDFSDK_FormFillEnvironment* GetCPDFSDKFormFillEnv() const {
    return formfill_env_;
  }
  CPWL_Wnd* GetWindow() const { return window_; }

 private:
  CFFL_FormField* form_filler_;
  CPDFSDK_Widget* widget_checkbox_;
  CPDFSDK_Widget* widget_readonly_checkbox_;
  CPDFSDK_Widget* widget_radiobutton_;
  CPDFSDK_Widget* widget_readonly_radiobutton_;
  CPDFSDK_FormFillEnvironment* formfill_env_;
  CPWL_Wnd* window_;
};

TEST_F(CPWLSpecialButtonEmbedderTest, EnterOnReadOnlyCheckBox) {
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  CreateAndInitializeFormPDF();

  FormFillerAndWindowSetup(GetCPDFSDKWidgetReadOnlyCheckBox());
  CPWL_CheckBox* check_box = static_cast<CPWL_CheckBox*>(GetWindow());
  EXPECT_TRUE(check_box->IsChecked());
  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKWidgetReadOnlyCheckBox(), '\r', {}));
  EXPECT_TRUE(check_box->IsChecked());
}

TEST_F(CPWLSpecialButtonEmbedderTest, EnterOnCheckBox) {
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  CreateAndInitializeFormPDF();

  FormFillerAndWindowSetup(GetCPDFSDKWidgetCheckBox());
  CPWL_CheckBox* check_box = static_cast<CPWL_CheckBox*>(GetWindow());
  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKWidgetCheckBox(), '\r', {}));
  EXPECT_TRUE(check_box->IsChecked());

  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKWidgetCheckBox(), '\r', {}));
  EXPECT_FALSE(check_box->IsChecked());
}

TEST_F(CPWLSpecialButtonEmbedderTest, EnterOnReadOnlyRadioButton) {
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  CreateAndInitializeFormPDF();

  FormFillerAndWindowSetup(GetCPDFSDKWidgetReadOnlyRadioButton());
  CPWL_RadioButton* radio_button = static_cast<CPWL_RadioButton*>(GetWindow());
  EXPECT_FALSE(radio_button->IsChecked());
  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKWidgetReadOnlyRadioButton(), '\r', {}));
  EXPECT_FALSE(radio_button->IsChecked());
}

TEST_F(CPWLSpecialButtonEmbedderTest, EnterOnRadioButton) {
  ScopedEmbedderTestPage page = LoadScopedPage(0);
  ASSERT_TRUE(page);
  CreateAndInitializeFormPDF();

  FormFillerAndWindowSetup(GetCPDFSDKWidgetRadioButton());
  CPWL_RadioButton* radio_button = static_cast<CPWL_RadioButton*>(GetWindow());
  EXPECT_TRUE(GetCPDFSDKFormFillEnv()->GetInteractiveFormFiller()->OnChar(
      GetCPDFSDKWidgetRadioButton(), '\r', {}));
  EXPECT_TRUE(radio_button->IsChecked());
}

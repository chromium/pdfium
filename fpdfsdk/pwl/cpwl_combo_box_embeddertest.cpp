// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/pwl/cpwl_combo_box_embeddertest.h"

#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "fpdfsdk/pwl/cpwl_combo_box.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "public/fpdf_fwlevent.h"
#include "testing/gtest/include/gtest/gtest.h"

void CPWLComboBoxEmbedderTest::SetUp() {
  EmbedderTest::SetUp();
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
}

CPWLComboBoxEmbedderTest::ScopedPage
CPWLComboBoxEmbedderTest::CreateAndInitializeFormComboboxPDF() {
  ScopedPage page = LoadScopedPage(0);
  if (!page) {
    ADD_FAILURE();
    return ScopedPage();
  }
  form_fill_env_ = CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
  page_view_ = form_fill_env_->GetPageViewAtIndex(0);
  CPDFSDK_AnnotIterator iter(page_view_, {CPDF_Annot::Subtype::WIDGET});

  // User editable combobox.
  annot_editable_ = ToCPDFSDKWidget(iter.GetFirstAnnot());
  if (!annot_editable_) {
    ADD_FAILURE();
    return ScopedPage();
  }

  // Normal combobox with pre-selected value.
  annot_normal_ = ToCPDFSDKWidget(iter.GetNextAnnot(annot_editable_));
  if (!annot_editable_) {
    ADD_FAILURE();
    return ScopedPage();
  }

  // Read-only combobox.
  CPDFSDK_Annot* pAnnotReadOnly = iter.GetNextAnnot(annot_normal_);
  CPDFSDK_Annot* pLastAnnot = iter.GetLastAnnot();
  if (pAnnotReadOnly != pLastAnnot) {
    ADD_FAILURE();
    return ScopedPage();
  }
  return page;
}

void CPWLComboBoxEmbedderTest::FormFillerAndWindowSetup(
    CPDFSDK_Widget* pAnnotCombobox) {
  CFFL_InteractiveFormFiller* pInteractiveFormFiller =
      form_fill_env_->GetInteractiveFormFiller();
  {
    ObservedPtr<CPDFSDK_Widget> pObserved(pAnnotCombobox);
    EXPECT_TRUE(pInteractiveFormFiller->OnSetFocus(pObserved, {}));
  }

  form_field_ = pInteractiveFormFiller->GetFormFieldForTesting(pAnnotCombobox);
  ASSERT_TRUE(form_field_);

  CPWL_Wnd* pWindow =
      form_field_->GetPWLWindow(form_fill_env_->GetPageViewAtIndex(0));
  ASSERT_TRUE(pWindow);
  combo_box_ = static_cast<CPWL_ComboBox*>(pWindow);
}

void CPWLComboBoxEmbedderTest::TypeTextIntoTextField(int num_chars) {
  // Type text starting with 'A' to as many chars as specified by |num_chars|.
  for (int i = 0; i < num_chars; ++i) {
    EXPECT_TRUE(
        GetCFFLFormField()->OnChar(GetCPDFSDKAnnotUserEditable(), i + 'A', {}));
  }
}

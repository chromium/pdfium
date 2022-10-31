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
  CreateAndInitializeFormComboboxPDF();
}

void CPWLComboBoxEmbedderTest::TearDown() {
  UnloadPage(GetPage());
  EmbedderTest::TearDown();
}

void CPWLComboBoxEmbedderTest::CreateAndInitializeFormComboboxPDF() {
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  m_page = LoadPage(0);
  ASSERT_TRUE(m_page);

  m_pFormFillEnv = CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
  m_pPageView = m_pFormFillEnv->GetPageViewAtIndex(0);
  CPDFSDK_AnnotIterator iter(m_pPageView, {CPDF_Annot::Subtype::WIDGET});

  // User editable combobox.
  m_pAnnotEditable = ToCPDFSDKWidget(iter.GetFirstAnnot());
  ASSERT_TRUE(m_pAnnotEditable);

  // Normal combobox with pre-selected value.
  m_pAnnotNormal = ToCPDFSDKWidget(iter.GetNextAnnot(m_pAnnotEditable));
  ASSERT_TRUE(m_pAnnotNormal);

  // Read-only combobox.
  CPDFSDK_Annot* pAnnotReadOnly = iter.GetNextAnnot(m_pAnnotNormal);
  CPDFSDK_Annot* pLastAnnot = iter.GetLastAnnot();
  ASSERT_EQ(pAnnotReadOnly, pLastAnnot);
}

void CPWLComboBoxEmbedderTest::FormFillerAndWindowSetup(
    CPDFSDK_Widget* pAnnotCombobox) {
  CFFL_InteractiveFormFiller* pInteractiveFormFiller =
      m_pFormFillEnv->GetInteractiveFormFiller();
  {
    ObservedPtr<CPDFSDK_Widget> pObserved(pAnnotCombobox);
    EXPECT_TRUE(pInteractiveFormFiller->OnSetFocus(pObserved, {}));
  }

  m_pFormField = pInteractiveFormFiller->GetFormFieldForTesting(pAnnotCombobox);
  ASSERT_TRUE(m_pFormField);

  CPWL_Wnd* pWindow =
      m_pFormField->GetPWLWindow(m_pFormFillEnv->GetPageViewAtIndex(0));
  ASSERT_TRUE(pWindow);
  m_pComboBox = static_cast<CPWL_ComboBox*>(pWindow);
}

void CPWLComboBoxEmbedderTest::TypeTextIntoTextField(int num_chars) {
  // Type text starting with 'A' to as many chars as specified by |num_chars|.
  for (int i = 0; i < num_chars; ++i) {
    EXPECT_TRUE(
        GetCFFLFormField()->OnChar(GetCPDFSDKAnnotUserEditable(), i + 'A', {}));
  }
}

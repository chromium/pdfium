// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/cba_annotiterator.h"
#include "fpdfsdk/cpdfsdk_annot.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/formfiller/cffl_formfiller.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "fpdfsdk/pdfwindow/cpwl_combo_box.h"
#include "fpdfsdk/pdfwindow/cpwl_wnd.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPWLComboBoxEditEmbeddertest : public EmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    CreateAndInitializeFormComboboxPDF();
  }

  void TearDown() override {
    UnloadPage(GetPage());
    EmbedderTest::TearDown();
  }

  void CreateAndInitializeFormComboboxPDF() {
    EXPECT_TRUE(OpenDocument("combobox_form.pdf"));
    m_page = LoadPage(0);
    ASSERT_TRUE(m_page);

    m_pFormFillEnv = static_cast<CPDFSDK_FormFillEnvironment*>(form_handle());
    CBA_AnnotIterator iter(m_pFormFillEnv->GetPageView(0),
                           CPDF_Annot::Subtype::WIDGET);

    // User editable combobox.
    m_pAnnotEditable = iter.GetFirstAnnot();
    ASSERT_TRUE(m_pAnnotEditable);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET, m_pAnnotEditable->GetAnnotSubtype());

    // Normal combobox with pre-selected value.
    m_pAnnotNormal = iter.GetNextAnnot(m_pAnnotEditable);
    ASSERT_TRUE(m_pAnnotNormal);
    ASSERT_EQ(CPDF_Annot::Subtype::WIDGET, m_pAnnotNormal->GetAnnotSubtype());

    // Read-only combobox.
    CPDFSDK_Annot* pAnnotReadOnly = iter.GetNextAnnot(m_pAnnotNormal);
    CPDFSDK_Annot* pLastAnnot = iter.GetLastAnnot();
    ASSERT_EQ(pAnnotReadOnly, pLastAnnot);
  }

  void FormFillerAndWindowSetup(CPDFSDK_Annot* pAnnotCombobox) {
    CFFL_InteractiveFormFiller* pInteractiveFormFiller =
        m_pFormFillEnv->GetInteractiveFormFiller();
    {
      CPDFSDK_Annot::ObservedPtr pObserved(pAnnotCombobox);
      EXPECT_TRUE(pInteractiveFormFiller->OnSetFocus(&pObserved, 0));
    }

    m_pFormFiller =
        pInteractiveFormFiller->GetFormFiller(pAnnotCombobox, false);
    ASSERT_TRUE(m_pFormFiller);

    CPWL_Wnd* pWindow =
        m_pFormFiller->GetPDFWindow(m_pFormFillEnv->GetPageView(0), false);
    ASSERT_TRUE(pWindow);
    ASSERT_EQ("CPWL_ComboBox", pWindow->GetClassName());
    m_pComboBox = static_cast<CPWL_ComboBox*>(pWindow);
  }

  FPDF_PAGE GetPage() const { return m_page; }
  CPWL_ComboBox* GetCPWLComboBox() const { return m_pComboBox; }
  CFFL_FormFiller* GetCFFLFormFiller() const { return m_pFormFiller; }
  CPDFSDK_Annot* GetCPDFSDKAnnotNormal() const { return m_pAnnotNormal; }
  CPDFSDK_Annot* GetCPDFSDKAnnotUserEditable() const {
    return m_pAnnotEditable;
  }
  CPDFSDK_FormFillEnvironment* GetCPDFSDKFormFillEnv() const {
    return m_pFormFillEnv;
  }

 private:
  FPDF_PAGE m_page;
  CPWL_ComboBox* m_pComboBox;
  CFFL_FormFiller* m_pFormFiller;
  CPDFSDK_Annot* m_pAnnotNormal;
  CPDFSDK_Annot* m_pAnnotEditable;
  CPDFSDK_FormFillEnvironment* m_pFormFillEnv;
};

TEST_F(CPWLComboBoxEditEmbeddertest, GetSelectedTextEmptyAndBasicNormal) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());

  // Automatically pre-filled with "Banana".
  EXPECT_FALSE(GetCPWLComboBox()->GetText().IsEmpty());
  EXPECT_STREQ(L"Banana", GetCPWLComboBox()->GetText().c_str());

  // Check that selection is intially empty, then select entire word.
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetSelectText();
  EXPECT_STREQ(L"Banana", GetCPWLComboBox()->GetSelectedText().c_str());

  // Select other options.
  GetCPWLComboBox()->SetSelect(0);
  EXPECT_STREQ(L"Apple", GetCPWLComboBox()->GetSelectedText().c_str());
  GetCPWLComboBox()->SetSelect(2);
  EXPECT_STREQ(L"Cherry", GetCPWLComboBox()->GetSelectedText().c_str());

  // Verify that combobox text cannot be edited.
  EXPECT_FALSE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnotNormal(), 'a', 0));
}

TEST_F(CPWLComboBoxEditEmbeddertest, GetSelectedTextFragmentsNormal) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());
  EXPECT_STREQ(L"Banana", GetCPWLComboBox()->GetText().c_str());

  GetCPWLComboBox()->SetEditSel(0, 0);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSel(0, 1);
  EXPECT_STREQ(L"B", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSel(0, -1);
  EXPECT_STREQ(L"Banana", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSel(-8, -1);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSel(4, 1);
  EXPECT_STREQ(L"ana", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSel(1, 4);
  EXPECT_STREQ(L"ana", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSel(5, 6);
  EXPECT_STREQ(L"a", GetCPWLComboBox()->GetSelectedText().c_str());
}

TEST_F(CPWLComboBoxEditEmbeddertest, GetSelectedTextEmptyAndBasicEditable) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  EXPECT_TRUE(GetCPWLComboBox()->GetText().IsEmpty());

  // Check selection is intially empty, then select a provided option.
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetSelect(0);
  GetCPWLComboBox()->SetSelectText();
  EXPECT_STREQ(L"Foo", GetCPWLComboBox()->GetSelectedText().c_str());

  // Select another option and then select last char of that option.
  GetCPWLComboBox()->SetSelect(1);
  EXPECT_STREQ(L"Bar", GetCPWLComboBox()->GetSelectedText().c_str());
  GetCPWLComboBox()->SetEditSel(2, 3);
  EXPECT_STREQ(L"r", GetCPWLComboBox()->GetSelectedText().c_str());

  // Type into editable combobox text field and select new text.
  EXPECT_TRUE(
      GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnotUserEditable(), 'a', 0));
  EXPECT_TRUE(
      GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnotUserEditable(), 'b', 0));
  EXPECT_TRUE(
      GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnotUserEditable(), 'c', 0));

  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetEditSel(0, 5);
  EXPECT_STREQ(L"Baabc", GetCPWLComboBox()->GetSelectedText().c_str());
}

TEST_F(CPWLComboBoxEditEmbeddertest, GetSelectedTextFragmentsEditable) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  for (int i = 0; i < 50; ++i) {
    EXPECT_TRUE(
        GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnotUserEditable(), i + 'A', 0));
  }

  GetCPWLComboBox()->SetEditSel(0, 0);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSel(0, 1);
  EXPECT_STREQ(L"A", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSel(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSel(-8, -1);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSel(23, 12);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSel(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSel(49, 50);
  EXPECT_STREQ(L"r", GetCPWLComboBox()->GetSelectedText().c_str());
}

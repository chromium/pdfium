// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/pwl/cpwl_combo_box_embeddertest.h"

#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "fpdfsdk/pwl/cpwl_combo_box.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "public/fpdf_fwlevent.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPWLComboBoxEditEmbedderTest : public CPWLComboBoxEmbedderTest {};

TEST_F(CPWLComboBoxEditEmbedderTest, GetSelectedTextEmptyAndBasicNormal) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());

  // Automatically pre-filled with "Banana".
  EXPECT_FALSE(GetCPWLComboBox()->GetText().IsEmpty());
  EXPECT_EQ(L"Banana", GetCPWLComboBox()->GetText());

  // Check that selection is initially empty, then select entire word.
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetSelectText();
  EXPECT_EQ(L"Banana", GetCPWLComboBox()->GetSelectedText());

  // Select other options.
  GetCPWLComboBox()->SetSelect(0);
  EXPECT_EQ(L"Apple", GetCPWLComboBox()->GetSelectedText());
  GetCPWLComboBox()->SetSelect(2);
  EXPECT_EQ(L"Cherry", GetCPWLComboBox()->GetSelectedText());

  // Verify that combobox text cannot be edited.
  EXPECT_FALSE(GetCFFLFormField()->OnChar(GetCPDFSDKAnnotNormal(), 'a', {}));
}

TEST_F(CPWLComboBoxEditEmbedderTest, GetSelectedTextFragmentsNormal) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());
  EXPECT_EQ(L"Banana", GetCPWLComboBox()->GetText());

  GetCPWLComboBox()->SetEditSelection(0, 0);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSelection(0, 1);
  EXPECT_EQ(L"B", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(0, -1);
  EXPECT_EQ(L"Banana", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(-8, -1);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSelection(4, 1);
  EXPECT_EQ(L"ana", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(1, 4);
  EXPECT_EQ(L"ana", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(5, 6);
  EXPECT_EQ(L"a", GetCPWLComboBox()->GetSelectedText());
}

TEST_F(CPWLComboBoxEditEmbedderTest, GetSelectedTextEmptyAndBasicEditable) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  EXPECT_TRUE(GetCPWLComboBox()->GetText().IsEmpty());

  // Check selection is initially empty, then select a provided option.
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetSelect(0);
  GetCPWLComboBox()->SetSelectText();
  EXPECT_EQ(L"Foo", GetCPWLComboBox()->GetSelectedText());

  // Select another option and then select last char of that option.
  GetCPWLComboBox()->SetSelect(1);
  EXPECT_EQ(L"Bar", GetCPWLComboBox()->GetSelectedText());
  GetCPWLComboBox()->SetEditSelection(2, 3);
  EXPECT_EQ(L"r", GetCPWLComboBox()->GetSelectedText());

  // Type into editable combobox text field and select new text.
  EXPECT_TRUE(
      GetCFFLFormField()->OnChar(GetCPDFSDKAnnotUserEditable(), 'a', {}));
  EXPECT_TRUE(
      GetCFFLFormField()->OnChar(GetCPDFSDKAnnotUserEditable(), 'b', {}));
  EXPECT_TRUE(
      GetCFFLFormField()->OnChar(GetCPDFSDKAnnotUserEditable(), 'c', {}));

  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetEditSelection(0, 5);
  EXPECT_EQ(L"Baabc", GetCPWLComboBox()->GetSelectedText());
}

TEST_F(CPWLComboBoxEditEmbedderTest, GetSelectedTextFragmentsEditable) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(0, 0);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSelection(0, 1);
  EXPECT_EQ(L"A", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(-8, -1);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSelection(23, 12);
  EXPECT_EQ(L"MNOPQRSTUVW", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(12, 23);
  EXPECT_EQ(L"MNOPQRSTUVW", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(49, 50);
  EXPECT_EQ(L"r", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(49, 55);
  EXPECT_EQ(L"r", GetCPWLComboBox()->GetSelectedText());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteEntireTextSelection) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_TRUE(GetCPWLComboBox()->GetText().IsEmpty());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteTextSelectionMiddle) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(12, 23);
  EXPECT_EQ(L"MNOPQRSTUVW", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteTextSelectionLeft) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(0, 5);
  EXPECT_EQ(L"ABCDE", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_EQ(L"FGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteTextSelectionRight) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(45, 50);
  EXPECT_EQ(L"nopqr", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklm",
            GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteEmptyTextSelection) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest, InsertTextInEmptyEditableComboBox) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"Hello", GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextInPopulatedEditableComboBoxLeft) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  // Move cursor to beginning of user-editable combobox text field.
  EXPECT_TRUE(GetCFFLFormField()->OnKeyDown(FWL_VKEY_Home, {}));

  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"HelloABCDEFGHIJ", GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextInPopulatedEditableComboBoxMiddle) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  // Move cursor to middle of user-editable combobox text field.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(GetCFFLFormField()->OnKeyDown(FWL_VKEY_Left, {}));
  }

  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEHelloFGHIJ", GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextInPopulatedEditableComboBoxRight) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEFGHIJHello", GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxWhole) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJ", GetCPWLComboBox()->GetSelectedText());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"Hello", GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxLeft) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(0, 5);
  EXPECT_EQ(L"ABCDE", GetCPWLComboBox()->GetSelectedText());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"HelloFGHIJ", GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxMiddle) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(2, 7);
  EXPECT_EQ(L"CDEFG", GetCPWLComboBox()->GetSelectedText());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABHelloHIJ", GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxRight) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(5, 10);
  EXPECT_EQ(L"FGHIJ", GetCPWLComboBox()->GetSelectedText());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEHello", GetCPWLComboBox()->GetText());
}

TEST_F(CPWLComboBoxEditEmbedderTest, ReplaceAndKeepSelection) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormComboboxPDF();
  ASSERT_TRUE(page);

  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(1, 3);
  EXPECT_EQ(L"ABCDEFGHIJ", GetCPWLComboBox()->GetText());
  GetCPWLComboBox()->ReplaceAndKeepSelection(L"xyz");
  EXPECT_EQ(L"AxyzDEFGHIJ", GetCPWLComboBox()->GetText());
  EXPECT_EQ(L"xyz", GetCPWLComboBox()->GetSelectedText());

  GetCPWLComboBox()->SetEditSelection(4, 1);
  GetCPWLComboBox()->ReplaceAndKeepSelection(L"12");
  EXPECT_EQ(L"A12DEFGHIJ", GetCPWLComboBox()->GetText());
  EXPECT_EQ(L"12", GetCPWLComboBox()->GetSelectedText());
}

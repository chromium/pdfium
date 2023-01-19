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
  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());

  // Automatically pre-filled with "Banana".
  EXPECT_FALSE(GetCPWLComboBox()->GetText().IsEmpty());
  EXPECT_STREQ(L"Banana", GetCPWLComboBox()->GetText().c_str());

  // Check that selection is initially empty, then select entire word.
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetSelectText();
  EXPECT_STREQ(L"Banana", GetCPWLComboBox()->GetSelectedText().c_str());

  // Select other options.
  GetCPWLComboBox()->SetSelect(0);
  EXPECT_STREQ(L"Apple", GetCPWLComboBox()->GetSelectedText().c_str());
  GetCPWLComboBox()->SetSelect(2);
  EXPECT_STREQ(L"Cherry", GetCPWLComboBox()->GetSelectedText().c_str());

  // Verify that combobox text cannot be edited.
  EXPECT_FALSE(GetCFFLFormField()->OnChar(GetCPDFSDKAnnotNormal(), 'a', {}));
}

TEST_F(CPWLComboBoxEditEmbedderTest, GetSelectedTextFragmentsNormal) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotNormal());
  EXPECT_STREQ(L"Banana", GetCPWLComboBox()->GetText().c_str());

  GetCPWLComboBox()->SetEditSelection(0, 0);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSelection(0, 1);
  EXPECT_STREQ(L"B", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(0, -1);
  EXPECT_STREQ(L"Banana", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(-8, -1);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSelection(4, 1);
  EXPECT_STREQ(L"ana", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(1, 4);
  EXPECT_STREQ(L"ana", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(5, 6);
  EXPECT_STREQ(L"a", GetCPWLComboBox()->GetSelectedText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest, GetSelectedTextEmptyAndBasicEditable) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  EXPECT_TRUE(GetCPWLComboBox()->GetText().IsEmpty());

  // Check selection is initially empty, then select a provided option.
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetSelect(0);
  GetCPWLComboBox()->SetSelectText();
  EXPECT_STREQ(L"Foo", GetCPWLComboBox()->GetSelectedText().c_str());

  // Select another option and then select last char of that option.
  GetCPWLComboBox()->SetSelect(1);
  EXPECT_STREQ(L"Bar", GetCPWLComboBox()->GetSelectedText().c_str());
  GetCPWLComboBox()->SetEditSelection(2, 3);
  EXPECT_STREQ(L"r", GetCPWLComboBox()->GetSelectedText().c_str());

  // Type into editable combobox text field and select new text.
  EXPECT_TRUE(
      GetCFFLFormField()->OnChar(GetCPDFSDKAnnotUserEditable(), 'a', {}));
  EXPECT_TRUE(
      GetCFFLFormField()->OnChar(GetCPDFSDKAnnotUserEditable(), 'b', {}));
  EXPECT_TRUE(
      GetCFFLFormField()->OnChar(GetCPDFSDKAnnotUserEditable(), 'c', {}));

  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());
  GetCPWLComboBox()->SetEditSelection(0, 5);
  EXPECT_STREQ(L"Baabc", GetCPWLComboBox()->GetSelectedText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest, GetSelectedTextFragmentsEditable) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(0, 0);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSelection(0, 1);
  EXPECT_STREQ(L"A", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(-8, -1);
  EXPECT_TRUE(GetCPWLComboBox()->GetSelectedText().IsEmpty());

  GetCPWLComboBox()->SetEditSelection(23, 12);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(49, 50);
  EXPECT_STREQ(L"r", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(49, 55);
  EXPECT_STREQ(L"r", GetCPWLComboBox()->GetSelectedText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteEntireTextSelection) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_TRUE(GetCPWLComboBox()->GetText().IsEmpty());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteTextSelectionMiddle) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(12, 23);
  EXPECT_STREQ(L"MNOPQRSTUVW", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_STREQ(L"ABCDEFGHIJKLXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteTextSelectionLeft) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(0, 5);
  EXPECT_STREQ(L"ABCDE", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_STREQ(L"FGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteTextSelectionRight) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->SetEditSelection(45, 50);
  EXPECT_STREQ(L"nopqr", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklm",
               GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest, DeleteEmptyTextSelection) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(50);

  GetCPWLComboBox()->ReplaceSelection(L"");
  EXPECT_STREQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
               GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest, InsertTextInEmptyEditableComboBox) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_STREQ(L"Hello", GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextInPopulatedEditableComboBoxLeft) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  // Move cursor to beginning of user-editable combobox text field.
  EXPECT_TRUE(GetCFFLFormField()->OnKeyDown(FWL_VKEY_Home, {}));

  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_STREQ(L"HelloABCDEFGHIJ", GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextInPopulatedEditableComboBoxMiddle) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  // Move cursor to middle of user-editable combobox text field.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(GetCFFLFormField()->OnKeyDown(FWL_VKEY_Left, {}));
  }

  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_STREQ(L"ABCDEHelloFGHIJ", GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextInPopulatedEditableComboBoxRight) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_STREQ(L"ABCDEFGHIJHello", GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxWhole) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(0, -1);
  EXPECT_STREQ(L"ABCDEFGHIJ", GetCPWLComboBox()->GetSelectedText().c_str());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_STREQ(L"Hello", GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxLeft) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(0, 5);
  EXPECT_STREQ(L"ABCDE", GetCPWLComboBox()->GetSelectedText().c_str());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_STREQ(L"HelloFGHIJ", GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxMiddle) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(2, 7);
  EXPECT_STREQ(L"CDEFG", GetCPWLComboBox()->GetSelectedText().c_str());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_STREQ(L"ABHelloHIJ", GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxRight) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(5, 10);
  EXPECT_STREQ(L"FGHIJ", GetCPWLComboBox()->GetSelectedText().c_str());
  GetCPWLComboBox()->ReplaceSelection(L"Hello");
  EXPECT_STREQ(L"ABCDEHello", GetCPWLComboBox()->GetText().c_str());
}

TEST_F(CPWLComboBoxEditEmbedderTest, ReplaceAndKeepSelection) {
  FormFillerAndWindowSetup(GetCPDFSDKAnnotUserEditable());
  TypeTextIntoTextField(10);

  GetCPWLComboBox()->SetEditSelection(1, 3);
  EXPECT_STREQ(L"ABCDEFGHIJ", GetCPWLComboBox()->GetText().c_str());
  GetCPWLComboBox()->ReplaceAndKeepSelection(L"xyz");
  EXPECT_STREQ(L"AxyzDEFGHIJ", GetCPWLComboBox()->GetText().c_str());
  EXPECT_STREQ(L"xyz", GetCPWLComboBox()->GetSelectedText().c_str());

  GetCPWLComboBox()->SetEditSelection(4, 1);
  GetCPWLComboBox()->ReplaceAndKeepSelection(L"12");
  EXPECT_STREQ(L"A12DEFGHIJ", GetCPWLComboBox()->GetText().c_str());
  EXPECT_STREQ(L"12", GetCPWLComboBox()->GetSelectedText().c_str());
}

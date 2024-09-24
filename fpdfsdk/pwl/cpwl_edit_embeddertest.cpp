// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/pwl/cpwl_edit.h"

#include <utility>

#include "fpdfsdk/cpdfsdk_annotiterator.h"
#include "fpdfsdk/cpdfsdk_formfillenvironment.h"
#include "fpdfsdk/cpdfsdk_helpers.h"
#include "fpdfsdk/cpdfsdk_widget.h"
#include "fpdfsdk/formfiller/cffl_formfield.h"
#include "fpdfsdk/formfiller/cffl_interactiveformfiller.h"
#include "public/fpdf_fwlevent.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"

class CPWLEditEmbedderTest : public EmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    ASSERT_TRUE(OpenDocument("text_form_multiple.pdf"));
  }

  ScopedEmbedderTestPage CreateAndInitializeFormPDF() {
    ScopedEmbedderTestPage page = LoadScopedPage(0);
    if (!page) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }
    m_pFormFillEnv =
        CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    CPDFSDK_AnnotIterator iter(m_pFormFillEnv->GetPageViewAtIndex(0),
                               {CPDF_Annot::Subtype::WIDGET});
    // Normal text field.
    m_pAnnot = ToCPDFSDKWidget(iter.GetFirstAnnot());
    if (!m_pAnnot) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }

    // Read-only text field.
    CPDFSDK_Annot* pAnnotReadOnly = iter.GetNextAnnot(m_pAnnot);

    // Pre-filled text field with char limit of 10.
    m_pAnnotCharLimit = ToCPDFSDKWidget(iter.GetNextAnnot(pAnnotReadOnly));
    if (!m_pAnnotCharLimit) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }

    // Password text field.
    CPDFSDK_Annot* password_annot = iter.GetNextAnnot(m_pAnnotCharLimit);
    if (!password_annot) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }
    if (CPDF_Annot::Subtype::WIDGET != password_annot->GetAnnotSubtype()) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }

    CPDFSDK_Annot* pLastAnnot = iter.GetLastAnnot();
    if (password_annot != pLastAnnot) {
      ADD_FAILURE();
      return ScopedEmbedderTestPage();
    }
    return page;
  }

  void FormFillerAndWindowSetup(CPDFSDK_Widget* pAnnotTextField) {
    CFFL_InteractiveFormFiller* pInteractiveFormFiller =
        m_pFormFillEnv->GetInteractiveFormFiller();
    {
      ObservedPtr<CPDFSDK_Widget> pObserved(pAnnotTextField);
      EXPECT_TRUE(pInteractiveFormFiller->OnSetFocus(pObserved, {}));
    }

    m_pFormFiller =
        pInteractiveFormFiller->GetFormFieldForTesting(pAnnotTextField);
    ASSERT_TRUE(m_pFormFiller);

    CPWL_Wnd* pWindow =
        m_pFormFiller->GetPWLWindow(m_pFormFillEnv->GetPageViewAtIndex(0));
    ASSERT_TRUE(pWindow);
    m_pEdit = static_cast<CPWL_Edit*>(pWindow);
  }

  void TypeTextIntoTextField(int num_chars) {
    // Type text starting with 'A' to as many chars as specified by |num_chars|.
    for (int i = 0; i < num_chars; ++i) {
      EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', {}));
    }
  }

  CPWL_Edit* GetCPWLEdit() { return m_pEdit; }
  CFFL_FormField* GetCFFLFormFiller() { return m_pFormFiller; }
  CPDFSDK_Widget* GetCPDFSDKAnnot() { return m_pAnnot; }
  CPDFSDK_Widget* GetCPDFSDKAnnotCharLimit() { return m_pAnnotCharLimit; }

 private:
  CPWL_Edit* m_pEdit;
  CFFL_FormField* m_pFormFiller;
  CPDFSDK_Widget* m_pAnnot;
  CPDFSDK_Widget* m_pAnnotCharLimit;
  CPDFSDK_FormFillEnvironment* m_pFormFillEnv;
};

TEST_F(CPWLEditEmbedderTest, TypeText) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  EXPECT_TRUE(GetCPWLEdit()->GetText().IsEmpty());
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'a', {}));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'b', {}));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'c', {}));

  EXPECT_EQ(L"abc", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, GetSelectedTextEmptyAndBasic) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  // Attempt to set selection before text has been typed to test that
  // selection is identified as empty.
  //
  // Select from character index [0, 3) within form text field.
  GetCPWLEdit()->SetSelection(0, 3);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'a', {}));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'b', {}));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'c', {}));
  GetCPWLEdit()->SetSelection(0, 2);

  EXPECT_EQ(L"ab", GetCPWLEdit()->GetSelectedText());
}

TEST_F(CPWLEditEmbedderTest, GetSelectedTextFragments) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(0, 0);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  GetCPWLEdit()->SetSelection(0, 1);
  EXPECT_EQ(L"A", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->SetSelection(-8, -1);
  EXPECT_TRUE(GetCPWLEdit()->GetSelectedText().IsEmpty());

  GetCPWLEdit()->SetSelection(23, 12);
  EXPECT_EQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->SetSelection(12, 23);
  EXPECT_EQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->SetSelection(49, 50);
  EXPECT_EQ(L"r", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->SetSelection(49, 55);
  EXPECT_EQ(L"r", GetCPWLEdit()->GetSelectedText());
}

TEST_F(CPWLEditEmbedderTest, DeleteEntireTextSelection) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_TRUE(GetCPWLEdit()->GetText().IsEmpty());
}

TEST_F(CPWLEditEmbedderTest, DeleteTextSelectionMiddle) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(12, 23);
  EXPECT_EQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, DeleteTextSelectionLeft) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(0, 5);
  EXPECT_EQ(L"ABCDE", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_EQ(L"FGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, DeleteTextSelectionRight) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(45, 50);
  EXPECT_EQ(L"nopqr", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklm",
            GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, DeleteEmptyTextSelection) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(50);

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInEmptyTextField) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"Hello", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedTextFieldLeft) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(10);

  // Move cursor to beginning of text field.
  EXPECT_TRUE(GetCFFLFormFiller()->OnKeyDown(FWL_VKEY_Home, {}));

  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"HelloABCDEFGHIJ", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedTextFieldMiddle) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(10);

  // Move cursor to middle of text field.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnKeyDown(FWL_VKEY_Left, {}));
  }

  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEHelloFGHIJ", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedTextFieldRight) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(10);

  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEFGHIJHello", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldWhole) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJ", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"Hello", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldLeft) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(0, 5);
  EXPECT_EQ(L"ABCDE", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"HelloFGHIJ", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldMiddle) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(2, 7);
  EXPECT_EQ(L"CDEFG", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABHelloHIJ", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldRight) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(5, 10);
  EXPECT_EQ(L"FGHIJ", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEHello", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInEmptyCharLimitTextFieldOverflow) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"Elephant", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"");

  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"Hippopotam", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInEmptyCharLimitTextFieldFit) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"Elephant", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"");

  GetCPWLEdit()->ReplaceSelection(L"Zebra");
  EXPECT_EQ(L"Zebra", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedCharLimitTextFieldLeft) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"HiElephant", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedCharLimitTextFieldMiddle) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  // Move cursor to middle of text field.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnKeyDown(FWL_VKEY_Right, {}));
  }

  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"ElephHiant", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedCharLimitTextFieldRight) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  // Move cursor to end of text field.
  EXPECT_TRUE(GetCFFLFormFiller()->OnKeyDown(FWL_VKEY_End, {}));

  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"ElephantHi", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldWhole) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"Elephant", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"Hippopotam", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldLeft) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(0, 4);
  EXPECT_EQ(L"Elep", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"Hippophant", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldMiddle) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(2, 6);
  EXPECT_EQ(L"epha", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"ElHippopnt", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldRight) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit());
  GetCPWLEdit()->SetSelection(4, 8);
  EXPECT_EQ(L"hant", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"ElepHippop", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithEndCarriageFeed) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->SetText(L"Foo\r");
  EXPECT_EQ(L"Foo", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithEndNewline) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->SetText(L"Foo\n");
  EXPECT_EQ(L"Foo", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithEndCarriageFeedAndNewLine) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->SetText(L"Foo\r\n");
  EXPECT_EQ(L"Foo", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithEndNewLineAndCarriageFeed) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->SetText(L"Foo\n\r");
  EXPECT_EQ(L"Foo", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithBodyCarriageFeed) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->SetText(L"Foo\rBar");
  EXPECT_EQ(L"FooBar", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithBodyNewline) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->SetText(L"Foo\nBar");
  EXPECT_EQ(L"FooBar", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithBodyCarriageFeedAndNewLine) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->SetText(L"Foo\r\nBar");
  EXPECT_EQ(L"FooBar", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithBodyNewLineAndCarriageFeed) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  GetCPWLEdit()->SetText(L"Foo\n\rBar");
  EXPECT_EQ(L"FooBar", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, ReplaceAndKeepSelection) {
  ScopedEmbedderTestPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  FormFillerAndWindowSetup(GetCPDFSDKAnnot());
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(1, 3);
  EXPECT_EQ(L"ABCDEFGHIJ", GetCPWLEdit()->GetText());
  GetCPWLEdit()->ReplaceAndKeepSelection(L"xyz");
  EXPECT_EQ(L"AxyzDEFGHIJ", GetCPWLEdit()->GetText());
  EXPECT_EQ(L"xyz", GetCPWLEdit()->GetSelectedText());
  EXPECT_EQ(GetCPWLEdit()->GetSelection(), std::make_pair(1, 4));

  GetCPWLEdit()->SetSelection(4, 1);
  GetCPWLEdit()->ReplaceAndKeepSelection(L"12");
  EXPECT_EQ(L"A12DEFGHIJ", GetCPWLEdit()->GetText());
  EXPECT_EQ(L"12", GetCPWLEdit()->GetSelectedText());
  EXPECT_EQ(GetCPWLEdit()->GetSelection(), std::make_pair(1, 3));
}

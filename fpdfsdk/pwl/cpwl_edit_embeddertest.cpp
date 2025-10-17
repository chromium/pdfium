// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "fpdfsdk/pwl/cpwl_edit.h"

#include <utility>

#include "constants/ascii.h"
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

  ScopedPage CreateAndInitializeFormPDF() {
    ScopedPage page = LoadScopedPage(0);
    if (!page) {
      ADD_FAILURE();
      return ScopedPage();
    }
    form_fill_env_ =
        CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    CPDFSDK_AnnotIterator iter(form_fill_env_->GetPageViewAtIndex(0),
                               {CPDF_Annot::Subtype::WIDGET});
    // Normal text field.
    annot_ = ToCPDFSDKWidget(iter.GetFirstAnnot());
    if (!annot_) {
      ADD_FAILURE();
      return ScopedPage();
    }

    // Read-only text field.
    CPDFSDK_Annot* annot_read_only = iter.GetNextAnnot(annot_);

    // Pre-filled text field with char limit of 10.
    annot_char_limit_ = ToCPDFSDKWidget(iter.GetNextAnnot(annot_read_only));
    if (!annot_char_limit_) {
      ADD_FAILURE();
      return ScopedPage();
    }

    // Password text field.
    CPDFSDK_Annot* password_annot = iter.GetNextAnnot(annot_char_limit_);
    if (!password_annot) {
      ADD_FAILURE();
      return ScopedPage();
    }
    if (CPDF_Annot::Subtype::WIDGET != password_annot->GetAnnotSubtype()) {
      ADD_FAILURE();
      return ScopedPage();
    }

    CPDFSDK_Annot* last_annot = iter.GetLastAnnot();
    if (password_annot != last_annot) {
      ADD_FAILURE();
      return ScopedPage();
    }
    return page;
  }

  void FormFillerAndWindowSetup(CPDFSDK_Widget* annot_text_field) {
    CFFL_InteractiveFormFiller* interactive_form_filler =
        form_fill_env_->GetInteractiveFormFiller();
    {
      ObservedPtr<CPDFSDK_Widget> observed(annot_text_field);
      EXPECT_TRUE(interactive_form_filler->OnSetFocus(observed, {}));
    }

    form_filler_ =
        interactive_form_filler->GetFormFieldForTesting(annot_text_field);
    ASSERT_TRUE(form_filler_);

    CPWL_Wnd* window =
        form_filler_->GetPWLWindow(form_fill_env_->GetPageViewAtIndex(0));
    ASSERT_TRUE(window);
    edit_ = static_cast<CPWL_Edit*>(window);
  }

  void TypeTextIntoTextField(int num_chars) {
    // Type text starting with 'A' to as many chars as specified by |num_chars|.
    for (int i = 0; i < num_chars; ++i) {
      EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', {}));
    }
  }

  CPWL_Edit* GetCPWLEdit() { return edit_; }
  CFFL_FormField* GetCFFLFormFiller() { return form_filler_; }
  CPDFSDK_Widget* GetCPDFSDKAnnot() { return annot_; }
  CPDFSDK_Widget* GetCPDFSDKAnnotCharLimit() { return annot_char_limit_; }

 private:
  CPWL_Edit* edit_;
  CFFL_FormField* form_filler_;
  CPDFSDK_Widget* annot_;
  CPDFSDK_Widget* annot_char_limit_;
  CPDFSDK_FormFillEnvironment* form_fill_env_;
};

class CPWLEditMultilineEmbedderTest : public EmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    ASSERT_TRUE(OpenDocument("text_form_multiline.pdf"));
  }

  ScopedPage CreateAndInitializeFormPDF() {
    ScopedPage page = LoadScopedPage(0);
    if (!page) {
      ADD_FAILURE();
      return ScopedPage();
    }
    form_fill_env_ =
        CPDFSDKFormFillEnvironmentFromFPDFFormHandle(form_handle());
    CPDFSDK_AnnotIterator iter(form_fill_env_->GetPageViewAtIndex(0),
                               {CPDF_Annot::Subtype::WIDGET});
    // Normal text field.
    annot_ = ToCPDFSDKWidget(iter.GetFirstAnnot());
    if (!annot_) {
      ADD_FAILURE();
      return ScopedPage();
    }

    return page;
  }

  void FormFillerAndWindowSetup(CPDFSDK_Widget* annot_text_field) {
    CFFL_InteractiveFormFiller* interactive_form_filler =
        form_fill_env_->GetInteractiveFormFiller();
    {
      ObservedPtr<CPDFSDK_Widget> observed(annot_text_field);
      EXPECT_TRUE(interactive_form_filler->OnSetFocus(observed, {}));
    }

    form_filler_ =
        interactive_form_filler->GetFormFieldForTesting(annot_text_field);
    ASSERT_TRUE(form_filler_);

    CPWL_Wnd* window =
        form_filler_->GetPWLWindow(form_fill_env_->GetPageViewAtIndex(0));
    ASSERT_TRUE(window);
    edit_ = static_cast<CPWL_Edit*>(window);
  }

  void TypeTextIntoTextField(int num_chars) {
    // Type text starting with 'A' to as many chars as specified by |num_chars|.
    for (int i = 0; i < num_chars; ++i) {
      EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), i + 'A', {}));
    }
  }

  CPWL_Edit* GetCPWLEdit() { return edit_; }
  CFFL_FormField* GetCFFLFormFiller() { return form_filler_; }
  CPDFSDK_Widget* GetCPDFSDKAnnot() { return annot_; }
  CPDFSDK_Widget* GetCPDFSDKAnnotCharLimit() { return annot_char_limit_; }

 private:
  CPWL_Edit* edit_;
  CFFL_FormField* form_filler_;
  CPDFSDK_Widget* annot_;
  CPDFSDK_Widget* annot_char_limit_;
  CPDFSDK_FormFillEnvironment* form_fill_env_;
};

TEST_F(CPWLEditEmbedderTest, TypeText) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  EXPECT_TRUE(GetCPWLEdit()->GetText().IsEmpty());
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'a', {}));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'b', {}));
  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'c', {}));

  EXPECT_EQ(L"abc", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, GetSelectedTextEmptyAndBasic) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
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
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
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
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_TRUE(GetCPWLEdit()->GetText().IsEmpty());
}

TEST_F(CPWLEditEmbedderTest, DeleteTextSelectionMiddle) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(12, 23);
  EXPECT_EQ(L"MNOPQRSTUVW", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, DeleteTextSelectionLeft) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(0, 5);
  EXPECT_EQ(L"ABCDE", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_EQ(L"FGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, DeleteTextSelectionRight) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(50);

  GetCPWLEdit()->SetSelection(45, 50);
  EXPECT_EQ(L"nopqr", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklm",
            GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, DeleteEmptyTextSelection) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(50);

  GetCPWLEdit()->ReplaceSelection(L"");
  EXPECT_EQ(L"ABCDEFGHIJKLMNOPQRSTUVWXYZ[\\]^_`abcdefghijklmnopqr",
            GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInEmptyTextField) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"Hello", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedTextFieldLeft) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(10);

  // Move cursor to beginning of text field.
  EXPECT_TRUE(GetCFFLFormFiller()->OnKeyDown(FWL_VKEY_Home, {}));

  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"HelloABCDEFGHIJ", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedTextFieldMiddle) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(10);

  // Move cursor to middle of text field.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnKeyDown(FWL_VKEY_Left, {}));
  }

  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEHelloFGHIJ", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedTextFieldRight) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(10);

  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEFGHIJHello", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldWhole) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJ", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"Hello", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldLeft) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(0, 5);
  EXPECT_EQ(L"ABCDE", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"HelloFGHIJ", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldMiddle) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(2, 7);
  EXPECT_EQ(L"CDEFG", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABHelloHIJ", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldRight) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  TypeTextIntoTextField(10);

  GetCPWLEdit()->SetSelection(5, 10);
  EXPECT_EQ(L"FGHIJ", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hello");
  EXPECT_EQ(L"ABCDEHello", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInEmptyCharLimitTextFieldOverflow) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"Elephant", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"");

  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"Hippopotam", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInEmptyCharLimitTextFieldFit) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"Elephant", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"");

  GetCPWLEdit()->ReplaceSelection(L"Zebra");
  EXPECT_EQ(L"Zebra", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedCharLimitTextFieldLeft) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"HiElephant", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedCharLimitTextFieldMiddle) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  // Move cursor to middle of text field.
  for (int i = 0; i < 5; ++i) {
    EXPECT_TRUE(GetCFFLFormFiller()->OnKeyDown(FWL_VKEY_Right, {}));
  }

  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"ElephHiant", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, InsertTextInPopulatedCharLimitTextFieldRight) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  // Move cursor to end of text field.
  EXPECT_TRUE(GetCFFLFormFiller()->OnKeyDown(FWL_VKEY_End, {}));

  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"ElephantHi", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldWhole) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"Elephant", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"Hippopotam", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldLeft) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  GetCPWLEdit()->SetSelection(0, 4);
  EXPECT_EQ(L"Elep", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"Hippophant", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldMiddle) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  GetCPWLEdit()->SetSelection(2, 6);
  EXPECT_EQ(L"epha", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"ElHippopnt", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldRight) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnotCharLimit()));
  GetCPWLEdit()->SetSelection(4, 8);
  EXPECT_EQ(L"hant", GetCPWLEdit()->GetSelectedText());
  GetCPWLEdit()->ReplaceSelection(L"Hippopotamus");
  EXPECT_EQ(L"ElepHippop", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithEndCarriageFeed) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetText(L"Foo\r");
  EXPECT_EQ(L"Foo", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithEndNewline) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetText(L"Foo\n");
  EXPECT_EQ(L"Foo", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithEndCarriageFeedAndNewLine) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetText(L"Foo\r\n");
  EXPECT_EQ(L"Foo", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithEndNewLineAndCarriageFeed) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetText(L"Foo\n\r");
  EXPECT_EQ(L"Foo", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithBodyCarriageFeed) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetText(L"Foo\rBar");
  EXPECT_EQ(L"FooBar", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithBodyNewline) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetText(L"Foo\nBar");
  EXPECT_EQ(L"FooBar", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithBodyCarriageFeedAndNewLine) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetText(L"Foo\r\nBar");
  EXPECT_EQ(L"FooBar", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, SetTextWithBodyNewLineAndCarriageFeed) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetText(L"Foo\n\rBar");
  EXPECT_EQ(L"FooBar", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, ReplaceAndKeepSelection) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
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

TEST_F(CPWLEditEmbedderTest, ReplaceSelectionUndoRedo) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));

  TypeTextIntoTextField(13);

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJKLM", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"123");
  EXPECT_EQ(L"123", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"ABCDEFGHIJKLM", GetCPWLEdit()->GetSelectedText());

  EXPECT_TRUE(GetCPWLEdit()->CanRedo());
  EXPECT_TRUE(GetCPWLEdit()->Redo());
  EXPECT_EQ(L"123", GetCPWLEdit()->GetText());
}

TEST_F(CPWLEditEmbedderTest, ReplaceSelectionUndoQueueLimit) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetMaxUndoItemsForTest(4);

  TypeTextIntoTextField(13);

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJKLM", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"123");
  EXPECT_EQ(L"123", GetCPWLEdit()->GetText());

  TypeTextIntoTextField(2);
  EXPECT_EQ(L"123AB", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"123A", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"123", GetCPWLEdit()->GetText());

  // The entire group of 4 ReplaceSelection undo items should have been dropped
  // off the end of the queue.
  EXPECT_FALSE(GetCPWLEdit()->CanUndo());
}

TEST_F(CPWLEditEmbedderTest, ReplaceSelectionRedoQueueLimit) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));
  GetCPWLEdit()->SetMaxUndoItemsForTest(4);

  TypeTextIntoTextField(13);

  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABCDEFGHIJKLM", GetCPWLEdit()->GetSelectedText());

  GetCPWLEdit()->ReplaceSelection(L"123");
  EXPECT_EQ(L"123", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"ABCDEFGHIJKLM", GetCPWLEdit()->GetSelectedText());
  // Select none with no undo item added to the stack (selection is not normally
  // an undo item).
  //
  // This is necessary so that TypeTextIntoTextField(1), which adds a single
  // undo item, does the standard typing behavior and not the ReplaceSelection
  // behavior (which would add several undo items).
  GetCPWLEdit()->SetSelection(-1, 0);

  EXPECT_TRUE(GetCPWLEdit()->CanRedo());
  TypeTextIntoTextField(1);  // Adds undo item #5, over the limit
  EXPECT_EQ(L"ABCDEFGHIJKLMA", GetCPWLEdit()->GetText());
  // The 4 undo items for the ReplaceSelection() call should all be dropped off
  // of the stack now. Covers UndoStack::RemoveTails().
  EXPECT_FALSE(GetCPWLEdit()->CanRedo());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"ABCDEFGHIJKLM", GetCPWLEdit()->GetText());
  // Typing "A" was the only item left on the undo stack
  EXPECT_FALSE(GetCPWLEdit()->Undo());
}

TEST_F(CPWLEditEmbedderTest, BackspaceUndoRedo) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));

  TypeTextIntoTextField(3);
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(),
                                          pdfium::ascii::kBackspace, {}));
  EXPECT_EQ(L"AB", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanRedo());
  EXPECT_TRUE(GetCPWLEdit()->Redo());
  EXPECT_EQ(L"AB", GetCPWLEdit()->GetText());
  EXPECT_FALSE(GetCPWLEdit()->CanRedo());
}

TEST_F(CPWLEditEmbedderTest, BackspaceOverSelectionUndoRedo) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));

  TypeTextIntoTextField(3);
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetSelectedText());

  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(),
                                          pdfium::ascii::kBackspace, {}));
  EXPECT_EQ(L"", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetSelectedText());

  EXPECT_TRUE(GetCPWLEdit()->CanRedo());
  EXPECT_TRUE(GetCPWLEdit()->Redo());
  EXPECT_EQ(L"", GetCPWLEdit()->GetText());
  EXPECT_FALSE(GetCPWLEdit()->CanRedo());
}

TEST_F(CPWLEditEmbedderTest, TypeOverSelectionUndoRedo) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));

  TypeTextIntoTextField(3);
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetSelectedText());

  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(), 'Z', {}));
  EXPECT_EQ(L"Z", GetCPWLEdit()->GetText());
  EXPECT_EQ(L"", GetCPWLEdit()->GetSelectedText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetSelectedText());

  EXPECT_TRUE(GetCPWLEdit()->CanRedo());
  EXPECT_TRUE(GetCPWLEdit()->Redo());
  EXPECT_EQ(L"Z", GetCPWLEdit()->GetText());
  EXPECT_EQ(L"", GetCPWLEdit()->GetSelectedText());
  EXPECT_FALSE(GetCPWLEdit()->CanRedo());
}

TEST_F(CPWLEditMultilineEmbedderTest, ReturnUndoRedo) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));

  TypeTextIntoTextField(3);
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(),
                                          pdfium::ascii::kReturn, {}));
  EXPECT_EQ(L"ABC\r\n", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanRedo());
  EXPECT_TRUE(GetCPWLEdit()->Redo());
  EXPECT_EQ(L"ABC\r\n", GetCPWLEdit()->GetText());
  EXPECT_FALSE(GetCPWLEdit()->CanRedo());
}

TEST_F(CPWLEditMultilineEmbedderTest, ReturnOverSelectionUndoRedo) {
  ScopedPage page = CreateAndInitializeFormPDF();
  ASSERT_TRUE(page);
  ASSERT_NO_FATAL_FAILURE(FormFillerAndWindowSetup(GetCPDFSDKAnnot()));

  TypeTextIntoTextField(3);
  GetCPWLEdit()->SetSelection(0, -1);
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetSelectedText());

  EXPECT_TRUE(GetCFFLFormFiller()->OnChar(GetCPDFSDKAnnot(),
                                          pdfium::ascii::kReturn, {}));
  EXPECT_EQ(L"\r\n", GetCPWLEdit()->GetText());

  EXPECT_TRUE(GetCPWLEdit()->CanUndo());
  EXPECT_TRUE(GetCPWLEdit()->Undo());
  EXPECT_EQ(L"ABC", GetCPWLEdit()->GetSelectedText());

  EXPECT_TRUE(GetCPWLEdit()->CanRedo());
  EXPECT_TRUE(GetCPWLEdit()->Redo());
  EXPECT_EQ(L"\r\n", GetCPWLEdit()->GetText());
  EXPECT_FALSE(GetCPWLEdit()->CanRedo());
}

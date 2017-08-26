// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "public/cpp/fpdf_deleters.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_mock_delegate.h"
#include "testing/embedder_test_timer_handling_delegate.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;

using FPDFFormFillEmbeddertest = EmbedderTest;

// A base class for many related tests that involve clicking and typing into
// form fields.
class FPDFFormFillInteractiveEmbeddertest : public FPDFFormFillEmbeddertest {
 protected:
  FPDFFormFillInteractiveEmbeddertest() = default;
  ~FPDFFormFillInteractiveEmbeddertest() override = default;

  void SetUp() override {
    FPDFFormFillEmbeddertest::SetUp();
    ASSERT_TRUE(OpenDocument(GetDocumentName()));
    page_ = LoadPage(0);
    ASSERT_TRUE(page_);
    FormSanityChecks();
  }

  void TearDown() override {
    UnloadPage(page_);
    FPDFFormFillEmbeddertest::TearDown();
  }

  // Returns the name of the PDF to use.
  virtual const char* GetDocumentName() const = 0;

  // Returns the type of field(s) in the PDF.
  virtual int GetFormType() const = 0;

  // Optionally do some sanity check on the document after loading.
  virtual void FormSanityChecks() {}

  FPDF_PAGE page() { return page_; }

  void ClickOnFormFieldAtPoint(double x, double y) {
    // Click on the text field or combobox as specified by coordinates.
    FORM_OnMouseMove(form_handle(), page_, 0, x, y);
    FORM_OnLButtonDown(form_handle(), page_, 0, x, y);
    FORM_OnLButtonUp(form_handle(), page_, 0, x, y);
  }

  void TypeTextIntoTextField(int num_chars, double x, double y) {
    EXPECT_EQ(GetFormType(),
              FPDFPage_HasFormFieldAtPoint(form_handle(), page_, x, y));
    ClickOnFormFieldAtPoint(x, y);

    // Type text starting with 'A' to as many chars as specified by |num_chars|.
    for (int i = 0; i < num_chars; ++i) {
      FORM_OnChar(form_handle(), page_, 'A' + i, 0);
    }
  }

  // Navigates to text field using the mouse and then selects text via the
  // shift and specfied left or right arrow key.
  void SelectTextWithKeyboard(int num_chars,
                              int arrow_key,
                              double x,
                              double y) {
    // Navigate to starting position for selection.
    ClickOnFormFieldAtPoint(x, y);

    // Hold down shift (and don't release until entire text is selected).
    FORM_OnKeyDown(form_handle(), page_, FWL_VKEY_Shift, 0);

    // Select text char by char via left or right arrow key.
    for (int i = 0; i < num_chars; ++i) {
      FORM_OnKeyDown(form_handle(), page_, arrow_key, FWL_EVENTFLAG_ShiftKey);
      FORM_OnKeyUp(form_handle(), page_, arrow_key, FWL_EVENTFLAG_ShiftKey);
    }
    FORM_OnKeyUp(form_handle(), page_, FWL_VKEY_Shift, 0);
  }

  // Uses the mouse to navigate to text field and select text.
  void SelectTextWithMouse(double start_x, double end_x, double y) {
    // Navigate to starting position and click mouse.
    FORM_OnMouseMove(form_handle(), page_, 0, start_x, y);
    FORM_OnLButtonDown(form_handle(), page_, 0, start_x, y);

    // Hold down mouse until reach end of desired selection.
    FORM_OnMouseMove(form_handle(), page_, 0, end_x, y);
    FORM_OnLButtonUp(form_handle(), page_, 0, end_x, y);
  }

  void CheckSelection(const CFX_WideStringC& expected_string) {
    // Calculate expected length for selected text.
    int num_chars = expected_string.GetLength();

    // Check actual selection against expected selection.
    const unsigned long expected_length =
        sizeof(unsigned short) * (num_chars + 1);
    unsigned long sel_text_len =
        FORM_GetSelectedText(form_handle(), page_, nullptr, 0);
    ASSERT_EQ(expected_length, sel_text_len);

    std::vector<unsigned short> buf(sel_text_len);
    EXPECT_EQ(expected_length, FORM_GetSelectedText(form_handle(), page_,
                                                    buf.data(), sel_text_len));

    EXPECT_EQ(expected_string,
              CFX_WideString::FromUTF16LE(buf.data(), num_chars));
  }

 private:
  FPDF_PAGE page_ = nullptr;
};

class FPDFFormFillTextFormEmbeddertest
    : public FPDFFormFillInteractiveEmbeddertest {
 protected:
  FPDFFormFillTextFormEmbeddertest() = default;
  ~FPDFFormFillTextFormEmbeddertest() override = default;

  const char* GetDocumentName() const override {
    // PDF with several form text fields:
    // - "Text Box" - No special attributes.
    // - "ReadOnly" - Ff: 1.
    // - "CharLimit" - MaxLen: 10, V: Elephant.
    return "text_form_multiple.pdf";
  }

  int GetFormType() const override { return FPDF_FORMFIELD_TEXTFIELD; }
};

class FPDFFormFillComboBoxFormEmbeddertest
    : public FPDFFormFillInteractiveEmbeddertest {
 protected:
  FPDFFormFillComboBoxFormEmbeddertest() = default;
  ~FPDFFormFillComboBoxFormEmbeddertest() override = default;

  const char* GetDocumentName() const override {
    // PDF with form comboboxes.
    return "combobox_form.pdf";
  }

  int GetFormType() const override { return FPDF_FORMFIELD_COMBOBOX; }

  void FormSanityChecks() override {
    EXPECT_EQ(GetFormType(), FPDFPage_HasFormFieldAtPoint(form_handle(), page(),
                                                          102.0, 113.0));
  }

  // Selects one of the pre-selected values from a combobox with three options.
  // Options are specified by |item_index|, which is 0-based.
  void SelectOption(int32_t item_index, double x, double y) {
    // Only relevant for comboboxes with three choices and the same dimensions
    // as those in combobox_form.pdf.
    ASSERT(item_index >= 0);
    ASSERT(item_index < 3);

    // Navigate to button for drop down and click mouse to reveal options.
    ClickOnFormFieldAtPoint(x, y);

    // Y coordinate of dropdown option to be selected.
    constexpr double kChoiceHeight = 15;
    double option_y = y - kChoiceHeight * (item_index + 1);

    // Navigate to option and click mouse to select it.
    ClickOnFormFieldAtPoint(x, option_y);
  }
};

TEST_F(FPDFFormFillEmbeddertest, FirstTest) {
  EmbedderTestMockDelegate mock;
  EXPECT_CALL(mock, Alert(_, _, _, _)).Times(0);
  EXPECT_CALL(mock, UnsupportedHandler(_)).Times(0);
  EXPECT_CALL(mock, SetTimer(_, _)).Times(0);
  EXPECT_CALL(mock, KillTimer(_)).Times(0);
  SetDelegate(&mock);

  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbeddertest, BUG_487928) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_487928.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  DoOpenActions();
  delegate.AdvanceTime(5000);
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbeddertest, BUG_507316) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_507316.pdf"));
  FPDF_PAGE page = LoadPage(2);
  EXPECT_TRUE(page);
  DoOpenActions();
  delegate.AdvanceTime(4000);
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbeddertest, BUG_514690) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  // Test that FORM_OnMouseMove() etc. permit null HANDLES and PAGES.
  FORM_OnMouseMove(nullptr, page, 0, 10.0, 10.0);
  FORM_OnMouseMove(form_handle(), nullptr, 0, 10.0, 10.0);

  UnloadPage(page);
}

#ifdef PDF_ENABLE_V8
TEST_F(FPDFFormFillEmbeddertest, BUG_551248) {
  // Test that timers fire once and intervals fire repeatedly.
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_551248.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  DoOpenActions();

  const auto& alerts = delegate.GetAlerts();
  EXPECT_EQ(0U, alerts.size());

  delegate.AdvanceTime(1000);
  EXPECT_EQ(0U, alerts.size());  // nothing fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(1U, alerts.size());  // interval fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(2U, alerts.size());  // timer fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(3U, alerts.size());  // interval fired again.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(3U, alerts.size());  // nothing fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(4U, alerts.size());  // interval fired again.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(4U, alerts.size());  // nothing fired.
  UnloadPage(page);

  ASSERT_EQ(4U, alerts.size());  // nothing else fired.

  EXPECT_STREQ(L"interval fired", alerts[0].message.c_str());
  EXPECT_STREQ(L"Alert", alerts[0].title.c_str());
  EXPECT_EQ(0, alerts[0].type);
  EXPECT_EQ(0, alerts[0].icon);

  EXPECT_STREQ(L"timer fired", alerts[1].message.c_str());
  EXPECT_STREQ(L"Alert", alerts[1].title.c_str());
  EXPECT_EQ(0, alerts[1].type);
  EXPECT_EQ(0, alerts[1].icon);

  EXPECT_STREQ(L"interval fired", alerts[2].message.c_str());
  EXPECT_STREQ(L"Alert", alerts[2].title.c_str());
  EXPECT_EQ(0, alerts[2].type);
  EXPECT_EQ(0, alerts[2].icon);

  EXPECT_STREQ(L"interval fired", alerts[3].message.c_str());
  EXPECT_STREQ(L"Alert", alerts[3].title.c_str());
  EXPECT_EQ(0, alerts[3].type);
  EXPECT_EQ(0, alerts[3].icon);
}

TEST_F(FPDFFormFillEmbeddertest, BUG_620428) {
  // Test that timers and intervals are cancelable.
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_620428.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  DoOpenActions();
  delegate.AdvanceTime(5000);
  UnloadPage(page);

  const auto& alerts = delegate.GetAlerts();
  ASSERT_EQ(1U, alerts.size());
  EXPECT_STREQ(L"done", alerts[0].message.c_str());
}

TEST_F(FPDFFormFillEmbeddertest, BUG_634394) {
  // Cancel timer inside timer callback.
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_634394.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  DoOpenActions();

  // Timers fire at most once per AdvanceTime(), allow intervals
  // to fire several times if possible.
  delegate.AdvanceTime(1000);
  delegate.AdvanceTime(1000);
  delegate.AdvanceTime(1000);
  delegate.AdvanceTime(1000);
  delegate.AdvanceTime(1000);
  UnloadPage(page);

  const auto& alerts = delegate.GetAlerts();
  EXPECT_EQ(2U, alerts.size());
}

TEST_F(FPDFFormFillEmbeddertest, BUG_634716) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_634716.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  DoOpenActions();

  // Timers fire at most once per AdvanceTime(), allow intervals
  // to fire several times if possible.
  delegate.AdvanceTime(1000);
  delegate.AdvanceTime(1000);
  delegate.AdvanceTime(1000);
  delegate.AdvanceTime(1000);
  delegate.AdvanceTime(1000);
  UnloadPage(page);

  const auto& alerts = delegate.GetAlerts();
  EXPECT_EQ(2U, alerts.size());
}

TEST_F(FPDFFormFillEmbeddertest, BUG_679649) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_679649.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  delegate.SetFailNextTimer();
  DoOpenActions();
  delegate.AdvanceTime(2000);
  UnloadPage(page);

  const auto& alerts = delegate.GetAlerts();
  EXPECT_EQ(0u, alerts.size());
}

TEST_F(FPDFFormFillEmbeddertest, BUG_707673) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_707673.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  DoOpenActions();
  FORM_OnLButtonDown(form_handle(), page, 0, 140, 590);
  FORM_OnLButtonUp(form_handle(), page, 0, 140, 590);
  delegate.AdvanceTime(1000);
  UnloadPage(page);

  const auto& alerts = delegate.GetAlerts();
  EXPECT_EQ(0u, alerts.size());
}

#endif  // PDF_ENABLE_V8

TEST_F(FPDFFormFillEmbeddertest, FormText) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  const char md5_1[] = "5f11dbe575fe197a37c3fb422559f8ff";
  const char md5_2[] = "35b1a4b679eafc749a0b6fda750c0e8d";
  const char md5_3[] = "65c64a7c355388f719a752aa1e23f6fe";
#else
  const char md5_1[] = "a5e3ac74c2ee123ec6710e2f0ef8424a";
  const char md5_2[] = "4526b09382e144d5506ad92149399de6";
  const char md5_3[] = "80356067d860088864cf50ff85d8459e";
#endif
  {
    EXPECT_TRUE(OpenDocument("text_form.pdf"));
    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);
    std::unique_ptr<void, FPDFBitmapDeleter> bitmap1(RenderPage(page));
    CompareBitmap(bitmap1.get(), 300, 300, md5_1);

    // Click on the textfield
    EXPECT_EQ(FPDF_FORMFIELD_TEXTFIELD,
              FPDFPage_HasFormFieldAtPoint(form_handle(), page, 120.0, 120.0));
    FORM_OnMouseMove(form_handle(), page, 0, 120.0, 120.0);
    FORM_OnLButtonDown(form_handle(), page, 0, 120.0, 120.0);
    FORM_OnLButtonUp(form_handle(), page, 0, 120.0, 120.0);

    // Write "ABC"
    FORM_OnChar(form_handle(), page, 65, 0);
    FORM_OnChar(form_handle(), page, 66, 0);
    FORM_OnChar(form_handle(), page, 67, 0);
    std::unique_ptr<void, FPDFBitmapDeleter> bitmap2(RenderPage(page));
    CompareBitmap(bitmap2.get(), 300, 300, md5_2);

    // Take out focus by clicking out of the textfield
    FORM_OnMouseMove(form_handle(), page, 0, 15.0, 15.0);
    FORM_OnLButtonDown(form_handle(), page, 0, 15.0, 15.0);
    FORM_OnLButtonUp(form_handle(), page, 0, 15.0, 15.0);
    std::unique_ptr<void, FPDFBitmapDeleter> bitmap3(RenderPage(page));
    CompareBitmap(bitmap3.get(), 300, 300, md5_3);

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

    // Close page
    UnloadPage(page);
  }
  // Check saved document
  TestAndCloseSaved(300, 300, md5_3);
}

TEST_F(FPDFFormFillTextFormEmbeddertest, GetSelectedTextEmptyAndBasicKeyboard) {
  // Test empty selection.
  CheckSelection(L"");

  // Test basic selection.
  TypeTextIntoTextField(3, 120.0, 120.0);
  SelectTextWithKeyboard(3, FWL_VKEY_Left, 123.0, 115.5);
  CheckSelection(L"ABC");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, GetSelectedTextEmptyAndBasicMouse) {
  // Test empty selection.
  CheckSelection(L"");

  // Test basic selection.
  TypeTextIntoTextField(3, 120.0, 120.0);
  SelectTextWithMouse(125.0, 102.0, 115.5);
  CheckSelection(L"ABC");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, GetSelectedTextFragmentsKeyBoard) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Test selecting first character in forward direction.
  SelectTextWithKeyboard(1, FWL_VKEY_Right, 102.0, 115.5);
  CheckSelection(L"A");

  // Test selecting entire long string in backwards direction.
  SelectTextWithKeyboard(12, FWL_VKEY_Left, 191.0, 115.5);
  CheckSelection(L"ABCDEFGHIJKL");

  // Test selecting middle section in backwards direction.
  SelectTextWithKeyboard(6, FWL_VKEY_Left, 170.0, 115.5);
  CheckSelection(L"DEFGHI");

  // Test selecting middle selection in forward direction.
  SelectTextWithKeyboard(6, FWL_VKEY_Right, 125.0, 115.5);
  CheckSelection(L"DEFGHI");

  // Test selecting last character in backwards direction.
  SelectTextWithKeyboard(1, FWL_VKEY_Left, 191.0, 115.5);
  CheckSelection(L"L");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, GetSelectedTextFragmentsMouse) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Test selecting first character in forward direction.
  SelectTextWithMouse(102.0, 106.0, 115.5);
  CheckSelection(L"A");

  // Test selecting entire long string in backwards direction.
  SelectTextWithMouse(191.0, 102.0, 115.5);
  CheckSelection(L"ABCDEFGHIJKL");

  // Test selecting middle section in backwards direction.
  SelectTextWithMouse(170.0, 125.0, 115.5);
  CheckSelection(L"DEFGHI");

  // Test selecting middle selection in forward direction.
  SelectTextWithMouse(125.0, 170.0, 115.5);
  CheckSelection(L"DEFGHI");

  // Test selecting last character in backwards direction.
  SelectTextWithMouse(191.0, 186.0, 115.5);
  CheckSelection(L"L");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       GetSelectedTextEmptyAndBasicNormalComboBox) {
  // Test empty selection.
  CheckSelection(L"");

  // Non-editable comboboxes don't allow selection with keyboard.
  SelectTextWithMouse(102.0, 142.0, 113.0);
  CheckSelection(L"Banana");

  // Select other another provided option.
  SelectOption(0, 192.0, 110.0);
  CheckSelection(L"Apple");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       GetSelectedTextEmptyAndBasicEditableComboBoxKeyboard) {
  // Test empty selection.
  CheckSelection(L"");

  // Test basic selection of text within user editable combobox using keyboard.
  TypeTextIntoTextField(3, 102.0, 62.0);
  SelectTextWithKeyboard(3, FWL_VKEY_Left, 128.0, 62.0);
  CheckSelection(L"ABC");

  // Select a provided option.
  SelectOption(1, 192.0, 60.0);
  CheckSelection(L"Bar");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       GetSelectedTextEmptyAndBasicEditableComboBoxMouse) {
  // Test empty selection.
  CheckSelection(L"");

  // Test basic selection of text within user editable combobox using mouse.
  TypeTextIntoTextField(3, 102.0, 62.0);
  SelectTextWithMouse(128.0, 103.0, 62.0);
  CheckSelection(L"ABC");

  // Select a provided option.
  SelectOption(2, 192.0, 60.0);
  CheckSelection(L"Qux");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       GetSelectedTextFragmentsNormalComboBox) {
  // Test selecting first character in forward direction.
  SelectTextWithMouse(102.0, 107.0, 113.0);
  CheckSelection(L"B");

  // Test selecting entire string in backwards direction.
  SelectTextWithMouse(142.0, 102.0, 113.0);
  CheckSelection(L"Banana");

  // Test selecting middle section in backwards direction.
  SelectTextWithMouse(135.0, 117.0, 113.0);
  CheckSelection(L"nan");

  // Test selecting middle section in forward direction.
  SelectTextWithMouse(117.0, 135.0, 113.0);
  CheckSelection(L"nan");

  // Test selecting last character in backwards direction.
  SelectTextWithMouse(142.0, 138.0, 113.0);
  CheckSelection(L"a");

  // Select another option and then reset selection as first three chars.
  SelectOption(2, 192.0, 110.0);
  CheckSelection(L"Cherry");
  SelectTextWithMouse(102.0, 122.0, 113.0);
  CheckSelection(L"Che");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       GetSelectedTextFragmentsEditableComboBoxKeyboard) {
  TypeTextIntoTextField(10, 102.0, 62.0);

  // Test selecting first character in forward direction.
  SelectTextWithKeyboard(1, FWL_VKEY_Right, 102.0, 62.0);
  CheckSelection(L"A");

  // Test selecting entire long string in backwards direction.
  SelectTextWithKeyboard(10, FWL_VKEY_Left, 178.0, 62.0);
  CheckSelection(L"ABCDEFGHIJ");

  // Test selecting middle section in backwards direction.
  SelectTextWithKeyboard(5, FWL_VKEY_Left, 168.0, 62.0);
  CheckSelection(L"DEFGH");

  // Test selecting middle selection in forward direction.
  SelectTextWithKeyboard(5, FWL_VKEY_Right, 127.0, 62.0);
  CheckSelection(L"DEFGH");

  // Test selecting last character in backwards direction.
  SelectTextWithKeyboard(1, FWL_VKEY_Left, 178.0, 62.0);
  CheckSelection(L"J");

  // Select a provided option and then reset selection as first two chars.
  SelectOption(0, 192.0, 60.0);
  CheckSelection(L"Foo");
  SelectTextWithKeyboard(2, FWL_VKEY_Right, 102.0, 62.0);
  CheckSelection(L"Fo");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       GetSelectedTextFragmentsEditableComboBoxMouse) {
  TypeTextIntoTextField(10, 102.0, 62.0);

  // Test selecting first character in forward direction.
  SelectTextWithMouse(102.0, 107.0, 62.0);
  CheckSelection(L"A");

  // Test selecting entire long string in backwards direction.
  SelectTextWithMouse(178.0, 102.0, 62.0);
  CheckSelection(L"ABCDEFGHIJ");

  // Test selecting middle section in backwards direction.
  SelectTextWithMouse(168.0, 127.0, 62.0);
  CheckSelection(L"DEFGH");

  // Test selecting middle selection in forward direction.
  SelectTextWithMouse(127.0, 168.0, 62.0);
  CheckSelection(L"DEFGH");

  // Test selecting last character in backwards direction.
  SelectTextWithMouse(178.0, 174.0, 62.0);
  CheckSelection(L"J");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, DeleteTextFieldEntireSelection) {
  // Select entire contents of text field.
  TypeTextIntoTextField(12, 120.0, 120.0);
  SelectTextWithMouse(191.0, 102.0, 115.5);
  CheckSelection(L"ABCDEFGHIJKL");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);

  SelectTextWithKeyboard(12, FWL_VKEY_Left, 191.0, 115.5);
  CheckSelection(L"");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, DeleteTextFieldSelectionMiddle) {
  // Select middle section of text.
  TypeTextIntoTextField(12, 120.0, 120.0);
  SelectTextWithMouse(170.0, 125.0, 115.5);
  CheckSelection(L"DEFGHI");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithKeyboard(12, FWL_VKEY_Left, 191.0, 115.5);
  CheckSelection(L"ABCJKL");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, DeleteTextFieldSelectionLeft) {
  // Select first few characters of text.
  TypeTextIntoTextField(12, 120.0, 120.0);
  SelectTextWithMouse(102.0, 132.0, 115.5);
  CheckSelection(L"ABCD");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithKeyboard(12, FWL_VKEY_Left, 191.0, 115.5);
  CheckSelection(L"EFGHIJKL");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, DeleteTextFieldSelectionRight) {
  // Select last few characters of text.
  TypeTextIntoTextField(12, 120.0, 120.0);
  SelectTextWithMouse(191.0, 165.0, 115.5);
  CheckSelection(L"IJKL");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithKeyboard(12, FWL_VKEY_Left, 191.0, 115.5);
  CheckSelection(L"ABCDEFGH");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, DeleteEmptyTextFieldSelection) {
  // Do not select text.
  TypeTextIntoTextField(12, 120.0, 120.0);
  CheckSelection(L"");

  // Test that attempt to delete empty text selection has no effect.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithKeyboard(12, FWL_VKEY_Left, 191.0, 115.5);
  CheckSelection(L"ABCDEFGHIJKL");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       DeleteEditableComboBoxEntireSelection) {
  // Select entire contents of user-editable combobox text field.
  TypeTextIntoTextField(10, 102.0, 62.0);
  SelectTextWithMouse(178.0, 102.0, 62.0);
  CheckSelection(L"ABCDEFGHIJ");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithMouse(178.0, 102.0, 62.0);
  CheckSelection(L"");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       DeleteEditableComboBoxSelectionMiddle) {
  // Select middle section of text.
  TypeTextIntoTextField(10, 102.0, 62.0);
  SelectTextWithMouse(168.0, 127.0, 62.0);
  CheckSelection(L"DEFGH");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithMouse(178.0, 102.0, 62.0);
  CheckSelection(L"ABCIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       DeleteEditableComboBoxSelectionLeft) {
  // Select first few characters of text.
  TypeTextIntoTextField(10, 102.0, 62.0);
  SelectTextWithMouse(102.0, 132.0, 62.0);
  CheckSelection(L"ABCD");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithMouse(178.0, 102.0, 62.0);
  CheckSelection(L"EFGHIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       DeleteEditableComboBoxSelectionRight) {
  // Select last few characters of text.
  TypeTextIntoTextField(10, 102.0, 62.0);
  SelectTextWithMouse(178.0, 152.0, 62.0);
  CheckSelection(L"GHIJ");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithMouse(178.0, 102.0, 62.0);
  CheckSelection(L"ABCDEF");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       DeleteEmptyEditableComboBoxSelection) {
  // Do not select text.
  TypeTextIntoTextField(10, 102.0, 62.0);
  CheckSelection(L"");

  // Test that attempt to delete empty text selection has no effect.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectTextWithMouse(178.0, 102.0, 62.0);
  CheckSelection(L"ABCDEFGHIJ");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, InsertTextInEmptyTextField) {
  ClickOnFormFieldAtPoint(120.0, 120.0);

  // Test inserting text into empty text field.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 115.5);
  CheckSelection(L"Hello");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, InsertTextInPopulatedTextFieldLeft) {
  TypeTextIntoTextField(8, 120.0, 120.0);

  // Click on the leftmost part of the text field.
  ClickOnFormFieldAtPoint(102.0, 115.5);

  // Test inserting text in front of existing text in text field.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 115.5);
  CheckSelection(L"HelloABCDEFGH");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, InsertTextInPopulatedTextFieldMiddle) {
  TypeTextIntoTextField(8, 120.0, 120.0);

  // Click on the middle of the text field.
  ClickOnFormFieldAtPoint(134.0, 115.5);

  // Test inserting text in the middle of existing text in text field.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 115.5);
  CheckSelection(L"ABCDHelloEFGH");
}

TEST_F(FPDFFormFillTextFormEmbeddertest, InsertTextInPopulatedTextFieldRight) {
  TypeTextIntoTextField(8, 120.0, 120.0);

  // Click on the rightmost part of the text field.
  ClickOnFormFieldAtPoint(166.0, 115.5);

  // Test inserting text behind existing text in text field.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 115.5);
  CheckSelection(L"ABCDEFGHHello");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldWhole) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Select entire string in text field.
  SelectTextWithKeyboard(12, FWL_VKEY_Left, 195.0, 115.0);
  CheckSelection(L"ABCDEFGHIJKL");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 115.5);
  CheckSelection(L"Hello");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldLeft) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Select left portion of string in text field.
  SelectTextWithKeyboard(6, FWL_VKEY_Left, 148.0, 115.0);
  CheckSelection(L"ABCDEF");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 115.5);
  CheckSelection(L"HelloGHIJKL");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldMiddle) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Select middle portion of string in text field.
  SelectTextWithKeyboard(6, FWL_VKEY_Left, 171.0, 115.0);
  CheckSelection(L"DEFGHI");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 115.5);
  CheckSelection(L"ABCHelloJKL");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldRight) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Select right portion of string in text field.
  SelectTextWithKeyboard(6, FWL_VKEY_Left, 195.0, 115.0);
  CheckSelection(L"GHIJKL");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 115.5);
  CheckSelection(L"ABCDEFHello");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       InsertTextInEmptyEditableComboBox) {
  ClickOnFormFieldAtPoint(102.0, 62.0);

  // Test inserting text into empty user-editable combobox.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  SelectTextWithMouse(183.0, 102.0, 62.0);
  CheckSelection(L"Hello");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       InsertTextInPopulatedEditableComboBoxLeft) {
  TypeTextIntoTextField(6, 102.0, 62.0);

  // Click on the leftmost part of the user-editable combobox.
  ClickOnFormFieldAtPoint(102.0, 62.0);

  // Test inserting text in front of existing text in user-editable combobox.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  SelectTextWithMouse(183.0, 102.0, 62.0);
  CheckSelection(L"HelloABCDEF");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       InsertTextInPopulatedEditableComboBoxMiddle) {
  TypeTextIntoTextField(6, 102.0, 62.0);

  // Click on the middle of the user-editable combobox.
  ClickOnFormFieldAtPoint(126.0, 62.0);

  // Test inserting text in the middle of existing text in user-editable
  // combobox.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  SelectTextWithMouse(183.0, 102.0, 62.0);
  CheckSelection(L"ABCHelloDEF");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       InsertTextInPopulatedEditableComboBoxRight) {
  TypeTextIntoTextField(6, 102.0, 62.0);

  // Click on the rightmost part of the user-editable combobox.
  ClickOnFormFieldAtPoint(150.0, 62.0);

  // Test inserting text behind existing text in user-editable combobox.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  SelectTextWithMouse(183.0, 102.0, 62.0);
  CheckSelection(L"ABCDEFHello");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxWhole) {
  TypeTextIntoTextField(10, 102.0, 62.0);

  // Select entire string in user-editable combobox.
  SelectTextWithKeyboard(10, FWL_VKEY_Left, 183.0, 62.0);
  CheckSelection(L"ABCDEFGHIJ");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  SelectTextWithMouse(183.0, 102.0, 62.0);
  CheckSelection(L"Hello");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxLeft) {
  TypeTextIntoTextField(10, 102.0, 62.0);

  // Select left portion of string in user-editable combobox.
  SelectTextWithKeyboard(5, FWL_VKEY_Left, 142.0, 62.0);
  CheckSelection(L"ABCDE");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  SelectTextWithMouse(183.0, 102.0, 62.0);
  CheckSelection(L"HelloFGHIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxMiddle) {
  TypeTextIntoTextField(10, 102.0, 62.0);

  // Select middle portion of string in user-editable combobox.
  SelectTextWithKeyboard(5, FWL_VKEY_Left, 167.0, 62.0);
  CheckSelection(L"DEFGH");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  SelectTextWithMouse(183.0, 102.0, 62.0);
  CheckSelection(L"ABCHelloIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxRight) {
  TypeTextIntoTextField(10, 102.0, 62.0);

  // Select right portion of string in user-editable combobox.
  SelectTextWithKeyboard(5, FWL_VKEY_Left, 183.0, 62.0);
  CheckSelection(L"FGHIJ");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  SelectTextWithMouse(183.0, 102.0, 62.0);
  CheckSelection(L"ABCDEHello");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextInEmptyCharLimitTextFieldOverflow) {
  // Click on the textfield.
  ClickOnFormFieldAtPoint(195.0, 60.0);

  // Delete pre-filled contents of text field with char limit.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"Elephant");
  FORM_ReplaceSelection(form_handle(), page(), nullptr);

  // Test inserting text into now empty text field so text to be inserted
  // exceeds the char limit and is cut off.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"Hippopotam");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextInEmptyCharLimitTextFieldFit) {
  // Click on the textfield.
  ClickOnFormFieldAtPoint(195.0, 60.0);

  // Delete pre-filled contents of text field with char limit.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"Elephant");
  FORM_ReplaceSelection(form_handle(), page(), nullptr);

  // Test inserting text into now empty text field so text to be inserted
  // exceeds the char limit and is cut off.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Zebra");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"Zebra");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextInPopulatedCharLimitTextFieldLeft) {
  // Click on the leftmost part of the text field.
  ClickOnFormFieldAtPoint(102.0, 60.0);

  // Test inserting text in front of existing text in text field.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"HiElephant");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextInPopulatedCharLimitTextFieldMiddle) {
  TypeTextIntoTextField(8, 120.0, 120.0);

  // Click on the middle of the text field.
  ClickOnFormFieldAtPoint(134.0, 60.0);

  // Test inserting text in the middle of existing text in text field.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"ElephHiant");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextInPopulatedCharLimitTextFieldRight) {
  TypeTextIntoTextField(8, 120.0, 120.0);

  // Click on the rightmost part of the text field.
  ClickOnFormFieldAtPoint(166.0, 60.0);

  // Test inserting text behind existing text in text field.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"ElephantHi");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldWhole) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Select entire string in text field.
  SelectTextWithKeyboard(12, FWL_VKEY_Left, 195.0, 60.0);
  CheckSelection(L"Elephant");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"Hippopotam");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldLeft) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Select left portion of string in text field.
  SelectTextWithKeyboard(4, FWL_VKEY_Left, 122.0, 60.0);
  CheckSelection(L"Elep");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"Hippophant");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldMiddle) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Select middle portion of string in text field.
  SelectTextWithKeyboard(4, FWL_VKEY_Left, 136.0, 60.0);
  CheckSelection(L"epha");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"ElHippopnt");
}

TEST_F(FPDFFormFillTextFormEmbeddertest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldRight) {
  TypeTextIntoTextField(12, 120.0, 120.0);

  // Select right portion of string in text field.
  SelectTextWithKeyboard(4, FWL_VKEY_Left, 152.0, 60.0);
  CheckSelection(L"hant");

  // Test replacing text selection with text to be inserted.
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> text_to_insert =
      GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  SelectTextWithMouse(195.0, 102.0, 60.0);
  CheckSelection(L"ElepHippop");
}

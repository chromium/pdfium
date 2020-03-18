// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>
#include <vector>

#include "build/build_config.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "public/cpp/fpdf_scopers.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"
#include "public/fpdf_progressive.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_mock_delegate.h"
#include "testing/embedder_test_timer_handling_delegate.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;

using FPDFFormFillEmbedderTest = EmbedderTest;

// A base class for many related tests that involve clicking and typing into
// form fields.
class FPDFFormFillInteractiveEmbedderTest : public FPDFFormFillEmbedderTest {
 protected:
  FPDFFormFillInteractiveEmbedderTest() = default;
  ~FPDFFormFillInteractiveEmbedderTest() override = default;

  void SetUp() override {
    FPDFFormFillEmbedderTest::SetUp();
    ASSERT_TRUE(OpenDocument(GetDocumentName()));
    page_ = LoadPage(0);
    ASSERT_TRUE(page_);
    FormSanityChecks();
  }

  void TearDown() override {
    UnloadPage(page_);
    FPDFFormFillEmbedderTest::TearDown();
  }

  // Returns the name of the PDF to use.
  virtual const char* GetDocumentName() const = 0;

  // Returns the type of field(s) in the PDF.
  virtual int GetFormType() const = 0;

  // Optionally do some sanity check on the document after loading.
  virtual void FormSanityChecks() {}

  FPDF_PAGE page() { return page_; }

  int GetFormTypeAtPoint(const CFX_PointF& point) {
    return FPDFPage_HasFormFieldAtPoint(form_handle(), page_, point.x, point.y);
  }

  void ClickOnFormFieldAtPoint(const CFX_PointF& point) {
    // Click on the text field or combobox as specified by coordinates.
    FORM_OnMouseMove(form_handle(), page_, 0, point.x, point.y);
    FORM_OnLButtonDown(form_handle(), page_, 0, point.x, point.y);
    FORM_OnLButtonUp(form_handle(), page_, 0, point.x, point.y);
  }

  void DoubleClickOnFormFieldAtPoint(const CFX_PointF& point) {
    // Click on the text field or combobox as specified by coordinates.
    FORM_OnMouseMove(form_handle(), page_, 0, point.x, point.y);
    FORM_OnLButtonDoubleClick(form_handle(), page_, 0, point.x, point.y);
  }

  void TypeTextIntoTextField(int num_chars, const CFX_PointF& point) {
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(point));
    ClickOnFormFieldAtPoint(point);

    // Type text starting with 'A' to as many chars as specified by |num_chars|.
    for (int i = 0; i < num_chars; ++i) {
      FORM_OnChar(form_handle(), page_, 'A' + i, 0);
    }
  }

  // Navigates to text field using the mouse and then selects text via the
  // shift and specfied left or right arrow key.
  void SelectTextWithKeyboard(int num_chars,
                              int arrow_key,
                              const CFX_PointF& point) {
    // Navigate to starting position for selection.
    ClickOnFormFieldAtPoint(point);

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
  void SelectTextWithMouse(const CFX_PointF& start, const CFX_PointF& end) {
    ASSERT(start.y == end.y);

    // Navigate to starting position and click mouse.
    FORM_OnMouseMove(form_handle(), page_, 0, start.x, start.y);
    FORM_OnLButtonDown(form_handle(), page_, 0, start.x, start.y);

    // Hold down mouse until reach end of desired selection.
    FORM_OnMouseMove(form_handle(), page_, 0, end.x, end.y);
    FORM_OnLButtonUp(form_handle(), page_, 0, end.x, end.y);
  }

  void CheckSelection(WideStringView expected_string) {
    unsigned long actual_len =
        FORM_GetSelectedText(form_handle(), page_, nullptr, 0);
    ASSERT_NE(actual_len, 0U);
    ASSERT_LT(actual_len, 1000U);

    std::vector<unsigned short> buf(actual_len);
    ASSERT_EQ(actual_len, FORM_GetSelectedText(form_handle(), page_, buf.data(),
                                               actual_len));

    int num_chars = (actual_len / sizeof(unsigned short)) - 1;
    EXPECT_EQ(expected_string, WideString::FromUTF16LE(buf.data(), num_chars));
  }

  void CheckFocusedFieldText(WideStringView expected_string) {
    unsigned long actual_len =
        FORM_GetFocusedText(form_handle(), page_, nullptr, 0);
    ASSERT_NE(actual_len, 0U);
    ASSERT_LT(actual_len, 1000U);

    std::vector<unsigned short> buf(actual_len);
    ASSERT_EQ(actual_len, FORM_GetFocusedText(form_handle(), page_, buf.data(),
                                              actual_len));

    int num_chars = (actual_len / sizeof(unsigned short)) - 1;
    EXPECT_EQ(expected_string, WideString::FromUTF16LE(buf.data(), num_chars));
  }

  void CheckCanUndo(bool expected_result) {
    EXPECT_EQ(expected_result, !!FORM_CanUndo(form_handle(), page_));
  }

  void CheckCanRedo(bool expected_result) {
    EXPECT_EQ(expected_result, !!FORM_CanRedo(form_handle(), page_));
  }

  void PerformUndo() { EXPECT_TRUE(FORM_Undo(form_handle(), page_)); }

  void PerformRedo() { EXPECT_TRUE(FORM_Redo(form_handle(), page_)); }

  void SetIndexSelectedShouldSucceed(int index, bool selected) {
    EXPECT_TRUE(FORM_SetIndexSelected(form_handle(), page_, index, selected));
  }

  void SetIndexSelectedShouldFail(int index, bool selected) {
    EXPECT_FALSE(FORM_SetIndexSelected(form_handle(), page_, index, selected));
  }

  void CheckIsIndexSelected(int index, bool expected) {
    EXPECT_EQ(expected, FORM_IsIndexSelected(form_handle(), page_, index));
  }

 private:
  FPDF_PAGE page_ = nullptr;
};

class FPDFFormFillTextFormEmbedderTest
    : public FPDFFormFillInteractiveEmbedderTest {
 protected:
  FPDFFormFillTextFormEmbedderTest() = default;
  ~FPDFFormFillTextFormEmbedderTest() override = default;

  const char* GetDocumentName() const override {
    // PDF with several form text fields:
    // - "Text Box" - Regular text box with no special attributes.
    // - "ReadOnly" - Ff: 1.
    // - "CharLimit" - MaxLen: 10, V: Elephant.
    return "text_form_multiple.pdf";
  }

  int GetFormType() const override { return FPDF_FORMFIELD_TEXTFIELD; }

  void FormSanityChecks() override {
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(CharLimitFormBegin()));
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(CharLimitFormEnd()));
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(RegularFormBegin()));
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(RegularFormEnd()));
  }

  void SelectAllCharLimitFormTextWithMouse() {
    SelectTextWithMouse(CharLimitFormEnd(), CharLimitFormBegin());
  }

  void SelectAllRegularFormTextWithMouse() {
    SelectTextWithMouse(RegularFormEnd(), RegularFormBegin());
  }

  const CFX_PointF& CharLimitFormBegin() const {
    static const CFX_PointF point = CharLimitFormAtX(kFormBeginX);
    return point;
  }

  const CFX_PointF& CharLimitFormEnd() const {
    static const CFX_PointF point = CharLimitFormAtX(kFormEndX);
    return point;
  }

  const CFX_PointF& RegularFormBegin() const {
    static const CFX_PointF point = RegularFormAtX(kFormBeginX);
    return point;
  }

  const CFX_PointF& RegularFormEnd() const {
    static const CFX_PointF point = RegularFormAtX(kFormEndX);
    return point;
  }

  static CFX_PointF CharLimitFormAtX(float x) {
    ASSERT(x >= kFormBeginX);
    ASSERT(x <= kFormEndX);
    return CFX_PointF(x, kCharLimitFormY);
  }

  static CFX_PointF RegularFormAtX(float x) {
    ASSERT(x >= kFormBeginX);
    ASSERT(x <= kFormEndX);
    return CFX_PointF(x, kRegularFormY);
  }

 private:
  static constexpr float kFormBeginX = 102.0;
  static constexpr float kFormEndX = 195.0;
  static constexpr float kCharLimitFormY = 60.0;
  static constexpr float kRegularFormY = 115.0;
};

class FPDFFormFillComboBoxFormEmbedderTest
    : public FPDFFormFillInteractiveEmbedderTest {
 protected:
  FPDFFormFillComboBoxFormEmbedderTest() = default;
  ~FPDFFormFillComboBoxFormEmbedderTest() override = default;

  const char* GetDocumentName() const override {
    // PDF with form comboboxes:
    // - "Combo_Editable" - Ff: 393216, 3 options with pair values.
    // - "Combo1" - Ff: 131072, 3 options with single values.
    // - "Combo_ReadOnly" - Ff: 131073, 3 options with single values.
    return "combobox_form.pdf";
  }

  int GetFormType() const override { return FPDF_FORMFIELD_COMBOBOX; }

  void FormSanityChecks() override {
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(EditableFormBegin()));
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(EditableFormEnd()));
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(EditableFormDropDown()));
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(NonEditableFormBegin()));
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(NonEditableFormEnd()));
    EXPECT_EQ(GetFormType(), GetFormTypeAtPoint(NonEditableFormDropDown()));
  }

  void SelectEditableFormOption(int item_index) {
    ASSERT(item_index >= 0);
    ASSERT(item_index < 3);
    SelectOption(item_index, EditableFormDropDown());
  }

  void SelectNonEditableFormOption(int item_index) {
    ASSERT(item_index >= 0);
    ASSERT(item_index < 26);
    SelectOption(item_index, NonEditableFormDropDown());
  }

  void SelectAllEditableFormTextWithMouse() {
    SelectTextWithMouse(EditableFormEnd(), EditableFormBegin());
  }

  void FocusOnEditableForm() { FocusOnPoint(EditableFormDropDown()); }

  void FocusOnNonEditableForm() { FocusOnPoint(NonEditableFormDropDown()); }

  void FocusOnPoint(const CFX_PointF& point) {
    EXPECT_EQ(true, FORM_OnFocus(form_handle(), page(), 0, point.x, point.y));
  }

  const CFX_PointF& EditableFormBegin() const {
    static const CFX_PointF point = EditableFormAtX(kFormBeginX);
    return point;
  }

  const CFX_PointF& EditableFormEnd() const {
    static const CFX_PointF point = EditableFormAtX(kFormEndX);
    return point;
  }

  const CFX_PointF& EditableFormDropDown() const {
    static const CFX_PointF point(kFormDropDownX, kEditableFormY);
    return point;
  }

  const CFX_PointF& NonEditableFormBegin() const {
    static const CFX_PointF point = NonEditableFormAtX(kFormBeginX);
    return point;
  }

  const CFX_PointF& NonEditableFormEnd() const {
    static const CFX_PointF point = NonEditableFormAtX(kFormEndX);
    return point;
  }

  const CFX_PointF& NonEditableFormDropDown() const {
    static const CFX_PointF point(kFormDropDownX, kNonEditableFormY);
    return point;
  }

  static CFX_PointF EditableFormAtX(float x) {
    ASSERT(x >= kFormBeginX);
    ASSERT(x <= kFormEndX);
    return CFX_PointF(x, kEditableFormY);
  }

  static CFX_PointF NonEditableFormAtX(float x) {
    ASSERT(x >= kFormBeginX);
    ASSERT(x <= kFormEndX);
    return CFX_PointF(x, kNonEditableFormY);
  }

 private:
  // Selects one of the pre-selected values from a combobox with three options.
  // Options are specified by |item_index|, which is 0-based.
  void SelectOption(int item_index, const CFX_PointF& point) {
    // Navigate to button for drop down and click mouse to reveal options.
    ClickOnFormFieldAtPoint(point);

    // Calculate to Y-coordinate of dropdown option to be selected.
    constexpr double kChoiceHeight = 15;
    CFX_PointF option_point = point;
    option_point.y -= kChoiceHeight * (item_index + 1);

    // Move left to avoid scrollbar.
    option_point.x -= 20;

    // Navigate to option and click mouse to select it.
    ClickOnFormFieldAtPoint(option_point);
  }

  static constexpr float kFormBeginX = 102.0;
  static constexpr float kFormEndX = 183.0;
  static constexpr float kFormDropDownX = 192.0;
  static constexpr float kEditableFormY = 360.0;
  static constexpr float kNonEditableFormY = 410.0;
};

class FPDFFormFillListBoxFormEmbedderTest
    : public FPDFFormFillInteractiveEmbedderTest {
 protected:
  FPDFFormFillListBoxFormEmbedderTest() = default;
  ~FPDFFormFillListBoxFormEmbedderTest() override = default;

  const char* GetDocumentName() const override {
    // PDF with form listboxes:
    // - "Listbox_SingleSelect" - Ff: 0, 3 options with pair values.
    // - "Listbox_MultiSelect" - Ff: 2097152, 26 options with single values.
    // - "Listbox_ReadOnly" - Ff: 1, 3 options with single values.
    // - "Listbox_MultiSelectMultipleSelected" - Ff: 2097152, 5 options with
    // single values.
    // - "Listbox_SingleSelectLastSelected" - Ff: 0, 10 options with single
    // values.
    return "listbox_form.pdf";
  }

  int GetFormType() const override { return FPDF_FORMFIELD_LISTBOX; }

  void FormSanityChecks() override {
    EXPECT_EQ(GetFormType(),
              GetFormTypeAtPoint(SingleSelectFirstVisibleOption()));
    EXPECT_EQ(GetFormType(),
              GetFormTypeAtPoint(SingleSelectSecondVisibleOption()));
    EXPECT_EQ(GetFormType(),
              GetFormTypeAtPoint(MultiSelectFirstVisibleOption()));
    EXPECT_EQ(GetFormType(),
              GetFormTypeAtPoint(MultiSelectSecondVisibleOption()));
    EXPECT_EQ(
        GetFormType(),
        GetFormTypeAtPoint(MultiSelectMultipleSelectedFirstVisibleOption()));
    EXPECT_EQ(
        GetFormType(),
        GetFormTypeAtPoint(MultiSelectMultipleSelectedSecondVisibleOption()));
    EXPECT_EQ(GetFormType(),
              GetFormTypeAtPoint(SingleSelectLastSelectedFirstVisibleOption()));
    EXPECT_EQ(
        GetFormType(),
        GetFormTypeAtPoint(SingleSelectLastSelectedSecondVisibleOption()));
  }

  void ClickOnSingleSelectFormOption(int item_index) {
    // Only the first two indices are visible so can only click on those
    // without scrolling.
    ASSERT(item_index >= 0);
    ASSERT(item_index < 2);
    if (item_index == 0) {
      ClickOnFormFieldAtPoint(SingleSelectFirstVisibleOption());
    } else {
      ClickOnFormFieldAtPoint(SingleSelectSecondVisibleOption());
    }
  }

  void ClickOnMultiSelectFormOption(int item_index) {
    // Only the first two indices are visible so can only click on those
    // without scrolling.
    ASSERT(item_index >= 0);
    ASSERT(item_index < 2);
    if (item_index == 0) {
      ClickOnFormFieldAtPoint(MultiSelectFirstVisibleOption());
    } else {
      ClickOnFormFieldAtPoint(MultiSelectSecondVisibleOption());
    }
  }

  void ClickOnMultiSelectMultipleSelectedFormOption(int item_index) {
    // Only two indices are visible so can only click on those
    // without scrolling.
    ASSERT(item_index >= 0);
    ASSERT(item_index < 2);
    if (item_index == 0) {
      ClickOnFormFieldAtPoint(MultiSelectMultipleSelectedFirstVisibleOption());
    } else {
      ClickOnFormFieldAtPoint(MultiSelectMultipleSelectedSecondVisibleOption());
    }
  }

  void ClickOnSingleSelectLastSelectedFormOption(int item_index) {
    // Only two indices are visible so can only click on those
    // without scrolling.
    ASSERT(item_index >= 0);
    ASSERT(item_index < 2);
    if (item_index == 0) {
      ClickOnFormFieldAtPoint(SingleSelectLastSelectedFirstVisibleOption());
    } else {
      ClickOnFormFieldAtPoint(SingleSelectLastSelectedSecondVisibleOption());
    }
  }

  void FocusOnSingleSelectForm() {
    FocusOnPoint(SingleSelectFirstVisibleOption());
  }

  void FocusOnMultiSelectForm() {
    FocusOnPoint(MultiSelectFirstVisibleOption());
  }

  void FocusOnMultiSelectMultipleSelectedForm() {
    FocusOnPoint(MultiSelectMultipleSelectedFirstVisibleOption());
  }

  void FocusOnSingleSelectLastSelectedForm() {
    FocusOnPoint(SingleSelectLastSelectedFirstVisibleOption());
  }

  void FocusOnPoint(const CFX_PointF& point) {
    EXPECT_EQ(true, FORM_OnFocus(form_handle(), page(), 0, point.x, point.y));
  }

  const CFX_PointF& SingleSelectFirstVisibleOption() const {
    static const CFX_PointF point(kFormBeginX, kSingleFormYFirstVisibleOption);
    return point;
  }

  const CFX_PointF& SingleSelectSecondVisibleOption() const {
    static const CFX_PointF point(kFormBeginX, kSingleFormYSecondVisibleOption);
    return point;
  }

  const CFX_PointF& MultiSelectFirstVisibleOption() const {
    static const CFX_PointF point(kFormBeginX, kMultiFormYFirstVisibleOption);
    return point;
  }

  const CFX_PointF& MultiSelectSecondVisibleOption() const {
    static const CFX_PointF point(kFormBeginX, kMultiFormYSecondVisibleOption);
    return point;
  }

  const CFX_PointF& MultiSelectMultipleSelectedFirstVisibleOption() const {
    static const CFX_PointF point(
        kFormBeginX, kMultiFormMultipleSelectedYFirstVisibleOption);
    return point;
  }

  const CFX_PointF& MultiSelectMultipleSelectedSecondVisibleOption() const {
    static const CFX_PointF point(
        kFormBeginX, kMultiFormMultipleSelectedYSecondVisibleOption);
    return point;
  }

  const CFX_PointF& SingleSelectLastSelectedFirstVisibleOption() const {
    static const CFX_PointF point(kFormBeginX,
                                  kSingleFormLastSelectedYFirstVisibleOption);
    return point;
  }

  const CFX_PointF& SingleSelectLastSelectedSecondVisibleOption() const {
    static const CFX_PointF point(kFormBeginX,
                                  kSingleFormLastSelectedYSecondVisibleOption);
    return point;
  }

 private:
  static constexpr float kFormBeginX = 102.0;
  static constexpr float kSingleFormYFirstVisibleOption = 371.0;
  static constexpr float kSingleFormYSecondVisibleOption = 358.0;
  static constexpr float kMultiFormYFirstVisibleOption = 423.0;
  static constexpr float kMultiFormYSecondVisibleOption = 408.0;
  static constexpr float kMultiFormMultipleSelectedYFirstVisibleOption = 223.0;
  static constexpr float kMultiFormMultipleSelectedYSecondVisibleOption = 208.0;
  static constexpr float kSingleFormLastSelectedYFirstVisibleOption = 123.0;
  static constexpr float kSingleFormLastSelectedYSecondVisibleOption = 108.0;
};

class FPDFFormFillTextFormEmbedderTestVersion2
    : public FPDFFormFillTextFormEmbedderTest {
  void SetUp() override {
    SetFormFillInfoVersion(2);
    FPDFFormFillInteractiveEmbedderTest::SetUp();
  }
};

TEST_F(FPDFFormFillEmbedderTest, FirstTest) {
  EmbedderTestMockDelegate mock;
  EXPECT_CALL(mock, Alert(_, _, _, _)).Times(0);
  EXPECT_CALL(mock, UnsupportedHandler(_)).Times(0);
  EXPECT_CALL(mock, SetTimer(_, _)).Times(0);
  EXPECT_CALL(mock, KillTimer(_)).Times(0);
  EXPECT_CALL(mock, OnFocusChange(_, _, _)).Times(0);
  SetDelegate(&mock);

  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, BUG_487928) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_487928.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  DoOpenActions();
  delegate.AdvanceTime(5000);
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, BUG_507316) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_507316.pdf"));
  FPDF_PAGE page = LoadPage(2);
  EXPECT_TRUE(page);
  DoOpenActions();
  delegate.AdvanceTime(4000);
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, BUG_514690) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  // Test that FORM_OnMouseMove() etc. permit null HANDLES and PAGES.
  FORM_OnMouseMove(nullptr, page, 0, 10.0, 10.0);
  FORM_OnMouseMove(form_handle(), nullptr, 0, 10.0, 10.0);

  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, BUG_900552) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_900552.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  DoOpenActions();
  delegate.AdvanceTime(4000);

  // Simulate a repaint.
  FPDF_BITMAP bitmap = FPDFBitmap_Create(512, 512, 0);
  ASSERT_TRUE(bitmap);
  FPDF_RenderPageBitmap_Start(bitmap, page, 0, 0, 512, 512, 0, 0, nullptr);
  FPDFBitmap_Destroy(bitmap);
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, BUG_901654) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_901654.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  DoOpenActions();
  delegate.AdvanceTime(4000);

  // Simulate a repaint.
  {
    ScopedFPDFBitmap bitmap(FPDFBitmap_Create(512, 512, 0));
    FPDF_RenderPageBitmap_Start(bitmap.get(), page, 0, 0, 512, 512, 0, 0,
                                nullptr);
  }
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, BUG_901654_2) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("bug_901654_2.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  DoOpenActions();
  delegate.AdvanceTime(4000);

  // Simulate a repaint.
  {
    ScopedFPDFBitmap bitmap(FPDFBitmap_Create(512, 512, 0));
    FPDF_RenderPageBitmap_Start(bitmap.get(), page, 0, 0, 512, 512, 0, 0,
                                nullptr);
  }
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, GetFocusedAnnotation) {
  ASSERT_TRUE(OpenDocument("annotiter.pdf"));
  std::vector<FPDF_PAGE> pages;
  for (size_t i = 0; i < 3; ++i) {
    pages.push_back(LoadPage(i));
    ASSERT_TRUE(pages.back());
  }

  // Ensure that there is no focused annotation.
  FPDF_ANNOTATION annot = nullptr;
  int page_index = -2;
  ASSERT_TRUE(FORM_GetFocusedAnnot(form_handle(), &page_index, &annot));
  EXPECT_FALSE(annot);
  EXPECT_EQ(-1, page_index);

  // Validate that nullptr values are handled properly.
  EXPECT_FALSE(FORM_GetFocusedAnnot(nullptr, &page_index, &annot));
  EXPECT_FALSE(FORM_GetFocusedAnnot(form_handle(), &page_index, nullptr));
  EXPECT_FALSE(FORM_GetFocusedAnnot(form_handle(), nullptr, &annot));

  const CFX_PointF right_bottom_annot_point(410.0f, 210.0f);
  constexpr int kExpectedAnnotIndex = 3;

  for (size_t i = 0; i < pages.size(); ++i) {
    // Invoke click on the form field to bring it to focus.
    FORM_OnMouseMove(form_handle(), pages[i], 0, right_bottom_annot_point.x,
                     right_bottom_annot_point.y);
    FORM_OnLButtonDown(form_handle(), pages[i], 0, right_bottom_annot_point.x,
                       right_bottom_annot_point.y);
    FORM_OnLButtonUp(form_handle(), pages[i], 0, right_bottom_annot_point.x,
                     right_bottom_annot_point.y);

    ASSERT_TRUE(FORM_GetFocusedAnnot(form_handle(), &page_index, &annot));
    ASSERT_TRUE(annot);

    EXPECT_EQ(kExpectedAnnotIndex, FPDFPage_GetAnnotIndex(pages[i], annot));
    EXPECT_EQ(static_cast<int>(i), page_index);

    FPDFPage_CloseAnnot(annot);
  }

  for (FPDF_PAGE page : pages)
    UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, SetFocusedAnnotation) {
  ASSERT_TRUE(OpenDocument("annotiter.pdf"));
  std::vector<FPDF_PAGE> pages;
  for (size_t i = 0; i < 3; ++i) {
    pages.push_back(LoadPage(i));
    ASSERT_TRUE(pages.back());
  }

  // Ensure that there is no focused annotation.
  FPDF_ANNOTATION annot = nullptr;
  int page_index = -2;
  ASSERT_TRUE(FORM_GetFocusedAnnot(form_handle(), &page_index, &annot));
  EXPECT_FALSE(annot);
  EXPECT_EQ(-1, page_index);

  // Validate that nullptr values are handled properly.
  EXPECT_FALSE(FORM_SetFocusedAnnot(nullptr, pages[0], annot));
  EXPECT_FALSE(FORM_SetFocusedAnnot(form_handle(), nullptr, annot));
  EXPECT_FALSE(FORM_SetFocusedAnnot(form_handle(), pages[0], nullptr));

  constexpr int kExpectedAnnotIndex = 2;

  for (size_t i = 0; i < pages.size(); ++i) {
    // Setting focus on an annotation on page i.
    ScopedFPDFAnnotation focused_annot(
        FPDFPage_GetAnnot(pages[i], kExpectedAnnotIndex));
    ASSERT_TRUE(focused_annot);

    ASSERT_TRUE(
        FORM_SetFocusedAnnot(form_handle(), pages[i], focused_annot.get()));

    ASSERT_TRUE(FORM_GetFocusedAnnot(form_handle(), &page_index, &annot));
    EXPECT_EQ(kExpectedAnnotIndex, FPDFPage_GetAnnotIndex(pages[i], annot));
    EXPECT_EQ(static_cast<int>(i), page_index);

    FPDFPage_CloseAnnot(annot);
  }

  for (FPDF_PAGE page : pages)
    UnloadPage(page);
}

class DoURIActionBlockedDelegate final : public EmbedderTest::Delegate {
 public:
  void DoURIAction(FPDF_BYTESTRING uri) override {
    FAIL() << "Navigated to " << uri;
  }
};

TEST_F(FPDFFormFillEmbedderTest, BUG_851821) {
  DoURIActionBlockedDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("redirect.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  DoOpenActions();

  UnloadPage(page);
}

#ifdef PDF_ENABLE_V8
TEST_F(FPDFFormFillEmbedderTest, DisableJavaScript) {
  // Test that timers and intervals can't fire without JS.
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocumentWithoutJavaScript("bug_551248.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  DoOpenActions();

  const auto& alerts = delegate.GetAlerts();
  EXPECT_EQ(0U, alerts.size());

  delegate.AdvanceTime(1000);
  EXPECT_EQ(0U, alerts.size());  // nothing fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(0U, alerts.size());  // nothing fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(0U, alerts.size());  // nothing fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(0U, alerts.size());  // nothing fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(0U, alerts.size());  // nothing fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(0U, alerts.size());  // nothing fired.
  delegate.AdvanceTime(1000);
  EXPECT_EQ(0U, alerts.size());  // nothing fired.
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, DocumentAActions) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("document_aactions.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  const auto& alerts = delegate.GetAlerts();
  EXPECT_EQ(0U, alerts.size());

  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_WS);
  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_DS);
  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_WP);
  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_DP);
  UnloadPage(page);

  ASSERT_EQ(4U, alerts.size());
  EXPECT_STREQ(L"Will Save", alerts[0].message.c_str());
  EXPECT_STREQ(L"Did Save", alerts[1].message.c_str());
  EXPECT_STREQ(L"Will Print", alerts[2].message.c_str());
  EXPECT_STREQ(L"Did Print", alerts[3].message.c_str());
}

TEST_F(FPDFFormFillEmbedderTest, DocumentAActionsDisableJavaScript) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocumentWithoutJavaScript("document_aactions.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  const auto& alerts = delegate.GetAlerts();
  EXPECT_EQ(0U, alerts.size());

  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_WS);
  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_DS);
  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_WP);
  FORM_DoDocumentAAction(form_handle(), FPDFDOC_AACTION_DP);
  UnloadPage(page);

  ASSERT_EQ(0U, alerts.size());
}

TEST_F(FPDFFormFillEmbedderTest, BUG_551248) {
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

TEST_F(FPDFFormFillEmbedderTest, BUG_620428) {
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

TEST_F(FPDFFormFillEmbedderTest, BUG_634394) {
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

TEST_F(FPDFFormFillEmbedderTest, BUG_634716) {
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

TEST_F(FPDFFormFillEmbedderTest, BUG_679649) {
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

TEST_F(FPDFFormFillEmbedderTest, BUG_707673) {
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

TEST_F(FPDFFormFillEmbedderTest, BUG_765384) {
  EXPECT_TRUE(OpenDocument("bug_765384.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  DoOpenActions();
  FORM_OnLButtonDown(form_handle(), page, 0, 140, 590);
  FORM_OnLButtonUp(form_handle(), page, 0, 140, 590);
  UnloadPage(page);
}
#endif  // PDF_ENABLE_V8

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_FormText DISABLED_FormText
#else
#define MAYBE_FormText FormText
#endif
TEST_F(FPDFFormFillEmbedderTest, MAYBE_FormText) {
#if defined(OS_MACOSX)
  const char md5_1[] = "d485541d958fef08d24e8eca3e537023";
  const char md5_2[] = "c6e4a2fb10661116771ee74f54d9c5e0";
  const char md5_3[] = "e0c8d5099301d7c10ed831a43e974d9d";
#elif defined(OS_WIN)
  const char md5_1[] = "d3204faa62b607f0bd3893c9c22cabcb";
  const char md5_2[] = "29d1c3fd226ca6a69597f75937690320";
  const char md5_3[] = "5e678a55912cb568fd677bf34abb8727";
#else
  const char md5_1[] = "b890950d4b9bc163b1a96797f3004b53";
  const char md5_2[] = "11487d5597599a26e8912b9c1d9422cb";
  const char md5_3[] = "bffe0ecea9a533f217047ee41d6be466";
#endif
  {
    EXPECT_TRUE(OpenDocument("text_form.pdf"));
    FPDF_PAGE page = LoadPage(0);
    ASSERT_TRUE(page);
    ScopedFPDFBitmap bitmap1 = RenderLoadedPage(page);
    CompareBitmap(bitmap1.get(), 300, 300, md5_1);

    // Click on the textfield
    EXPECT_EQ(FPDF_FORMFIELD_TEXTFIELD,
              FPDFPage_HasFormFieldAtPoint(form_handle(), page, 120.0, 120.0));
    EXPECT_EQ(
        0, FPDFPage_FormFieldZOrderAtPoint(form_handle(), page, 120.0, 120.0));
    FORM_OnMouseMove(form_handle(), page, 0, 120.0, 120.0);
    FORM_OnLButtonDown(form_handle(), page, 0, 120.0, 120.0);
    FORM_OnLButtonUp(form_handle(), page, 0, 120.0, 120.0);

    // Write "ABC"
    FORM_OnChar(form_handle(), page, 65, 0);
    FORM_OnChar(form_handle(), page, 66, 0);
    FORM_OnChar(form_handle(), page, 67, 0);
    ScopedFPDFBitmap bitmap2 = RenderLoadedPage(page);
    CompareBitmap(bitmap2.get(), 300, 300, md5_2);

    // Focus remains despite right clicking out of the textfield
    FORM_OnMouseMove(form_handle(), page, 0, 15.0, 15.0);
    FORM_OnRButtonDown(form_handle(), page, 0, 15.0, 15.0);
    FORM_OnRButtonUp(form_handle(), page, 0, 15.0, 15.0);
    ScopedFPDFBitmap bitmap3 = RenderLoadedPage(page);
    CompareBitmap(bitmap3.get(), 300, 300, md5_2);

    // Take out focus by clicking out of the textfield
    FORM_OnMouseMove(form_handle(), page, 0, 15.0, 15.0);
    FORM_OnLButtonDown(form_handle(), page, 0, 15.0, 15.0);
    FORM_OnLButtonUp(form_handle(), page, 0, 15.0, 15.0);
    ScopedFPDFBitmap bitmap4 = RenderLoadedPage(page);
    CompareBitmap(bitmap4.get(), 300, 300, md5_3);

    EXPECT_TRUE(FPDF_SaveAsCopy(document(), this, 0));

    // Close page
    UnloadPage(page);
  }
  // Check saved document
  VerifySavedDocument(300, 300, md5_3);
}

// Tests using FPDF_REVERSE_BYTE_ORDER with FPDF_FFLDraw(). The two rendered
// bitmaps should be different.
// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_BUG_1281 DISABLED_BUG_1281
#else
#define MAYBE_BUG_1281 BUG_1281
#endif
TEST_F(FPDFFormFillEmbedderTest, MAYBE_BUG_1281) {
  const char kMd5Normal[] = "6c674642154408e877d88c6c082d67e9";
  const char kMd5ReverseByteOrder[] = "24fff03d1e663b7ece5f6e69ad837124";

  ASSERT_TRUE(OpenDocument("bug_890322.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  ScopedFPDFBitmap bitmap_normal = RenderLoadedPage(page);
  CompareBitmap(bitmap_normal.get(), 200, 200, kMd5Normal);

  ScopedFPDFBitmap bitmap_reverse_byte_order =
      RenderLoadedPageWithFlags(page, FPDF_REVERSE_BYTE_ORDER);
  CompareBitmap(bitmap_reverse_byte_order.get(), 200, 200,
                kMd5ReverseByteOrder);

  UnloadPage(page);
}

#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_RemoveFormFieldHighlight DISABLED_RemoveFormFieldHighlight
#else
#define MAYBE_RemoveFormFieldHighlight RemoveFormFieldHighlight
#endif
TEST_F(FPDFFormFillEmbedderTest, MAYBE_RemoveFormFieldHighlight) {
#if defined(OS_MACOSX)
  const char kMd5Normal[] = "d485541d958fef08d24e8eca3e537023";
  const char kMd5NoHighlight[] = "5e4b87c5b304c6fa9bd5f6311260494e";
#elif defined(OS_WIN)
  const char kMd5Normal[] = "d3204faa62b607f0bd3893c9c22cabcb";
  const char kMd5NoHighlight[] = "3ec0938828e0a37ef23f687ee95a80e1";
#else
  const char kMd5Normal[] = "b890950d4b9bc163b1a96797f3004b53";
  const char kMd5NoHighlight[] = "006010c318457810a518aa5e0b33c498";
#endif

  EXPECT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);
  ScopedFPDFBitmap bitmap1 = RenderLoadedPage(page);
  CompareBitmap(bitmap1.get(), 300, 300, kMd5Normal);

  // Removing the highlight changes the rendering.
  FPDF_RemoveFormFieldHighlight(form_handle());
  ScopedFPDFBitmap bitmap2 = RenderLoadedPage(page);
  CompareBitmap(bitmap2.get(), 300, 300, kMd5NoHighlight);

  // Restoring it gives the original rendering.
  SetInitialFormFieldHighlight(form_handle());
  ScopedFPDFBitmap bitmap3 = RenderLoadedPage(page);
  CompareBitmap(bitmap3.get(), 300, 300, kMd5Normal);

  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, HasFormInfoNone) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  EXPECT_EQ(FORMTYPE_NONE, FPDF_GetFormType(document_));
}

TEST_F(FPDFFormFillEmbedderTest, HasFormInfoAcroForm) {
  EXPECT_TRUE(OpenDocument("text_form.pdf"));
  EXPECT_EQ(FORMTYPE_ACRO_FORM, FPDF_GetFormType(document_));
}

TEST_F(FPDFFormFillEmbedderTest, HasFormInfoXFAFull) {
  EXPECT_TRUE(OpenDocument("simple_xfa.pdf"));
  EXPECT_EQ(FORMTYPE_XFA_FULL, FPDF_GetFormType(document_));
}

TEST_F(FPDFFormFillEmbedderTest, HasFormInfoXFAForeground) {
  EXPECT_TRUE(OpenDocument("bug_216.pdf"));
  EXPECT_EQ(FORMTYPE_XFA_FOREGROUND, FPDF_GetFormType(document_));
}

TEST_F(FPDFFormFillEmbedderTest, BadApiInputsText) {
  ASSERT_TRUE(OpenDocument("text_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_FALSE(FORM_SetIndexSelected(nullptr, nullptr, 0, true));
  EXPECT_FALSE(FORM_SetIndexSelected(nullptr, page, 0, true));
  EXPECT_FALSE(FORM_SetIndexSelected(form_handle(), nullptr, 0, true));
  EXPECT_FALSE(FORM_SetIndexSelected(form_handle(), page, -1, true));
  EXPECT_FALSE(FORM_IsIndexSelected(nullptr, nullptr, 0));
  EXPECT_FALSE(FORM_IsIndexSelected(nullptr, page, 0));
  EXPECT_FALSE(FORM_IsIndexSelected(form_handle(), nullptr, 0));
  EXPECT_FALSE(FORM_IsIndexSelected(form_handle(), page, -1));

  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, BadApiInputsComboBox) {
  ASSERT_TRUE(OpenDocument("combobox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_FALSE(FORM_SetIndexSelected(form_handle(), page, -1, true));
  EXPECT_FALSE(FORM_SetIndexSelected(form_handle(), page, 100, true));
  EXPECT_FALSE(FORM_IsIndexSelected(form_handle(), page, -1));
  EXPECT_FALSE(FORM_IsIndexSelected(form_handle(), page, 100));

  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbedderTest, BadApiInputsListBox) {
  ASSERT_TRUE(OpenDocument("listbox_form.pdf"));
  FPDF_PAGE page = LoadPage(0);
  ASSERT_TRUE(page);

  EXPECT_FALSE(FORM_SetIndexSelected(form_handle(), page, -1, true));
  EXPECT_FALSE(FORM_SetIndexSelected(form_handle(), page, 100, true));
  EXPECT_FALSE(FORM_IsIndexSelected(form_handle(), page, -1));
  EXPECT_FALSE(FORM_IsIndexSelected(form_handle(), page, 100));

  UnloadPage(page);
}

TEST_F(FPDFFormFillTextFormEmbedderTest, GetSelectedTextEmptyAndBasicKeyboard) {
  // Test empty selection.
  CheckFocusedFieldText(L"");
  CheckSelection(L"");

  // Test basic selection.
  TypeTextIntoTextField(3, RegularFormBegin());
  CheckFocusedFieldText(L"ABC");
  SelectTextWithKeyboard(3, FWL_VKEY_Left, RegularFormAtX(123.0));
  CheckSelection(L"ABC");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, GetSelectedTextEmptyAndBasicMouse) {
  // Test empty selection.
  CheckFocusedFieldText(L"");
  CheckSelection(L"");

  // Test basic selection.
  TypeTextIntoTextField(3, RegularFormBegin());
  CheckFocusedFieldText(L"ABC");
  SelectTextWithMouse(RegularFormAtX(125.0), RegularFormBegin());
  CheckSelection(L"ABC");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, GetSelectedTextFragmentsKeyBoard) {
  TypeTextIntoTextField(12, RegularFormBegin());
  CheckFocusedFieldText(L"ABCDEFGHIJKL");

  // Test selecting first character in forward direction.
  SelectTextWithKeyboard(1, FWL_VKEY_Right, RegularFormBegin());
  CheckSelection(L"A");

  // Test selecting entire long string in backwards direction.
  SelectTextWithKeyboard(12, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"ABCDEFGHIJKL");

  // Test selecting middle section in backwards direction.
  SelectTextWithKeyboard(6, FWL_VKEY_Left, RegularFormAtX(170.0));
  CheckSelection(L"DEFGHI");

  // Test selecting middle selection in forward direction.
  SelectTextWithKeyboard(6, FWL_VKEY_Right, RegularFormAtX(125.0));
  CheckSelection(L"DEFGHI");

  // Test selecting last character in backwards direction.
  SelectTextWithKeyboard(1, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"L");
  CheckFocusedFieldText(L"ABCDEFGHIJKL");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, GetSelectedTextFragmentsMouse) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Test selecting first character in forward direction.
  SelectTextWithMouse(RegularFormBegin(), RegularFormAtX(106.0));
  CheckSelection(L"A");

  // Test selecting entire long string in backwards direction.
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"ABCDEFGHIJKL");

  // Test selecting middle section in backwards direction.
  SelectTextWithMouse(RegularFormAtX(170.0), RegularFormAtX(125.0));
  CheckSelection(L"DEFGHI");

  // Test selecting middle selection in forward direction.
  SelectTextWithMouse(RegularFormAtX(125.0), RegularFormAtX(170.0));
  CheckSelection(L"DEFGHI");

  // Test selecting last character in backwards direction.
  SelectTextWithMouse(RegularFormEnd(), RegularFormAtX(186.0));
  CheckSelection(L"L");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       GetSelectedTextEmptyAndBasicNormalComboBox) {
  // Test empty selection.
  CheckSelection(L"");
  CheckFocusedFieldText(L"");

  // Non-editable comboboxes don't allow selection with keyboard.
  SelectTextWithMouse(NonEditableFormBegin(), NonEditableFormAtX(142.0));
  CheckFocusedFieldText(L"Banana");
  CheckSelection(L"Banana");

  // Select other another provided option.
  SelectNonEditableFormOption(0);
  CheckFocusedFieldText(L"Apple");
  CheckSelection(L"Apple");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       GetSelectedTextEmptyAndBasicEditableComboBoxKeyboard) {
  // Test empty selection.
  CheckSelection(L"");
  CheckFocusedFieldText(L"");

  // Test basic selection of text within user editable combobox using keyboard.
  TypeTextIntoTextField(3, EditableFormBegin());
  CheckFocusedFieldText(L"ABC");
  SelectTextWithKeyboard(3, FWL_VKEY_Left, EditableFormAtX(128.0));
  CheckSelection(L"ABC");

  // Select a provided option.
  SelectEditableFormOption(1);
  CheckSelection(L"Bar");
  CheckFocusedFieldText(L"Bar");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       GetSelectedTextEmptyAndBasicEditableComboBoxMouse) {
  // Test empty selection.
  CheckSelection(L"");

  // Test basic selection of text within user editable combobox using mouse.
  TypeTextIntoTextField(3, EditableFormBegin());
  SelectTextWithMouse(EditableFormAtX(128.0), EditableFormBegin());
  CheckSelection(L"ABC");

  // Select a provided option.
  SelectEditableFormOption(2);
  CheckFocusedFieldText(L"Qux");
  CheckSelection(L"Qux");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       GetSelectedTextFragmentsNormalComboBox) {
  CheckFocusedFieldText(L"");

  // Test selecting first character in forward direction.
  SelectTextWithMouse(NonEditableFormBegin(), NonEditableFormAtX(107.0));
  CheckFocusedFieldText(L"Banana");
  CheckSelection(L"B");

  // Test selecting entire string in backwards direction.
  SelectTextWithMouse(NonEditableFormAtX(142.0), NonEditableFormBegin());
  CheckSelection(L"Banana");

  // Test selecting middle section in backwards direction.
  SelectTextWithMouse(NonEditableFormAtX(135.0), NonEditableFormAtX(117.0));
  CheckSelection(L"nan");

  // Test selecting middle section in forward direction.
  SelectTextWithMouse(NonEditableFormAtX(117.0), NonEditableFormAtX(135.0));
  CheckSelection(L"nan");

  // Test selecting last character in backwards direction.
  SelectTextWithMouse(NonEditableFormAtX(142.0), NonEditableFormAtX(138.0));
  CheckSelection(L"a");
  CheckFocusedFieldText(L"Banana");

  // Select another option and then reset selection as first three chars.
  SelectNonEditableFormOption(2);
  CheckFocusedFieldText(L"Cherry");
  CheckSelection(L"Cherry");
  SelectTextWithMouse(NonEditableFormBegin(), NonEditableFormAtX(122.0));
  CheckSelection(L"Che");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       GetSelectedTextFragmentsEditableComboBoxKeyboard) {
  CheckFocusedFieldText(L"");
  TypeTextIntoTextField(10, EditableFormBegin());
  CheckFocusedFieldText(L"ABCDEFGHIJ");

  // Test selecting first character in forward direction.
  SelectTextWithKeyboard(1, FWL_VKEY_Right, EditableFormBegin());
  CheckSelection(L"A");

  // Test selecting entire long string in backwards direction.
  SelectTextWithKeyboard(10, FWL_VKEY_Left, EditableFormEnd());
  CheckSelection(L"ABCDEFGHIJ");

  // Test selecting middle section in backwards direction.
  SelectTextWithKeyboard(5, FWL_VKEY_Left, EditableFormAtX(168.0));
  CheckSelection(L"DEFGH");

  // Test selecting middle selection in forward direction.
  SelectTextWithKeyboard(5, FWL_VKEY_Right, EditableFormAtX(127.0));
  CheckSelection(L"DEFGH");

  // Test selecting last character in backwards direction.
  SelectTextWithKeyboard(1, FWL_VKEY_Left, EditableFormEnd());
  CheckSelection(L"J");

  // Select a provided option and then reset selection as first two chars.
  SelectEditableFormOption(0);
  CheckSelection(L"Foo");
  SelectTextWithKeyboard(2, FWL_VKEY_Right, EditableFormBegin());
  CheckSelection(L"Fo");
  CheckFocusedFieldText(L"Foo");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       GetSelectedTextFragmentsEditableComboBoxMouse) {
  TypeTextIntoTextField(10, EditableFormBegin());

  // Test selecting first character in forward direction.
  SelectTextWithMouse(EditableFormBegin(), EditableFormAtX(107.0));
  CheckSelection(L"A");

  // Test selecting entire long string in backwards direction.
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCDEFGHIJ");

  // Test selecting middle section in backwards direction.
  SelectTextWithMouse(EditableFormAtX(168.0), EditableFormAtX(127.0));
  CheckSelection(L"DEFGH");

  // Test selecting middle selection in forward direction.
  SelectTextWithMouse(EditableFormAtX(127.0), EditableFormAtX(168.0));
  CheckSelection(L"DEFGH");

  // Test selecting last character in backwards direction.
  SelectTextWithMouse(EditableFormEnd(), EditableFormAtX(174.0));
  CheckSelection(L"J");
  CheckFocusedFieldText(L"ABCDEFGHIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       SetSelectionProgrammaticallyNonEditableField) {
  // Focus on non-editable form field and check that the value is as expected.
  // This is the value that is present in the field upon opening, we have not
  // changed it by setting focus.
  FocusOnNonEditableForm();
  CheckFocusedFieldText(L"Banana");

  // Make selections to change the value of the focused annotation
  // programmatically.
  SetIndexSelectedShouldSucceed(0, true);
  CheckFocusedFieldText(L"Apple");

  // Selecting an index that is already selected is success.
  SetIndexSelectedShouldSucceed(0, true);
  CheckFocusedFieldText(L"Apple");

  SetIndexSelectedShouldSucceed(9, true);
  CheckFocusedFieldText(L"Jackfruit");

  // Cannot deselect a combobox field - value unchanged.
  SetIndexSelectedShouldFail(9, false);
  CheckFocusedFieldText(L"Jackfruit");

  // Cannot select indices that are out of range - value unchanged.
  SetIndexSelectedShouldFail(100, true);
  SetIndexSelectedShouldFail(-100, true);
  CheckFocusedFieldText(L"Jackfruit");

  // Check that above actions are interchangeable with click actions, should be
  // able to use a combination of both.
  SelectNonEditableFormOption(1);
  CheckFocusedFieldText(L"Banana");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       SetSelectionProgrammaticallyEditableField) {
  // Focus on editable form field and check that the value is as expected.
  // This is the value that is present in the field upon opening, we have not
  // changed it by setting focus.
  FocusOnEditableForm();
  CheckFocusedFieldText(L"");

  // Make selections to change value of the focused annotation
  // programmatically.
  SetIndexSelectedShouldSucceed(0, true);
  CheckFocusedFieldText(L"Foo");

  SetIndexSelectedShouldSucceed(1, true);
  CheckFocusedFieldText(L"Bar");

  // Selecting an index that is already selected is success.
  SetIndexSelectedShouldSucceed(1, true);
  CheckFocusedFieldText(L"Bar");

  // Cannot deselect a combobox field - value unchanged.
  SetIndexSelectedShouldFail(0, false);
  CheckFocusedFieldText(L"Bar");

  // Cannot select indices that are out of range - value unchanged.
  SetIndexSelectedShouldFail(100, true);
  SetIndexSelectedShouldFail(-100, true);
  CheckFocusedFieldText(L"Bar");

  // Check that above actions are interchangeable with click actions, should be
  // able to use a combination of both.
  SelectEditableFormOption(0);
  CheckFocusedFieldText(L"Foo");

  // Check that above actions are interchangeable with typing actions, should
  // be able to use a combination of both. Typing text into a text field after
  // selecting indices programmatically should be equivalent to doing so after
  // a user selects an index via click on the dropdown.
  SetIndexSelectedShouldSucceed(1, true);
  CheckFocusedFieldText(L"Bar");
  TypeTextIntoTextField(5, EditableFormBegin());
  CheckFocusedFieldText(L"ABCDEBar");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       CheckIfIndexSelectedNonEditableField) {
  // Non-editable field is set to 'Banana' (index 1) upon opening.
  ClickOnFormFieldAtPoint(NonEditableFormBegin());
  for (int i = 0; i < 26; i++) {
    bool expected = i == 1;
    CheckIsIndexSelected(i, expected);
  }

  SelectNonEditableFormOption(0);
  CheckIsIndexSelected(0, true);
  for (int i = 1; i < 26; i++) {
    CheckIsIndexSelected(i, false);
  }
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       CheckIfIndexSelectedEditableField) {
  // Editable field has nothing selected upon opening.
  ClickOnFormFieldAtPoint(EditableFormBegin());
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, false);
  CheckIsIndexSelected(2, false);

  SelectEditableFormOption(0);
  CheckIsIndexSelected(0, true);
  CheckIsIndexSelected(1, false);
  CheckIsIndexSelected(2, false);
}

TEST_F(FPDFFormFillTextFormEmbedderTest, DeleteTextFieldEntireSelection) {
  // Select entire contents of text field.
  TypeTextIntoTextField(12, RegularFormBegin());
  SelectAllRegularFormTextWithMouse();
  CheckFocusedFieldText(L"ABCDEFGHIJKL");
  CheckSelection(L"ABCDEFGHIJKL");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  CheckFocusedFieldText(L"");

  SelectTextWithKeyboard(12, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, DeleteTextFieldSelectionMiddle) {
  // Select middle section of text.
  TypeTextIntoTextField(12, RegularFormBegin());
  SelectTextWithMouse(RegularFormAtX(170.0), RegularFormAtX(125.0));
  CheckFocusedFieldText(L"ABCDEFGHIJKL");
  CheckSelection(L"DEFGHI");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  CheckFocusedFieldText(L"ABCJKL");
  SelectTextWithKeyboard(12, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"ABCJKL");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, DeleteTextFieldSelectionLeft) {
  // Select first few characters of text.
  TypeTextIntoTextField(12, RegularFormBegin());
  SelectTextWithMouse(RegularFormBegin(), RegularFormAtX(132.0));
  CheckSelection(L"ABCD");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  CheckFocusedFieldText(L"EFGHIJKL");
  SelectTextWithKeyboard(12, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"EFGHIJKL");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, DeleteTextFieldSelectionRight) {
  // Select last few characters of text.
  TypeTextIntoTextField(12, RegularFormBegin());
  SelectTextWithMouse(RegularFormEnd(), RegularFormAtX(165.0));
  CheckSelection(L"IJKL");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  CheckFocusedFieldText(L"ABCDEFGH");
  SelectTextWithKeyboard(12, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"ABCDEFGH");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, DeleteEmptyTextFieldSelection) {
  // Do not select text.
  TypeTextIntoTextField(12, RegularFormBegin());
  CheckSelection(L"");

  // Test that attempt to delete empty text selection has no effect.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  CheckFocusedFieldText(L"ABCDEFGHIJKL");
  SelectTextWithKeyboard(12, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"ABCDEFGHIJKL");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       DeleteEditableComboBoxEntireSelection) {
  // Select entire contents of user-editable combobox text field.
  TypeTextIntoTextField(10, EditableFormBegin());
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCDEFGHIJ");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  CheckFocusedFieldText(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       DeleteEditableComboBoxSelectionMiddle) {
  // Select middle section of text.
  TypeTextIntoTextField(10, EditableFormBegin());
  SelectTextWithMouse(EditableFormAtX(168.0), EditableFormAtX(127.0));
  CheckSelection(L"DEFGH");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  CheckFocusedFieldText(L"ABCIJ");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       DeleteEditableComboBoxSelectionLeft) {
  // Select first few characters of text.
  TypeTextIntoTextField(10, EditableFormBegin());
  SelectTextWithMouse(EditableFormBegin(), EditableFormAtX(132.0));
  CheckSelection(L"ABCD");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"EFGHIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       DeleteEditableComboBoxSelectionRight) {
  // Select last few characters of text.
  TypeTextIntoTextField(10, EditableFormBegin());
  SelectTextWithMouse(EditableFormEnd(), EditableFormAtX(152.0));
  CheckSelection(L"GHIJ");

  // Test deleting current text selection. Select what remains after deletion to
  // check that remaining text is as expected.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCDEF");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       DeleteEmptyEditableComboBoxSelection) {
  // Do not select text.
  TypeTextIntoTextField(10, EditableFormBegin());
  CheckSelection(L"");

  // Test that attempt to delete empty text selection has no effect.
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCDEFGHIJ");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, InsertTextInEmptyTextField) {
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(RegularFormBegin());
  CheckFocusedFieldText(L"");

  // Test inserting text into empty text field.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"Hello");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"Hello");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, InsertTextInPopulatedTextFieldLeft) {
  TypeTextIntoTextField(8, RegularFormBegin());
  CheckFocusedFieldText(L"ABCDEFGH");

  // Click on the leftmost part of the text field.
  ClickOnFormFieldAtPoint(RegularFormBegin());
  CheckFocusedFieldText(L"ABCDEFGH");

  // Test inserting text in front of existing text in text field.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"HelloABCDEFGH");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"HelloABCDEFGH");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, InsertTextInPopulatedTextFieldMiddle) {
  TypeTextIntoTextField(8, RegularFormBegin());

  // Click on the middle of the text field.
  ClickOnFormFieldAtPoint(RegularFormAtX(134.0));

  // Test inserting text in the middle of existing text in text field.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"ABCDHelloEFGH");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"ABCDHelloEFGH");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, InsertTextInPopulatedTextFieldRight) {
  TypeTextIntoTextField(8, RegularFormBegin());

  // Click on the rightmost part of the text field.
  ClickOnFormFieldAtPoint(RegularFormAtX(166.0));

  // Test inserting text behind existing text in text field.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"ABCDEFGHHello");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"ABCDEFGHHello");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldWhole) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Select entire string in text field.
  CheckSelection(L"");
  SelectTextWithKeyboard(12, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"ABCDEFGHIJKL");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"Hello");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"Hello");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldLeft) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Select left portion of string in text field.
  CheckSelection(L"");
  SelectTextWithKeyboard(6, FWL_VKEY_Left, RegularFormAtX(148.0));
  CheckSelection(L"ABCDEF");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"HelloGHIJKL");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"HelloGHIJKL");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldMiddle) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Select middle portion of string in text field.
  CheckSelection(L"");
  SelectTextWithKeyboard(6, FWL_VKEY_Left, RegularFormAtX(171.0));
  CheckSelection(L"DEFGHI");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"ABCHelloJKL");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedTextFieldRight) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Select right portion of string in text field.
  CheckSelection(L"");
  SelectTextWithKeyboard(6, FWL_VKEY_Left, RegularFormEnd());
  CheckSelection(L"GHIJKL");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllRegularFormTextWithMouse();
  CheckSelection(L"ABCDEFHello");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       InsertTextInEmptyEditableComboBox) {
  ClickOnFormFieldAtPoint(EditableFormBegin());
  CheckFocusedFieldText(L"");

  // Test inserting text into empty user-editable combobox.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"Hello");

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  CheckSelection(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"Hello");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       InsertTextInPopulatedEditableComboBoxLeft) {
  TypeTextIntoTextField(6, EditableFormBegin());

  // Click on the leftmost part of the user-editable combobox.
  ClickOnFormFieldAtPoint(EditableFormBegin());

  // Test inserting text in front of existing text in user-editable combobox.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  CheckSelection(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"HelloABCDEF");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       InsertTextInPopulatedEditableComboBoxMiddle) {
  TypeTextIntoTextField(6, EditableFormBegin());

  // Click on the middle of the user-editable combobox.
  ClickOnFormFieldAtPoint(EditableFormAtX(126.0));

  // Test inserting text in the middle of existing text in user-editable
  // combobox.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  CheckSelection(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCHelloDEF");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       InsertTextInPopulatedEditableComboBoxRight) {
  TypeTextIntoTextField(6, EditableFormBegin());

  // Click on the rightmost part of the user-editable combobox.
  ClickOnFormFieldAtPoint(EditableFormEnd());

  // Test inserting text behind existing text in user-editable combobox.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  CheckSelection(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCDEFHello");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxWhole) {
  TypeTextIntoTextField(10, EditableFormBegin());

  // Select entire string in user-editable combobox.
  CheckSelection(L"");
  SelectTextWithKeyboard(10, FWL_VKEY_Left, EditableFormEnd());
  CheckSelection(L"ABCDEFGHIJ");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  CheckSelection(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"Hello");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxLeft) {
  TypeTextIntoTextField(10, EditableFormBegin());

  // Select left portion of string in user-editable combobox.
  CheckSelection(L"");
  SelectTextWithKeyboard(5, FWL_VKEY_Left, EditableFormAtX(142.0));
  CheckSelection(L"ABCDE");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  CheckSelection(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"HelloFGHIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxMiddle) {
  TypeTextIntoTextField(10, EditableFormBegin());

  // Select middle portion of string in user-editable combobox.
  CheckSelection(L"");
  SelectTextWithKeyboard(5, FWL_VKEY_Left, EditableFormAtX(167.0));
  CheckSelection(L"DEFGH");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  CheckSelection(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCHelloIJ");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedEditableComboBoxRight) {
  TypeTextIntoTextField(10, EditableFormBegin());

  // Select right portion of string in user-editable combobox.
  CheckSelection(L"");
  SelectTextWithKeyboard(5, FWL_VKEY_Left, EditableFormEnd());
  CheckSelection(L"FGHIJ");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of user-editable combobox text field to check that
  // insertion worked as expected.
  CheckSelection(L"");
  SelectAllEditableFormTextWithMouse();
  CheckSelection(L"ABCDEHello");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextInEmptyCharLimitTextFieldOverflow) {
  // Click on the textfield.
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(CharLimitFormEnd());
  CheckFocusedFieldText(L"Elephant");

  // Delete pre-filled contents of text field with char limit.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"Elephant");
  FORM_ReplaceSelection(form_handle(), page(), nullptr);
  CheckFocusedFieldText(L"");

  // Test inserting text into now empty text field so text to be inserted
  // exceeds the char limit and is cut off.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"Hippopotam");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"Hippopotam");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextInEmptyCharLimitTextFieldFit) {
  // Click on the textfield.
  ClickOnFormFieldAtPoint(CharLimitFormEnd());
  CheckFocusedFieldText(L"Elephant");

  // Delete pre-filled contents of text field with char limit.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"Elephant");
  FORM_ReplaceSelection(form_handle(), page(), nullptr);

  // Test inserting text into now empty text field so text to be inserted
  // exceeds the char limit and is cut off.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Zebra");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"Zebra");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"Zebra");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextInPopulatedCharLimitTextFieldLeft) {
  // Click on the leftmost part of the text field.
  ClickOnFormFieldAtPoint(CharLimitFormBegin());

  // Test inserting text in front of existing text in text field.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"HiElephant");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextInPopulatedCharLimitTextFieldMiddle) {
  CheckFocusedFieldText(L"");
  TypeTextIntoTextField(8, RegularFormBegin());
  CheckFocusedFieldText(L"ABCDEFGH");

  // Click on the middle of the text field.
  ClickOnFormFieldAtPoint(CharLimitFormAtX(134.0));
  CheckFocusedFieldText(L"Elephant");

  // Test inserting text in the middle of existing text in text field.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"ElephHiant");

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"ElephHiant");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextInPopulatedCharLimitTextFieldRight) {
  TypeTextIntoTextField(8, RegularFormBegin());

  // Click on the rightmost part of the text field.
  ClickOnFormFieldAtPoint(CharLimitFormAtX(166.0));

  // Test inserting text behind existing text in text field.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"ElephantHi");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldWhole) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Select entire string in text field.
  CheckSelection(L"");
  SelectTextWithKeyboard(12, FWL_VKEY_Left, CharLimitFormEnd());
  CheckSelection(L"Elephant");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"Hippopotam");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldLeft) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Select left portion of string in text field.
  CheckSelection(L"");
  SelectTextWithKeyboard(4, FWL_VKEY_Left, CharLimitFormAtX(122.0));
  CheckSelection(L"Elep");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"Hippophant");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldMiddle) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Select middle portion of string in text field.
  CheckSelection(L"");
  SelectTextWithKeyboard(4, FWL_VKEY_Left, CharLimitFormAtX(136.0));
  CheckSelection(L"epha");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"ElHippopnt");
}

TEST_F(FPDFFormFillTextFormEmbedderTest,
       InsertTextAndReplaceSelectionInPopulatedCharLimitTextFieldRight) {
  TypeTextIntoTextField(12, RegularFormBegin());

  // Select right portion of string in text field.
  CheckSelection(L"");
  SelectTextWithKeyboard(4, FWL_VKEY_Left, CharLimitFormAtX(152.0));
  CheckSelection(L"hant");

  // Test replacing text selection with text to be inserted.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hippopotamus");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());

  // Select entire contents of text field to check that insertion worked
  // as expected.
  CheckSelection(L"");
  SelectAllCharLimitFormTextWithMouse();
  CheckSelection(L"ElepHippop");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, DoubleClickInTextField) {
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(RegularFormBegin());
  CheckFocusedFieldText(L"");

  // Test inserting text into empty text field.
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"Hello World");
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"Hello World");

  // Make sure double clicking selects the entire line.
  CheckSelection(L"");
  DoubleClickOnFormFieldAtPoint(RegularFormBegin());
  CheckSelection(L"Hello World");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, FocusAnnotationUpdateToEmbedder) {
  testing::NiceMock<EmbedderTestMockDelegate> mock;
  SetDelegate(&mock);
  CheckFocusedFieldText(L"");

#ifdef PDF_ENABLE_XFA
  EXPECT_CALL(mock, OnFocusChange(_, _, 0)).Times(1);
#else   // PDF_ENABLE_XFA
  EXPECT_CALL(mock, OnFocusChange(_, _, 0)).Times(0);
#endif  // PDF_ENABLE_XFA

  ClickOnFormFieldAtPoint(RegularFormBegin());
}

TEST_F(FPDFFormFillTextFormEmbedderTestVersion2,
       FocusAnnotationUpdateToEmbedder) {
  testing::NiceMock<EmbedderTestMockDelegate> mock;
  SetDelegate(&mock);
  CheckFocusedFieldText(L"");

  EXPECT_CALL(mock, OnFocusChange(_, _, 0)).Times(1);
  ClickOnFormFieldAtPoint(RegularFormBegin());
}

TEST_F(FPDFFormFillTextFormEmbedderTest, FocusChanges) {
  static const CFX_PointF kNonFormPoint(1, 1);
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(CharLimitFormEnd());
  CheckFocusedFieldText(L"Elephant");
  ClickOnFormFieldAtPoint(RegularFormBegin());
  CheckFocusedFieldText(L"");
  TypeTextIntoTextField(3, CharLimitFormBegin());
  CheckFocusedFieldText(L"ABElephant");
  TypeTextIntoTextField(5, RegularFormBegin());
  CheckFocusedFieldText(L"ABCDE");
  ClickOnFormFieldAtPoint(CharLimitFormEnd());
  CheckFocusedFieldText(L"ABElephant");
  ClickOnFormFieldAtPoint(kNonFormPoint);
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(kNonFormPoint);
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(CharLimitFormBegin());
  CheckFocusedFieldText(L"ABElephant");
  ClickOnFormFieldAtPoint(CharLimitFormEnd());
  CheckFocusedFieldText(L"ABElephant");
  ClickOnFormFieldAtPoint(RegularFormEnd());
  CheckFocusedFieldText(L"ABCDE");
  ClickOnFormFieldAtPoint(RegularFormBegin());
  CheckFocusedFieldText(L"ABCDE");
  ClickOnFormFieldAtPoint(RegularFormBegin());
  CheckFocusedFieldText(L"ABCDE");
  ClickOnFormFieldAtPoint(CharLimitFormBegin());
  CheckFocusedFieldText(L"ABElephant");
  FORM_ForceToKillFocus(form_handle());
  CheckFocusedFieldText(L"");
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest, FocusChanges) {
  static const CFX_PointF kNonFormPoint(1, 1);
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(NonEditableFormBegin());
  CheckFocusedFieldText(L"Banana");
  ClickOnFormFieldAtPoint(EditableFormBegin());
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(NonEditableFormEnd());
  CheckFocusedFieldText(L"Banana");
  ClickOnFormFieldAtPoint(NonEditableFormBegin());
  CheckFocusedFieldText(L"Banana");
  FORM_ForceToKillFocus(form_handle());
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(EditableFormBegin());
  CheckFocusedFieldText(L"");
  TypeTextIntoTextField(3, EditableFormBegin());
  CheckFocusedFieldText(L"ABC");
  ClickOnFormFieldAtPoint(kNonFormPoint);
  CheckFocusedFieldText(L"");
  TypeTextIntoTextField(3, EditableFormEnd());
  CheckFocusedFieldText(L"ABCABC");
  ClickOnFormFieldAtPoint(kNonFormPoint);
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(EditableFormDropDown());
  CheckFocusedFieldText(L"ABCABC");
  FORM_ForceToKillFocus(form_handle());
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(NonEditableFormDropDown());
  CheckFocusedFieldText(L"Banana");
  ClickOnFormFieldAtPoint(kNonFormPoint);
  CheckFocusedFieldText(L"");
  ClickOnFormFieldAtPoint(NonEditableFormEnd());
  CheckFocusedFieldText(L"Banana");

  // Typing into non-editable field results in selecting a different option.
  TypeTextIntoTextField(1, NonEditableFormEnd());
  CheckFocusedFieldText(L"Apple");
  TypeTextIntoTextField(3, NonEditableFormEnd());
  CheckFocusedFieldText(L"Cherry");
  TypeTextIntoTextField(2, NonEditableFormEnd());
  CheckFocusedFieldText(L"Banana");

  SelectEditableFormOption(0);
  CheckFocusedFieldText(L"Foo");
  SelectEditableFormOption(1);
  CheckFocusedFieldText(L"Bar");
  SelectEditableFormOption(2);
  CheckFocusedFieldText(L"Qux");
  SelectNonEditableFormOption(1);
  CheckFocusedFieldText(L"Banana");
  SelectNonEditableFormOption(0);
  CheckFocusedFieldText(L"Apple");
  SelectNonEditableFormOption(2);
  CheckFocusedFieldText(L"Cherry");

  // Typing into an editable field changes the text in the option.
  SelectEditableFormOption(0);
  CheckFocusedFieldText(L"Foo");
  TypeTextIntoTextField(5, EditableFormBegin());
  CheckFocusedFieldText(L"ABCDEFoo");
  SelectEditableFormOption(2);
  CheckFocusedFieldText(L"Qux");
  TypeTextIntoTextField(2, EditableFormEnd());
  CheckFocusedFieldText(L"QuxAB");

  // But a previously edited option is reset when selected again.
  SelectEditableFormOption(0);
  CheckFocusedFieldText(L"Foo");
  TypeTextIntoTextField(1, EditableFormBegin());
  CheckFocusedFieldText(L"AFoo");
  SelectEditableFormOption(0);
  CheckFocusedFieldText(L"Foo");
}

TEST_F(FPDFFormFillTextFormEmbedderTest, UndoRedo) {
  ClickOnFormFieldAtPoint(RegularFormBegin());
  CheckFocusedFieldText(L"");
  CheckCanUndo(false);
  CheckCanRedo(false);

  TypeTextIntoTextField(5, RegularFormBegin());
  CheckFocusedFieldText(L"ABCDE");
  CheckCanUndo(true);
  CheckCanRedo(false);

  PerformUndo();
  CheckFocusedFieldText(L"ABCD");
  CheckCanUndo(true);
  CheckCanRedo(true);
  PerformUndo();
  CheckFocusedFieldText(L"ABC");
  CheckCanUndo(true);
  CheckCanRedo(true);

  PerformRedo();
  CheckFocusedFieldText(L"ABCD");
  CheckCanUndo(true);
  CheckCanRedo(true);
  PerformRedo();
  CheckFocusedFieldText(L"ABCDE");
  CheckCanUndo(true);
  CheckCanRedo(false);
}

// This action only applies to Listboxes and Comboboxes so should fail
// gracefully for Textboxes by returning false to all operations.
TEST_F(FPDFFormFillTextFormEmbedderTest, SetIndexSelectedShouldFailGracefully) {
  // set focus and read text to confirm we have it
  ClickOnFormFieldAtPoint(CharLimitFormEnd());
  CheckFocusedFieldText(L"Elephant");

  SetIndexSelectedShouldFail(0, true);
  SetIndexSelectedShouldFail(0, false);
  SetIndexSelectedShouldFail(1, true);
  SetIndexSelectedShouldFail(1, false);
  SetIndexSelectedShouldFail(-1, true);
  SetIndexSelectedShouldFail(-1, false);
}

// This action only applies to Listboxes and Comboboxes so should fail
// gracefully for Textboxes by returning false to all operations.
TEST_F(FPDFFormFillTextFormEmbedderTest, IsIndexSelectedShouldFailGracefully) {
  // set focus and read text to confirm we have it
  ClickOnFormFieldAtPoint(CharLimitFormEnd());
  CheckFocusedFieldText(L"Elephant");

  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, false);
  CheckIsIndexSelected(-1, false);
}

TEST_F(FPDFFormFillComboBoxFormEmbedderTest, UndoRedo) {
  ClickOnFormFieldAtPoint(NonEditableFormBegin());
  CheckFocusedFieldText(L"Banana");
  CheckCanUndo(false);
  CheckCanRedo(false);

  ClickOnFormFieldAtPoint(EditableFormBegin());
  CheckFocusedFieldText(L"");
  CheckCanUndo(false);
  CheckCanRedo(false);

  TypeTextIntoTextField(3, EditableFormBegin());
  CheckFocusedFieldText(L"ABC");
  CheckCanUndo(true);
  CheckCanRedo(false);

  PerformUndo();
  CheckFocusedFieldText(L"AB");
  CheckCanUndo(true);
  CheckCanRedo(true);
  PerformUndo();
  CheckFocusedFieldText(L"A");
  CheckCanUndo(true);
  CheckCanRedo(true);
  PerformUndo();
  CheckFocusedFieldText(L"");
  CheckCanUndo(false);
  CheckCanRedo(true);

  PerformRedo();
  CheckFocusedFieldText(L"A");
  CheckCanUndo(true);
  CheckCanRedo(true);
}

TEST_F(FPDFFormFillListBoxFormEmbedderTest,
       CheckIfIndexSelectedSingleSelectField) {
  // Nothing is selected in single select field upon opening.
  FocusOnSingleSelectForm();
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, false);
  CheckIsIndexSelected(2, false);

  ClickOnSingleSelectFormOption(1);
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, true);
  CheckIsIndexSelected(2, false);
}

TEST_F(FPDFFormFillListBoxFormEmbedderTest,
       CheckIfIndexSelectedMultiSelectField) {
  // Multiselect field set to 'Banana' (index 1) upon opening.
  FocusOnMultiSelectForm();
  for (int i = 0; i < 26; i++) {
    bool expected = i == 1;
    CheckIsIndexSelected(i, expected);
  }

  // TODO(bug_1377): Behavior should be changed to the one described below.
  // Multiselect field set to 'Cherry' (index 2), which is index 1 among the
  // visible form options because the listbox is scrolled down to have 'Banana'
  // (index 1) at the top.
  ClickOnMultiSelectFormOption(1);
  for (int i = 0; i < 26; i++) {
    bool expected = i == 1;
    CheckIsIndexSelected(i, expected);
  }
}

TEST_F(FPDFFormFillListBoxFormEmbedderTest,
       SetSelectionProgrammaticallySingleSelectField) {
  // Nothing is selected in single select field upon opening.
  FocusOnSingleSelectForm();
  CheckFocusedFieldText(L"");
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, false);
  CheckIsIndexSelected(2, false);

  // Make selections to change the value of the focused annotation
  // programmatically showing that only one value remains selected at a time.
  SetIndexSelectedShouldSucceed(0, true);
  CheckFocusedFieldText(L"Foo");
  CheckIsIndexSelected(0, true);
  CheckIsIndexSelected(1, false);
  CheckIsIndexSelected(2, false);

  SetIndexSelectedShouldSucceed(1, true);
  CheckFocusedFieldText(L"Bar");
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, true);
  CheckIsIndexSelected(2, false);

  // Selecting/deselecting an index that is already selected/deselected is
  // success.
  SetIndexSelectedShouldSucceed(1, true);
  CheckFocusedFieldText(L"Bar");
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, true);
  CheckIsIndexSelected(2, false);

  SetIndexSelectedShouldSucceed(2, false);
  CheckFocusedFieldText(L"Bar");
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, true);
  CheckIsIndexSelected(2, false);

  // Cannot select indices that are out of range.
  SetIndexSelectedShouldFail(100, true);
  SetIndexSelectedShouldFail(-100, true);
  SetIndexSelectedShouldFail(100, false);
  SetIndexSelectedShouldFail(-100, false);
  // Confirm that previous values were not changed.
  CheckFocusedFieldText(L"Bar");
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, true);
  CheckIsIndexSelected(2, false);

  // Unlike combobox, should be able to deselect all indices.
  SetIndexSelectedShouldSucceed(1, false);
  CheckFocusedFieldText(L"");
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, false);
  CheckIsIndexSelected(2, false);

  // Check that above actions are interchangeable with click actions, should be
  // able to use a combination of both.
  ClickOnSingleSelectFormOption(1);
  CheckFocusedFieldText(L"Bar");
  CheckIsIndexSelected(0, false);
  CheckIsIndexSelected(1, true);
  CheckIsIndexSelected(2, false);
}

// Re: Focus Field Text - For multiselect listboxes a caret is set on the last
// item to be selected/deselected. The text of that item should be returned.
TEST_F(FPDFFormFillListBoxFormEmbedderTest,
       SetSelectionProgrammaticallyMultiSelectField) {
  // Multiselect field set to 'Banana' (index 1) upon opening.
  FocusOnMultiSelectForm();
  for (int i = 0; i < 26; i++) {
    bool expected = i == 1;
    CheckIsIndexSelected(i, expected);
  }
  CheckFocusedFieldText(L"Banana");

  // Select some more options.
  SetIndexSelectedShouldSucceed(5, true);
  SetIndexSelectedShouldSucceed(6, true);
  SetIndexSelectedShouldSucceed(20, true);
  for (int i = 0; i < 26; i++) {
    bool expected = (i == 1 || i == 5 || i == 6 || i == 20);
    CheckIsIndexSelected(i, expected);
  }
  CheckFocusedFieldText(L"Ugli Fruit");

  // Selecting indices that are already selected is success - changes nothing.
  SetIndexSelectedShouldSucceed(5, true);
  SetIndexSelectedShouldSucceed(6, true);
  SetIndexSelectedShouldSucceed(20, true);
  for (int i = 0; i < 26; i++) {
    bool expected = (i == 1 || i == 5 || i == 6 || i == 20);
    CheckIsIndexSelected(i, expected);
  }
  CheckFocusedFieldText(L"Ugli Fruit");

  // Deselect some options.
  SetIndexSelectedShouldSucceed(20, false);
  SetIndexSelectedShouldSucceed(1, false);
  for (int i = 0; i < 26; i++) {
    bool expected = (i == 5 || i == 6);
    CheckIsIndexSelected(i, expected);
  }
  CheckFocusedFieldText(L"Banana");

  // Deselecting indices that already aren't selected is success - does not
  // change the selected values but moves the focus text caret to last item we
  // executed on.
  SetIndexSelectedShouldSucceed(1, false);
  SetIndexSelectedShouldSucceed(3, false);
  for (int i = 0; i < 26; i++) {
    bool expected = (i == 5 || i == 6);
    CheckIsIndexSelected(i, expected);
  }
  CheckFocusedFieldText(L"Date");

  // Cannot select indices that are out of range.
  SetIndexSelectedShouldFail(100, true);
  SetIndexSelectedShouldFail(-100, true);
  SetIndexSelectedShouldFail(100, false);
  SetIndexSelectedShouldFail(-100, false);
  // Confirm that previous values were not changed.
  CheckFocusedFieldText(L"Date");
  for (int i = 0; i < 26; i++) {
    bool expected = (i == 5 || i == 6);
    CheckIsIndexSelected(i, expected);
  }

  // Check that above actions are interchangeable with click actions, should be
  // able to use a combination of both.
  // TODO(bug_1377): Change to click on form option 0 instead of form option 1
  ClickOnMultiSelectFormOption(1);
  for (int i = 0; i < 26; i++) {
    bool expected = i == 1;
    CheckIsIndexSelected(i, expected);
  }
  CheckFocusedFieldText(L"Banana");
}

TEST_F(FPDFFormFillListBoxFormEmbedderTest, CheckIfMultipleSelected) {
  // Multiselect field set to 'Gamma' (index 2) and 'Epsilon' (index 4) upon
  // opening.
  FocusOnMultiSelectMultipleSelectedForm();
  for (int i = 0; i < 5; i++) {
    // TODO(bug_1377): Should be selected at index 2 and index 4.
    bool expected = false;
    CheckIsIndexSelected(i, expected);
  }
}

TEST_F(FPDFFormFillListBoxFormEmbedderTest,
       CheckIfVerticalScrollIsAtFirstSelected) {
  // Multiselect field set to 'Gamma' (index 2) and 'Epsilon' (index 4) upon
  // opening.

  // TODO(bug_1377): Behavior should be changed to the one described below.
  // The top visible option is 'Gamma' (index 2), so the first selection should
  // not change. The second selection, 'Epsilon,' should be deselected.
  ClickOnMultiSelectMultipleSelectedFormOption(0);
  for (int i = 0; i < 5; i++) {
    bool expected = i == 0;
    CheckIsIndexSelected(i, expected);
  }
}

TEST_F(FPDFFormFillListBoxFormEmbedderTest, CheckForNoOverscroll) {
  // Only the last option in the list, 'Saskatchewan', is selected.
  FocusOnSingleSelectLastSelectedForm();
  for (int i = 0; i < 10; i++) {
    bool expected = i == 9;
    CheckIsIndexSelected(i, expected);
  }

  // Even though the top index is specified to be at 'Saskatchewan' (index 9),
  // the top visible option will be the one above it, 'Quebec' (index 8), to
  // prevent overscrolling. Therefore, clicking on the first visible option of
  // the list should select 'Quebec' instead of 'Saskatchewan.'
  ClickOnSingleSelectLastSelectedFormOption(0);
  for (int i = 0; i < 10; i++) {
    bool expected = i == 8;
    CheckIsIndexSelected(i, expected);
  }
}

TEST_F(FPDFFormFillTextFormEmbedderTest, ReplaceSelection) {
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"XYZ");
  ClickOnFormFieldAtPoint(RegularFormBegin());
  CheckCanUndo(false);
  CheckCanRedo(false);

  TypeTextIntoTextField(2, RegularFormBegin());
  CheckFocusedFieldText(L"AB");
  CheckSelection(L"");
  SelectTextWithKeyboard(1, FWL_VKEY_Right, RegularFormBegin());
  CheckSelection(L"A");

  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
  CheckFocusedFieldText(L"XYZB");
  CheckCanUndo(true);
  CheckCanRedo(false);

  PerformUndo();
  CheckFocusedFieldText(L"AB");
  CheckCanUndo(true);
  CheckCanRedo(true);

  PerformUndo();
  CheckFocusedFieldText(L"A");
  CheckCanUndo(true);
  CheckCanRedo(true);

  PerformUndo();
  CheckFocusedFieldText(L"");
  CheckCanUndo(false);
  CheckCanRedo(true);

  PerformRedo();
  CheckFocusedFieldText(L"A");
  CheckCanUndo(true);
  CheckCanRedo(true);

  PerformRedo();
  CheckFocusedFieldText(L"AB");
  CheckCanUndo(true);
  CheckCanRedo(true);

  PerformRedo();
  CheckFocusedFieldText(L"XYZB");
  CheckCanUndo(true);
  CheckCanRedo(false);
}

class FPDFXFAFormBug1055869EmbedderTest
    : public FPDFFormFillInteractiveEmbedderTest {
 protected:
  FPDFXFAFormBug1055869EmbedderTest() = default;
  ~FPDFXFAFormBug1055869EmbedderTest() override = default;

  const char* GetDocumentName() const override { return "bug_1055869.pdf"; }
  int GetFormType() const override { return FORMTYPE_XFA_FULL; }
};

TEST_F(FPDFXFAFormBug1055869EmbedderTest, Paste) {
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"XYZ");
  DoubleClickOnFormFieldAtPoint(CFX_PointF(100, 100));
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
}

class FPDFXFAFormBug1058653EmbedderTest
    : public FPDFFormFillInteractiveEmbedderTest {
 protected:
  FPDFXFAFormBug1058653EmbedderTest() = default;
  ~FPDFXFAFormBug1058653EmbedderTest() override = default;

  const char* GetDocumentName() const override { return "bug_1058653.pdf"; }
  int GetFormType() const override { return FORMTYPE_XFA_FULL; }
};

TEST_F(FPDFXFAFormBug1058653EmbedderTest, Paste) {
  ScopedFPDFWideString text_to_insert = GetFPDFWideString(L"");
  DoubleClickOnFormFieldAtPoint(CFX_PointF(22, 22));
  FORM_ReplaceSelection(form_handle(), page(), text_to_insert.get());
}

// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/cfde_texteditengine.h"

#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/fx_extension.h"
#include "core/fxge/text_char_pos.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_test_environment.h"
#include "xfa/fgas/font/cfgas_gefont.h"

namespace pdfium {

class CFDE_TextEditEngineTest : public testing::Test {
 public:
  class Delegate final : public CFDE_TextEditEngine::Delegate {
   public:
    void Reset() {
      text_is_full = false;
      fail_validation = false;
    }

    void NotifyTextFull() override { text_is_full = true; }

    void OnCaretChanged() override {}
    void OnTextWillChange(CFDE_TextEditEngine::TextChange* change) override {}
    void OnTextChanged() override {}
    void OnSelChanged() override {}
    bool OnValidate(const WideString& wsText) override {
      return !fail_validation;
    }
    void SetScrollOffset(float fScrollOffset) override {}

    bool fail_validation = false;
    bool text_is_full = false;
  };

  CFDE_TextEditEngineTest() = default;
  ~CFDE_TextEditEngineTest() override = default;

  void SetUp() override {
    const wchar_t kFontFamily[] = L"Arimo Bold";
    font_ = CFGAS_GEFont::LoadFont(kFontFamily, 0, FX_CodePage::kDefANSI);
    ASSERT_TRUE(font_);

    engine_ = std::make_unique<CFDE_TextEditEngine>();
    engine_->SetFont(font_);
    engine_->SetFontSize(12.0f);
  }

  void TearDown() override {
    engine_.reset();
    font_.Reset();
  }

  CFDE_TextEditEngine* engine() const { return engine_.get(); }

 private:
  RetainPtr<CFGAS_GEFont> font_;
  std::unique_ptr<CFDE_TextEditEngine> engine_;
};

TEST_F(CFDE_TextEditEngineTest, Insert) {
  EXPECT_EQ(L"", engine()->GetText());

  engine()->Insert(0, L"");
  EXPECT_EQ(L"", engine()->GetText());
  EXPECT_EQ(0U, engine()->GetLength());

  engine()->Insert(0, L"Hello");
  EXPECT_EQ(L"Hello", engine()->GetText());
  EXPECT_EQ(5U, engine()->GetLength());

  engine()->Insert(5, L" World");
  EXPECT_EQ(L"Hello World", engine()->GetText());
  EXPECT_EQ(11U, engine()->GetLength());

  engine()->Insert(5, L" New");
  EXPECT_EQ(L"Hello New World", engine()->GetText());

  engine()->Insert(100, L" Cat");
  EXPECT_EQ(L"Hello New World Cat", engine()->GetText());

  engine()->Clear();

  engine()->SetHasCharacterLimit(true);
  engine()->SetCharacterLimit(5);
  engine()->Insert(0, L"Hello");

  // No delegate
  engine()->Insert(5, L" World");
  EXPECT_EQ(L"Hello", engine()->GetText());

  engine()->SetCharacterLimit(8);
  engine()->Insert(5, L" World");
  EXPECT_EQ(L"Hello Wo", engine()->GetText());

  engine()->Clear();

  // With Delegate
  auto delegate = std::make_unique<CFDE_TextEditEngineTest::Delegate>();
  engine()->SetDelegate(delegate.get());

  engine()->SetCharacterLimit(5);
  engine()->Insert(0, L"Hello");

  // Insert when full.
  engine()->Insert(5, L" World");
  EXPECT_TRUE(delegate->text_is_full);
  EXPECT_EQ(L"Hello", engine()->GetText());
  delegate->Reset();

  engine()->SetCharacterLimit(8);
  engine()->Insert(5, L" World");
  EXPECT_TRUE(delegate->text_is_full);
  EXPECT_EQ(L"Hello Wo", engine()->GetText());
  delegate->Reset();
  engine()->SetHasCharacterLimit(false);

  engine()->Clear();
  engine()->Insert(0, L"Hello");

  // Insert Invalid text
  delegate->fail_validation = true;
  engine()->EnableValidation(true);
  engine()->Insert(5, L" World");
  EXPECT_EQ(L"Hello", engine()->GetText());

  delegate->fail_validation = false;
  engine()->Insert(5, L" World");
  EXPECT_EQ(L"Hello World", engine()->GetText());
  engine()->EnableValidation(false);

  engine()->Clear();

  engine()->Insert(0, L"Hello\nWorld");
  EXPECT_FALSE(delegate->text_is_full);
  EXPECT_EQ(L"Hello\nWorld", engine()->GetText());
  delegate->Reset();
  engine()->Clear();

  // Insert with limited area and over-fill
  engine()->LimitHorizontalScroll(true);
  engine()->SetAvailableWidth(52.0f);  // Fits 'Hello Wo'.
  engine()->Insert(0, L"Hello");
  EXPECT_FALSE(delegate->text_is_full);
  engine()->Insert(5, L" World");
  EXPECT_TRUE(delegate->text_is_full);
  EXPECT_EQ(L"Hello Wo", engine()->GetText());
  engine()->LimitHorizontalScroll(false);

  delegate->Reset();
  engine()->Clear();

  engine()->SetLineSpace(12.0f);
  engine()->LimitVerticalScroll(true);
  // Default is one line of text.
  engine()->Insert(0, L"Hello");
  EXPECT_FALSE(delegate->text_is_full);
  engine()->Insert(5, L" Wo\nrld");
  EXPECT_TRUE(delegate->text_is_full);
  EXPECT_EQ(L"Hello Wo\n", engine()->GetText());
  engine()->LimitVerticalScroll(false);

  engine()->SetDelegate(nullptr);
}

TEST_F(CFDE_TextEditEngineTest, InsertToggleLimit) {
  engine()->SetHasCharacterLimit(true);
  engine()->Insert(0, L"Hello World");
  engine()->SetCharacterLimit(5);
  engine()->Insert(0, L"Not Inserted before ");
  EXPECT_EQ(L"Hello World", engine()->GetText());

  engine()->SetHasCharacterLimit(false);
  engine()->Insert(0, L"Inserted before ");
  engine()->SetHasCharacterLimit(true);
  engine()->Insert(0, L"Not Inserted before ");
  EXPECT_EQ(L"Inserted before Hello World", engine()->GetText());
}

TEST_F(CFDE_TextEditEngineTest, InsertSkipNotify) {
  engine()->SetHasCharacterLimit(true);
  engine()->SetCharacterLimit(8);
  engine()->Insert(0, L"Hello");
  engine()->Insert(5, L" World",
                   CFDE_TextEditEngine::RecordOperation::kSkipNotify);
  EXPECT_EQ(L"Hello World", engine()->GetText());

  engine()->Insert(0, L"Not inserted");
  EXPECT_EQ(L"Hello World", engine()->GetText());

  engine()->Delete(5, 1);
  EXPECT_EQ(L"HelloWorld", engine()->GetText());

  engine()->Insert(0, L"****");
  EXPECT_EQ(L"*HelloWorld", engine()->GetText());
}

TEST_F(CFDE_TextEditEngineTest, InsertGrowGap) {
  engine()->Insert(0, L"||");
  for (size_t i = 1; i < 1023; ++i) {
    engine()->Insert(i, L"a");
  }
  WideString result = engine()->GetText();
  ASSERT_EQ(result.GetLength(), 1024u);
  EXPECT_EQ(result[0], L'|');
  EXPECT_EQ(result[1], L'a');
  EXPECT_EQ(result[2], L'a');
  // ...
  EXPECT_EQ(result[1022], L'a');
  EXPECT_EQ(result[1023], L'|');
}

TEST_F(CFDE_TextEditEngineTest, Delete) {
  EXPECT_EQ(L"", engine()->Delete(0, 50));
  EXPECT_EQ(L"", engine()->GetText());

  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(L" World", engine()->Delete(5, 6));
  EXPECT_EQ(L"Hello", engine()->GetText());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(L" ", engine()->Delete(5, 1));
  EXPECT_EQ(L"HelloWorld", engine()->GetText());

  EXPECT_EQ(L"elloWorld", engine()->Delete(1, 50));
  EXPECT_EQ(L"H", engine()->GetText());
}

TEST_F(CFDE_TextEditEngineTest, Clear) {
  EXPECT_EQ(L"", engine()->GetText());

  engine()->Clear();
  EXPECT_EQ(L"", engine()->GetText());

  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(L"Hello World", engine()->GetText());

  engine()->Clear();
  EXPECT_EQ(L"", engine()->GetText());
  EXPECT_EQ(0U, engine()->GetLength());
}

TEST_F(CFDE_TextEditEngineTest, GetChar) {
  // Out of bounds.
  EXPECT_EQ(L'\0', engine()->GetChar(0));

  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(L'H', engine()->GetChar(0));
  EXPECT_EQ(L'd', engine()->GetChar(engine()->GetLength() - 1));
  EXPECT_EQ(L' ', engine()->GetChar(5));

  engine()->Insert(5, L" A");
  EXPECT_EQ(L"Hello A World", engine()->GetText());
  EXPECT_EQ(L'W', engine()->GetChar(8));

  engine()->EnablePasswordMode(true);
  EXPECT_EQ(L'*', engine()->GetChar(8));

  engine()->SetAliasChar(L'+');
  EXPECT_EQ(L'+', engine()->GetChar(8));
}

TEST_F(CFDE_TextEditEngineTest, GetWidthOfChar) {
  // Out of Bounds.
  EXPECT_EQ(0, engine()->GetWidthOfChar(0));

  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(173280, engine()->GetWidthOfChar(0));
  EXPECT_EQ(133440, engine()->GetWidthOfChar(1));

  engine()->Insert(0, L"\t");
  EXPECT_EQ(0, engine()->GetWidthOfChar(0));
}

TEST_F(CFDE_TextEditEngineTest, GetDisplayPos) {
  EXPECT_EQ(0U, engine()->GetDisplayPos(FDE_TEXTEDITPIECE()).size());
}

TEST_F(CFDE_TextEditEngineTest, Selection) {
  EXPECT_FALSE(engine()->HasSelection());
  engine()->SelectAll();
  EXPECT_FALSE(engine()->HasSelection());

  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(L"", engine()->DeleteSelectedText());

  EXPECT_FALSE(engine()->HasSelection());
  engine()->SelectAll();
  EXPECT_TRUE(engine()->HasSelection());
  EXPECT_EQ(L"Hello World", engine()->GetSelectedText());

  engine()->ClearSelection();
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_EQ(L"", engine()->GetSelectedText());

  engine()->SelectAll();
  auto [start_idx, count] = engine()->GetSelection();
  EXPECT_EQ(0U, start_idx);
  EXPECT_EQ(11U, count);

  // Selection before gap.
  EXPECT_EQ(L"Hello World", engine()->GetSelectedText());
  EXPECT_TRUE(engine()->HasSelection());
  EXPECT_EQ(L"Hello World", engine()->GetText());

  engine()->Insert(5, L" A");
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_EQ(L"", engine()->GetSelectedText());

  // Selection over the gap.
  engine()->SelectAll();
  EXPECT_TRUE(engine()->HasSelection());
  EXPECT_EQ(L"Hello A World", engine()->GetSelectedText());
  engine()->Clear();

  engine()->Insert(0, L"Hello World");
  engine()->SelectAll();

  EXPECT_EQ(L"Hello World", engine()->DeleteSelectedText());
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_EQ(L"", engine()->GetText());

  engine()->Insert(0, L"Hello World");
  engine()->SetSelection(5, 5);
  EXPECT_EQ(L" Worl", engine()->DeleteSelectedText());
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_EQ(L"Hellod", engine()->GetText());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  engine()->SelectAll();
  engine()->ReplaceSelectedText(L"Goodbye Everybody");
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_EQ(L"Goodbye Everybody", engine()->GetText());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  engine()->SetSelection(1, 4);
  engine()->ReplaceSelectedText(L"i,");
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_EQ(L"Hi, World", engine()->GetText());

  // Selection fully after gap.
  engine()->Clear();
  engine()->Insert(0, L"Hello");
  engine()->Insert(0, L"A ");
  engine()->SetSelection(3, 6);
  EXPECT_EQ(L"ello", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  engine()->ClearSelection();
  engine()->DeleteSelectedText();
  EXPECT_EQ(L"Hello World", engine()->GetText());
}

TEST_F(CFDE_TextEditEngineTest, UndoRedo) {
  EXPECT_FALSE(engine()->CanUndo());
  EXPECT_FALSE(engine()->CanRedo());
  EXPECT_FALSE(engine()->Undo());
  EXPECT_FALSE(engine()->Redo());

  engine()->Insert(0, L"Hello");
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_FALSE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"", engine()->GetText());
  EXPECT_FALSE(engine()->CanUndo());
  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_EQ(L"Hello", engine()->GetText());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_FALSE(engine()->CanRedo());

  engine()->Clear();
  EXPECT_FALSE(engine()->CanUndo());
  EXPECT_FALSE(engine()->CanRedo());

  engine()->Insert(0, L"Hello World");
  engine()->SelectAll();
  engine()->DeleteSelectedText();
  EXPECT_EQ(L"", engine()->GetText());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"Hello World", engine()->GetText());
  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_EQ(L"", engine()->GetText());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_FALSE(engine()->CanRedo());

  engine()->Insert(0, L"Hello World");
  engine()->SelectAll();
  engine()->ReplaceSelectedText(L"Goodbye Friend");
  EXPECT_EQ(L"Goodbye Friend", engine()->GetText());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"Hello World", engine()->GetText());
  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_EQ(L"Goodbye Friend", engine()->GetText());

  engine()->Clear();
  engine()->SetMaxEditOperationsForTesting(3);
  engine()->Insert(0, L"First ");
  engine()->Insert(engine()->GetLength(), L"Second ");
  engine()->Insert(engine()->GetLength(), L"Third");

  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"First Second ", engine()->GetText());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_FALSE(
      engine()->CanUndo());  // Can't undo First; undo buffer too small.
  EXPECT_EQ(L"First ", engine()->GetText());

  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_FALSE(engine()->CanRedo());
  EXPECT_EQ(L"First Second Third", engine()->GetText());

  engine()->Clear();

  engine()->SetMaxEditOperationsForTesting(4);

  // Go beyond the max operations limit.
  engine()->Insert(0, L"H");
  engine()->Insert(1, L"e");
  engine()->Insert(2, L"l");
  engine()->Insert(3, L"l");
  engine()->Insert(4, L"o");
  engine()->Insert(5, L" World");
  EXPECT_EQ(L"Hello World", engine()->GetText());

  // Do A, undo. Do B, undo. Redo should cause B.
  engine()->Delete(4, 3);
  EXPECT_EQ(L"Hellorld", engine()->GetText());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"Hello World", engine()->GetText());
  engine()->Delete(5, 6);
  EXPECT_EQ(L"Hello", engine()->GetText());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"Hello World", engine()->GetText());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_EQ(L"Hello", engine()->GetText());

  // Undo down to the limit.
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"Hello World", engine()->GetText());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"Hello", engine()->GetText());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_EQ(L"Hell", engine()->GetText());
  EXPECT_FALSE(engine()->Undo());
  EXPECT_EQ(L"Hell", engine()->GetText());
}

TEST_F(CFDE_TextEditEngineTest, GetIndexForPoint) {
  engine()->SetFontSize(10.0f);
  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(0U, engine()->GetIndexForPoint({0.0f, 0.0f}));
  EXPECT_EQ(11U, engine()->GetIndexForPoint({999999.0f, 9999999.0f}));
  EXPECT_EQ(11U, engine()->GetIndexForPoint({999999.0f, 0.0f}));
  EXPECT_EQ(1U, engine()->GetIndexForPoint({5.0f, 5.0f}));
  EXPECT_EQ(2U, engine()->GetIndexForPoint({10.0f, 5.0f}));
}

TEST_F(CFDE_TextEditEngineTest, GetIndexForPointLineWrap) {
  engine()->SetFontSize(10.0f);
  engine()->Insert(0,
                   L"A text long enough to span multiple lines and test "
                   L"getting indexes on multi-line edits.");
  EXPECT_EQ(0U, engine()->GetIndexForPoint({0.0f, 0.0f}));
  EXPECT_EQ(87U, engine()->GetIndexForPoint({999999.0f, 9999999.0f}));
  EXPECT_EQ(18U, engine()->GetIndexForPoint({999999.0f, 0.0f}));
  EXPECT_EQ(19U, engine()->GetIndexForPoint({1.0f, 10.0f}));
  EXPECT_EQ(1U, engine()->GetIndexForPoint({5.0f, 5.0f}));
  EXPECT_EQ(2U, engine()->GetIndexForPoint({10.0f, 5.0f}));
}

TEST_F(CFDE_TextEditEngineTest, GetIndexForPointSpaceAtEnd) {
  engine()->SetFontSize(10.0f);
  engine()->Insert(0, L"Hello World ");
  EXPECT_EQ(0U, engine()->GetIndexForPoint({0.0f, 0.0f}));
  EXPECT_EQ(12U, engine()->GetIndexForPoint({999999.0f, 9999999.0f}));
  EXPECT_EQ(12U, engine()->GetIndexForPoint({999999.0f, 0.0f}));
}

TEST_F(CFDE_TextEditEngineTest, GetIndexForPointLineBreaks) {
  engine()->SetFontSize(10.0f);
  engine()->Insert(0, L"Hello\nWorld");
  EXPECT_EQ(0U, engine()->GetIndexForPoint({0.0f, 0.0f}));
  EXPECT_EQ(5U, engine()->GetIndexForPoint({999999.0f, 0.0f}));
  EXPECT_EQ(6U, engine()->GetIndexForPoint({0.0f, 10.0f}));
  EXPECT_EQ(11U, engine()->GetIndexForPoint({999999.0f, 9999999.0f}));
}

TEST_F(CFDE_TextEditEngineTest, CanGenerateCharacterInfo) {
  RetainPtr<CFGAS_GEFont> font = engine()->GetFont();
  ASSERT_TRUE(font);

  // Has font but no text.
  EXPECT_FALSE(engine()->CanGenerateCharacterInfo());

  // Has font and text.
  engine()->Insert(0, L"Hi!");
  EXPECT_TRUE(engine()->CanGenerateCharacterInfo());

  // Has text but no font.
  engine()->SetFont(nullptr);
  EXPECT_FALSE(engine()->CanGenerateCharacterInfo());

  // Has no text and no font.
  engine()->Clear();
  EXPECT_FALSE(engine()->CanGenerateCharacterInfo());
}

TEST_F(CFDE_TextEditEngineTest, GetCharacterInfo) {
  std::pair<int32_t, CFX_RectF> char_info;

  engine()->Insert(0, L"Hi!");
  ASSERT_EQ(3U, engine()->GetLength());

  char_info = engine()->GetCharacterInfo(0);
  EXPECT_EQ(0, char_info.first);
  EXPECT_FLOAT_EQ(0.0f, char_info.second.Left());
  EXPECT_FLOAT_EQ(0.0f, char_info.second.Top());
  EXPECT_FLOAT_EQ(8.664f, char_info.second.Width());
  EXPECT_FLOAT_EQ(12.0f, char_info.second.Height());

  char_info = engine()->GetCharacterInfo(1);
  EXPECT_EQ(0, char_info.first);
  EXPECT_FLOAT_EQ(8.664f, char_info.second.Left());
  EXPECT_FLOAT_EQ(0.0f, char_info.second.Top());
  EXPECT_FLOAT_EQ(3.324f, char_info.second.Width());
  EXPECT_FLOAT_EQ(12.0f, char_info.second.Height());

  char_info = engine()->GetCharacterInfo(2);
  EXPECT_EQ(0, char_info.first);
  EXPECT_FLOAT_EQ(11.988f, char_info.second.Left());
  EXPECT_FLOAT_EQ(0.0f, char_info.second.Top());
  EXPECT_FLOAT_EQ(3.996f, char_info.second.Width());
  EXPECT_FLOAT_EQ(12.0f, char_info.second.Height());

  // Allow retrieving the character info for the end of the text, as that
  // information can be used to determine where to draw a cursor positioned at
  // the end.
  char_info = engine()->GetCharacterInfo(3);
  EXPECT_EQ(0, char_info.first);
  EXPECT_FLOAT_EQ(15.984, char_info.second.Left());
  EXPECT_FLOAT_EQ(0.0f, char_info.second.Top());
  EXPECT_FLOAT_EQ(0.0f, char_info.second.Width());
  EXPECT_FLOAT_EQ(12.0f, char_info.second.Height());
}

TEST_F(CFDE_TextEditEngineTest, BoundsForWordAt) {
  auto [start_idx, count] = engine()->BoundsForWordAt(100);
  EXPECT_EQ(0U, start_idx);
  EXPECT_EQ(0U, count);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"Hello");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(0);
  EXPECT_EQ(0U, start_idx);
  EXPECT_EQ(5U, count);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"Hello", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(100);
  EXPECT_EQ(0U, start_idx);
  EXPECT_EQ(0U, count);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"", engine()->GetSelectedText());

  std::tie(start_idx, count) = engine()->BoundsForWordAt(0);
  EXPECT_EQ(0U, start_idx);
  EXPECT_EQ(5U, count);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"Hello", engine()->GetSelectedText());

  std::tie(start_idx, count) = engine()->BoundsForWordAt(1);
  EXPECT_EQ(0U, start_idx);
  EXPECT_EQ(5U, count);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"Hello", engine()->GetSelectedText());

  std::tie(start_idx, count) = engine()->BoundsForWordAt(4);
  EXPECT_EQ(0U, start_idx);
  EXPECT_EQ(5U, count);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"Hello", engine()->GetSelectedText());

  // Select the space
  std::tie(start_idx, count) = engine()->BoundsForWordAt(5);
  EXPECT_EQ(5U, start_idx);
  EXPECT_EQ(1U, count);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L" ", engine()->GetSelectedText());

  std::tie(start_idx, count) = engine()->BoundsForWordAt(6);
  EXPECT_EQ(6U, start_idx);
  EXPECT_EQ(5U, count);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"World", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"123 456 789");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(5);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"456", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"123def789");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(5);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"123def789", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"abc456ghi");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(5);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"abc456ghi", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"hello, world");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(0);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"hello", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"hello, world");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(5);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L",", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"np-complete");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(6);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"complete", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"(123) 456-7890");
  std::tie(start_idx, count) = engine()->BoundsForWordAt(0);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"(", engine()->GetSelectedText());

  std::tie(start_idx, count) = engine()->BoundsForWordAt(1);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"123", engine()->GetSelectedText());

  std::tie(start_idx, count) = engine()->BoundsForWordAt(7);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"456", engine()->GetSelectedText());

  std::tie(start_idx, count) = engine()->BoundsForWordAt(11);
  engine()->SetSelection(start_idx, count);
  EXPECT_EQ(L"7890", engine()->GetSelectedText());

  // Tests from:
  // http://unicode.org/Public/UNIDATA/auxiliary/WordBreakTest.html#samples
  struct bounds {
    size_t start;
    size_t end;
  };
  struct {
    const wchar_t* str;
    std::vector<const wchar_t*> results;
  } tests[] = {
      // {L"\r\na\n\u0308", {L"\r\n", L"a", L"\n", L"\u0308"}},
      // {L"a\u0308", {L"a\u0308"}},
      // {L" \u200d\u0646", {L" \u200d", L"\u0646"}},
      // {L"\u0646\u200d ", {L"\u0646\u200d", L" "}},
      {L"AAA", {L"AAA"}},
      {L"A:A", {L"A:A"}},
      {L"A::A", {L"A", L":", L":", L"A"}},
      // {L"\u05d0'", {L"\u05d0'"}},
      // {L"\u05d0\"\u05d0", {L"\u05d0\"\u05d0"}},
      {L"A00A", {L"A00A"}},
      {L"0,0", {L"0,0"}},
      {L"0,,0", {L"0", L",", L",", L"0"}},
      {L"\u3031\u3031", {L"\u3031\u3031"}},
      {L"A_0_\u3031_", {L"A_0_\u3031_"}},
      {L"A__A", {L"A__A"}},
      // {L"\u200d\u2640", {L"\u200d\u2640"}},
      // {L"a\u0308\u200b\u0308b", {L"a\u0308\u200b\u0308b"}},
  };

  for (auto t : tests) {
    engine()->Clear();
    engine()->Insert(0, t.str);

    size_t idx = 0;
    for (const auto* res : t.results) {
      std::tie(start_idx, count) = engine()->BoundsForWordAt(idx);
      engine()->SetSelection(start_idx, count);
      EXPECT_EQ(res, engine()->GetSelectedText()) << "Input: '" << t.str << "'";
      idx += count;
    }
  }
}

TEST_F(CFDE_TextEditEngineTest, CursorMovement) {
  engine()->Clear();
  engine()->Insert(0, L"Hello");

  EXPECT_EQ(0U, engine()->GetIndexLeft(0));
  EXPECT_EQ(5U, engine()->GetIndexRight(5));
  EXPECT_EQ(2U, engine()->GetIndexUp(2));
  EXPECT_EQ(2U, engine()->GetIndexDown(2));
  EXPECT_EQ(1U, engine()->GetIndexLeft(2));
  EXPECT_EQ(3U, engine()->GetIndexRight(2));
  EXPECT_EQ(0U, engine()->GetIndexAtStartOfLine(2));
  EXPECT_EQ(5U, engine()->GetIndexAtEndOfLine(2));

  engine()->Clear();
  engine()->Insert(0, L"The book is \"مدخل إلى C++\"");
  EXPECT_FALSE(FX_IsOdd(engine()->GetCharacterInfo(3).first));
  EXPECT_EQ(2U, engine()->GetIndexLeft(3));
  EXPECT_EQ(4U, engine()->GetIndexRight(3));
  EXPECT_TRUE(FX_IsOdd(engine()->GetCharacterInfo(15).first));
  EXPECT_EQ(14U, engine()->GetIndexLeft(15));
  EXPECT_EQ(16U, engine()->GetIndexRight(15));
  EXPECT_FALSE(FX_IsOdd(engine()->GetCharacterInfo(23).first));
  EXPECT_EQ(22U, engine()->GetIndexLeft(23));
  EXPECT_EQ(24U, engine()->GetIndexRight(23));

  engine()->Clear();
  engine()->Insert(0, L"Hello\r\nWorld\r\nTest");
  // Move to end of Hello from start of World.
  engine()->SetSelection(engine()->GetIndexLeft(7U), 7);
  EXPECT_EQ(L"\r\nWorld", engine()->GetSelectedText());

  // Second letter in Hello from second letter in World.
  engine()->SetSelection(engine()->GetIndexUp(8U), 2);
  EXPECT_EQ(L"el", engine()->GetSelectedText());

  // Second letter in World from second letter in Test.
  engine()->SetSelection(engine()->GetIndexUp(15U), 2);
  EXPECT_EQ(L"or", engine()->GetSelectedText());

  // Second letter in World from second letter in Hello.
  engine()->SetSelection(engine()->GetIndexDown(1U), 2);
  EXPECT_EQ(L"or", engine()->GetSelectedText());

  // Second letter in Test from second letter in World.
  engine()->SetSelection(engine()->GetIndexDown(8U), 2);
  EXPECT_EQ(L"es", engine()->GetSelectedText());

  size_t start_idx = engine()->GetIndexAtStartOfLine(8U);
  size_t end_idx = engine()->GetIndexAtEndOfLine(8U);
  engine()->SetSelection(start_idx, end_idx - start_idx);
  EXPECT_EQ(L"World", engine()->GetSelectedText());

  // Move past \r\n to before W.
  engine()->SetSelection(engine()->GetIndexRight(5U), 5);
  EXPECT_EQ(L"World", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"Short\nAnd a very long line");
  engine()->SetSelection(engine()->GetIndexUp(14U), 11);
  EXPECT_EQ(L"\nAnd a very", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"A Very long line\nShort");
  EXPECT_EQ(engine()->GetLength(), engine()->GetIndexDown(8U));

  engine()->Clear();
  engine()->Insert(0, L"Hello\rWorld\rTest");
  // Move to end of Hello from start of World.
  engine()->SetSelection(engine()->GetIndexLeft(6U), 6);
  EXPECT_EQ(L"\rWorld", engine()->GetSelectedText());

  // Second letter in Hello from second letter in World.
  engine()->SetSelection(engine()->GetIndexUp(7U), 2);
  EXPECT_EQ(L"el", engine()->GetSelectedText());

  // Second letter in World from second letter in Test.
  engine()->SetSelection(engine()->GetIndexUp(13U), 2);
  EXPECT_EQ(L"or", engine()->GetSelectedText());

  // Second letter in World from second letter in Hello.
  engine()->SetSelection(engine()->GetIndexDown(1U), 2);
  EXPECT_EQ(L"or", engine()->GetSelectedText());

  // Second letter in Test from second letter in World.
  engine()->SetSelection(engine()->GetIndexDown(7U), 2);
  EXPECT_EQ(L"es", engine()->GetSelectedText());

  start_idx = engine()->GetIndexAtStartOfLine(7U);
  end_idx = engine()->GetIndexAtEndOfLine(7U);
  engine()->SetSelection(start_idx, end_idx - start_idx);
  EXPECT_EQ(L"World", engine()->GetSelectedText());

  // Move past \r to before W.
  engine()->SetSelection(engine()->GetIndexRight(5U), 5);
  EXPECT_EQ(L"World", engine()->GetSelectedText());

  engine()->Clear();
  engine()->Insert(0, L"Hello\nWorld\nTest");
  // Move to end of Hello from start of World.
  engine()->SetSelection(engine()->GetIndexLeft(6U), 6);
  EXPECT_EQ(L"\nWorld", engine()->GetSelectedText());

  // Second letter in Hello from second letter in World.
  engine()->SetSelection(engine()->GetIndexUp(7U), 2);
  EXPECT_EQ(L"el", engine()->GetSelectedText());

  // Second letter in World from second letter in Test.
  engine()->SetSelection(engine()->GetIndexUp(13U), 2);
  EXPECT_EQ(L"or", engine()->GetSelectedText());

  // Second letter in World from second letter in Hello.
  engine()->SetSelection(engine()->GetIndexDown(1U), 2);
  EXPECT_EQ(L"or", engine()->GetSelectedText());

  // Second letter in Test from second letter in World.
  engine()->SetSelection(engine()->GetIndexDown(7U), 2);
  EXPECT_EQ(L"es", engine()->GetSelectedText());

  start_idx = engine()->GetIndexAtStartOfLine(7U);
  end_idx = engine()->GetIndexAtEndOfLine(7U);
  engine()->SetSelection(start_idx, end_idx - start_idx);
  EXPECT_EQ(L"World", engine()->GetSelectedText());

  // Move past \r to before W.
  engine()->SetSelection(engine()->GetIndexRight(5U), 5);
  EXPECT_EQ(L"World", engine()->GetSelectedText());
}

}  // namespace pdfium

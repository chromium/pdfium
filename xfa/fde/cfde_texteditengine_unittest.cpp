// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/cfde_texteditengine.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

class CFDE_TextEditEngineTest : public testing::Test {
 public:
  class Delegate : public CFDE_TextEditEngine::Delegate {
   public:
    void Reset() {
      text_is_full = false;
      fail_validation = false;
    }

    void NotifyTextFull() override { text_is_full = true; }

    void OnCaretChanged() override {}
    void OnTextChanged(const CFX_WideString& prevText) override {}
    void OnSelChanged() override {}
    bool OnValidate(const CFX_WideString& wsText) override {
      return !fail_validation;
    }
    void SetScrollOffset(float fScrollOffset) override {}

    bool fail_validation = false;
    bool text_is_full = false;
  };

  CFDE_TextEditEngineTest() {}
  ~CFDE_TextEditEngineTest() override {}

  void SetUp() override {
    font_ =
        CFGAS_GEFont::LoadFont(L"Arial Black", 0, 0, GetGlobalFontManager());
    ASSERT(font_.Get() != nullptr);

    engine_ = pdfium::MakeUnique<CFDE_TextEditEngine>();
    engine_->SetFont(font_);
    engine_->SetFontSize(12.0f);
  }

  void TearDown() override { engine_.reset(); }

  CFDE_TextEditEngine* engine() const { return engine_.get(); }

 private:
  CFX_RetainPtr<CFGAS_GEFont> font_;
  std::unique_ptr<CFDE_TextEditEngine> engine_;
};

TEST_F(CFDE_TextEditEngineTest, Insert) {
  EXPECT_STREQ(L"", engine()->GetText().c_str());

  engine()->Insert(0, L"");
  EXPECT_STREQ(L"", engine()->GetText().c_str());
  EXPECT_EQ(0U, engine()->GetLength());

  engine()->Insert(0, L"Hello");
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());
  EXPECT_EQ(5U, engine()->GetLength());

  engine()->Insert(5, L" World");
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());
  EXPECT_EQ(11U, engine()->GetLength());

  engine()->Insert(5, L" New");
  EXPECT_STREQ(L"Hello New World", engine()->GetText().c_str());

  engine()->Insert(100, L" Cat");
  EXPECT_STREQ(L"Hello New World Cat", engine()->GetText().c_str());

  engine()->Clear();

  engine()->SetHasCharacterLimit(true);
  engine()->SetCharacterLimit(5);
  engine()->Insert(0, L"Hello");

  // No delegate
  engine()->Insert(5, L" World");
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());

  engine()->SetCharacterLimit(8);
  engine()->Insert(5, L" World");
  EXPECT_STREQ(L"Hello Wo", engine()->GetText().c_str());

  engine()->Clear();

  // With Delegate
  auto delegate = pdfium::MakeUnique<CFDE_TextEditEngineTest::Delegate>();
  engine()->SetDelegate(delegate.get());

  engine()->SetCharacterLimit(5);
  engine()->Insert(0, L"Hello");

  // Insert when full.
  engine()->Insert(5, L" World");
  EXPECT_TRUE(delegate->text_is_full);
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());
  delegate->Reset();

  engine()->SetCharacterLimit(8);
  engine()->Insert(5, L" World");
  EXPECT_TRUE(delegate->text_is_full);
  EXPECT_STREQ(L"Hello Wo", engine()->GetText().c_str());
  delegate->Reset();
  engine()->SetHasCharacterLimit(false);

  engine()->Clear();
  engine()->Insert(0, L"Hello");

  // Insert Invalid text
  delegate->fail_validation = true;
  engine()->EnableValidation(true);
  engine()->Insert(5, L" World");
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());

  delegate->fail_validation = false;
  engine()->Insert(5, L" World");
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());
  engine()->EnableValidation(false);

  engine()->Clear();

  engine()->Insert(0, L"Hello\nWorld");
  EXPECT_FALSE(delegate->text_is_full);
  EXPECT_STREQ(L"Hello\nWorld", engine()->GetText().c_str());
  delegate->Reset();
  engine()->Clear();

  // Insert with limited area and over-fill
  engine()->LimitHorizontalScroll(true);
  engine()->SetAvailableWidth(60.0f);  // Fits 'Hello Wo'.
  engine()->Insert(0, L"Hello");
  EXPECT_FALSE(delegate->text_is_full);
  engine()->Insert(5, L" World");
  EXPECT_TRUE(delegate->text_is_full);
  EXPECT_STREQ(L"Hello Wo", engine()->GetText().c_str());
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
  EXPECT_STREQ(L"Hello Wo\n", engine()->GetText().c_str());
  engine()->LimitVerticalScroll(false);

  engine()->SetDelegate(nullptr);
}

TEST_F(CFDE_TextEditEngineTest, Delete) {
  EXPECT_STREQ(L"", engine()->Delete(0, 50).c_str());
  EXPECT_STREQ(L"", engine()->GetText().c_str());

  engine()->Insert(0, L"Hello World");
  EXPECT_STREQ(L" World", engine()->Delete(5, 6).c_str());
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  EXPECT_STREQ(L" ", engine()->Delete(5, 1).c_str());
  EXPECT_STREQ(L"HelloWorld", engine()->GetText().c_str());

  EXPECT_STREQ(L"elloWorld", engine()->Delete(1, 50).c_str());
  EXPECT_STREQ(L"H", engine()->GetText().c_str());
}

TEST_F(CFDE_TextEditEngineTest, Clear) {
  EXPECT_STREQ(L"", engine()->GetText().c_str());

  engine()->Clear();
  EXPECT_STREQ(L"", engine()->GetText().c_str());

  engine()->Insert(0, L"Hello World");
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());

  engine()->Clear();
  EXPECT_STREQ(L"", engine()->GetText().c_str());
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
  EXPECT_STREQ(L"Hello A World", engine()->GetText().c_str());
  EXPECT_EQ(L'W', engine()->GetChar(8));

  engine()->EnablePasswordMode(true);
  EXPECT_EQ(L'*', engine()->GetChar(8));

  engine()->SetAliasChar(L'+');
  EXPECT_EQ(L'+', engine()->GetChar(8));
}

TEST_F(CFDE_TextEditEngineTest, GetWidthOfChar) {
  // Out of Bounds.
  EXPECT_EQ(0U, engine()->GetWidthOfChar(0));

  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(199920U, engine()->GetWidthOfChar(0));
  EXPECT_EQ(159840U, engine()->GetWidthOfChar(1));

  engine()->Insert(0, L"\t");
  EXPECT_EQ(0U, engine()->GetWidthOfChar(0));
}

TEST_F(CFDE_TextEditEngineTest, GetDisplayPos) {
  EXPECT_EQ(0U, engine()->GetDisplayPos(FDE_TEXTEDITPIECE()).size());
}

TEST_F(CFDE_TextEditEngineTest, Selection) {
  EXPECT_FALSE(engine()->HasSelection());
  engine()->SelectAll();
  EXPECT_FALSE(engine()->HasSelection());

  engine()->Insert(0, L"Hello World");
  EXPECT_STREQ(L"", engine()->DeleteSelectedText().c_str());

  EXPECT_FALSE(engine()->HasSelection());
  engine()->SelectAll();
  EXPECT_TRUE(engine()->HasSelection());
  EXPECT_STREQ(L"Hello World", engine()->GetSelectedText().c_str());

  engine()->ClearSelection();
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_STREQ(L"", engine()->GetSelectedText().c_str());

  engine()->SelectAll();
  size_t start_idx;
  size_t end_idx;
  std::tie(start_idx, end_idx) = engine()->GetSelection();
  EXPECT_EQ(0U, start_idx);
  EXPECT_EQ(10U, end_idx);

  // Selection before gap.
  EXPECT_STREQ(L"Hello World", engine()->GetSelectedText().c_str());
  EXPECT_TRUE(engine()->HasSelection());
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());

  engine()->Insert(5, L" A");
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_STREQ(L"", engine()->GetSelectedText().c_str());

  // Selection over the gap.
  engine()->SelectAll();
  EXPECT_TRUE(engine()->HasSelection());
  EXPECT_STREQ(L"Hello A World", engine()->GetSelectedText().c_str());
  engine()->Clear();

  engine()->Insert(0, L"Hello World");
  engine()->SelectAll();

  EXPECT_STREQ(L"Hello World", engine()->DeleteSelectedText().c_str());
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_STREQ(L"", engine()->GetText().c_str());

  engine()->Insert(0, L"Hello World");
  engine()->SetSelection(5, 9);
  EXPECT_STREQ(L" Worl", engine()->DeleteSelectedText().c_str());
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_STREQ(L"Hellod", engine()->GetText().c_str());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  engine()->SelectAll();
  engine()->ReplaceSelectedText(L"Goodbye Everybody");
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_STREQ(L"Goodbye Everybody", engine()->GetText().c_str());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  engine()->SetSelection(1, 4);
  engine()->ReplaceSelectedText(L"i,");
  EXPECT_FALSE(engine()->HasSelection());
  EXPECT_STREQ(L"Hi, World", engine()->GetText().c_str());

  // Selection fully after gap.
  engine()->Clear();
  engine()->Insert(0, L"Hello");
  engine()->Insert(0, L"A ");
  engine()->SetSelection(3, 6);
  EXPECT_STREQ(L"ello", engine()->GetSelectedText().c_str());

  engine()->Clear();
  engine()->Insert(0, L"Hello World");
  engine()->ClearSelection();
  engine()->DeleteSelectedText();
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());
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
  EXPECT_STREQ(L"", engine()->GetText().c_str());
  EXPECT_FALSE(engine()->CanUndo());
  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_FALSE(engine()->CanRedo());

  engine()->Clear();
  EXPECT_FALSE(engine()->CanUndo());
  EXPECT_FALSE(engine()->CanRedo());

  engine()->Insert(0, L"Hello World");
  engine()->SelectAll();
  engine()->DeleteSelectedText();
  EXPECT_STREQ(L"", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_STREQ(L"", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_FALSE(engine()->CanRedo());

  engine()->Insert(0, L"Hello World");
  engine()->SelectAll();
  engine()->ReplaceSelectedText(L"Goodbye Friend");
  EXPECT_STREQ(L"Goodbye Friend", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_STREQ(L"Goodbye Friend", engine()->GetText().c_str());

  engine()->Clear();
  engine()->SetMaxEditOperationsForTesting(3);
  engine()->Insert(0, L"First ");
  engine()->Insert(engine()->GetLength(), L"Second ");
  engine()->Insert(engine()->GetLength(), L"Third");

  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_STREQ(L"First Second ", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->CanUndo());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_FALSE(
      engine()->CanUndo());  // Can't undo First; undo buffer too small.
  EXPECT_STREQ(L"First ", engine()->GetText().c_str());

  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_TRUE(engine()->CanRedo());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_FALSE(engine()->CanRedo());
  EXPECT_STREQ(L"First Second Third", engine()->GetText().c_str());

  engine()->Clear();

  engine()->SetMaxEditOperationsForTesting(4);

  // Go beyond the max operations limit.
  engine()->Insert(0, L"H");
  engine()->Insert(1, L"e");
  engine()->Insert(2, L"l");
  engine()->Insert(3, L"l");
  engine()->Insert(4, L"o");
  engine()->Insert(5, L" World");
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());

  // Do A, undo. Do B, undo. Redo should cause B.
  engine()->Delete(4, 3);
  EXPECT_STREQ(L"Hellorld", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());
  engine()->Delete(5, 6);
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->Redo());
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());

  // Undo down to the limit.
  EXPECT_TRUE(engine()->Undo());
  EXPECT_STREQ(L"Hello World", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_STREQ(L"Hello", engine()->GetText().c_str());
  EXPECT_TRUE(engine()->Undo());
  EXPECT_STREQ(L"Hell", engine()->GetText().c_str());
  EXPECT_FALSE(engine()->Undo());
  EXPECT_STREQ(L"Hell", engine()->GetText().c_str());
}

TEST_F(CFDE_TextEditEngineTest, GetIndexForPoint) {
  engine()->SetFontSize(10.0f);
  engine()->Insert(0, L"Hello World");
  EXPECT_EQ(0U, engine()->GetIndexForPoint({0.0f, 0.0f}));
  EXPECT_EQ(11U, engine()->GetIndexForPoint({999999.0f, 9999999.0f}));
  EXPECT_EQ(1U, engine()->GetIndexForPoint({10.0f, 5.0f}));
}

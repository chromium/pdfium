// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fde/cfde_txtedtbuf.h"

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"
#include "third_party/base/ptr_util.h"

class CFDE_TxtEdtBufTest : public testing::Test {
 public:
  void SetUp() override {
    buf_ = pdfium::MakeUnique<CFDE_TxtEdtBuf>();
    buf_->SetChunkSizeForTesting(5);
  }
  size_t ChunkCount() const { return buf_->m_chunks.size(); }

  std::unique_ptr<CFDE_TxtEdtBuf> buf_;
};

TEST_F(CFDE_TxtEdtBufTest, SetTextLessThenChunkSize) {
  buf_->SetText(L"Hi");
  EXPECT_EQ(1UL, ChunkCount());
  EXPECT_EQ(2, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(2, res.GetLength());
  EXPECT_EQ(L"Hi", res);
}

TEST_F(CFDE_TxtEdtBufTest, InsertAppendChunk) {
  buf_->SetText(L"Hi");

  CFX_WideString end = L" World";
  buf_->Insert(2, end.c_str(), end.GetLength());
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(8, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(8, res.GetLength());
  EXPECT_EQ(L"Hi World", res);
}

TEST_F(CFDE_TxtEdtBufTest, InsertPrependChunk) {
  buf_->SetText(L"Hi");

  CFX_WideString end = L"World ";
  buf_->Insert(0, end.c_str(), end.GetLength());
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(8, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"World Hi", res);
  EXPECT_EQ(8, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, InsertBetweenChunks) {
  buf_->SetText(L"Hello World");
  EXPECT_EQ(3UL, ChunkCount());

  CFX_WideString inst = L"there ";
  buf_->Insert(6, inst.c_str(), inst.GetLength());
  EXPECT_EQ(5UL, ChunkCount());
  EXPECT_EQ(17, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"Hello there World", res);
  EXPECT_EQ(17, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, SetText) {
  buf_->SetText(L"Hello World");
  EXPECT_EQ(11, buf_->GetTextLength());

  buf_->SetText(L"Hi");
  // Don't remove chunks on setting shorter text.
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(2, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"Hi", res);
  EXPECT_EQ(2, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, DeleteMiddleText) {
  buf_->SetText(L"Hello there World");
  buf_->Delete(6, 6);
  EXPECT_EQ(4UL, ChunkCount());
  EXPECT_EQ(11, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"Hello World", res);
  EXPECT_EQ(11, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, DeleteEndText) {
  buf_->SetText(L"Hello World");
  buf_->Delete(5, 6);
  EXPECT_EQ(1UL, ChunkCount());
  EXPECT_EQ(5, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"Hello", res);
  EXPECT_EQ(5, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, DeleteStartText) {
  buf_->SetText(L"Hello World");
  buf_->Delete(0, 6);
  EXPECT_EQ(2UL, ChunkCount());
  EXPECT_EQ(5, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"World", res);
  EXPECT_EQ(5, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, DeleteAllText) {
  buf_->SetText(L"Hello World");
  buf_->Delete(0, 11);
  EXPECT_EQ(0UL, ChunkCount());
  EXPECT_EQ(0, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"", res);
  EXPECT_EQ(0, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, ClearWithRelease) {
  buf_->SetText(L"Hello World");
  buf_->Clear(true);
  EXPECT_EQ(0UL, ChunkCount());
  EXPECT_EQ(0, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"", res);
  EXPECT_EQ(0, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, ClearWithoutRelease) {
  buf_->SetText(L"Hello World");
  buf_->Clear(false);
  EXPECT_EQ(3UL, ChunkCount());
  EXPECT_EQ(0, buf_->GetTextLength());

  CFX_WideString res = buf_->GetText();
  EXPECT_EQ(L"", res);
  EXPECT_EQ(0, res.GetLength());
}

TEST_F(CFDE_TxtEdtBufTest, GetCharByIndex) {
  buf_->SetText(L"Hello world");
  EXPECT_EQ(L"e", CFX_WideString(buf_->GetCharByIndex(1)));
  EXPECT_EQ(L"o", CFX_WideString(buf_->GetCharByIndex(7)));
}

TEST_F(CFDE_TxtEdtBufTest, GetRange) {
  buf_->SetText(L"Hello World");
  EXPECT_EQ(L"", buf_->GetRange(1, 0));
  EXPECT_EQ(L"ello", buf_->GetRange(1, 4));
  EXPECT_EQ(L"lo Wo", buf_->GetRange(3, 5));
}

#ifndef NDEBUG
using CFDE_TxtEdtBufTestDeathTest = CFDE_TxtEdtBufTest;

TEST_F(CFDE_TxtEdtBufTestDeathTest, InsertBadIndexes) {
  CFX_WideString inst = L"there ";

  buf_->SetText(L"Hi");
  EXPECT_DEATH(buf_->Insert(-4, inst.c_str(), inst.GetLength()), "Assertion");
  EXPECT_DEATH(buf_->Insert(9999, inst.c_str(), inst.GetLength()), "Assertion");
  EXPECT_DEATH(buf_->Insert(1, inst.c_str(), -6), "Assertion");
}

TEST_F(CFDE_TxtEdtBufTestDeathTest, DeleteWithBadIdx) {
  buf_->SetText(L"Hi");
  EXPECT_DEATH(buf_->Delete(-10, 4), "Assertion");
  EXPECT_DEATH(buf_->Delete(1, -5), "Assertion");
  EXPECT_DEATH(buf_->Delete(5, 1), "Assertion");
  EXPECT_DEATH(buf_->Delete(0, 10000), "Assertion");
}

TEST_F(CFDE_TxtEdtBufTestDeathTest, GetCharByIndex) {
  buf_->SetText(L"Hi");
  EXPECT_DEATH(buf_->GetCharByIndex(-1), "Assertion");
  EXPECT_DEATH(buf_->GetCharByIndex(100), "Assertion");
}

TEST_F(CFDE_TxtEdtBufTestDeathTest, GetRange) {
  buf_->SetText(L"Hi");
  EXPECT_DEATH(buf_->GetRange(1, -1), "Assertion");
  EXPECT_DEATH(buf_->GetRange(-1, 1), "Assertion");
  EXPECT_DEATH(buf_->GetRange(10, 1), "Assertion");
  EXPECT_DEATH(buf_->GetRange(1, 100), "Assertion");
}

#endif  // NDEBUG

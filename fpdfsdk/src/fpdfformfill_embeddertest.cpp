// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.


#include "../../public/fpdf_formfill.h"
#include "../../testing/embedder_test.h"
#include "../../testing/embedder_test_mock_delegate.h"
#include "../../testing/embedder_test_timer_handling_delegate.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::Return;

class FPDFFormFillEmbeddertest : public EmbedderTest {
};

TEST_F(FPDFFormFillEmbeddertest, FirstTest) {
  EmbedderTestMockDelegate mock;
  EXPECT_CALL(mock, Alert(_, _, _, _)).Times(0);
  EXPECT_CALL(mock, UnsupportedHandler(_)).Times(0);
  EXPECT_CALL(mock, SetTimer(_, _)).Times(0);
  EXPECT_CALL(mock, KillTimer(_)).Times(0);
  SetDelegate(&mock);

  EXPECT_TRUE(OpenDocument("testing/resources/hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  UnloadPage(page);
}

TEST_F(FPDFFormFillEmbeddertest, BUG_487928) {
  EmbedderTestTimerHandlingDelegate delegate;
  SetDelegate(&delegate);

  EXPECT_TRUE(OpenDocument("testing/resources/bug_487928.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_NE(nullptr, page);
  DoOpenActions();
  delegate.AdvanceTime(5000);
  UnloadPage(page);
}

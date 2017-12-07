// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/widestring.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_timer_handling_delegate.h"
#include "testing/gtest/include/gtest/gtest.h"

class CFWLEditEmbeddertest : public EmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    SetDelegate(&delegate_);
    CreateAndInitializeFormPDF();
  }

  void TearDown() override {
    UnloadPage(page());
    EmbedderTest::TearDown();
  }

  void CreateAndInitializeFormPDF() {
    EXPECT_TRUE(OpenDocument("xfa/email_recommended.pdf"));
    page_ = LoadPage(0);
    ASSERT_TRUE(page_);
  }

  FPDF_PAGE page() const { return page_; }
  EmbedderTestTimerHandlingDelegate delegate() const { return delegate_; }

 private:
  FPDF_PAGE page_;
  EmbedderTestTimerHandlingDelegate delegate_;
};

TEST_F(CFWLEditEmbeddertest, Trivial) {
  ASSERT_EQ(1u, delegate().GetAlerts().size());
  auto alert = delegate().GetAlerts()[0];
  EXPECT_STREQ(L"PDFium", alert.title.c_str());
  EXPECT_STREQ(L"The value you entered for Text Field is invalid.",
               alert.message.c_str());
}

TEST_F(CFWLEditEmbeddertest, LeftClickMouseSelection) {
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);
  for (size_t i = 0; i < 10; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);

  // Mouse selection
  FORM_OnLButtonDown(form_handle(), page(), 0, 128, 58);
  FORM_OnLButtonDown(form_handle(), page(), FWL_EVENTFLAG_ShiftKey, 152, 58);

  // 12 == (2 * strlen(defgh)) + 2 (for \0\0)
  EXPECT_EQ(12UL, FORM_GetSelectedText(form_handle(), page(), nullptr, 0));

  unsigned short buf[128];
  unsigned long len = FORM_GetSelectedText(form_handle(), page(), &buf, 128);
  EXPECT_STREQ(L"defgh", WideString::FromUTF16LE(buf, len).c_str());
}

TEST_F(CFWLEditEmbeddertest, DragMouseSelection) {
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);
  for (size_t i = 0; i < 10; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);

  // Mouse selection
  FORM_OnLButtonDown(form_handle(), page(), 0, 128, 58);
  FORM_OnMouseMove(form_handle(), page(), FWL_EVENTFLAG_ShiftKey, 152, 58);

  // 12 == (2 * strlen(defgh)) + 2 (for \0\0)
  EXPECT_EQ(12UL, FORM_GetSelectedText(form_handle(), page(), nullptr, 0));

  unsigned short buf[128];
  unsigned long len = FORM_GetSelectedText(form_handle(), page(), &buf, 128);
  EXPECT_STREQ(L"defgh", WideString::FromUTF16LE(buf, len).c_str());
}

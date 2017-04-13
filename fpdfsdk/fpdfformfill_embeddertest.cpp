// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>
#include <string>

#include "core/fxcrt/fx_system.h"
#include "public/cpp/fpdf_deleters.h"
#include "public/fpdf_formfill.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_mock_delegate.h"
#include "testing/embedder_test_timer_handling_delegate.h"
#include "testing/gmock/include/gmock/gmock.h"
#include "testing/gtest/include/gtest/gtest.h"

using testing::_;
using testing::Return;

class FPDFFormFillEmbeddertest : public EmbedderTest, public TestSaver {};

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

#endif  // PDF_ENABLE_V8

TEST_F(FPDFFormFillEmbeddertest, FormText) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
  const char md5_1[] = "5f11dbe575fe197a37c3fb422559f8ff";
  const char md5_2[] = "35b1a4b679eafc749a0b6fda750c0e8d";
  const char md5_3[] = "65c64a7c355388f719a752aa1e23f6fe";
#else
  const char md5_1[] = "23baecc6e94d4c8b894cd39aa04c584c";
  const char md5_2[] = "499df95d477dfe35ee65b823c69743b5";
  const char md5_3[] = "8f91b62895fc505d9e17ff2d633756d4";
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

    // Close everything
    UnloadPage(page);
    FPDFDOC_ExitFormFillEnvironment(form_handle_);
    FPDF_CloseDocument(document_);
  }
  // Check saved document
  std::string new_file = GetString();
  FPDF_FILEACCESS file_access;
  memset(&file_access, 0, sizeof(file_access));
  file_access.m_FileLen = new_file.size();
  file_access.m_GetBlock = GetBlockFromString;
  file_access.m_Param = &new_file;
  document_ = FPDF_LoadCustomDocument(&file_access, nullptr);
  SetupFormFillEnvironment();
  EXPECT_EQ(1, FPDF_GetPageCount(document_));
  std::unique_ptr<void, FPDFPageDeleter> new_page(FPDF_LoadPage(document_, 0));
  ASSERT_TRUE(new_page.get());
  std::unique_ptr<void, FPDFBitmapDeleter> new_bitmap(
      RenderPage(new_page.get()));
  CompareBitmap(new_bitmap.get(), 300, 300, md5_3);
}

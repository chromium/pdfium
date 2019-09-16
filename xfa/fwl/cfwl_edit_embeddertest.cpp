// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fxcrt/widestring.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_timer_handling_delegate.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

class CFWLEditEmbedderTest : public XFAJSEmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    SetDelegate(&delegate_);
  }

  void TearDown() override {
    UnloadPage(page());
    EmbedderTest::TearDown();
  }

  void CreateAndInitializeFormPDF(const char* filename) {
    EXPECT_TRUE(OpenDocument(filename));
    page_ = LoadPage(0);
    ASSERT_TRUE(page_);
  }

  FPDF_PAGE page() const { return page_; }
  EmbedderTestTimerHandlingDelegate delegate() const { return delegate_; }

 private:
  FPDF_PAGE page_;
  EmbedderTestTimerHandlingDelegate delegate_;
};

TEST_F(CFWLEditEmbedderTest, Trivial) {
  CreateAndInitializeFormPDF("xfa/email_recommended.pdf");
  ASSERT_EQ(0u, delegate().GetAlerts().size());
}

TEST_F(CFWLEditEmbedderTest, LeftClickMouseSelection) {
  CreateAndInitializeFormPDF("xfa/email_recommended.pdf");
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

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_DragMouseSelection DISABLED_DragMouseSelection
#else
#define MAYBE_DragMouseSelection DragMouseSelection
#endif
TEST_F(CFWLEditEmbedderTest, MAYBE_DragMouseSelection) {
  CreateAndInitializeFormPDF("xfa/email_recommended.pdf");
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

  // TODO(hnakashima): This is incorrect. Visually 'abcdefgh' are selected.
  const char kDraggedMD5[] = "f131526c8edd04e44de17b2647ec54c8";
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kDraggedMD5);
  }
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_SimpleFill DISABLED_SimpleFill
#else
#define MAYBE_SimpleFill SimpleFill
#endif
TEST_F(CFWLEditEmbedderTest, MAYBE_SimpleFill) {
  CreateAndInitializeFormPDF("xfa/email_recommended.pdf");
  const char kBlankMD5[] = "8dda78a3afaf9f7b5210eb81cacc4600";
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kBlankMD5);
  }

  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);
  for (size_t i = 0; i < 10; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);

  const char kFilledMD5[] = "211e4e46eb347aa2bc7c425556d600b0";
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_FillWithNewLineWithoutMultiline \
  DISABLED_FillWithNewLineWithoutMultiline
#else
#define MAYBE_FillWithNewLineWithoutMultiline FillWithNewLineWithoutMultiline
#endif
TEST_F(CFWLEditEmbedderTest, MAYBE_FillWithNewLineWithoutMultiline) {
  CreateAndInitializeFormPDF("xfa/email_recommended.pdf");
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);
  for (size_t i = 0; i < 5; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);
  FORM_OnChar(form_handle(), page(), '\r', 0);
  for (size_t i = 5; i < 10; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);

  const char kFilledMD5[] = "211e4e46eb347aa2bc7c425556d600b0";
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }
}

// Disabled due to flakiness.
TEST_F(CFWLEditEmbedderTest, DISABLED_FillWithNewLineWithMultiline) {
  CreateAndInitializeFormPDF("xfa/xfa_multiline_textfield.pdf");
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);

  for (size_t i = 0; i < 5; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);
  FORM_OnChar(form_handle(), page(), '\r', 0);
  for (size_t i = 5; i < 10; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);

  // Should look like:
  // abcde
  // fghij|
  {
#if _FX_PLATFORM_ == _FX_PLATFORM_LINUX_
    const char kFilledMultilineMD5[] = "fc1f4d5fdb2c5755005fc525b0a60ec9";
#else
    const char kFilledMultilineMD5[] = "a5654e027d8b1667c20f3b86d1918003";
#endif  // _FX_PLATFORM_ == _FX_PLATFORM_LINUX_
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kFilledMultilineMD5);
  }

  for (size_t i = 0; i < 4; ++i)
    FORM_OnKeyDown(form_handle(), page(), FWL_VKEY_Left, 0);

  // Should look like:
  // abcde
  // f|ghij

  // Two backspaces is a workaround because left arrow does not behave well
  // in the first character of a line. It skips back to the previous line.
  for (size_t i = 0; i < 2; ++i)
    FORM_OnChar(form_handle(), page(), '\b', 0);

  // Should look like:
  // abcde|ghij
  {
#if _FX_PLATFORM_ == _FX_PLATFORM_LINUX_
    const char kMultilineBackspaceMD5[] = "8bb62a8100ff1e1cc113d4033e0d824e";
#else
    const char kMultilineBackspaceMD5[] = "a2f1dcab92bb1fb7c2f9ccc70100c989";
#endif  // _FX_PLATFORM_ == _FX_PLATFORM_LINUX_
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kMultilineBackspaceMD5);
  }
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_DateTimePickerTest DISABLED_DateTimePickerTest
#else
#define MAYBE_DateTimePickerTest DateTimePickerTest
#endif
TEST_F(CFWLEditEmbedderTest, MAYBE_DateTimePickerTest) {
  CreateAndInitializeFormPDF("xfa/xfa_date_time_edit.pdf");
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);

  const char kFilledMD5[] = "1036b8837a9dba75c6bd8f9347ae2eb2";
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }
}

TEST_F(CFWLEditEmbedderTest, ImageEditTest) {
  CreateAndInitializeFormPDF("xfa/xfa_image_edit.pdf");
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);

  const char kFilledMD5[] = "1940568c9ba33bac5d0b1ee9558c76b3";
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }
}

// TODO(crbug.com/pdfium/11): Fix this test and enable.
#if defined(_SKIA_SUPPORT_) || defined(_SKIA_SUPPORT_PATHS_)
#define MAYBE_ComboBoxTest DISABLED_ComboBoxTest
#else
#define MAYBE_ComboBoxTest ComboBoxTest
#endif
TEST_F(CFWLEditEmbedderTest, MAYBE_ComboBoxTest) {
  CreateAndInitializeFormPDF("xfa/xfa_combobox.pdf");
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);

  const char kFilledMD5[] = "dad642ae8a5afce2591ffbcabbfc58dd";
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }
}

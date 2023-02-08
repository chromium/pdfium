// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fwl/cfwl_edit.h"

#include <memory>

#include "core/fxcrt/widestring.h"
#include "core/fxge/cfx_defaultrenderdevice.h"
#include "public/fpdf_ext.h"
#include "public/fpdf_formfill.h"
#include "public/fpdf_fwlevent.h"
#include "testing/embedder_test.h"
#include "testing/embedder_test_environment.h"
#include "testing/embedder_test_timer_handling_delegate.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/xfa_js_embedder_test.h"

namespace {

const char kEmailRecommendedFilledChecksum[] =
    "211e4e46eb347aa2bc7c425556d600b0";

}  // namespace

class CFWLEditEmbedderTest : public XFAJSEmbedderTest {
 protected:
  void SetUp() override {
    EmbedderTest::SetUp();
    SetDelegate(&delegate_);

    // Arbitrary, picked nice even number, 2020-09-13 12:26:40.
    FSDK_SetTimeFunction([]() -> time_t { return 1600000000; });
    FSDK_SetLocaltimeFunction([](const time_t* t) { return gmtime(t); });
  }

  void TearDown() override {
    FSDK_SetTimeFunction(nullptr);
    FSDK_SetLocaltimeFunction(nullptr);
    // TODO(crbug.com/pdfium/11): A page might not have been loaded if a test
    // is skipped at runtime. This check for a non-null page should be able to
    // removed once none of the tests are being skipped for Skia.
    if (page())
      UnloadPage(page());
    EmbedderTest::TearDown();
  }

  void CreateAndInitializeFormPDF(const char* filename) {
    ASSERT_TRUE(OpenDocument(filename));
    page_ = LoadPage(0);
    ASSERT_TRUE(page_);
  }

  FPDF_PAGE page() const { return page_; }
  EmbedderTestTimerHandlingDelegate delegate() const { return delegate_; }

 private:
  FPDF_PAGE page_ = nullptr;
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

TEST_F(CFWLEditEmbedderTest, DragMouseSelection) {
  // TODO(crbug.com/pdfium/11): Fix this test and enable for Skia variants.
  if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
    return;

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

TEST_F(CFWLEditEmbedderTest, SimpleFill) {
  // TODO(crbug.com/pdfium/11): Fix this test and enable for Skia variants.
  if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
    return;

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

  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kEmailRecommendedFilledChecksum);
  }
}

TEST_F(CFWLEditEmbedderTest, FillWithNewLineWithoutMultiline) {
  // TODO(crbug.com/pdfium/11): Fix this test and enable for Skia variants.
  if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
    return;

  CreateAndInitializeFormPDF("xfa/email_recommended.pdf");
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);
  for (size_t i = 0; i < 5; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);
  FORM_OnChar(form_handle(), page(), '\r', 0);
  for (size_t i = 5; i < 10; ++i)
    FORM_OnChar(form_handle(), page(), 'a' + i, 0);

  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kEmailRecommendedFilledChecksum);
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
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
    const char kFilledMultilineMD5[] = "fc1f4d5fdb2c5755005fc525b0a60ec9";
#else
    const char kFilledMultilineMD5[] = "a5654e027d8b1667c20f3b86d1918003";
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
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
#if BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
    const char kMultilineBackspaceMD5[] = "8bb62a8100ff1e1cc113d4033e0d824e";
#else
    const char kMultilineBackspaceMD5[] = "a2f1dcab92bb1fb7c2f9ccc70100c989";
#endif  // BUILDFLAG(IS_LINUX) || BUILDFLAG(IS_CHROMEOS)
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kMultilineBackspaceMD5);
  }
}

TEST_F(CFWLEditEmbedderTest, DateTimePickerTest) {
  // TODO(crbug.com/pdfium/11): Fix this test and enable for Skia variants.
  if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
    return;

  CreateAndInitializeFormPDF("xfa/xfa_date_time_edit.pdf");

  // Give focus to date time widget, creating down-arrow button.
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);
  FORM_OnLButtonUp(form_handle(), page(), 0, 115, 58);
  const char kSelectedMD5[] = "1036b8837a9dba75c6bd8f9347ae2eb2";
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, kSelectedMD5);
  }

  // Click down-arrow button, bringing up calendar widget.
  FORM_OnLButtonDown(form_handle(), page(), 0, 446, 54);
  FORM_OnLButtonUp(form_handle(), page(), 0, 446, 54);
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);

    // TODO(tsepez): hermetic fonts.
    // const char kCalendarOpenMD5[] = "02de64e7e83c82c1ef0ae484d671a51d";
    // CompareBitmap(page_bitmap.get(), 612, 792, kCalendarOpenMD5);
  }

  // Click on date on calendar, putting result into field as text.
  FORM_OnLButtonDown(form_handle(), page(), 0, 100, 162);
  FORM_OnLButtonUp(form_handle(), page(), 0, 100, 162);
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);

    // TODO(tsepez): hermetic fonts.
    // const char kFilledMD5[] = "1bce66c11f1c87b8d639ce0076ac36d3";
    // CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }
}

TEST_F(CFWLEditEmbedderTest, ImageEditTest) {
  CreateAndInitializeFormPDF("xfa/xfa_image_edit.pdf");
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);
  const char* filled_checksum = []() {
    if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
      return "23658ed124114f05518372d41c80e41b";
    return "101cf6223fa2403fba4c413a8310ab02";
  }();
  ScopedFPDFBitmap page_bitmap = RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
  CompareBitmap(page_bitmap.get(), 612, 792, filled_checksum);
}

TEST_F(CFWLEditEmbedderTest, ComboBoxTest) {
  CreateAndInitializeFormPDF("xfa/xfa_combobox.pdf");

  // Give focus to widget.
  FORM_OnLButtonDown(form_handle(), page(), 0, 115, 58);
  FORM_OnLButtonUp(form_handle(), page(), 0, 115, 58);
  {
    const char* filled_checksum = []() {
      if (CFX_DefaultRenderDevice::SkiaIsDefaultRenderer())
        return "8c555487e09ee4acf3ace77db5929bdc";
      return "dad642ae8a5afce2591ffbcabbfc58dd";
    }();
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    CompareBitmap(page_bitmap.get(), 612, 792, filled_checksum);
  }

  // Click on down-arrow button, dropdown list appears.
  FORM_OnLButtonDown(form_handle(), page(), 0, 438, 53);
  FORM_OnLButtonUp(form_handle(), page(), 0, 438, 53);
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    // TODO(tsepez): hermetic fonts.
    // const char kFilledMD5[] = "dad642ae8a5afce2591ffbcabbfc58dd";
    // CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }

  // Enter drop-down list, selection highlighted.
  FORM_OnMouseMove(form_handle(), page(), 0, 253, 107);
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    // TODO(tsepez): hermetic fonts.
    // const char kFilledMD5[] = "dad642ae8a5afce2591ffbcabbfc58dd";
    // CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }

  // Click on selection, putting result into field.
  FORM_OnLButtonDown(form_handle(), page(), 0, 253, 107);
  FORM_OnLButtonUp(form_handle(), page(), 0, 253, 107);
  {
    ScopedFPDFBitmap page_bitmap =
        RenderLoadedPageWithFlags(page(), FPDF_ANNOT);
    // TODO(tsepez): hermetic fonts.
    // const char kFilledMD5[] = "dad642ae8a5afce2591ffbcabbfc58dd";
    // CompareBitmap(page_bitmap.get(), 612, 792, kFilledMD5);
  }
}

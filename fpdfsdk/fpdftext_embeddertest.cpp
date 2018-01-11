// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <memory>

#include "core/fxcrt/fx_memory.h"
#include "public/fpdf_text.h"
#include "public/fpdfview.h"
#include "testing/embedder_test.h"
#include "testing/gtest/include/gtest/gtest.h"
#include "testing/test_support.h"

namespace {

bool check_unsigned_shorts(const char* expected,
                           const unsigned short* actual,
                           size_t length) {
  if (length > strlen(expected) + 1)
    return false;

  for (size_t i = 0; i < length; ++i) {
    if (actual[i] != static_cast<unsigned short>(expected[i]))
      return false;
  }
  return true;
}

}  // namespace

class FPDFTextEmbeddertest : public EmbedderTest {};

TEST_F(FPDFTextEmbeddertest, Text) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  static const char expected[] = "Hello, world!\r\nGoodbye, world!";
  unsigned short fixed_buffer[128];
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));

  // Check that edge cases are handled gracefully
  EXPECT_EQ(0, FPDFText_GetText(textpage, 0, 128, nullptr));
  EXPECT_EQ(0, FPDFText_GetText(textpage, -1, 128, fixed_buffer));
  EXPECT_EQ(0, FPDFText_GetText(textpage, 0, -1, fixed_buffer));
  EXPECT_EQ(1, FPDFText_GetText(textpage, 0, 0, fixed_buffer));
  EXPECT_EQ(0, fixed_buffer[0]);

  // Keep going and check the next case.
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(2, FPDFText_GetText(textpage, 0, 1, fixed_buffer));
  EXPECT_EQ(expected[0], fixed_buffer[0]);
  EXPECT_EQ(0, fixed_buffer[1]);

  // Check includes the terminating NUL that is provided.
  int num_chars = FPDFText_GetText(textpage, 0, 128, fixed_buffer);
  ASSERT_GE(num_chars, 0);
  EXPECT_EQ(sizeof(expected), static_cast<size_t>(num_chars));
  EXPECT_TRUE(check_unsigned_shorts(expected, fixed_buffer, sizeof(expected)));

  // Count does not include the terminating NUL in the string literal.
  EXPECT_EQ(sizeof(expected) - 1,
            static_cast<size_t>(FPDFText_CountChars(textpage)));
  for (size_t i = 0; i < sizeof(expected) - 1; ++i) {
    EXPECT_EQ(static_cast<unsigned int>(expected[i]),
              FPDFText_GetUnicode(textpage, i))
        << " at " << i;
  }

  // Extracting using a buffer that will be completely filled. Small buffer is
  // 12 elements long, since it will need 2 locations per displayed character in
  // the expected string, plus 2 more for the terminating character.
  static const char small_expected[] = "Hello";
  unsigned short small_buffer[12];
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(6, FPDFText_GetText(textpage, 0, 5, small_buffer));
  EXPECT_TRUE(check_unsigned_shorts(small_expected, small_buffer,
                                    sizeof(small_expected)));

  EXPECT_EQ(12.0, FPDFText_GetFontSize(textpage, 0));
  EXPECT_EQ(16.0, FPDFText_GetFontSize(textpage, 15));

  double left = 0.0;
  double right = 0.0;
  double bottom = 0.0;
  double top = 0.0;
  EXPECT_FALSE(FPDFText_GetCharBox(nullptr, 4, &left, &right, &bottom, &top));
  EXPECT_DOUBLE_EQ(0.0, left);
  EXPECT_DOUBLE_EQ(0.0, right);
  EXPECT_DOUBLE_EQ(0.0, bottom);
  EXPECT_DOUBLE_EQ(0.0, top);
  EXPECT_FALSE(FPDFText_GetCharBox(textpage, -1, &left, &right, &bottom, &top));
  EXPECT_DOUBLE_EQ(0.0, left);
  EXPECT_DOUBLE_EQ(0.0, right);
  EXPECT_DOUBLE_EQ(0.0, bottom);
  EXPECT_DOUBLE_EQ(0.0, top);
  EXPECT_FALSE(FPDFText_GetCharBox(textpage, 55, &left, &right, &bottom, &top));
  EXPECT_DOUBLE_EQ(0.0, left);
  EXPECT_DOUBLE_EQ(0.0, right);
  EXPECT_DOUBLE_EQ(0.0, bottom);
  EXPECT_DOUBLE_EQ(0.0, top);

  EXPECT_TRUE(FPDFText_GetCharBox(textpage, 4, &left, &right, &bottom, &top));
  EXPECT_NEAR(41.071, left, 0.001);
  EXPECT_NEAR(46.243, right, 0.001);
  EXPECT_NEAR(49.844, bottom, 0.001);
  EXPECT_NEAR(55.520, top, 0.001);

  double x = 0.0;
  double y = 0.0;
  EXPECT_TRUE(FPDFText_GetCharOrigin(textpage, 4, &x, &y));
  EXPECT_NEAR(40.664, x, 0.001);
  EXPECT_NEAR(50.000, y, 0.001);

  EXPECT_EQ(4, FPDFText_GetCharIndexAtPos(textpage, 42.0, 50.0, 1.0, 1.0));
  EXPECT_EQ(-1, FPDFText_GetCharIndexAtPos(textpage, 0.0, 0.0, 1.0, 1.0));
  EXPECT_EQ(-1, FPDFText_GetCharIndexAtPos(textpage, 199.0, 199.0, 1.0, 1.0));

  // Test out of range indicies.
  EXPECT_EQ(-1,
            FPDFText_GetCharIndexAtPos(textpage, 42.0, 10000000.0, 1.0, 1.0));
  EXPECT_EQ(-1, FPDFText_GetCharIndexAtPos(textpage, -1.0, 50.0, 1.0, 1.0));

  // Count does not include the terminating NUL in the string literal.
  EXPECT_EQ(2, FPDFText_CountRects(textpage, 0, sizeof(expected) - 1));

  left = 0.0;
  right = 0.0;
  bottom = 0.0;
  top = 0.0;
  EXPECT_TRUE(FPDFText_GetRect(textpage, 1, &left, &top, &right, &bottom));
  EXPECT_NEAR(20.847, left, 0.001);
  EXPECT_NEAR(135.167, right, 0.001);
  EXPECT_NEAR(96.655, bottom, 0.001);
  EXPECT_NEAR(116.000, top, 0.001);

  // Test out of range indicies set outputs to (0.0, 0.0, 0.0, 0.0).
  left = -1.0;
  right = -1.0;
  bottom = -1.0;
  top = -1.0;
  EXPECT_FALSE(FPDFText_GetRect(textpage, -1, &left, &top, &right, &bottom));
  EXPECT_EQ(0.0, left);
  EXPECT_EQ(0.0, right);
  EXPECT_EQ(0.0, bottom);
  EXPECT_EQ(0.0, top);

  left = -2.0;
  right = -2.0;
  bottom = -2.0;
  top = -2.0;
  EXPECT_FALSE(FPDFText_GetRect(textpage, 2, &left, &top, &right, &bottom));
  EXPECT_EQ(0.0, left);
  EXPECT_EQ(0.0, right);
  EXPECT_EQ(0.0, bottom);
  EXPECT_EQ(0.0, top);

  EXPECT_EQ(9, FPDFText_GetBoundedText(textpage, 41.0, 56.0, 82.0, 48.0, 0, 0));

  // Extract starting at character 4 as above.
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(1, FPDFText_GetBoundedText(textpage, 41.0, 56.0, 82.0, 48.0,
                                       fixed_buffer, 1));
  EXPECT_TRUE(check_unsigned_shorts(expected + 4, fixed_buffer, 1));
  EXPECT_EQ(0xbdbd, fixed_buffer[1]);

  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(9, FPDFText_GetBoundedText(textpage, 41.0, 56.0, 82.0, 48.0,
                                       fixed_buffer, 9));
  EXPECT_TRUE(check_unsigned_shorts(expected + 4, fixed_buffer, 9));
  EXPECT_EQ(0xbdbd, fixed_buffer[9]);

  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(10, FPDFText_GetBoundedText(textpage, 41.0, 56.0, 82.0, 48.0,
                                        fixed_buffer, 128));
  EXPECT_TRUE(check_unsigned_shorts(expected + 4, fixed_buffer, 9));
  EXPECT_EQ(0u, fixed_buffer[9]);
  EXPECT_EQ(0xbdbd, fixed_buffer[10]);

  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, TextSearch) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  std::unique_ptr<unsigned short, pdfium::FreeDeleter> nope =
      GetFPDFWideString(L"nope");
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> world =
      GetFPDFWideString(L"world");
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> world_caps =
      GetFPDFWideString(L"WORLD");
  std::unique_ptr<unsigned short, pdfium::FreeDeleter> world_substr =
      GetFPDFWideString(L"orld");

  // No occurences of "nope" in test page.
  FPDF_SCHHANDLE search = FPDFText_FindStart(textpage, nope.get(), 0, 0);
  EXPECT_TRUE(search);
  EXPECT_EQ(0, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(0, FPDFText_GetSchCount(search));

  // Advancing finds nothing.
  EXPECT_FALSE(FPDFText_FindNext(search));
  EXPECT_EQ(0, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(0, FPDFText_GetSchCount(search));

  // Retreating finds nothing.
  EXPECT_FALSE(FPDFText_FindPrev(search));
  EXPECT_EQ(0, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(0, FPDFText_GetSchCount(search));
  FPDFText_FindClose(search);

  // Two occurences of "world" in test page.
  search = FPDFText_FindStart(textpage, world.get(), 0, 2);
  EXPECT_TRUE(search);

  // Remains not found until advanced.
  EXPECT_EQ(0, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(0, FPDFText_GetSchCount(search));

  // First occurence of "world" in this test page.
  EXPECT_TRUE(FPDFText_FindNext(search));
  EXPECT_EQ(7, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(5, FPDFText_GetSchCount(search));

  // Last occurence of "world" in this test page.
  EXPECT_TRUE(FPDFText_FindNext(search));
  EXPECT_EQ(24, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(5, FPDFText_GetSchCount(search));

  // Found position unchanged when fails to advance.
  EXPECT_FALSE(FPDFText_FindNext(search));
  EXPECT_EQ(24, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(5, FPDFText_GetSchCount(search));

  // Back to first occurence.
  EXPECT_TRUE(FPDFText_FindPrev(search));
  EXPECT_EQ(7, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(5, FPDFText_GetSchCount(search));

  // Found position unchanged when fails to retreat.
  EXPECT_FALSE(FPDFText_FindPrev(search));
  EXPECT_EQ(7, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(5, FPDFText_GetSchCount(search));
  FPDFText_FindClose(search);

  // Exact search unaffected by case sensitiity and whole word flags.
  search = FPDFText_FindStart(textpage, world.get(),
                              FPDF_MATCHCASE | FPDF_MATCHWHOLEWORD, 0);
  EXPECT_TRUE(search);
  EXPECT_TRUE(FPDFText_FindNext(search));
  EXPECT_EQ(7, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(5, FPDFText_GetSchCount(search));
  FPDFText_FindClose(search);

  // Default is case-insensitive, so matching agaist caps works.
  search = FPDFText_FindStart(textpage, world_caps.get(), 0, 0);
  EXPECT_TRUE(search);
  EXPECT_TRUE(FPDFText_FindNext(search));
  EXPECT_EQ(7, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(5, FPDFText_GetSchCount(search));
  FPDFText_FindClose(search);

  // But can be made case sensitive, in which case this fails.
  search = FPDFText_FindStart(textpage, world_caps.get(), FPDF_MATCHCASE, 0);
  EXPECT_FALSE(FPDFText_FindNext(search));
  EXPECT_EQ(0, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(0, FPDFText_GetSchCount(search));
  FPDFText_FindClose(search);

  // Default is match anywhere within word, so matching substirng works.
  search = FPDFText_FindStart(textpage, world_substr.get(), 0, 0);
  EXPECT_TRUE(FPDFText_FindNext(search));
  EXPECT_EQ(8, FPDFText_GetSchResultIndex(search));
  EXPECT_EQ(4, FPDFText_GetSchCount(search));
  FPDFText_FindClose(search);

  // But can be made to mach word boundaries, in which case this fails.
  search =
      FPDFText_FindStart(textpage, world_substr.get(), FPDF_MATCHWHOLEWORD, 0);
  EXPECT_FALSE(FPDFText_FindNext(search));
  // TODO(tsepez): investigate strange index/count values in this state.
  FPDFText_FindClose(search);

  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

// Test that the page has characters despite a bad stream length.
TEST_F(FPDFTextEmbeddertest, StreamLengthPastEndOfFile) {
  EXPECT_TRUE(OpenDocument("bug_57.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);
  EXPECT_EQ(13, FPDFText_CountChars(textpage));

  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, WebLinks) {
  EXPECT_TRUE(OpenDocument("weblinks.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  FPDF_PAGELINK pagelink = FPDFLink_LoadWebLinks(textpage);
  EXPECT_TRUE(pagelink);

  // Page contains two HTTP-style URLs.
  EXPECT_EQ(2, FPDFLink_CountWebLinks(pagelink));

  // Only a terminating NUL required for bogus links.
  EXPECT_EQ(1, FPDFLink_GetURL(pagelink, 2, nullptr, 0));
  EXPECT_EQ(1, FPDFLink_GetURL(pagelink, 1400, nullptr, 0));
  EXPECT_EQ(1, FPDFLink_GetURL(pagelink, -1, nullptr, 0));

  // Query the number of characters required for each link (incl NUL).
  EXPECT_EQ(25, FPDFLink_GetURL(pagelink, 0, nullptr, 0));
  EXPECT_EQ(26, FPDFLink_GetURL(pagelink, 1, nullptr, 0));

  static const char expected_url[] = "http://example.com?q=foo";
  static const size_t expected_len = sizeof(expected_url);
  unsigned short fixed_buffer[128];

  // Retrieve a link with too small a buffer.  Buffer will not be
  // NUL-terminated, but must not be modified past indicated length,
  // so pre-fill with a pattern to check write bounds.
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(1, FPDFLink_GetURL(pagelink, 0, fixed_buffer, 1));
  EXPECT_TRUE(check_unsigned_shorts(expected_url, fixed_buffer, 1));
  EXPECT_EQ(0xbdbd, fixed_buffer[1]);

  // Check buffer that doesn't have space for a terminating NUL.
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(static_cast<int>(expected_len - 1),
            FPDFLink_GetURL(pagelink, 0, fixed_buffer, expected_len - 1));
  EXPECT_TRUE(
      check_unsigned_shorts(expected_url, fixed_buffer, expected_len - 1));
  EXPECT_EQ(0xbdbd, fixed_buffer[expected_len - 1]);

  // Retreive link with exactly-sized buffer.
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(static_cast<int>(expected_len),
            FPDFLink_GetURL(pagelink, 0, fixed_buffer, expected_len));
  EXPECT_TRUE(check_unsigned_shorts(expected_url, fixed_buffer, expected_len));
  EXPECT_EQ(0u, fixed_buffer[expected_len - 1]);
  EXPECT_EQ(0xbdbd, fixed_buffer[expected_len]);

  // Retreive link with ample-sized-buffer.
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  EXPECT_EQ(static_cast<int>(expected_len),
            FPDFLink_GetURL(pagelink, 0, fixed_buffer, 128));
  EXPECT_TRUE(check_unsigned_shorts(expected_url, fixed_buffer, expected_len));
  EXPECT_EQ(0u, fixed_buffer[expected_len - 1]);
  EXPECT_EQ(0xbdbd, fixed_buffer[expected_len]);

  // Each link rendered in a single rect in this test page.
  EXPECT_EQ(1, FPDFLink_CountRects(pagelink, 0));
  EXPECT_EQ(1, FPDFLink_CountRects(pagelink, 1));

  // Each link rendered in a single rect in this test page.
  EXPECT_EQ(0, FPDFLink_CountRects(pagelink, -1));
  EXPECT_EQ(0, FPDFLink_CountRects(pagelink, 2));
  EXPECT_EQ(0, FPDFLink_CountRects(pagelink, 10000));

  // Check boundary of valid link index with valid rect index.
  double left = 0.0;
  double right = 0.0;
  double top = 0.0;
  double bottom = 0.0;
  EXPECT_TRUE(FPDFLink_GetRect(pagelink, 0, 0, &left, &top, &right, &bottom));
  EXPECT_NEAR(50.791, left, 0.001);
  EXPECT_NEAR(187.963, right, 0.001);
  EXPECT_NEAR(97.624, bottom, 0.001);
  EXPECT_NEAR(108.736, top, 0.001);

  // Check that valid link with invalid rect index leaves parameters unchanged.
  left = -1.0;
  right = -1.0;
  top = -1.0;
  bottom = -1.0;
  EXPECT_FALSE(FPDFLink_GetRect(pagelink, 0, 1, &left, &top, &right, &bottom));
  EXPECT_EQ(-1.0, left);
  EXPECT_EQ(-1.0, right);
  EXPECT_EQ(-1.0, bottom);
  EXPECT_EQ(-1.0, top);

  // Check that invalid link index leaves parameters unchanged.
  left = -2.0;
  right = -2.0;
  top = -2.0;
  bottom = -2.0;
  EXPECT_FALSE(FPDFLink_GetRect(pagelink, -1, 0, &left, &top, &right, &bottom));
  EXPECT_EQ(-2.0, left);
  EXPECT_EQ(-2.0, right);
  EXPECT_EQ(-2.0, bottom);
  EXPECT_EQ(-2.0, top);

  FPDFLink_CloseWebLinks(pagelink);
  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, WebLinksAcrossLines) {
  EXPECT_TRUE(OpenDocument("weblinks_across_lines.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  FPDF_PAGELINK pagelink = FPDFLink_LoadWebLinks(textpage);
  EXPECT_TRUE(pagelink);

  static const char* const kExpectedUrls[] = {
      "http://example.com",           // from "http://www.example.com?\r\nfoo"
      "http://example.com/",          // from "http://www.example.com/\r\nfoo"
      "http://example.com/test-foo",  // from "http://example.com/test-\r\nfoo"
      "http://abc.com/test-foo",      // from "http://abc.com/test-\r\n\r\nfoo"
      // Next two links from "http://www.example.com/\r\nhttp://www.abc.com/"
      "http://example.com/", "http://www.abc.com",
  };
  static const int kNumLinks = static_cast<int>(FX_ArraySize(kExpectedUrls));

  EXPECT_EQ(kNumLinks, FPDFLink_CountWebLinks(pagelink));

  unsigned short fixed_buffer[128];
  for (int i = 0; i < kNumLinks; i++) {
    const size_t expected_len = strlen(kExpectedUrls[i]) + 1;
    memset(fixed_buffer, 0, FX_ArraySize(fixed_buffer));
    EXPECT_EQ(static_cast<int>(expected_len),
              FPDFLink_GetURL(pagelink, i, nullptr, 0));
    EXPECT_EQ(
        static_cast<int>(expected_len),
        FPDFLink_GetURL(pagelink, i, fixed_buffer, FX_ArraySize(fixed_buffer)));
    EXPECT_TRUE(
        check_unsigned_shorts(kExpectedUrls[i], fixed_buffer, expected_len));
  }

  FPDFLink_CloseWebLinks(pagelink);
  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, WebLinksAcrossLinesBug) {
  EXPECT_TRUE(OpenDocument("bug_650.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  FPDF_PAGELINK pagelink = FPDFLink_LoadWebLinks(textpage);
  EXPECT_TRUE(pagelink);

  EXPECT_EQ(2, FPDFLink_CountWebLinks(pagelink));
  unsigned short fixed_buffer[128] = {0};
  static const char kExpectedUrl[] =
      "http://tutorial45.com/learn-autocad-basics-day-166/";
  static const int kUrlSize = static_cast<int>(sizeof(kExpectedUrl));

  EXPECT_EQ(kUrlSize, FPDFLink_GetURL(pagelink, 1, nullptr, 0));
  EXPECT_EQ(kUrlSize, FPDFLink_GetURL(pagelink, 1, fixed_buffer,
                                      FX_ArraySize(fixed_buffer)));
  EXPECT_TRUE(check_unsigned_shorts(kExpectedUrl, fixed_buffer, kUrlSize));

  FPDFLink_CloseWebLinks(pagelink);
  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, GetFontSize) {
  EXPECT_TRUE(OpenDocument("hello_world.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  const double kExpectedFontsSizes[] = {12, 12, 12, 12, 12, 12, 12, 12, 12, 12,
                                        12, 12, 12, 1,  1,  16, 16, 16, 16, 16,
                                        16, 16, 16, 16, 16, 16, 16, 16, 16, 16};

  int count = FPDFText_CountChars(textpage);
  ASSERT_EQ(FX_ArraySize(kExpectedFontsSizes), static_cast<size_t>(count));
  for (int i = 0; i < count; ++i)
    EXPECT_EQ(kExpectedFontsSizes[i], FPDFText_GetFontSize(textpage, i)) << i;

  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, ToUnicode) {
  EXPECT_TRUE(OpenDocument("bug_583.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  ASSERT_EQ(1, FPDFText_CountChars(textpage));
  EXPECT_EQ(static_cast<unsigned int>(0), FPDFText_GetUnicode(textpage, 0));

  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, Bug_921) {
  EXPECT_TRUE(OpenDocument("bug_921.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  static constexpr unsigned int kData[] = {
      1095, 1077, 1083, 1086, 1074, 1077, 1095, 1077, 1089, 1082, 1086, 1077,
      32,   1089, 1090, 1088, 1072, 1076, 1072, 1085, 1080, 1077, 46,   32};
  static constexpr int kStartIndex = 238;

  ASSERT_EQ(268, FPDFText_CountChars(textpage));
  for (size_t i = 0; i < FX_ArraySize(kData); ++i)
    EXPECT_EQ(kData[i], FPDFText_GetUnicode(textpage, kStartIndex + i));

  unsigned short buffer[FX_ArraySize(kData) + 1];
  memset(buffer, 0xbd, sizeof(buffer));
  int count =
      FPDFText_GetText(textpage, kStartIndex, FX_ArraySize(kData), buffer);
  ASSERT_GT(count, 0);
  ASSERT_EQ(FX_ArraySize(kData) + 1, static_cast<size_t>(count));
  for (size_t i = 0; i < FX_ArraySize(kData); ++i)
    EXPECT_EQ(kData[i], buffer[i]);
  EXPECT_EQ(0, buffer[FX_ArraySize(kData)]);

  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, GetTextWithHyphen) {
  EXPECT_TRUE(OpenDocument("bug_781804.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  // Check that soft hyphens are not included
  // Expecting 'Veritaserum', except there is a \uFFFE where the hyphen was in
  // the original text. This is a weird thing that Adobe does, which we
  // replicate.
  constexpr unsigned short soft_expected[] = {
      0x0056, 0x0065, 0x0072, 0x0069, 0x0074, 0x0061, 0xfffe,
      0x0073, 0x0065, 0x0072, 0x0075, 0x006D, 0x0000};
  {
    constexpr int count = FX_ArraySize(soft_expected) - 1;
    unsigned short buffer[FX_ArraySize(soft_expected)];
    memset(buffer, 0, sizeof(buffer));

    EXPECT_EQ(count + 1, FPDFText_GetText(textpage, 0, count, buffer));
    for (int i = 0; i < count; i++)
      EXPECT_EQ(soft_expected[i], buffer[i]);
  }

  // Check that hard hyphens are included
  {
    // There isn't the \0 in the actual doc, but there is a \r\n, so need to
    // add 1 to get aligned.
    constexpr size_t offset = FX_ArraySize(soft_expected) + 1;
    // Expecting 'User-\r\ngenerated', the - is a unicode character, so cannnot
    // store in a char[].
    constexpr unsigned short hard_expected[] = {
        0x0055, 0x0073, 0x0065, 0x0072, 0x2010, 0x000d, 0x000a, 0x0067, 0x0065,
        0x006e, 0x0065, 0x0072, 0x0061, 0x0074, 0x0065, 0x0064, 0x0000};
    constexpr int count = FX_ArraySize(hard_expected) - 1;
    unsigned short buffer[FX_ArraySize(hard_expected)];

    EXPECT_EQ(count + 1, FPDFText_GetText(textpage, offset, count, buffer));
    for (int i = 0; i < count; i++)
      EXPECT_EQ(hard_expected[i], buffer[i]);
  }

  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, bug_782596) {
  // If there is a regression in this test, it will only fail under ASAN
  EXPECT_TRUE(OpenDocument("bug_782596.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);
  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);
  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

TEST_F(FPDFTextEmbeddertest, ControlCharacters) {
  EXPECT_TRUE(OpenDocument("control_characters.pdf"));
  FPDF_PAGE page = LoadPage(0);
  EXPECT_TRUE(page);

  FPDF_TEXTPAGE textpage = FPDFText_LoadPage(page);
  EXPECT_TRUE(textpage);

  // Should not include the control characters in the output
  static const char expected[] = "Hello, world!\r\nGoodbye, world!";
  unsigned short fixed_buffer[128];
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  int num_chars = FPDFText_GetText(textpage, 0, 128, fixed_buffer);

  ASSERT_GE(num_chars, 0);
  EXPECT_EQ(sizeof(expected), static_cast<size_t>(num_chars));
  EXPECT_TRUE(check_unsigned_shorts(expected, fixed_buffer, sizeof(expected)));

  // Attempting to get a chunk of text after the control characters
  static const char expected_substring[] = "Goodbye, world!";
  // Offset is the length of 'Hello, world!\r\n' + 2 control characters in the
  // original stream
  static const int offset = 17;
  memset(fixed_buffer, 0xbd, sizeof(fixed_buffer));
  num_chars = FPDFText_GetText(textpage, offset, 128, fixed_buffer);

  ASSERT_GE(num_chars, 0);
  EXPECT_EQ(sizeof(expected_substring), static_cast<size_t>(num_chars));
  EXPECT_TRUE(check_unsigned_shorts(expected_substring, fixed_buffer,
                                    sizeof(expected_substring)));

  FPDFText_ClosePage(textpage);
  UnloadPage(page);
}

// Copyright 2015 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdftext/cpdf_linkextract.h"

#include <utility>

#include "testing/gtest/include/gtest/gtest.h"

// Class to help test functions in CPDF_LinkExtract class.
class CPDF_TestLinkExtract final : public CPDF_LinkExtract {
 public:
  CPDF_TestLinkExtract() : CPDF_LinkExtract(nullptr) {}

 private:
  // Add test cases as friends to access protected member functions.
  // Access CheckMailLink and CheckWebLink.
  FRIEND_TEST(CPDFLinkExtractTest, CheckMailLink);
  FRIEND_TEST(CPDFLinkExtractTest, CheckWebLink);
};

TEST(CPDFLinkExtractTest, CheckMailLink) {
  CPDF_TestLinkExtract extractor;
  // Check cases that fail to extract valid mail link.
  const wchar_t* const kInvalidStrings[] = {
      L"",
      L"peter.pan",       // '@' is required.
      L"abc@server",      // Domain name needs at least one '.'.
      L"abc.@gmail.com",  // '.' can not immediately precede '@'.
      L"abc@xyz&q.org",   // Domain name should not contain '&'.
      L"abc@.xyz.org",    // Domain name should not start with '.'.
      L"fan@g..com"       // Domain name should not have consecutive '.'
  };
  for (const wchar_t* input : kInvalidStrings) {
    WideString text_str(input);
    EXPECT_FALSE(extractor.CheckMailLink(&text_str)) << input;
  }

  // A struct of {input_string, expected_extracted_email_address}.
  struct IOPair {
    const wchar_t* input;
    const wchar_t* expected_output;
  };
  // Check cases that can extract valid mail link.
  constexpr IOPair kValidStrings[] = {
      {L"peter@abc.d", L"peter@abc.d"},
      {L"red.teddy.b@abc.com", L"red.teddy.b@abc.com"},
      {L"abc_@gmail.com", L"abc_@gmail.com"},  // '_' is ok before '@'.
      {L"dummy-hi@gmail.com",
       L"dummy-hi@gmail.com"},                  // '-' is ok in user name.
      {L"a..df@gmail.com", L"df@gmail.com"},    // Stop at consecutive '.'.
      {L".john@yahoo.com", L"john@yahoo.com"},  // Remove heading '.'.
      {L"abc@xyz.org?/", L"abc@xyz.org"},       // Trim ending invalid chars.
      {L"fan{abc@xyz.org", L"abc@xyz.org"},     // Trim beginning invalid chars.
      {L"fan@g.com..", L"fan@g.com"},           // Trim the ending periods.
      {L"CAP.cap@Gmail.Com", L"CAP.cap@Gmail.Com"},  // Keep the original case.
  };
  for (const auto& it : kValidStrings) {
    WideString text_str(it.input);
    WideString expected_str(L"mailto:");
    expected_str += it.expected_output;
    EXPECT_TRUE(extractor.CheckMailLink(&text_str)) << it.input;
    EXPECT_EQ(expected_str.c_str(), text_str);
  }
}

TEST(CPDFLinkExtractTest, CheckWebLink) {
  CPDF_TestLinkExtract extractor;
  // Check cases that fail to extract valid web link.
  // The last few are legit web addresses that we don't handle now.
  const wchar_t* const kInvalidCases[] = {
      L"",                          // Blank.
      L"http",                      // No colon.
      L"www.",                      // Missing domain.
      L"https-and-www",             // Dash not colon.
      L"http:/abc.com",             // Missing slash.
      L"http://((()),",             // Only invalid chars in host name.
      L"ftp://example.com",         // Ftp scheme is not supported.
      L"http:example.com",          // Missing slashes.
      L"http//[example.com",        // Invalid IPv6 address.
      L"http//[00:00:00:00:00:00",  // Invalid IPv6 address.
      L"http//[]",                  // Empty IPv6 address.
      L"abc.example.com",           // URL without scheme.
  };
  for (const wchar_t* input : kInvalidCases) {
    auto maybe_link = extractor.CheckWebLink(input);
    EXPECT_FALSE(maybe_link.has_value()) << input;
  }

  // Check cases that can extract valid web link.
  // An array of {input_string, expected_extracted_web_link}.
  struct ValidCase {
    const wchar_t* const input_string;
    const wchar_t* const url_extracted;
    const size_t start_offset;
    const size_t count;
  };
  const ValidCase kValidCases[] = {
      {L"http://www.example.com", L"http://www.example.com", 0,
       22},  // standard URL.
      {L"http://www.example.com:88", L"http://www.example.com:88", 0,
       25},  // URL with port number.
      {L"http://test@www.example.com", L"http://test@www.example.com", 0,
       27},  // URL with username.
      {L"http://test:test@example.com", L"http://test:test@example.com", 0,
       28},  // URL with username and password.
      {L"http://example", L"http://example", 0,
       14},  // URL with short domain name.
      {L"http////www.server", L"http://www.server", 8,
       10},  // URL starts with "www.".
      {L"http:/www.abc.com", L"http://www.abc.com", 6,
       11},                                       // URL starts with "www.".
      {L"www.a.b.c", L"http://www.a.b.c", 0, 9},  // URL starts with "www.".
      {L"https://a.us", L"https://a.us", 0, 12},  // Secure http URL.
      {L"https://www.t.us", L"https://www.t.us", 0, 16},  // Secure http URL.
      {L"www.example-test.com", L"http://www.example-test.com", 0,
       20},  // '-' in host is ok.
      {L"www.example.com,", L"http://www.example.com", 0,
       15},  // Trim ending invalid chars.
      {L"www.example.com;(", L"http://www.example.com", 0,
       15},  // Trim ending invalid chars.
      {L"test:www.abc.com", L"http://www.abc.com", 5,
       11},  // Trim chars before URL.
      {L"(http://www.abc.com)", L"http://www.abc.com", 1,
       18},  // Trim external brackets.
      {L"0(http://www.abc.com)0", L"http://www.abc.com", 2,
       18},  // Trim chars outside brackets as well.
      {L"0(www.abc.com)0", L"http://www.abc.com", 2,
       11},  // Links without http should also have brackets trimmed.
      {L"http://www.abc.com)0", L"http://www.abc.com)0", 0,
       20},  // Do not trim brackets that were not opened.
      {L"{(<http://www.abc.com>)}", L"http://www.abc.com", 3,
       18},  // Trim chars with multiple levels of brackets.
      {L"[http://www.abc.com/z(1)]", L"http://www.abc.com/z(1)", 1,
       23},  // Brackets opened inside the URL should not be trimmed.
      {L"(http://www.abc.com/z(1))", L"http://www.abc.com/z(1)", 1,
       23},  // Brackets opened inside the URL should not be trimmed.
      {L"\"http://www.abc.com\"", L"http://www.abc.com", 1,
       18},  // External quotes can also be escaped
      {L"www.g.com..", L"http://www.g.com..", 0, 11},  // Leave ending periods.

      // Web links can contain IP addresses too.
      {L"http://192.168.0.1", L"http://192.168.0.1", 0, 18},  // IPv4 address.
      {L"http://192.168.0.1:80", L"http://192.168.0.1:80", 0,
       21},  // IPv4 address with port.
      {L"http://[aa::00:bb::00:cc:00]", L"http://[aa::00:bb::00:cc:00]", 0,
       28},  // IPv6 reference.
      {L"http://[aa::00:bb::00:cc:00]:12", L"http://[aa::00:bb::00:cc:00]:12",
       0, 31},  // IPv6 reference with port.
      {L"http://[aa]:12", L"http://[aa]:12", 0,
       14},  // Not validate IP address.
      {L"http://[aa]:12abc", L"http://[aa]:12", 0,
       14},                                      // Trim for IPv6 address.
      {L"http://[aa]:", L"http://[aa]", 0, 11},  // Trim for IPv6 address.

      // Path and query parts can be anything.
      {L"www.abc.com/#%%^&&*(", L"http://www.abc.com/#%%^&&*(", 0, 20},
      {L"www.a.com/#a=@?q=rr&r=y", L"http://www.a.com/#a=@?q=rr&r=y", 0, 23},
      {L"http://a.com/1/2/3/4\5\6", L"http://a.com/1/2/3/4\5\6", 0, 22},
      {L"http://www.example.com/foo;bar", L"http://www.example.com/foo;bar", 0,
       30},

      // Invalid chars inside host name are ok as we don't validate them.
      {L"http://ex[am]ple", L"http://ex[am]ple", 0, 16},
      {L"http://:example.com", L"http://:example.com", 0, 19},
      {L"http://((())/path?", L"http://((())/path?", 0, 18},
      {L"http:////abc.server", L"http:////abc.server", 0, 19},

      // Non-ASCII chars are not validated either.
      {L"www.测试.net", L"http://www.测试.net", 0, 10},
      {L"www.测试。net。", L"http://www.测试。net。", 0, 11},
      {L"www.测试.net；", L"http://www.测试.net；", 0, 11},
  };
  for (const auto& it : kValidCases) {
    auto maybe_link = extractor.CheckWebLink(it.input_string);
    ASSERT_TRUE(maybe_link.has_value()) << it.input_string;
    EXPECT_EQ(it.url_extracted, maybe_link.value().m_strUrl);
    EXPECT_EQ(it.start_offset, maybe_link.value().m_Start) << it.input_string;
    EXPECT_EQ(it.count, maybe_link.value().m_Count) << it.input_string;
  }
}

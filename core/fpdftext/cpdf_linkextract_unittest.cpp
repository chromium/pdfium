// Copyright 2015 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdftext/cpdf_linkextract.h"

#include "testing/gtest/include/gtest/gtest.h"

// Class to help test functions in CPDF_LinkExtract class.
class CPDF_TestLinkExtract : public CPDF_LinkExtract {
 public:
  CPDF_TestLinkExtract() : CPDF_LinkExtract(nullptr) {}

 private:
  // Add test cases as friends to access protected member functions.
  // Access CheckMailLink and CheckWebLink.
  FRIEND_TEST(CPDF_LinkExtractTest, CheckMailLink);
  FRIEND_TEST(CPDF_LinkExtractTest, CheckWebLink);
};

TEST(CPDF_LinkExtractTest, CheckMailLink) {
  CPDF_TestLinkExtract extractor;
  // Check cases that fail to extract valid mail link.
  const wchar_t* const invalid_strs[] = {
      L"",
      L"peter.pan",       // '@' is required.
      L"abc@server",      // Domain name needs at least one '.'.
      L"abc.@gmail.com",  // '.' can not immediately precede '@'.
      L"abc@xyz&q.org",   // Domain name should not contain '&'.
      L"abc@.xyz.org",    // Domain name should not start with '.'.
      L"fan@g..com"       // Domain name should not have consecutive '.'
  };
  for (size_t i = 0; i < FX_ArraySize(invalid_strs); ++i) {
    CFX_WideString text_str(invalid_strs[i]);
    EXPECT_FALSE(extractor.CheckMailLink(text_str)) << text_str.c_str();
  }

  // Check cases that can extract valid mail link.
  // An array of {input_string, expected_extracted_email_address}.
  const wchar_t* const valid_strs[][2] = {
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
  for (size_t i = 0; i < FX_ArraySize(valid_strs); ++i) {
    CFX_WideString text_str(valid_strs[i][0]);
    CFX_WideString expected_str(L"mailto:");
    expected_str += valid_strs[i][1];
    EXPECT_TRUE(extractor.CheckMailLink(text_str)) << text_str.c_str();
    EXPECT_STREQ(expected_str.c_str(), text_str.c_str());
  }
}

TEST(CPDF_LinkExtractTest, CheckWebLink) {
  CPDF_TestLinkExtract extractor;
  // Check cases that fail to extract valid web link.
  // The last few are legit web addresses that we don't handle now.
  const wchar_t* const invalid_cases[] = {
      L"", L"http", L"www.", L"https-and-www",
      L"http:/abc.com",             // Missing slash.
      L"http://((()),",             // Only invalid chars in host name.
      L"ftp://example.com",         // Ftp scheme is not supported.
      L"http:example.com",          // Missing slashes.
      L"http//[example.com",        // Invalid IPv6 address.
      L"http//[00:00:00:00:00:00",  // Invalid IPv6 address.
      L"http//[]",                  // Empty IPv6 address.
      // Web addresses that in correct format that we don't handle.
      L"abc.example.com",  // URL without scheme.
  };
  for (size_t i = 0; i < FX_ArraySize(invalid_cases); ++i) {
    CFX_WideString text_str(invalid_cases[i]);
    EXPECT_FALSE(extractor.CheckWebLink(text_str)) << text_str.c_str();
  }

  // Check cases that can extract valid web link.
  // An array of {input_string, expected_extracted_web_link}.
  const wchar_t* const valid_cases[][2] = {
      {L"http://www.example.com", L"http://www.example.com"},  // standard URL.
      {L"http://www.example.com:88",
       L"http://www.example.com:88"},  // URL with port number.
      {L"http://test@www.example.com",
       L"http://test@www.example.com"},  // URL with username.
      {L"http://test:test@example.com",
       L"http://test:test@example.com"},  // URL with username and password.
      {L"http://example", L"http://example"},  // URL with short domain name.
      {L"http////www.server", L"http://www.server"},  // URL starts with "www.".
      {L"http:/www.abc.com", L"http://www.abc.com"},  // URL starts with "www.".
      {L"www.a.b.c", L"http://www.a.b.c"},            // URL starts with "www.".
      {L"https://a.us", L"https://a.us"},             // Secure http URL.
      {L"https://www.t.us", L"https://www.t.us"},     // Secure http URL.
      {L"www.example-test.com",
       L"http://www.example-test.com"},  // '-' in host is ok.
      {L"www.example.com,",
       L"http://www.example.com"},  // Trim ending invalid chars.
      {L"www.example.com;(",
       L"http://www.example.com"},  // Trim ending invalid chars.
      {L"test:www.abc.com", L"http://www.abc.com"},  // Trim chars before URL.
      {L"www.g.com..", L"http://www.g.com.."},       // Leave ending periods.
      // Web link can contain IP address too.
      {L"http://192.168.0.1", L"http://192.168.0.1"},  // IPv4 address.
      {L"http://192.168.0.1:80",
       L"http://192.168.0.1:80"},  // IPv4 address with port.
      {L"http://[aa::00:bb::00:cc:00]",
       L"http://[aa::00:bb::00:cc:00]"},  // IPv6 reference.
      {L"http://[aa::00:bb::00:cc:00]:12",
       L"http://[aa::00:bb::00:cc:00]:12"},       // IPv6 reference with port.
      {L"http://[aa]:12", L"http://[aa]:12"},     // Not validate IP address.
      {L"http://[aa]:12abc", L"http://[aa]:12"},  // Trim for IPv6 address.
      {L"http://[aa]:", L"http://[aa]"},          // Trim for IPv6 address.
      // Path and query parts can be anything.
      {L"www.abc.com/#%%^&&*(", L"http://www.abc.com/#%%^&&*("},
      {L"www.a.com/#a=@?q=rr&r=y", L"http://www.a.com/#a=@?q=rr&r=y"},
      {L"http://a.com/1/2/3/4\5\6", L"http://a.com/1/2/3/4\5\6"},
      {L"http://www.example.com/foo;bar", L"http://www.example.com/foo;bar"},
      // Invalid chars inside host name are ok as we don't validate them.
      {L"http://ex[am]ple", L"http://ex[am]ple"},
      {L"http://:example.com", L"http://:example.com"},
      {L"http://((())/path?", L"http://((())/path?"},
      {L"http:////abc.server", L"http:////abc.server"},
      // Non-ASCII chars are not validated either.
      {L"www.测试.net", L"http://www.测试.net"},
      {L"www.测试。net。", L"http://www.测试。net。"},
      {L"www.测试.net；", L"http://www.测试.net；"},
  };
  for (size_t i = 0; i < FX_ArraySize(valid_cases); ++i) {
    CFX_WideString text_str(valid_cases[i][0]);
    EXPECT_TRUE(extractor.CheckWebLink(text_str)) << text_str.c_str();
    EXPECT_STREQ(valid_cases[i][1], text_str.c_str());
  }
}

// Copyright 2021 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/cxfa_document.h"

#include "core/fxcrt/fx_string.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CXFA_DocumentTest, ParseXFAVersion) {
  // Malformed
  EXPECT_EQ(XFA_VERSION_UNKNOWN, CXFA_Document::ParseXFAVersion(L""));
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template"));
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-templatX/"));
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/"));
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/2"));

  // Out-of-range
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/-1.0"));
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/1.9"));
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/4.1"));

  // Missing digits
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/."));
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/.3"));
  EXPECT_EQ(XFA_VERSION_300, CXFA_Document::ParseXFAVersion(
                                 L"http://www.xfa.org/schema/xfa-template/3."));
  EXPECT_EQ(XFA_VERSION_UNKNOWN,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/clams.6"));
  EXPECT_EQ(XFA_VERSION_300,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/3.clams"));

  // Min / max values
  EXPECT_EQ(XFA_VERSION_200,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/2.0"));
  EXPECT_EQ(400, CXFA_Document::ParseXFAVersion(
                     L"http://www.xfa.org/schema/xfa-template/4.0"));

  // Number and decimal point parsing.
  EXPECT_EQ(XFA_VERSION_306,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/3.6"));

  // TODO(tsepez): maybe fail on these dubious values?
  EXPECT_EQ(XFA_VERSION_306,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/0003.00006"));
  EXPECT_EQ(XFA_VERSION_306,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/0003.00006.0000"));
  EXPECT_EQ(XFA_VERSION_206,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/2.6clams"));
  EXPECT_EQ(XFA_VERSION_206,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/1.106"));
  EXPECT_EQ(XFA_VERSION_306,
            CXFA_Document::ParseXFAVersion(
                L"http://www.xfa.org/schema/xfa-template/4.-94"));
  EXPECT_EQ(317, CXFA_Document::ParseXFAVersion(
                     L"http://www.xfa.org/schema/xfa-template/3.17"));
}

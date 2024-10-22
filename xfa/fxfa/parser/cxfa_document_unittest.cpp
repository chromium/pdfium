// Copyright 2021 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "xfa/fxfa/parser/cxfa_document.h"

#include "core/fxcrt/fx_string.h"
#include "testing/gtest/include/gtest/gtest.h"

TEST(CXFADocumentTest, ParseXFAVersion) {
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

TEST(CXFADocumentTest, ParseUseHref) {
  {
    WideString wsEmpty;  // Must outlive views.
    WideStringView wsURI;
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUseHref(wsEmpty, wsURI, wsID, wsSOM);
    EXPECT_EQ(L"", wsURI);
    EXPECT_EQ(L"", wsID);
    EXPECT_EQ(L"", wsSOM);
  }
  {
    WideString wsNoSharp(L"url-part-only");  // Must outlive views.
    WideStringView wsURI;
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUseHref(wsNoSharp, wsURI, wsID, wsSOM);
    EXPECT_EQ(L"url-part-only", wsURI);
    EXPECT_EQ(L"", wsID);
    EXPECT_EQ(L"", wsSOM);
  }
  {
    WideString wsNoSom(L"url-part#frag");  // Must outlive views.
    WideStringView wsURI;
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUseHref(wsNoSom, wsURI, wsID, wsSOM);
    EXPECT_EQ(L"url-part", wsURI);
    EXPECT_EQ(L"frag", wsID);
    EXPECT_EQ(L"", wsSOM);
  }
  {
    WideString wsIncompleteSom(L"url-part#som(");  // Must outlive views.
    WideStringView wsURI;
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUseHref(wsIncompleteSom, wsURI, wsID, wsSOM);
    EXPECT_EQ(L"url-part", wsURI);
    EXPECT_EQ(L"som(", wsID);
    EXPECT_EQ(L"", wsSOM);
  }
  {
    WideString wsEmptySom(L"url-part#som()");  // Must outlive views.
    WideStringView wsURI;
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUseHref(wsEmptySom, wsURI, wsID, wsSOM);
    EXPECT_EQ(L"url-part", wsURI);
    EXPECT_EQ(L"", wsID);
    EXPECT_EQ(L"", wsSOM);
  }
  {
    WideString wsHasSom(
        L"url-part#som(nested(foo.bar))");  // Must outlive views.
    WideStringView wsURI;
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUseHref(wsHasSom, wsURI, wsID, wsSOM);
    EXPECT_EQ(L"url-part", wsURI);
    EXPECT_EQ(L"", wsID);
    EXPECT_EQ(L"nested(foo.bar)", wsSOM);
  }
}

TEST(CXFADocumentTest, ParseUse) {
  {
    WideString wsUseVal(L"");  // Must outlive views.
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUse(wsUseVal, wsID, wsSOM);
    EXPECT_EQ(L"", wsID);
    EXPECT_EQ(L"", wsSOM);
  }
  {
    WideString wsUseVal(L"clams");  // Must outlive views.
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUse(wsUseVal, wsID, wsSOM);
    EXPECT_EQ(L"", wsID);
    EXPECT_EQ(L"clams", wsSOM);
  }
  {
    WideString wsUseVal(L"#clams");  // Must outlive views.
    WideStringView wsID;
    WideStringView wsSOM;
    CXFA_Document::ParseUse(wsUseVal, wsID, wsSOM);
    EXPECT_EQ(L"clams", wsID);
    EXPECT_EQ(L"", wsSOM);
  }
}

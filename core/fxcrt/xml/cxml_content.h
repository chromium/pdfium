// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_CONTENT_H_
#define CORE_FXCRT_XML_CXML_CONTENT_H_

class CXML_Content {
 public:
  CXML_Content() : m_bCDATA(false), m_Content() {}

  void Set(bool bCDATA, const CFX_WideStringC& content) {
    m_bCDATA = bCDATA;
    m_Content = content;
  }

  bool m_bCDATA;
  CFX_WideString m_Content;
};

#endif  // CORE_FXCRT_XML_CXML_CONTENT_H_

// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cxml_content.h"

CXML_Content::CXML_Content(bool bCDATA, const WideStringView& content)
    : m_bCDATA(bCDATA), m_Content(content) {}

CXML_Content::~CXML_Content() {}

CXML_Content* CXML_Content::AsContent() {
  return this;
}

const CXML_Content* CXML_Content::AsContent() const {
  return this;
}

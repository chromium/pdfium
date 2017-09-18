// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxcrt/xml/cfx_xmlchardata.h"

#include "third_party/base/ptr_util.h"

CFX_XMLCharData::CFX_XMLCharData(const WideString& wsCData)
    : CFX_XMLText(wsCData) {}

CFX_XMLCharData::~CFX_XMLCharData() {}

FX_XMLNODETYPE CFX_XMLCharData::GetType() const {
  return FX_XMLNODE_CharData;
}

std::unique_ptr<CFX_XMLNode> CFX_XMLCharData::Clone() {
  return pdfium::MakeUnique<CFX_XMLCharData>(GetText());
}

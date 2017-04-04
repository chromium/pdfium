// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/xml/cfde_xmlchardata.h"

#include "third_party/base/ptr_util.h"

CFDE_XMLCharData::CFDE_XMLCharData(const CFX_WideString& wsCData)
    : CFDE_XMLText(wsCData) {}

CFDE_XMLCharData::~CFDE_XMLCharData() {}

FDE_XMLNODETYPE CFDE_XMLCharData::GetType() const {
  return FDE_XMLNODE_CharData;
}

std::unique_ptr<CFDE_XMLNode> CFDE_XMLCharData::Clone() {
  return pdfium::MakeUnique<CFDE_XMLCharData>(GetText());
}

// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_metadata.h"

#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/xml/cxml_element.h"

CPDF_Metadata::CPDF_Metadata(const CPDF_Document* pDoc) {
  const CPDF_Dictionary* pRoot = pDoc->GetRoot();
  if (!pRoot)
    return;

  CPDF_Stream* pStream = pRoot->GetStreamFor("Metadata");
  if (!pStream)
    return;

  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(pStream);
  pAcc->LoadAllDataFiltered();
  m_pXmlElement = CXML_Element::Parse(pAcc->GetData(), pAcc->GetSize());
}

CPDF_Metadata::~CPDF_Metadata() {}

const CXML_Element* CPDF_Metadata::GetRoot() const {
  return m_pXmlElement.get();
}

// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_metadata.h"

#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/xml/cxml_content.h"
#include "core/fxcrt/xml/cxml_element.h"

namespace {

void CheckForSharedFormInternal(CXML_Element* element,
                                std::vector<UnsupportedFeature>* unsupported) {
  size_t count = element->CountAttrs();
  for (size_t i = 0; i < count; ++i) {
    ByteString space;
    ByteString name;
    WideString value;
    element->GetAttrByIndex(i, &space, &name, &value);
    if (space != "xmlns" || name != "adhocwf" ||
        value != L"http://ns.adobe.com/AcrobatAdhocWorkflow/1.0/") {
      continue;
    }

    CXML_Element* pVersion = element->GetElement("adhocwf", "workflowType", 0);
    if (!pVersion)
      continue;

    CXML_Content* pContent = ToContent(pVersion->GetChild(0));
    if (!pContent)
      continue;

    switch (pContent->m_Content.GetInteger()) {
      case 0:
        unsupported->push_back(UnsupportedFeature::kDocumentSharedFormEmail);
        break;
      case 1:
        unsupported->push_back(UnsupportedFeature::kDocumentSharedFormAcrobat);
        break;
      case 2:
        unsupported->push_back(
            UnsupportedFeature::kDocumentSharedFormFilesystem);
        break;
    }
  }

  count = element->CountChildren();
  for (size_t i = 0; i < count; ++i) {
    CXML_Element* child = ToElement(element->GetChild(i));
    if (!child)
      continue;

    CheckForSharedFormInternal(child, unsupported);
  }
}

}  // namespace

CPDF_Metadata::CPDF_Metadata(const CPDF_Stream* pStream) : stream_(pStream) {
  ASSERT(pStream);
}

CPDF_Metadata::~CPDF_Metadata() = default;

std::vector<UnsupportedFeature> CPDF_Metadata::CheckForSharedForm() const {
  auto pAcc = pdfium::MakeRetain<CPDF_StreamAcc>(stream_.Get());
  pAcc->LoadAllDataFiltered();

  std::unique_ptr<CXML_Element> xml_root =
      CXML_Element::Parse(pAcc->GetData(), pAcc->GetSize());
  if (!xml_root)
    return {};

  std::vector<UnsupportedFeature> unsupported;
  CheckForSharedFormInternal(xml_root.get(), &unsupported);
  return unsupported;
}

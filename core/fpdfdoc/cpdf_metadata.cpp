// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_metadata.h"

#include "core/fpdfapi/parser/cpdf_stream.h"
#include "core/fpdfapi/parser/cpdf_stream_acc.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/xml/cfx_xmlelement.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"

namespace {

void CheckForSharedFormInternal(CFX_XMLElement* element,
                                std::vector<UnsupportedFeature>* unsupported) {
  for (const auto& pair : element->GetAttributes()) {
    if (pair.first != L"xmlns:adhocwf" ||
        pair.second != L"http://ns.adobe.com/AcrobatAdhocWorkflow/1.0/") {
      continue;
    }

    for (const auto* child = element->GetFirstChild(); child;
         child = child->GetNextSibling()) {
      if (child->GetType() != FX_XMLNODE_Element)
        continue;

      const auto* child_elem = static_cast<const CFX_XMLElement*>(child);
      if (child_elem->GetName() != L"adhocwf:workflowType")
        continue;

      switch (child_elem->GetTextData().GetInteger()) {
        case 0:
          unsupported->push_back(UnsupportedFeature::kDocumentSharedFormEmail);
          break;
        case 1:
          unsupported->push_back(
              UnsupportedFeature::kDocumentSharedFormAcrobat);
          break;
        case 2:
          unsupported->push_back(
              UnsupportedFeature::kDocumentSharedFormFilesystem);
          break;
      }
      // We only care about the first one we find.
      break;
    }
  }

  for (auto* child = element->GetFirstChild(); child;
       child = child->GetNextSibling()) {
    if (child->GetType() != FX_XMLNODE_Element)
      continue;

    CheckForSharedFormInternal(static_cast<CFX_XMLElement*>(child),
                               unsupported);
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

  auto root = pdfium::MakeUnique<CFX_XMLElement>(L"root");
  auto proxy = pdfium::MakeRetain<CFX_SeekableStreamProxy>(pAcc->GetData(),
                                                           pAcc->GetSize());
  proxy->SetCodePage(FX_CODEPAGE_UTF8);

  CFX_XMLParser parser(root.get(), proxy);
  if (!parser.Parse())
    return {};

  std::vector<UnsupportedFeature> unsupported;
  CheckForSharedFormInternal(root.get(), &unsupported);
  return unsupported;
}

// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstddef>
#include <cstdint>
#include <memory>

#include "core/fxcrt/fx_basic.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "third_party/base/ptr_util.h"
#include "xfa/fde/xml/fde_xml_imp.h"
#include "xfa/fxfa/parser/cxfa_xml_parser.h"
#include "xfa/fxfa/parser/cxfa_widetextread.h"

namespace {

CFDE_XMLNode* XFA_FDEExtension_GetDocumentNode(
    CFDE_XMLDoc* pXMLDoc,
    bool bVerifyWellFormness = false) {
  if (!pXMLDoc) {
    return nullptr;
  }
  CFDE_XMLNode* pXMLFakeRoot = pXMLDoc->GetRoot();
  for (CFDE_XMLNode* pXMLNode =
           pXMLFakeRoot->GetNodeItem(CFDE_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(CFDE_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
      if (bVerifyWellFormness) {
        for (CFDE_XMLNode* pNextNode =
                 pXMLNode->GetNodeItem(CFDE_XMLNode::NextSibling);
             pNextNode;
             pNextNode = pNextNode->GetNodeItem(CFDE_XMLNode::NextSibling)) {
          if (pNextNode->GetType() == FDE_XMLNODE_Element) {
            return nullptr;
          }
        }
      }
      return pXMLNode;
    }
  }
  return nullptr;
}

}  // namespace

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size) {
  FX_SAFE_STRSIZE safe_size = size;
  if (!safe_size.IsValid())
    return 0;

  CFX_WideString input =
      CFX_WideString::FromUTF8(CFX_ByteStringC(data, safe_size.ValueOrDie()));
  auto stream = pdfium::MakeRetain<CXFA_WideTextRead>(input);
  if (!stream)
    return 0;

  auto doc = pdfium::MakeUnique<CFDE_XMLDoc>();
  if (!doc->LoadXML(pdfium::MakeUnique<CXFA_XMLParser>(doc->GetRoot(), stream)))
    return 0;

  if (doc->DoLoad(nullptr) < 100)
    return 0;

  (void)XFA_FDEExtension_GetDocumentNode(doc.get());
  return 0;
}

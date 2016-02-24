// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstddef>
#include <cstdint>
#include <limits>
#include <memory>

#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxcrt/fx_system.h"
#include "xfa/src/foxitlib.h"
#include "xfa/src/fxfa/src/common/xfa_parser.h"
#include "xfa/src/fxfa/src/parser/xfa_parser_imp.h"

namespace {

IFDE_XMLNode* XFA_FDEExtension_GetDocumentNode(
    IFDE_XMLDoc* pXMLDoc,
    FX_BOOL bVerifyWellFormness = FALSE) {
  if (!pXMLDoc) {
    return nullptr;
  }
  IFDE_XMLNode* pXMLFakeRoot = pXMLDoc->GetRoot();
  for (IFDE_XMLNode* pXMLNode =
           pXMLFakeRoot->GetNodeItem(IFDE_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() == FDE_XMLNODE_Element) {
      if (bVerifyWellFormness) {
        for (IFDE_XMLNode* pNextNode =
                 pXMLNode->GetNodeItem(IFDE_XMLNode::NextSibling);
             pNextNode;
             pNextNode = pNextNode->GetNodeItem(IFDE_XMLNode::NextSibling)) {
          if (pNextNode->GetType() == FDE_XMLNODE_Element) {
            return FALSE;
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
  if (size > std::numeric_limits<FX_STRSIZE>::max())
    return 0;

  CFX_WideString input = CFX_WideString::FromUTF8(
      reinterpret_cast<const char*>(data), static_cast<FX_STRSIZE>(size));
  std::unique_ptr<IFX_Stream, ReleaseDeleter<IFX_Stream>> stream(
      XFA_CreateWideTextRead(input));
  if (!stream)
    return 0;

  std::unique_ptr<IFDE_XMLDoc> doc(IFDE_XMLDoc::Create());
  if (!doc)
    return 0;

  std::unique_ptr<IFDE_XMLParser, ReleaseDeleter<IFDE_XMLParser>> parser(
      new CXFA_XMLParser(doc->GetRoot(), stream.get()));
  if (!parser)
    return 0;

  if (!doc->LoadXML(parser.release()))
    return 0;

  int32_t load_result = doc->DoLoad(nullptr);
  if (load_result < 100)
    return 0;

  (void)XFA_FDEExtension_GetDocumentNode(doc.get());
  return 0;
}

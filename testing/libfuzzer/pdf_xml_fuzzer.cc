// Copyright 2016 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include <cstddef>
#include <cstdint>
#include <memory>

#include "core/fxcrt/cfx_seekablestreamproxy.h"
#include "core/fxcrt/fx_safe_types.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/xml/cfx_xmldoc.h"
#include "core/fxcrt/xml/cfx_xmlnode.h"
#include "core/fxcrt/xml/cfx_xmlparser.h"
#include "third_party/base/ptr_util.h"

namespace {

CFX_XMLNode* XFA_FDEExtension_GetDocumentNode(
    CFX_XMLDoc* pXMLDoc,
    bool bVerifyWellFormness = false) {
  if (!pXMLDoc) {
    return nullptr;
  }
  CFX_XMLNode* pXMLFakeRoot = pXMLDoc->GetRoot();
  for (CFX_XMLNode* pXMLNode =
           pXMLFakeRoot->GetNodeItem(CFX_XMLNode::FirstChild);
       pXMLNode; pXMLNode = pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
    if (pXMLNode->GetType() == FX_XMLNODE_Element) {
      if (bVerifyWellFormness) {
        for (CFX_XMLNode* pNextNode =
                 pXMLNode->GetNodeItem(CFX_XMLNode::NextSibling);
             pNextNode;
             pNextNode = pNextNode->GetNodeItem(CFX_XMLNode::NextSibling)) {
          if (pNextNode->GetType() == FX_XMLNODE_Element) {
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
  FX_SAFE_SIZE_T safe_size = size;
  if (!safe_size.IsValid())
    return 0;

  RetainPtr<CFX_SeekableStreamProxy> stream =
      pdfium::MakeRetain<CFX_SeekableStreamProxy>(const_cast<uint8_t*>(data),
                                                  size);
  auto doc = pdfium::MakeUnique<CFX_XMLDoc>();
  if (!doc->LoadXML(pdfium::MakeUnique<CFX_XMLParser>(doc->GetRoot(), stream)))
    return 0;

  if (doc->DoLoad() < 100)
    return 0;

  (void)XFA_FDEExtension_GetDocumentNode(doc.get());
  return 0;
}

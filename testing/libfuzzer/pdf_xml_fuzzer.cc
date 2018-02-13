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

  CFX_XMLNode* pXMLFakeRoot = doc->GetRoot();
  for (CFX_XMLNode* pXMLNode = pXMLFakeRoot->GetFirstChild(); pXMLNode;
       pXMLNode = pXMLNode->GetNextSibling()) {
    if (pXMLNode->GetType() == FX_XMLNODE_Element)
      break;
  }
  return 0;
}

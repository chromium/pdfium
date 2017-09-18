// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CFX_XMLCHARDATA_H_
#define CORE_FXCRT_XML_CFX_XMLCHARDATA_H_

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cfx_xmltext.h"

class CFX_XMLCharData : public CFX_XMLText {
 public:
  explicit CFX_XMLCharData(const WideString& wsCData);
  ~CFX_XMLCharData() override;

  FX_XMLNODETYPE GetType() const override;
  std::unique_ptr<CFX_XMLNode> Clone() override;
};

#endif  // CORE_FXCRT_XML_CFX_XMLCHARDATA_H_

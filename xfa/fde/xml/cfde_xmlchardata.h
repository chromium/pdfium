// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_XML_CFDE_XMLCHARDATA_H_
#define XFA_FDE_XML_CFDE_XMLCHARDATA_H_

#include <memory>

#include "core/fxcrt/fx_string.h"
#include "xfa/fde/xml/cfde_xmltext.h"

class CFDE_XMLCharData : public CFDE_XMLText {
 public:
  explicit CFDE_XMLCharData(const CFX_WideString& wsCData);
  ~CFDE_XMLCharData() override;

  FDE_XMLNODETYPE GetType() const override;
  std::unique_ptr<CFDE_XMLNode> Clone() override;
};

#endif  // XFA_FDE_XML_CFDE_XMLCHARDATA_H_

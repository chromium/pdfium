// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXCRT_XML_CXML_CONTENT_H_
#define CORE_FXCRT_XML_CXML_CONTENT_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/xml/cxml_object.h"

class CXML_Content : public CXML_Object {
 public:
  CXML_Content(bool bCDATA, const WideStringView& content);
  ~CXML_Content() override;

  // CXML_Object:
  CXML_Content* AsContent() override;
  const CXML_Content* AsContent() const override;

  bool m_bCDATA;
  WideString m_Content;
};

#endif  // CORE_FXCRT_XML_CXML_CONTENT_H_

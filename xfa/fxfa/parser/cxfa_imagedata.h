// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_IMAGEDATA_H_
#define XFA_FXFA_PARSER_CXFA_IMAGEDATA_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"

class CXFA_Node;

class CXFA_ImageData : public CXFA_DataData {
 public:
  explicit CXFA_ImageData(CXFA_Node* pNode);

  XFA_AttributeEnum GetAspect() const;
  WideString GetContent() const;

  WideString GetHref() const;
  void SetHref(const WideString& wsHref);

  XFA_AttributeEnum GetTransferEncoding() const;
  void SetTransferEncoding(XFA_AttributeEnum iTransferEncoding);

  WideString GetContentType() const;
  void SetContentType(const WideString& wsContentType);
};

#endif  // XFA_FXFA_PARSER_CXFA_IMAGEDATA_H_

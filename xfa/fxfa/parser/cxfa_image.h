// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_IMAGE_H_
#define XFA_FXFA_PARSER_CXFA_IMAGE_H_

#include "xfa/fxfa/parser/cxfa_node.h"

class CXFA_Image final : public CXFA_Node {
 public:
  static CXFA_Image* FromNode(CXFA_Node* pNode);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_Image() override;

  XFA_AttributeValue GetAspect();
  WideString GetContent();

  WideString GetHref();
  void SetHref(const WideString& wsHref);

  XFA_AttributeValue GetTransferEncoding();
  void SetTransferEncoding(XFA_AttributeValue iTransferEncoding);

  WideString GetContentType();
  void SetContentType(const WideString& wsContentType);

 private:
  CXFA_Image(CXFA_Document* doc, XFA_PacketType packet);
};

#endif  // XFA_FXFA_PARSER_CXFA_IMAGE_H_

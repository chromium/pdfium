// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CJX_BARCODE_H_
#define FXJS_XFA_CJX_BARCODE_H_

#include "fxjs/jse_define.h"
#include "fxjs/xfa/cjx_node.h"

class CXFA_Barcode;

class CJX_Barcode final : public CJX_Node {
 public:
  explicit CJX_Barcode(CXFA_Barcode* arc);
  ~CJX_Barcode() override;

  JSE_PROP(charEncoding);
  JSE_PROP(checksum);
  JSE_PROP(dataColumnCount);
  JSE_PROP(dataLength);
  JSE_PROP(dataPrep);
  JSE_PROP(dataRowCount);
  JSE_PROP(endChar);
  JSE_PROP(errorCorrectionLevel);
  JSE_PROP(moduleHeight);
  JSE_PROP(moduleWidth);
  JSE_PROP(printCheckDigit);
  JSE_PROP(rowColumnRatio);
  JSE_PROP(startChar);
  JSE_PROP(textLocation);
  JSE_PROP(truncate);
  JSE_PROP(type);
  JSE_PROP(upsMode);
  JSE_PROP(use);
  JSE_PROP(usehref);
  JSE_PROP(wideNarrowRatio);
};

#endif  // FXJS_XFA_CJX_BARCODE_H_

// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_VALUEDATA_H_
#define XFA_FXFA_PARSER_CXFA_VALUEDATA_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_arcdata.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_exdatadata.h"
#include "xfa/fxfa/parser/cxfa_imagedata.h"
#include "xfa/fxfa/parser/cxfa_linedata.h"
#include "xfa/fxfa/parser/cxfa_rectangledata.h"
#include "xfa/fxfa/parser/cxfa_textdata.h"

class CXFA_Node;

class CXFA_ValueData : public CXFA_DataData {
 public:
  explicit CXFA_ValueData(CXFA_Node* pNode) : CXFA_DataData(pNode) {}

  XFA_Element GetChildValueClassID() const;
  WideString GetChildValueContent() const;
  CXFA_ArcData GetArcData() const;
  CXFA_LineData GetLineData() const;
  CXFA_RectangleData GetRectangleData() const;
  CXFA_TextData GetTextData() const;
  CXFA_ExDataData GetExData() const;
  CXFA_ImageData GetImageData() const;
};

#endif  // XFA_FXFA_PARSER_CXFA_VALUEDATA_H_

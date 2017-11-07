// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_VALUEDATA_H_
#define XFA_FXFA_PARSER_CXFA_VALUEDATA_H_

#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/parser/cxfa_arcdata.h"
#include "xfa/fxfa/parser/cxfa_data.h"
#include "xfa/fxfa/parser/cxfa_exdata.h"
#include "xfa/fxfa/parser/cxfa_imagedata.h"
#include "xfa/fxfa/parser/cxfa_linedata.h"
#include "xfa/fxfa/parser/cxfa_rectangledata.h"
#include "xfa/fxfa/parser/cxfa_textdata.h"

class CXFA_Node;

class CXFA_ValueData : public CXFA_Data {
 public:
  explicit CXFA_ValueData(CXFA_Node* pNode) : CXFA_Data(pNode) {}

  XFA_Element GetChildValueClassID();
  bool GetChildValueContent(WideString& wsContent);
  CXFA_ArcData GetArcData();
  CXFA_LineData GetLineData();
  CXFA_RectangleData GetRectangleData();
  CXFA_TextData GetTextData();
  CXFA_ExData GetExData();
  CXFA_ImageData GetImageData();
};

#endif  // XFA_FXFA_PARSER_CXFA_VALUEDATA_H_

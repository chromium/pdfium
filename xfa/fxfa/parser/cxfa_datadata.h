// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_DATADATA_H_
#define XFA_FXFA_PARSER_CXFA_DATADATA_H_

#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fxfa/fxfa_basic.h"

class CXFA_Node;

class CXFA_DataData {
 public:
  static FX_ARGB ToColor(const WideStringView& wsValue);

  explicit CXFA_DataData(CXFA_Node* pNode);
  virtual ~CXFA_DataData();

  bool HasValidNode() const { return !!m_pNode; }
  CXFA_Node* GetNode() const { return m_pNode; }
  XFA_Element GetElementType() const;

 protected:
  pdfium::Optional<float> TryMeasureAsFloat(XFA_Attribute attr) const;

  CXFA_Node* m_pNode;
};

#endif  // XFA_FXFA_PARSER_CXFA_DATADATA_H_

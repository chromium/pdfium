// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFBARCODE_H_
#define XFA_FXFA_CXFA_FFBARCODE_H_

#include "fxbarcode/BC_Library.h"
#include "v8/include/cppgc/member.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/cxfa_fftextedit.h"

class CXFA_Barcode;

class CXFA_FFBarcode final : public CXFA_FFTextEdit {
 public:
  static BC_TYPE GetBarcodeTypeByName(const WideString& wsName);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFBarcode() override;

  void Trace(cppgc::Visitor* visitor) const override;

  // CXFA_FFTextEdit
  bool LoadWidget() override;
  void RenderWidget(CFGAS_GEGraphics* pGS,
                    const CFX_Matrix& matrix,
                    HighlightOption highlight) override;
  void UpdateWidgetProperty() override;
  bool AcceptsFocusOnButtonDown(
      Mask<XFA_FWL_KeyFlag> dwFlags,
      const CFX_PointF& point,
      CFWL_MessageMouse::MouseCommand command) override;

 private:
  CXFA_FFBarcode(CXFA_Node* pNode, CXFA_Barcode* barcode);

  cppgc::Member<CXFA_Barcode> const barcode_;
};

#endif  // XFA_FXFA_CXFA_FFBARCODE_H_

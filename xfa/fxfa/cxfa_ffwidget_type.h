// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFWIDGET_TYPE_H_
#define XFA_FXFA_CXFA_FFWIDGET_TYPE_H_

enum class XFA_FFWidgetType {
  kNone = 0,
  kBarcode,
  kButton,
  kCheckButton,
  kChoiceList,
  kDateTimeEdit,
  kImageEdit,
  kNumericEdit,
  kPasswordEdit,
  kSignature,
  kTextEdit,
  kArc,
  kLine,
  kRectangle,
  kText,
  kImage,
  kSubform,
  kExclGroup
};

#endif  // XFA_FXFA_CXFA_FFWIDGET_TYPE_H_

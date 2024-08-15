// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_FWL_WIDGETHIT_H_
#define XFA_FWL_FWL_WIDGETHIT_H_

namespace pdfium {

enum class FWL_WidgetHit {
  Unknown = 0,
  Client,
  Left,
  Top,
  Right,
  Bottom,
  LeftTop,
  RightTop,
  LeftBottom,
  RightBottom,
  Titlebar,
  MinBox,
  MaxBox,
  CloseBox,
  HScrollBar,
  VScrollBar,
  Border,
  Edge,
  Edit,
  HyperLink,
  UpButton,
  DownButton
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::FWL_WidgetHit;

#endif  // XFA_FWL_FWL_WIDGETHIT_H_

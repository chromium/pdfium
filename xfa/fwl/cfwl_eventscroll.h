// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_EVENTSCROLL_H_
#define XFA_FWL_CFWL_EVENTSCROLL_H_

#include "xfa/fwl/cfwl_event.h"

namespace pdfium {

class CFWL_EventScroll final : public CFWL_Event {
 public:
  enum class Code {
    None = 1,
    Min,
    Max,
    PageBackward,
    PageForward,
    StepBackward,
    StepForward,
    Pos,
    TrackPos,
    EndScroll,
  };

  CFWL_EventScroll(CFWL_Widget* pSrcTarget, Code code, float pos);
  ~CFWL_EventScroll() override;

  Code GetScrollCode() const { return scroll_code_; }
  float GetPos() const { return pos_; }

 private:
  const Code scroll_code_;
  const float pos_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_EventScroll;

#endif  // XFA_FWL_CFWL_EVENTSCROLL_H_

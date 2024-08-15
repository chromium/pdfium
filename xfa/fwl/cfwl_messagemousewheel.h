// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_MESSAGEMOUSEWHEEL_H_
#define XFA_FWL_CFWL_MESSAGEMOUSEWHEEL_H_

#include "core/fxcrt/fx_coordinates.h"
#include "xfa/fwl/cfwl_message.h"

namespace pdfium {

class CFWL_MessageMouseWheel final : public CFWL_Message {
 public:
  CFWL_MessageMouseWheel(CFWL_Widget* destination,
                         const CFX_PointF& pos,
                         const CFX_Vector& delta);
  ~CFWL_MessageMouseWheel() override;

  void set_pos(const CFX_PointF& pos) { pos_ = pos; }
  const CFX_PointF& pos() const { return pos_; }

  const CFX_Vector& delta() const { return delta_; }

 private:
  CFX_PointF pos_;
  const CFX_Vector delta_;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::CFWL_MessageMouseWheel;

#endif  // XFA_FWL_CFWL_MESSAGEMOUSEWHEEL_H_

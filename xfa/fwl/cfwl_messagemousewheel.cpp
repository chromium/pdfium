// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fwl/cfwl_messagemousewheel.h"

namespace pdfium {

CFWL_MessageMouseWheel::CFWL_MessageMouseWheel(CFWL_Widget* destination,
                                               const CFX_PointF& pos,
                                               const CFX_Vector& delta)
    : CFWL_Message(CFWL_Message::Type::kMouseWheel, destination),
      pos_(pos),
      delta_(delta) {}

CFWL_MessageMouseWheel::~CFWL_MessageMouseWheel() = default;

}  // namespace pdfium

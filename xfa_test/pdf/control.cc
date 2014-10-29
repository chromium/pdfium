// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/control.h"

#include "base/logging.h"
#include "pdf/draw_utils.h"

namespace chrome_pdf {

Control::Control()
    : id_(kInvalidControlId),
      visible_(false),
      owner_(NULL),
      transparency_(kOpaqueAlpha) {
}

Control::~Control() {
}

bool Control::Create(uint32 id, const pp::Rect& rc,
                     bool visible, Owner* owner) {
  DCHECK(owner);
  if (owner_ || id == kInvalidControlId)
    return false;  // Already created or id is invalid.
  id_ = id;
  rc_ = rc;
  visible_ = visible;
  owner_ = owner;
  return true;
}

bool Control::HandleEvent(const pp::InputEvent& event) {
  return false;
}

void Control::PaintMultipleRects(pp::ImageData* image_data,
                                 const std::list<pp::Rect>& rects) {
  DCHECK(rects.size() > 0);
  if (rects.size() == 1) {
    Paint(image_data, rects.front());
    return;
  }

  // Some rects in the input list may overlap. To prevent double
  // painting (causes problems with semi-transparent controls) we'll
  // paint control into buffer image data only once and copy requested
  // rectangles.
  pp::ImageData buffer(owner()->GetInstance(), image_data->format(),
                       rect().size(), false);
  if (buffer.is_null())
    return;

  pp::Rect draw_rc = pp::Rect(image_data->size()).Intersect(rect());
  pp::Rect ctrl_rc = pp::Rect(draw_rc.point() - rect().point(), draw_rc.size());
  CopyImage(*image_data, draw_rc, &buffer, ctrl_rc, false);

  // Temporary move control to origin (0,0) and draw it into temp buffer.
  // Move to the original position afterward. Since this is going on temp
  // buffer, we don't need to invalidate here.
  pp::Rect temp = rect();
  MoveTo(pp::Point(0, 0), false);
  Paint(&buffer, ctrl_rc);
  MoveTo(temp.point(), false);

  std::list<pp::Rect>::const_iterator iter;
  for (iter = rects.begin(); iter != rects.end(); ++iter) {
    pp::Rect draw_rc = rect().Intersect(*iter);
    if (!draw_rc.IsEmpty()) {
      // Copy requested rect from the buffer image.
      pp::Rect src_rc = draw_rc;
      src_rc.Offset(-rect().x(), -rect().y());
      CopyImage(buffer, src_rc, image_data, draw_rc, false);
    }
  }
}

void Control::Show(bool visible, bool invalidate) {
  if (visible_ != visible) {
    visible_ = visible;
    if (invalidate)
      owner_->Invalidate(id_, rc_);
  }
}

void Control::AdjustTransparency(uint8 transparency, bool invalidate) {
  if (transparency_ != transparency) {
    transparency_ = transparency;
    if (invalidate && visible_)
      owner_->Invalidate(id_, rc_);
  }
}

void Control::MoveBy(const pp::Point& offset, bool invalidate) {
  pp::Rect old_rc = rc_;
  rc_.Offset(offset);
  if (invalidate && visible_) {
    owner()->Invalidate(id(), old_rc);
    owner()->Invalidate(id(), rect());
  }
}

void Control::SetRect(const pp::Rect& rc, bool invalidate) {
  pp::Rect old_rc = rc_;
  rc_ = rc;
  if (invalidate && visible_) {
    owner()->Invalidate(id(), old_rc);
    owner()->Invalidate(id(), rect());
  }
}

void Control::MoveTo(const pp::Point& origin, bool invalidate) {
  MoveBy(origin - rc_.point(), invalidate);
}

}  // namespace chrome_pdf

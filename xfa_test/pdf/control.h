// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_CONTROL_H_
#define PDF_CONTROL_H_

#include <list>

#include "base/basictypes.h"
#include "ppapi/c/dev/pp_cursor_type_dev.h"
#include "ppapi/cpp/rect.h"

namespace pp {
class ImageData;
class InputEvent;
class Instance;
}

namespace chrome_pdf {

const uint32 kInvalidControlId = 0;

class Control {
 public:
  class Owner {
   public:
    virtual ~Owner() {}
    virtual void OnEvent(uint32 control_id, uint32 event_id, void* data) = 0;
    virtual void Invalidate(uint32 control_id, const pp::Rect& rc) = 0;
    virtual uint32 ScheduleTimer(uint32 control_id, uint32 timeout_ms) = 0;
    virtual void SetEventCapture(uint32 control_id, bool set_capture) = 0;
    virtual void SetCursor(uint32 control_id,
                           PP_CursorType_Dev cursor_type) = 0;
    virtual pp::Instance* GetInstance() = 0;
  };

  Control();
  virtual ~Control();
  virtual bool Create(uint32 id, const pp::Rect& rc,
                      bool visible, Owner* owner);

  virtual void Paint(pp::ImageData* image_data, const pp::Rect& rc) {}
  virtual bool HandleEvent(const pp::InputEvent& event);
  virtual void OnTimerFired(uint32 timer_id) {}
  virtual void EventCaptureReleased() {}

  // Paint control into multiple destination rects.
  virtual void PaintMultipleRects(pp::ImageData* image_data,
                                  const std::list<pp::Rect>& rects);

  virtual void Show(bool visible, bool invalidate);
  virtual void AdjustTransparency(uint8 transparency, bool invalidate);
  virtual void MoveBy(const pp::Point& offset, bool invalidate);
  virtual void SetRect(const pp::Rect& rc, bool invalidate);

  void MoveTo(const pp::Point& origin, bool invalidate);

  uint32 id() const { return id_; }
  const pp::Rect& rect() const { return rc_; }
  bool visible() const { return visible_; }
  Owner* owner() { return owner_; }
  uint8 transparency() const { return transparency_; }

 private:
  uint32 id_;
  pp::Rect rc_;
  bool visible_;
  Owner* owner_;
  uint8 transparency_;
};

typedef Control::Owner ControlOwner;

}  // namespace chrome_pdf

#endif  // PDF_CONTROL_H_

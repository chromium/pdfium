// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_FADING_CONTROLS_H_
#define PDF_FADING_CONTROLS_H_

#include <list>

#include "pdf/control.h"

namespace chrome_pdf {

class FadingControls : public Control,
                       public ControlOwner {
 public:
  enum FadingState {
    NONE,
    FADING_IN,
    FADING_OUT
  };

  FadingControls();
  virtual ~FadingControls();
  virtual bool CreateFadingControls(
      uint32 id, const pp::Rect& rc, bool visible,
      Control::Owner* delegate, uint8 transparency);

  // Control interface.
  virtual void Paint(pp::ImageData* image_data, const pp::Rect& rc);
  virtual bool HandleEvent(const pp::InputEvent& event);
  virtual void OnTimerFired(uint32 timer_id);
  virtual void EventCaptureReleased();
  virtual void MoveBy(const pp::Point& offset, bool invalidate);

  // ControlOwner interface.
  virtual void OnEvent(uint32 control_id, uint32 event_id, void* data);
  virtual void Invalidate(uint32 control_id, const pp::Rect& rc);
  virtual uint32 ScheduleTimer(uint32 control_id, uint32 timeout_ms);
  virtual void SetEventCapture(uint32 control_id, bool set_capture);
  virtual void SetCursor(uint32 control_id, PP_CursorType_Dev cursor_type);
  virtual pp::Instance* GetInstance();

  // FadingControls interface
  // This function takes ownership of the control, and will destoy it
  // when control is destroyed.
  // Input control MUST be located inside FadingControls boundaries, and has
  // this instance of FadingControls as a delegate.
  virtual bool AddControl(Control* control);
  virtual void RemoveControl(uint32 control_id);
  virtual Control* GetControl(uint32 id);
  virtual pp::Rect GetControlsRect();

  // Expand/Shrink area which triggers inner control appearance to the left.
  virtual bool ExpandLeft(int offset);

  // Fade-in, then show controls for time_ms, and then fade-out. Any mouse
  // event in this control area will interrupt splash mode.
  virtual void Splash(uint32 time_ms);

  uint8 current_transparency() const { return current_transparency_; }

 private:
  bool NotifyControls(const pp::InputEvent& event);
  void FadeIn();
  void FadeOut();
  void OnFadingComplete();
  void CancelSplashMode();

  std::list<Control*> controls_;
  FadingState state_;
  uint8 current_transparency_;
  uint32 fading_timer_id_;
  uint32 current_capture_control_;
  uint32 fading_timeout_;
  uint32 alpha_shift_;
  bool splash_;
  uint32 splash_timeout_;
};

}  // namespace chrome_pdf

#endif  // PDF_FADING_CONTROLS_H_


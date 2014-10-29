// Copyright (c) 2012 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/fading_controls.h"

#include "base/logging.h"
#include "base/stl_util.h"
#include "pdf/draw_utils.h"
#include "pdf/resource_consts.h"
#include "ppapi/cpp/input_event.h"

namespace chrome_pdf {

const uint32 kFadingAlphaShift = 64;
const uint32 kSplashFadingAlphaShift = 16;

FadingControls::FadingControls()
    : state_(NONE), current_transparency_(kOpaqueAlpha), fading_timer_id_(0),
      current_capture_control_(kInvalidControlId),
      fading_timeout_(kFadingTimeoutMs), alpha_shift_(kFadingAlphaShift),
      splash_(false), splash_timeout_(0) {
}

FadingControls::~FadingControls() {
  STLDeleteElements(&controls_);
}

bool FadingControls::CreateFadingControls(
    uint32 id, const pp::Rect& rc, bool visible,
    Control::Owner* owner, uint8 transparency) {
  current_transparency_ = transparency;
  return Control::Create(id, rc, visible, owner);
}

void FadingControls::Paint(pp::ImageData* image_data, const pp::Rect& rc) {
  // When this control is set to invisible the individual controls are not.
  // So we need to check for visible() here.
  if (!visible())
    return;

  std::list<Control*>::iterator iter;
  for (iter = controls_.begin(); iter != controls_.end(); ++iter) {
    (*iter)->Paint(image_data, rc);
  }
}

bool FadingControls::HandleEvent(const pp::InputEvent& event) {
  if (!visible())
    return false;

  pp::MouseInputEvent mouse_event(event);
  if (mouse_event.is_null())
    return NotifyControls(event);

  pp::Point pt = mouse_event.GetPosition();

  bool is_mouse_click =
      mouse_event.GetType() == PP_INPUTEVENT_TYPE_MOUSEDOWN ||
      mouse_event.GetType() == PP_INPUTEVENT_TYPE_MOUSEUP;

  if (rect().Contains(pt)) {
    CancelSplashMode();
    FadeIn();

    // Eat mouse click if are invisible or just fading in.
    // That prevents accidental clicks on the controls for touch devices.
    bool eat_mouse_click =
        (state_ == FADING_IN || current_transparency_ == kTransparentAlpha);
    if (eat_mouse_click && is_mouse_click &&
        mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_LEFT)
      return true;  // Eat this event here.
  }

  if ((!rect().Contains(pt)) ||
      event.GetType() == PP_INPUTEVENT_TYPE_MOUSELEAVE) {
    if (!splash_)
      FadeOut();
    pp::MouseInputEvent event_leave(pp::MouseInputEvent(
        owner()->GetInstance(),
        PP_INPUTEVENT_TYPE_MOUSELEAVE,
        event.GetTimeStamp(),
        event.GetModifiers(),
        mouse_event.GetButton(),
        mouse_event.GetPosition(),
        mouse_event.GetClickCount(),
        mouse_event.GetMovement()));
    return NotifyControls(event_leave);
  }

  return NotifyControls(event);
}

void FadingControls::OnTimerFired(uint32 timer_id) {
  if (timer_id == fading_timer_id_) {
    int32 current_alpha = static_cast<int32>(current_transparency_);
    if (state_ == FADING_IN)
      current_alpha += alpha_shift_;
    else if (state_ == FADING_OUT)
      current_alpha -= alpha_shift_;

    if (current_alpha >= kOpaqueAlpha) {
      state_ = NONE;
      current_alpha = kOpaqueAlpha;
    } else if (current_alpha <= kTransparentAlpha) {
      state_ = NONE;
      current_alpha = kTransparentAlpha;
    }
    current_transparency_ = static_cast<uint8>(current_alpha);

    // Invalidate controls with new alpha transparency.
    std::list<Control*>::iterator iter;
    for (iter = controls_.begin(); iter != controls_.end(); ++iter) {
      // We are going to invalidate the whole FadingControls area, to
      // allow simultaneous drawing.
      (*iter)->AdjustTransparency(current_transparency_, false);
    }
    owner()->Invalidate(id(), GetControlsRect());

    if (state_ != NONE)  // Fading still in progress.
      fading_timer_id_ = owner()->ScheduleTimer(id(), fading_timeout_);
    else
      OnFadingComplete();
  } else {
    // Dispatch timer to controls.
    std::list<Control*>::iterator iter;
    for (iter = controls_.begin(); iter != controls_.end(); ++iter) {
      (*iter)->OnTimerFired(timer_id);
    }
  }
}

void FadingControls::EventCaptureReleased() {
  if (current_capture_control_ != kInvalidControlId) {
    // Remove previous catpure.
    Control* ctrl = GetControl(current_capture_control_);
    if (ctrl)
      ctrl->EventCaptureReleased();
  }
}

void FadingControls::MoveBy(const pp::Point& offset, bool invalidate) {
  std::list<Control*>::iterator iter;
  for (iter = controls_.begin(); iter != controls_.end(); ++iter) {
    // We invalidate entire FadingControl later if needed.
    (*iter)->MoveBy(offset, false);
  }
  Control::MoveBy(offset, invalidate);
}

void FadingControls::OnEvent(uint32 control_id, uint32 event_id, void* data) {
  owner()->OnEvent(control_id, event_id, data);
}

void FadingControls::Invalidate(uint32 control_id, const pp::Rect& rc) {
  owner()->Invalidate(control_id, rc);
}

uint32 FadingControls::ScheduleTimer(uint32 control_id, uint32 timeout_ms) {
  // TODO(gene): implement timer routine properly.
  NOTIMPLEMENTED();
  //owner()->ScheduleTimer(control_id);
  return 0;
}

void FadingControls::SetEventCapture(uint32 control_id, bool set_capture) {
  if (control_id == current_capture_control_) {
    if (!set_capture)  // Remove event capture.
      current_capture_control_ = kInvalidControlId;
  } else {
    EventCaptureReleased();
    current_capture_control_ = control_id;
  }
}

void FadingControls::SetCursor(uint32 control_id,
                               PP_CursorType_Dev cursor_type) {
  owner()->SetCursor(control_id, cursor_type);
}

pp::Instance* FadingControls::GetInstance() {
  return owner()->GetInstance();
}

bool FadingControls::AddControl(Control* control) {
  DCHECK(control);
  if (control->owner() != this)
    return false;
  if (!rect().Contains(control->rect()))
    return false;

  control->AdjustTransparency(current_transparency_, false);
  controls_.push_back(control);
  return true;
}

void FadingControls::RemoveControl(uint32 control_id) {
  if (current_capture_control_ == control_id) {
    current_capture_control_ = kInvalidControlId;
  }
  std::list<Control*>::iterator iter;
  for (iter = controls_.begin(); iter != controls_.end(); ++iter) {
    if ((*iter)->id() == control_id) {
      delete (*iter);
      controls_.erase(iter);
      break;
    }
  }
}

Control* FadingControls::GetControl(uint32 id) {
  std::list<Control*>::iterator iter;
  for (iter = controls_.begin(); iter != controls_.end(); ++iter) {
    if ((*iter)->id() == id)
      return *iter;
  }
  return NULL;
}

pp::Rect FadingControls::GetControlsRect() {
  pp::Rect rc;
  std::list<Control*>::iterator iter;
  for (iter = controls_.begin(); iter != controls_.end(); ++iter) {
    rc = rc.Union((*iter)->rect());
  }
  return rc;
}

bool FadingControls::ExpandLeft(int offset) {
  pp::Rect rc = rect();
  rc.set_width(rc.width() + offset);
  rc.set_x(rc.x() - offset);
  if (!rc.Contains(GetControlsRect()))
    return false;
  // No need to invalidate since we are expanding triggering area only.
  SetRect(rc, false);
  return true;
}

void FadingControls::Splash(uint32 time_ms) {
  splash_ = true;
  splash_timeout_ = time_ms;
  alpha_shift_ = kSplashFadingAlphaShift;
  FadeIn();
}

bool FadingControls::NotifyControls(const pp::InputEvent& event) {
  // First pass event to a control that current capture is set to.
  Control* ctrl = GetControl(current_capture_control_);
  if (ctrl) {
    if (ctrl->HandleEvent(event))
      return true;
  }

  std::list<Control*>::iterator iter;
  for (iter = controls_.begin(); iter != controls_.end(); ++iter) {
    // Now pass event to all control except control with capture,
    // since we already passed to it above.
    if ((*iter) != ctrl && (*iter)->HandleEvent(event))
      return true;
  }
  return false;
}

void FadingControls::FadeIn() {
  bool already_visible =
      (state_ == NONE && current_transparency_ == kOpaqueAlpha);
  if (state_ != FADING_IN && !already_visible) {
    state_ = FADING_IN;
    fading_timer_id_ = owner()->ScheduleTimer(id(), fading_timeout_);
  }
  if (already_visible)
    OnFadingComplete();
}

void FadingControls::FadeOut() {
  bool already_invisible =
      (state_ == NONE && current_transparency_ == kTransparentAlpha);
  if (state_ != FADING_OUT && !already_invisible) {
    state_ = FADING_OUT;
    fading_timer_id_ = owner()->ScheduleTimer(id(), fading_timeout_);
  }
  if (already_invisible)
    OnFadingComplete();
}

void FadingControls::OnFadingComplete() {
  DCHECK(current_transparency_ == kOpaqueAlpha ||
      current_transparency_ == kTransparentAlpha);
  // In the splash mode following states are possible:
  // Fade-in complete: splash_==true, splash_timeout_ != 0
  //   We need to schedule timer for splash_timeout_.
  // Splash timeout complete: splash_==true, splash_timeout_ == 0
  //   We need to fade out still using splash settings.
  // Fade-out complete: current_transparency_ == kTransparentAlpha
  //   We need to cancel splash mode and go back to normal settings.
  if (splash_) {
    if (current_transparency_ == kOpaqueAlpha) {
      if (splash_timeout_) {
        fading_timer_id_ = owner()->ScheduleTimer(id(), splash_timeout_);
        splash_timeout_ = 0;
      } else {
        FadeOut();
      }
    } else {
      CancelSplashMode();
    }
  }
}

void FadingControls::CancelSplashMode() {
  splash_ = false;
  alpha_shift_ = kFadingAlphaShift;
}

}  // namespace chrome_pdf

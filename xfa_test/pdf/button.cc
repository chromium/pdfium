// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "pdf/button.h"

#include "base/logging.h"
#include "pdf/draw_utils.h"
#include "ppapi/cpp/input_event.h"

namespace chrome_pdf {

Button::Button()
    : style_(BUTTON_CLICKABLE), state_(BUTTON_NORMAL), is_pressed_(false) {
}

Button::~Button() {
}

bool Button::CreateButton(uint32 id,
                          const pp::Point& origin,
                          bool visible,
                          Control::Owner* owner,
                          ButtonStyle style,
                          const pp::ImageData& face_normal,
                          const pp::ImageData& face_highlighted,
                          const pp::ImageData& face_pressed) {
  DCHECK(face_normal.size().GetArea());
  DCHECK(face_normal.size() == face_highlighted.size());
  DCHECK(face_normal.size() == face_pressed.size());

  pp::Rect rc(origin, face_normal.size());
  if (!Control::Create(id, rc, visible, owner))
    return false;

  style_ = style;

  normal_ = face_normal;
  highlighted_ = face_highlighted;
  pressed_ = face_pressed;

  return true;
}


void Button::Paint(pp::ImageData* image_data, const pp::Rect& rc) {
  if (!visible())
    return;

  pp::Rect draw_rc = rc.Intersect(rect());
  if (draw_rc.IsEmpty())
    return;

  pp::Point origin = draw_rc.point();
  draw_rc.Offset(-rect().x(), -rect().y());

  AlphaBlend(GetCurrentImage(), draw_rc, image_data, origin, transparency());
}

bool Button::HandleEvent(const pp::InputEvent& event) {
  if (!visible())
    return false;

  // Button handles mouse events only.
  pp::MouseInputEvent mouse_event(event);
   if (mouse_event.is_null())
    return false;

  pp::Point pt = mouse_event.GetPosition();
  if (!rect().Contains(pt) ||
      event.GetType() == PP_INPUTEVENT_TYPE_MOUSELEAVE) {
    ChangeState(BUTTON_NORMAL, false);
    owner()->SetEventCapture(id(), false);
    return false;
  }

  owner()->SetCursor(id(), PP_CURSORTYPE_POINTER);
  owner()->SetEventCapture(id(), true);

  bool handled = true;
  switch (event.GetType()) {
    case PP_INPUTEVENT_TYPE_MOUSEMOVE:
      if (state_ == BUTTON_NORMAL)
        ChangeState(BUTTON_HIGHLIGHTED, false);
      break;
    case PP_INPUTEVENT_TYPE_MOUSEDOWN:
      if (mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_LEFT) {
        ChangeState(BUTTON_PRESSED, false);
        is_pressed_ = true;
      }
      break;
    case PP_INPUTEVENT_TYPE_MOUSEUP:
      if (mouse_event.GetButton() == PP_INPUTEVENT_MOUSEBUTTON_LEFT &&
          is_pressed_) {
        OnButtonClicked();
        is_pressed_ = false;
      } else {
        // Since button has not been pressed, return false to allow other
        // controls (scrollbar) to process mouse button up.
        return false;
      }
      break;
    default:
      handled = false;
      break;
  }

  return handled;
}

void Button::OnEventCaptureReleased() {
  ChangeState(BUTTON_NORMAL, false);
}

void Button::Show(bool visible, bool invalidate) {
  // If button become invisible, remove pressed flag.
  if (!visible)
    is_pressed_ = false;
  Control::Show(visible, invalidate);
}

void Button::AdjustTransparency(uint8 transparency, bool invalidate) {
  // If button become invisible, remove pressed flag.
  if (transparency == kTransparentAlpha)
    is_pressed_ = false;
  Control::AdjustTransparency(transparency, invalidate);
}

void Button::SetPressedState(bool pressed) {
  if (style_ == BUTTON_STATE) {
    if (IsPressed() != pressed)
      ChangeState(pressed ? BUTTON_PRESSED_STICKY : BUTTON_NORMAL, true);
  }
}

const pp::ImageData& Button::GetCurrentImage() {
  switch (state_) {
    case BUTTON_NORMAL: return normal_;
    case BUTTON_HIGHLIGHTED: return highlighted_;
    case BUTTON_PRESSED:
    case BUTTON_PRESSED_STICKY: return pressed_;
  }
  NOTREACHED();
  return normal_;
}

void Button::ChangeState(ButtonState new_state, bool force) {
  if (style_ == BUTTON_STATE && !force) {
    // If button is a state button and pressed state is sticky,
    // user have to click on this button again to unpress it.
    if ((state_ == BUTTON_PRESSED_STICKY && new_state != BUTTON_PRESSED_STICKY)
        ||
        (state_ != BUTTON_PRESSED_STICKY && new_state == BUTTON_PRESSED_STICKY))
      return;
  }

  if (state_ != new_state) {
    state_ = new_state;
    owner()->Invalidate(id(), rect());
  }
}

void Button::OnButtonClicked() {
  switch (style_) {
    case BUTTON_CLICKABLE:
      ChangeState(BUTTON_HIGHLIGHTED, true);
      owner()->OnEvent(id(), EVENT_ID_BUTTON_CLICKED, NULL);
      break;
    case BUTTON_STATE:
      SetPressedState(!IsPressed());
      owner()->OnEvent(id(), EVENT_ID_BUTTON_STATE_CHANGED, NULL);
      break;
    default:
      NOTREACHED();
  }
}

}  // namespace chrome_pdf


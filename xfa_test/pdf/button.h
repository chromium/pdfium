// Copyright (c) 2010 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef PDF_BUTTON_H_
#define PDF_BUTTON_H_

#include "pdf/control.h"
#include "ppapi/cpp/image_data.h"
#include "ppapi/cpp/rect.h"

namespace chrome_pdf {

class Button : public Control {
 public:
  enum ButtonEventIds {
    EVENT_ID_BUTTON_CLICKED,
    EVENT_ID_BUTTON_STATE_CHANGED,
  };

  enum ButtonStyle {
    BUTTON_CLICKABLE,
    BUTTON_STATE
  };

  enum ButtonState {
    BUTTON_NORMAL,
    BUTTON_HIGHLIGHTED,
    BUTTON_PRESSED,
    BUTTON_PRESSED_STICKY,
  };

  Button();
  virtual ~Button();
  virtual bool CreateButton(uint32 id,
                            const pp::Point& origin,
                            bool visible,
                            Control::Owner* delegate,
                            ButtonStyle style,
                            const pp::ImageData& face_normal,
                            const pp::ImageData& face_highlighted,
                            const pp::ImageData& face_pressed);

  virtual void Paint(pp::ImageData* image_data, const pp::Rect& rc);
  virtual bool HandleEvent(const pp::InputEvent& event);
  virtual void OnEventCaptureReleased();
  virtual void Show(bool visible, bool invalidate);
  virtual void AdjustTransparency(uint8 transparency, bool invalidate);

  ButtonState state() const { return state_; }
  bool IsPressed() const { return state() == BUTTON_PRESSED_STICKY; }
  void SetPressedState(bool pressed);

 private:
  void OnButtonClicked();

  const pp::ImageData& GetCurrentImage();
  void ChangeState(ButtonState new_state, bool force);

  ButtonStyle style_;
  ButtonState state_;
  bool is_pressed_;

  pp::ImageData normal_;
  pp::ImageData highlighted_;
  pp::ImageData pressed_;
};

}  // namespace chrome_pdf

#endif  // PDF_BUTTON_H_

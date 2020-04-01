// Copyright 2018 The PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "samples/pdfium_test_event_helper.h"

#include <stdio.h>

#include <string>
#include <vector>

#include "public/fpdf_fwlevent.h"
#include "public/fpdfview.h"
#include "testing/fx_string_testhelpers.h"

namespace {
void SendCharCodeEvent(FPDF_FORMHANDLE form,
                       FPDF_PAGE page,
                       const std::vector<std::string>& tokens) {
  if (tokens.size() != 2) {
    fprintf(stderr, "charcode: bad args\n");
    return;
  }

  int keycode = atoi(tokens[1].c_str());
  FORM_OnChar(form, page, keycode, 0);
}

void SendKeyCodeEvent(FPDF_FORMHANDLE form,
                      FPDF_PAGE page,
                      const std::vector<std::string>& tokens) {
  if (tokens.size() != 2) {
    fprintf(stderr, "keycode: bad args\n");
    return;
  }

  int keycode = atoi(tokens[1].c_str());
  FORM_OnKeyDown(form, page, keycode, 0);
  FORM_OnKeyUp(form, page, keycode, 0);
}

uint32_t GetModifiers(std::string modifiers_string) {
  int modifiers = 0;
  if (modifiers_string.find("shift") != std::string::npos)
    modifiers |= FWL_EVENTFLAG_ShiftKey;
  if (modifiers_string.find("control") != std::string::npos)
    modifiers |= FWL_EVENTFLAG_ControlKey;
  if (modifiers_string.find("alt") != std::string::npos)
    modifiers |= FWL_EVENTFLAG_AltKey;

  return modifiers;
}

void SendMouseDownEvent(FPDF_FORMHANDLE form,
                        FPDF_PAGE page,
                        const std::vector<std::string>& tokens) {
  if (tokens.size() != 4 && tokens.size() != 5) {
    fprintf(stderr, "mousedown: bad args\n");
    return;
  }

  int x = atoi(tokens[2].c_str());
  int y = atoi(tokens[3].c_str());
  uint32_t modifiers = tokens.size() >= 5 ? GetModifiers(tokens[4]) : 0;

  if (tokens[1] == "left")
    FORM_OnLButtonDown(form, page, modifiers, x, y);
  else if (tokens[1] == "right")
    FORM_OnRButtonDown(form, page, modifiers, x, y);
  else
    fprintf(stderr, "mousedown: bad button name\n");
}

void SendMouseUpEvent(FPDF_FORMHANDLE form,
                      FPDF_PAGE page,
                      const std::vector<std::string>& tokens) {
  if (tokens.size() != 4 && tokens.size() != 5) {
    fprintf(stderr, "mouseup: bad args\n");
    return;
  }

  int x = atoi(tokens[2].c_str());
  int y = atoi(tokens[3].c_str());
  int modifiers = tokens.size() >= 5 ? GetModifiers(tokens[4]) : 0;
  if (tokens[1] == "left")
    FORM_OnLButtonUp(form, page, modifiers, x, y);
  else if (tokens[1] == "right")
    FORM_OnRButtonUp(form, page, modifiers, x, y);
  else
    fprintf(stderr, "mouseup: bad button name\n");
}

void SendMouseDoubleClickEvent(FPDF_FORMHANDLE form,
                               FPDF_PAGE page,
                               const std::vector<std::string>& tokens) {
  if (tokens.size() != 4 && tokens.size() != 5) {
    fprintf(stderr, "mousedoubleclick: bad args\n");
    return;
  }

  int x = atoi(tokens[2].c_str());
  int y = atoi(tokens[3].c_str());
  int modifiers = tokens.size() >= 5 ? GetModifiers(tokens[4]) : 0;
  if (tokens[1] != "left") {
    fprintf(stderr, "mousedoubleclick: bad button name\n");
    return;
  }
  FORM_OnLButtonDoubleClick(form, page, modifiers, x, y);
}

void SendMouseMoveEvent(FPDF_FORMHANDLE form,
                        FPDF_PAGE page,
                        const std::vector<std::string>& tokens) {
  if (tokens.size() != 3) {
    fprintf(stderr, "mousemove: bad args\n");
    return;
  }

  int x = atoi(tokens[1].c_str());
  int y = atoi(tokens[2].c_str());
  FORM_OnMouseMove(form, page, 0, x, y);
}

void SendFocusEvent(FPDF_FORMHANDLE form,
                    FPDF_PAGE page,
                    const std::vector<std::string>& tokens) {
  if (tokens.size() != 3) {
    fprintf(stderr, "focus: bad args\n");
    return;
  }

  int x = atoi(tokens[1].c_str());
  int y = atoi(tokens[2].c_str());
  FORM_OnFocus(form, page, 0, x, y);
}
}  // namespace

void SendPageEvents(FPDF_FORMHANDLE form,
                    FPDF_PAGE page,
                    const std::string& events) {
  auto lines = StringSplit(events, '\n');
  for (const auto& line : lines) {
    auto command = StringSplit(line, '#');
    if (command[0].empty())
      continue;
    auto tokens = StringSplit(command[0], ',');
    if (tokens[0] == "charcode") {
      SendCharCodeEvent(form, page, tokens);
    } else if (tokens[0] == "keycode") {
      SendKeyCodeEvent(form, page, tokens);
    } else if (tokens[0] == "mousedown") {
      SendMouseDownEvent(form, page, tokens);
    } else if (tokens[0] == "mouseup") {
      SendMouseUpEvent(form, page, tokens);
    } else if (tokens[0] == "mousedoubleclick") {
      SendMouseDoubleClickEvent(form, page, tokens);
    } else if (tokens[0] == "mousemove") {
      SendMouseMoveEvent(form, page, tokens);
    } else if (tokens[0] == "focus") {
      SendFocusEvent(form, page, tokens);
    } else {
      fprintf(stderr, "Unrecognized event: %s\n", tokens[0].c_str());
    }
  }
}

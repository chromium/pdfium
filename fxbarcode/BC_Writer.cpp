// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxbarcode/BC_Writer.h"

CBC_Writer::CBC_Writer() = default;

CBC_Writer::~CBC_Writer() = default;

bool CBC_Writer::SetModuleHeight(int32_t moduleHeight) {
  if (moduleHeight > 10 || moduleHeight < 1) {
    return false;
  }

  module_height_ = moduleHeight;
  return true;
}

bool CBC_Writer::SetModuleWidth(int32_t moduleWidth) {
  if (moduleWidth > 10 || moduleWidth < 1) {
    return false;
  }

  module_width_ = moduleWidth;
  return true;
}

void CBC_Writer::SetHeight(int32_t height) {
  height_ = height;
}

void CBC_Writer::SetWidth(int32_t width) {
  width_ = width;
}

void CBC_Writer::SetTextLocation(BC_TEXT_LOC location) {}

bool CBC_Writer::SetWideNarrowRatio(int8_t ratio) {
  return false;
}

bool CBC_Writer::SetStartChar(char start) {
  return false;
}

bool CBC_Writer::SetEndChar(char end) {
  return false;
}

bool CBC_Writer::SetErrorCorrectionLevel(int32_t level) {
  return false;
}

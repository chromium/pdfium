// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cxfa_color.h"

CXFA_Color::CXFA_Color() : m_type(FX_COLOR_None) {}

CXFA_Color::CXFA_Color(const FX_ARGB argb) {
  Set(argb);
}

CXFA_Color::CXFA_Color(CXFA_Pattern* pattern, const FX_ARGB argb) {
  Set(pattern, argb);
}

CXFA_Color::CXFA_Color(CXFA_Shading* shading) {
  Set(shading);
}

CXFA_Color::~CXFA_Color() {
  m_type = FX_COLOR_None;
}

void CXFA_Color::Set(const FX_ARGB argb) {
  m_type = FX_COLOR_Solid;
  m_info.argb = argb;
  m_info.pattern = nullptr;
}

void CXFA_Color::Set(CXFA_Pattern* pattern, const FX_ARGB argb) {
  if (!pattern)
    return;
  m_type = FX_COLOR_Pattern;
  m_info.argb = argb;
  m_info.pattern = pattern;
}

void CXFA_Color::Set(CXFA_Shading* shading) {
  if (!shading)
    return;
  m_type = FX_COLOR_Shading;
  m_shading = shading;
}

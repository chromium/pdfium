// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxgraphics/cxfa_color.h"

CXFA_Color::CXFA_Color() : m_type(Invalid) {}

CXFA_Color::CXFA_Color(const FX_ARGB argb) : m_type(Solid), m_argb(argb) {
  m_pointer.pattern = nullptr;
}

CXFA_Color::CXFA_Color(CXFA_Pattern* pattern, const FX_ARGB argb)
    : m_type(Pattern), m_argb(argb) {
  m_pointer.pattern = pattern;
}

CXFA_Color::CXFA_Color(CXFA_Shading* shading) : m_type(Shading), m_argb(0) {
  m_pointer.shading = shading;
}

CXFA_Color::~CXFA_Color() {}

CXFA_Color& CXFA_Color::operator=(const CXFA_Color& that) {
  if (this != &that) {
    m_type = that.m_type;
    switch (m_type) {
      case Solid:
        m_argb = that.m_argb;
        m_pointer.pattern = nullptr;
        break;
      case Pattern:
        m_argb = that.m_argb;
        m_pointer.pattern = that.m_pointer.pattern;
        break;
      case Shading:
        m_argb = 0;
        m_pointer.shading = that.m_pointer.shading;
      default:
        break;
    }
  }
  return *this;
}

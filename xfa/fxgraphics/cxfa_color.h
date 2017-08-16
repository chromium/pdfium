// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CXFA_COLOR_H_
#define XFA_FXGRAPHICS_CXFA_COLOR_H_

#include "core/fxge/fx_dib.h"

class CXFA_Pattern;
class CXFA_Shading;

class CXFA_Color {
 public:
  enum Type { Invalid, Solid, Pattern, Shading };

  CXFA_Color();
  explicit CXFA_Color(const FX_ARGB argb);
  explicit CXFA_Color(CXFA_Shading* shading);
  CXFA_Color(CXFA_Pattern* pattern, const FX_ARGB argb);
  ~CXFA_Color();

  Type GetType() const { return m_type; }
  FX_ARGB GetArgb() const {
    ASSERT(m_type == Solid || m_type == Pattern);
    return m_argb;
  }
  CXFA_Pattern* GetPattern() const {
    ASSERT(m_type == Pattern);
    return m_pointer.pattern;
  }
  CXFA_Shading* GetShading() const {
    ASSERT(m_type == Shading);
    return m_pointer.shading;
  }

  CXFA_Color& operator=(const CXFA_Color& that);

 private:
  Type m_type;
  FX_ARGB m_argb;
  union {
    CXFA_Pattern* pattern;
    CXFA_Shading* shading;
  } m_pointer;
};

#endif  // XFA_FXGRAPHICS_CXFA_COLOR_H_

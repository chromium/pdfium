// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CXFA_COLOR_H_
#define XFA_FXGRAPHICS_CXFA_COLOR_H_

#include "core/fxge/fx_dib.h"
#include "xfa/fxgraphics/cxfa_graphics.h"

class CXFA_Pattern;
class CXFA_Shading;

enum { FX_COLOR_None = 0, FX_COLOR_Solid, FX_COLOR_Pattern, FX_COLOR_Shading };

class CXFA_Color {
 public:
  CXFA_Color();
  explicit CXFA_Color(const FX_ARGB argb);
  explicit CXFA_Color(CXFA_Shading* shading);
  CXFA_Color(CXFA_Pattern* pattern, const FX_ARGB argb);
  virtual ~CXFA_Color();

  void Set(const FX_ARGB argb);
  void Set(CXFA_Pattern* pattern, const FX_ARGB argb);
  void Set(CXFA_Shading* shading);

 private:
  friend class CXFA_Graphics;

  int32_t m_type;
  union {
    struct {
      FX_ARGB argb;
      CXFA_Pattern* pattern;
    } m_info;
    CXFA_Shading* m_shading;
  };
};

#endif  // XFA_FXGRAPHICS_CXFA_COLOR_H_

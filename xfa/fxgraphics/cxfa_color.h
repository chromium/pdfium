// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXGRAPHICS_CXFA_COLOR_H_
#define XFA_FXGRAPHICS_CXFA_COLOR_H_

#include "core/fxge/fx_dib.h"
#include "xfa/fxgraphics/cfx_graphics.h"

class CFX_Pattern;
class CFX_Shading;

enum { FX_COLOR_None = 0, FX_COLOR_Solid, FX_COLOR_Pattern, FX_COLOR_Shading };

class CXFA_Color {
 public:
  CXFA_Color();
  explicit CXFA_Color(const FX_ARGB argb);
  explicit CXFA_Color(CFX_Shading* shading);
  CXFA_Color(CFX_Pattern* pattern, const FX_ARGB argb);
  virtual ~CXFA_Color();

  void Set(const FX_ARGB argb);
  void Set(CFX_Pattern* pattern, const FX_ARGB argb);
  void Set(CFX_Shading* shading);

 private:
  friend class CFX_Graphics;

  int32_t m_type;
  union {
    struct {
      FX_ARGB argb;
      CFX_Pattern* pattern;
    } m_info;
    CFX_Shading* m_shading;
  };
};

#endif  // XFA_FXGRAPHICS_CXFA_COLOR_H_

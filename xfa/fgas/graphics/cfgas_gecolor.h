// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GECOLOR_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GECOLOR_H_

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "third_party/base/check.h"
#include "third_party/base/check_op.h"

class CFGAS_GEPattern;
class CFGAS_GEShading;

class CFGAS_GEColor {
 public:
  enum Type { Invalid, Solid, Pattern, Shading };

  explicit CFGAS_GEColor(const FX_ARGB argb);
  explicit CFGAS_GEColor(CFGAS_GEShading* shading);
  CFGAS_GEColor(CFGAS_GEPattern* pattern, const FX_ARGB argb);
  CFGAS_GEColor(const CFGAS_GEColor& that);
  ~CFGAS_GEColor();

  Type GetType() const { return m_type; }
  FX_ARGB GetArgb() const {
    DCHECK(m_type == Solid || m_type == Pattern);
    return m_argb;
  }
  CFGAS_GEPattern* GetPattern() const {
    DCHECK_EQ(m_type, Pattern);
    return m_pPattern;
  }
  CFGAS_GEShading* GetShading() const {
    DCHECK_EQ(m_type, Shading);
    return m_pShading;
  }

  CFGAS_GEColor& operator=(const CFGAS_GEColor& that);

 private:
  Type m_type = Invalid;
  FX_ARGB m_argb = 0;
  UnownedPtr<CFGAS_GEPattern> m_pPattern;
  UnownedPtr<CFGAS_GEShading> m_pShading;
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GECOLOR_H_

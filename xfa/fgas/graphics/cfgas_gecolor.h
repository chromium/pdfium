// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_GRAPHICS_CFGAS_GECOLOR_H_
#define XFA_FGAS_GRAPHICS_CFGAS_GECOLOR_H_

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"

class CFGAS_GEPattern;
class CFGAS_GEShading;

class CFGAS_GEColor {
 public:
  enum Type { Invalid, Solid, Pattern, Shading };

  explicit CFGAS_GEColor(FX_ARGB argb);
  explicit CFGAS_GEColor(CFGAS_GEShading* shading);
  CFGAS_GEColor(CFGAS_GEPattern* pattern, FX_ARGB argb);
  CFGAS_GEColor(const CFGAS_GEColor& that);
  ~CFGAS_GEColor();

  Type GetType() const { return type_; }
  FX_ARGB GetArgb() const {
    DCHECK(type_ == Solid || type_ == Pattern);
    return argb_;
  }
  CFGAS_GEPattern* GetPattern() const {
    DCHECK_EQ(type_, Pattern);
    return pattern_;
  }
  CFGAS_GEShading* GetShading() const {
    DCHECK_EQ(type_, Shading);
    return shading_;
  }

  CFGAS_GEColor& operator=(const CFGAS_GEColor& that);

  static ByteString ColorToString(FX_ARGB argb);

 private:
  Type type_ = Invalid;
  FX_ARGB argb_ = 0;
  UnownedPtr<CFGAS_GEPattern> pattern_;
  UnownedPtr<CFGAS_GEShading> shading_;
};

#endif  // XFA_FGAS_GRAPHICS_CFGAS_GECOLOR_H_

// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FDE_IFDE_VISUALSET_H_
#define XFA_FDE_IFDE_VISUALSET_H_

#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fde/cfde_path.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

class FXTEXT_CHARPOS;

enum FDE_VISUALOBJTYPE {
  FDE_VISUALOBJ_Canvas = 0x00,
  FDE_VISUALOBJ_Text = 0x01
};

struct FDE_TEXTEDITPIECE {
  FDE_TEXTEDITPIECE();
  FDE_TEXTEDITPIECE(const FDE_TEXTEDITPIECE& that);
  ~FDE_TEXTEDITPIECE();

  int32_t nStart;
  int32_t nCount;
  int32_t nBidiLevel;
  CFX_RectF rtPiece;
  uint32_t dwCharStyles;
};
inline FDE_TEXTEDITPIECE::FDE_TEXTEDITPIECE() = default;
inline FDE_TEXTEDITPIECE::FDE_TEXTEDITPIECE(const FDE_TEXTEDITPIECE& that) =
    default;
inline FDE_TEXTEDITPIECE::~FDE_TEXTEDITPIECE() = default;

class IFDE_VisualSet {
 public:
  virtual ~IFDE_VisualSet() {}
  virtual FDE_VISUALOBJTYPE GetType() = 0;
  virtual CFX_RectF GetRect(const FDE_TEXTEDITPIECE& hVisualObj) = 0;
};

#endif  // XFA_FDE_IFDE_VISUALSET_H_

// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_SUBSTFONT_H_
#define CORE_FXGE_CFX_SUBSTFONT_H_

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_codepage.h"

class CFX_SubstFont {
 public:
  CFX_SubstFont();
  ~CFX_SubstFont();

#if defined(PDF_USE_SKIA)
  int GetOriginalWeight() const;
#endif
  void UseChromeSerif();

  void SetIsBuiltInGenericFont() { flag_mm_ = true; }
  bool IsBuiltInGenericFont() const { return flag_mm_; }

  ByteString family_;
  FX_Charset charset_ = FX_Charset::kANSI;
  int weight_ = 0;
  int italic_angle_ = 0;
  int weight_cjk_ = 0;
  bool subst_cjk_ = false;
  bool italic_cjk_ = false;

 private:
  bool flag_mm_ = false;
};

#endif  // CORE_FXGE_CFX_SUBSTFONT_H_

// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_GLYPHBITMAP_H_
#define CORE_FXGE_CFX_GLYPHBITMAP_H_

#include "core/fxcrt/retain_ptr.h"

class CFX_DIBitmap;

class CFX_GlyphBitmap {
 public:
  CFX_GlyphBitmap(int left, int top);
  ~CFX_GlyphBitmap();

  CFX_GlyphBitmap(const CFX_GlyphBitmap&) = delete;
  CFX_GlyphBitmap& operator=(const CFX_GlyphBitmap&) = delete;

  const RetainPtr<CFX_DIBitmap>& GetBitmap() const { return bitmap_; }
  int left() const { return left_; }
  int top() const { return top_; }

 private:
  const int left_;
  const int top_;
  RetainPtr<CFX_DIBitmap> bitmap_;
};

#endif  // CORE_FXGE_CFX_GLYPHBITMAP_H_

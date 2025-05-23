// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_
#define CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_

#include <stdint.h>

#include "core/fxcrt/bytestring.h"

class CFPF_SkiaPathFont {
 public:
  CFPF_SkiaPathFont(const ByteString& path,
                    const ByteString& family,
                    uint32_t dwStyle,
                    int32_t iFaceIndex,
                    uint32_t dwCharsets,
                    int32_t iGlyphNum);
  ~CFPF_SkiaPathFont();

  const char* path() const { return path_.c_str(); }
  const char* family() const { return family_.c_str(); }
  uint32_t style() const { return style_; }
  int32_t face_index() const { return face_index_; }
  uint32_t charsets() const { return charsets_; }
  int32_t glyph_num() const { return glyph_num_; }

 private:
  const ByteString path_;
  const ByteString family_;
  const uint32_t style_;
  const int32_t face_index_;
  const uint32_t charsets_;
  const int32_t glyph_num_;
};

#endif  // CORE_FXGE_ANDROID_CFPF_SKIAPATHFONT_H_

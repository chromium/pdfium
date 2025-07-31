// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_RENDER_CPDF_TYPE3GLYPHMAP_H_
#define CORE_FPDFAPI_RENDER_CPDF_TYPE3GLYPHMAP_H_

#include <stdint.h>

#include <map>
#include <memory>
#include <utility>
#include <vector>

class CFX_GlyphBitmap;

class CPDF_Type3GlyphMap {
 public:
  CPDF_Type3GlyphMap();
  ~CPDF_Type3GlyphMap();

  // Returns a pair of integers (top_line, bottom_line).
  std::pair<int, int> AdjustBlue(float top, float bottom);

  const CFX_GlyphBitmap* GetBitmap(uint32_t charcode) const;
  void SetBitmap(uint32_t charcode, std::unique_ptr<CFX_GlyphBitmap> bitmap);

 private:
  std::vector<int> top_blue_;
  std::vector<int> bottom_blue_;
  std::map<uint32_t, std::unique_ptr<CFX_GlyphBitmap>> glyph_map_;
};

#endif  // CORE_FPDFAPI_RENDER_CPDF_TYPE3GLYPHMAP_H_

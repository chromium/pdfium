// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_UTILS_BITMAP_SAVER_H_
#define TESTING_UTILS_BITMAP_SAVER_H_

#include <string>

#include "public/fpdfview.h"

class CFX_DIBitmap;

class BitmapSaver {
 public:
  static void WriteBitmapToPng(FPDF_BITMAP bitmap, const std::string& filename);
  static void WriteBitmapToPng(CFX_DIBitmap* bitmap,
                               const std::string& filename);
};

#endif  // TESTING_UTILS_BITMAP_SAVER_H_

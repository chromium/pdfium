// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "testing/utils/bitmap_saver.h"

#include <fstream>
#include <vector>

#include "testing/gtest/include/gtest/gtest.h"
#include "testing/image_diff/image_diff_png.h"

// static
void BitmapSaver::WriteBitmapToPng(FPDF_BITMAP bitmap,
                                   const std::string& filename) {
  const int stride = FPDFBitmap_GetStride(bitmap);
  const int width = FPDFBitmap_GetWidth(bitmap);
  const int height = FPDFBitmap_GetHeight(bitmap);
  const auto* buffer =
      static_cast<const unsigned char*>(FPDFBitmap_GetBuffer(bitmap));

  std::vector<unsigned char> png_encoding;
  bool encoded;
  if (FPDFBitmap_GetFormat(bitmap) == FPDFBitmap_Gray) {
    encoded = image_diff_png::EncodeGrayPNG(buffer, width, height, stride,
                                            &png_encoding);
  } else {
    encoded = image_diff_png::EncodeBGRAPNG(buffer, width, height, stride,
                                            /*discard_transparency=*/false,
                                            &png_encoding);
  }

  ASSERT_TRUE(encoded);
  ASSERT_LT(filename.size(), 256u);

  std::ofstream png_file;
  png_file.open(filename, std::ios_base::out | std::ios_base::binary);
  png_file.write(reinterpret_cast<char*>(&png_encoding.front()),
                 png_encoding.size());
  ASSERT_TRUE(png_file.good());
  png_file.close();
}

// static
void BitmapSaver::WriteBitmapToPng(CFX_DIBitmap* bitmap,
                                   const std::string& filename) {
  WriteBitmapToPng(reinterpret_cast<FPDF_BITMAP>(bitmap), filename);
}

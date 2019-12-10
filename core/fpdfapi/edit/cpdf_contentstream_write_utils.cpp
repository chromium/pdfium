// Copyright 2019 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_contentstream_write_utils.h"

#include "third_party/skia_shared/SkFloatToDecimal.h"

std::ostream& WriteFloat(std::ostream& stream, float value) {
  char buffer[pdfium::skia::kMaximumSkFloatToDecimalLength];
  unsigned size = pdfium::skia::SkFloatToDecimal(value, buffer);
  stream.write(buffer, size);
  return stream;
}

std::ostream& operator<<(std::ostream& ar, const CFX_Matrix& matrix) {
  WriteFloat(ar, matrix.a) << " ";
  WriteFloat(ar, matrix.b) << " ";
  WriteFloat(ar, matrix.c) << " ";
  WriteFloat(ar, matrix.d) << " ";
  WriteFloat(ar, matrix.e) << " ";
  WriteFloat(ar, matrix.f);
  return ar;
}

std::ostream& operator<<(std::ostream& ar, const CFX_PointF& point) {
  WriteFloat(ar, point.x) << " ";
  WriteFloat(ar, point.y);
  return ar;
}

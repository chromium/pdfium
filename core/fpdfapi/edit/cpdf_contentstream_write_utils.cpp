// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "core/fpdfapi/edit/cpdf_contentstream_write_utils.h"

#include <ostream>

#include "third_party/skia_shared/SkFloatToDecimal.h"

std::ostream& WriteFloat(std::ostream& stream, float value) {
  char buffer[pdfium::skia::kMaximumSkFloatToDecimalLength];
  unsigned size = pdfium::skia::SkFloatToDecimal(value, buffer);
  stream.write(buffer, size);
  return stream;
}

std::ostream& WriteMatrix(std::ostream& stream, const CFX_Matrix& matrix) {
  WriteFloat(stream, matrix.a) << " ";
  WriteFloat(stream, matrix.b) << " ";
  WriteFloat(stream, matrix.c) << " ";
  WriteFloat(stream, matrix.d) << " ";
  WriteFloat(stream, matrix.e) << " ";
  WriteFloat(stream, matrix.f);
  return stream;
}

std::ostream& WritePoint(std::ostream& stream, const CFX_PointF& point) {
  WriteFloat(stream, point.x) << " ";
  WriteFloat(stream, point.y);
  return stream;
}

std::ostream& WriteRect(std::ostream& stream, const CFX_FloatRect& rect) {
  WriteFloat(stream, rect.left) << " ";
  WriteFloat(stream, rect.bottom) << " ";
  WriteFloat(stream, rect.Width()) << " ";
  WriteFloat(stream, rect.Height());
  return stream;
}

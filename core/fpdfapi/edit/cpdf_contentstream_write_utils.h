// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_EDIT_CPDF_CONTENTSTREAM_WRITE_UTILS_H_
#define CORE_FPDFAPI_EDIT_CPDF_CONTENTSTREAM_WRITE_UTILS_H_

#include <iosfwd>

#include "core/fxcrt/fx_coordinates.h"

std::ostream& WriteFloat(std::ostream& stream, float value);
std::ostream& WriteMatrix(std::ostream& stream, const CFX_Matrix& matrix);
std::ostream& WritePoint(std::ostream& stream, const CFX_PointF& point);
std::ostream& WriteRect(std::ostream& stream, const CFX_FloatRect& rect);

#endif  // CORE_FPDFAPI_EDIT_CPDF_CONTENTSTREAM_WRITE_UTILS_H_

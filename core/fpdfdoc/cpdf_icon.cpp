// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_icon.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"

CPDF_Icon::CPDF_Icon(RetainPtr<const CPDF_Stream> pStream)
    : stream_(std::move(pStream)) {}

CPDF_Icon::~CPDF_Icon() = default;

CFX_SizeF CPDF_Icon::GetImageSize() const {
  CFX_FloatRect rect = stream_->GetDict()->GetRectFor("BBox");
  return {rect.right - rect.left, rect.top - rect.bottom};
}

CFX_Matrix CPDF_Icon::GetImageMatrix() const {
  return stream_->GetDict()->GetMatrixFor("Matrix");
}

ByteString CPDF_Icon::GetImageAlias() const {
  return stream_->GetDict()->GetByteStringFor("Name");
}

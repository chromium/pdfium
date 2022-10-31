// Copyright 2019 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfdoc/cpdf_icon.h"

#include <utility>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_stream.h"

CPDF_Icon::CPDF_Icon(RetainPtr<const CPDF_Stream> pStream)
    : m_pStream(std::move(pStream)) {}

CPDF_Icon::~CPDF_Icon() = default;

CFX_SizeF CPDF_Icon::GetImageSize() const {
  RetainPtr<const CPDF_Dictionary> pDict = m_pStream->GetDict();
  if (!pDict)
    return CFX_SizeF();

  CFX_FloatRect rect = pDict->GetRectFor("BBox");
  return {rect.right - rect.left, rect.top - rect.bottom};
}

CFX_Matrix CPDF_Icon::GetImageMatrix() const {
  RetainPtr<const CPDF_Dictionary> pDict = m_pStream->GetDict();
  if (!pDict)
    return CFX_Matrix();

  return pDict->GetMatrixFor("Matrix");
}

ByteString CPDF_Icon::GetImageAlias() const {
  RetainPtr<const CPDF_Dictionary> pDict = m_pStream->GetDict();
  if (!pDict)
    return ByteString();

  return pDict->GetByteStringFor("Name");
}

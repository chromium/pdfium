// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/cpdfsdk_customaccess.h"

#include "core/fxcrt/fx_safe_types.h"

CPDFSDK_CustomAccess::CPDFSDK_CustomAccess(FPDF_FILEACCESS* pFileAccess)
    : m_FileAccess(*pFileAccess) {}

CPDFSDK_CustomAccess::~CPDFSDK_CustomAccess() = default;

FX_FILESIZE CPDFSDK_CustomAccess::GetSize() {
  return m_FileAccess.m_FileLen;
}

bool CPDFSDK_CustomAccess::ReadBlockAtOffset(void* buffer,
                                             FX_FILESIZE offset,
                                             size_t size) {
  if (!buffer || offset < 0 || !size)
    return false;

  if (!pdfium::base::IsValueInRangeForNumericType<FX_FILESIZE>(size))
    return false;

  FX_SAFE_FILESIZE new_pos = size;
  new_pos += offset;
  return new_pos.IsValid() && new_pos.ValueOrDie() <= GetSize() &&
         m_FileAccess.m_GetBlock(m_FileAccess.m_Param, offset,
                                 static_cast<uint8_t*>(buffer), size);
}

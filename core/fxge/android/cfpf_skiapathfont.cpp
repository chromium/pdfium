// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fxge/android/cfpf_skiapathfont.h"

#include "core/fxcrt/fx_memory.h"

CFPF_SkiaPathFont::CFPF_SkiaPathFont() {}

CFPF_SkiaPathFont::~CFPF_SkiaPathFont() {
  FX_Free(m_pFamily);
  FX_Free(m_pPath);
}

void CFPF_SkiaPathFont::SetPath(const char* pPath) {
  FX_Free(m_pPath);
  int32_t iSize = strlen(pPath);
  m_pPath = FX_Alloc(char, iSize + 1);
  memcpy(m_pPath, pPath, iSize * sizeof(char));
  m_pPath[iSize] = 0;
}

void CFPF_SkiaPathFont::SetFamily(const char* pFamily) {
  FX_Free(m_pFamily);
  int32_t iSize = strlen(pFamily);
  m_pFamily = FX_Alloc(char, iSize + 1);
  memcpy(m_pFamily, pFamily, iSize * sizeof(char));
  m_pFamily[iSize] = 0;
}

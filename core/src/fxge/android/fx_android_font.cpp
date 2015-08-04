// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fx_fpf.h"
#if _FX_OS_ == _FX_ANDROID_
CFX_AndroidFontInfo::CFX_AndroidFontInfo() : m_pFontMgr(NULL) {}
FX_BOOL CFX_AndroidFontInfo::Init(IFPF_FontMgr* pFontMgr) {
  if (!pFontMgr) {
    return FALSE;
  }
  pFontMgr->LoadSystemFonts();
  m_pFontMgr = pFontMgr;
  return TRUE;
}
FX_BOOL CFX_AndroidFontInfo::EnumFontList(CFX_FontMapper* pMapper) {
  return FALSE;
}
void* CFX_AndroidFontInfo::MapFont(int weight,
                                   FX_BOOL bItalic,
                                   int charset,
                                   int pitch_family,
                                   const FX_CHAR* face,
                                   int& iExact) {
  if (!m_pFontMgr) {
    return NULL;
  }
  FX_DWORD dwStyle = 0;
  if (weight >= 700) {
    dwStyle |= FXFONT_BOLD;
  }
  if (bItalic) {
    dwStyle |= FXFONT_ITALIC;
  }
  if (pitch_family & FXFONT_FF_FIXEDPITCH) {
    dwStyle |= FXFONT_FIXED_PITCH;
  }
  if (pitch_family & FXFONT_FF_SCRIPT) {
    dwStyle |= FXFONT_SCRIPT;
  }
  if (pitch_family & FXFONT_FF_ROMAN) {
    dwStyle |= FXFONT_SERIF;
  }
  return m_pFontMgr->CreateFont(face, charset, dwStyle,
                                FPF_MATCHFONT_REPLACEANSI);
}
void* CFX_AndroidFontInfo::GetFont(const FX_CHAR* face) {
  return NULL;
}
FX_DWORD CFX_AndroidFontInfo::GetFontData(void* hFont,
                                          FX_DWORD table,
                                          uint8_t* buffer,
                                          FX_DWORD size) {
  if (!hFont) {
    return 0;
  }
  return ((IFPF_Font*)hFont)->GetFontData(table, buffer, size);
}
FX_BOOL CFX_AndroidFontInfo::GetFaceName(void* hFont, CFX_ByteString& name) {
  if (!hFont) {
    return FALSE;
  }
  name = ((IFPF_Font*)hFont)->GetFamilyName();
  return TRUE;
}
FX_BOOL CFX_AndroidFontInfo::GetFontCharset(void* hFont, int& charset) {
  if (!hFont) {
    return FALSE;
  }
  charset = ((IFPF_Font*)hFont)->GetCharset();
  return FALSE;
}
void CFX_AndroidFontInfo::DeleteFont(void* hFont) {
  if (!hFont) {
    return;
  }
  ((IFPF_Font*)hFont)->Release();
}
void* CFX_AndroidFontInfo::RetainFont(void* hFont) {
  return NULL;
}
#endif

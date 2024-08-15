// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_LAYOUT_FGAS_ARABIC_H_
#define XFA_FGAS_LAYOUT_FGAS_ARABIC_H_

#include <optional>

class CFGAS_Char;

namespace pdfium {

constexpr wchar_t kArabicLetterLam = 0x0644;
constexpr wchar_t kArabicLetterHeh = 0x0647;
constexpr wchar_t kArabicShadda = 0x0651;
constexpr wchar_t kArabicLetterSuperscriptAlef = 0x0670;

wchar_t GetArabicFormChar(wchar_t wch, wchar_t prev, wchar_t next);
wchar_t GetArabicFormChar(const CFGAS_Char* cur,
                          const CFGAS_Char* prev,
                          const CFGAS_Char* next);
std::optional<wchar_t> GetArabicFromShaddaTable(wchar_t shadda);

}  // namespace pdfium

#endif  // XFA_FGAS_LAYOUT_FGAS_ARABIC_H_

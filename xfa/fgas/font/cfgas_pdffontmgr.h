// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_FONT_CFGAS_PDFFONTMGR_H_
#define XFA_FGAS_FONT_CFGAS_PDFFONTMGR_H_

#include <map>
#include <utility>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"

class CFGAS_GEFont;
class CPDF_Document;

class CFGAS_PDFFontMgr final {
 public:
  explicit CFGAS_PDFFontMgr(const CPDF_Document* pDoc);
  ~CFGAS_PDFFontMgr();

  RetainPtr<CFGAS_GEFont> GetFont(const WideString& wsFontFamily,
                                  uint32_t dwFontStyles,
                                  bool bStrictMatch);

 private:
  RetainPtr<CFGAS_GEFont> FindFont(const ByteString& strFamilyName,
                                   bool bBold,
                                   bool bItalic,
                                   bool bStrictMatch);

  UnownedPtr<const CPDF_Document> const m_pDoc;
  std::map<std::pair<WideString, uint32_t>, RetainPtr<CFGAS_GEFont>> m_FontMap;
};

#endif  // XFA_FGAS_FONT_CFGAS_PDFFONTMGR_H_

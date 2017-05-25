// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_PDFFONTMGR_H_
#define XFA_FXFA_CXFA_PDFFONTMGR_H_

#include <map>

#include "core/fpdfapi/parser/cpdf_dictionary.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/cfx_observable.h"
#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_string.h"
#include "xfa/fxfa/cxfa_ffdoc.h"

class CFGAS_GEFont;
class CPDF_Font;
class CXFA_FFDoc;

class CXFA_PDFFontMgr : public CFX_Observable<CXFA_PDFFontMgr> {
 public:
  explicit CXFA_PDFFontMgr(CXFA_FFDoc* pDoc);
  ~CXFA_PDFFontMgr();

  CFX_RetainPtr<CFGAS_GEFont> GetFont(const CFX_WideStringC& wsFontFamily,
                                      uint32_t dwFontStyles,
                                      CPDF_Font** pPDFFont,
                                      bool bStrictMatch);
  bool GetCharWidth(const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                    wchar_t wUnicode,
                    bool bCharCode,
                    int32_t* pWidth);
  void SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont, CPDF_Font* pPDFFont);

 private:
  CFX_RetainPtr<CFGAS_GEFont> FindFont(const CFX_ByteString& strFamilyName,
                                       bool bBold,
                                       bool bItalic,
                                       CPDF_Font** pPDFFont,
                                       bool bStrictMatch);
  CFX_ByteString PsNameToFontName(const CFX_ByteString& strPsName,
                                  bool bBold,
                                  bool bItalic);
  bool PsNameMatchDRFontName(const CFX_ByteStringC& bsPsName,
                             bool bBold,
                             bool bItalic,
                             const CFX_ByteString& bsDRFontName,
                             bool bStrictMatch);

  CFX_UnownedPtr<CXFA_FFDoc> const m_pDoc;
  std::map<CFX_RetainPtr<CFGAS_GEFont>, CPDF_Font*> m_FDE2PDFFont;
  std::map<CFX_ByteString, CFX_RetainPtr<CFGAS_GEFont>> m_FontMap;
};

#endif  // XFA_FXFA_CXFA_PDFFONTMGR_H_

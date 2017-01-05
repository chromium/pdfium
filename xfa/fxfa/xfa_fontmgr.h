// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_XFA_FONTMGR_H_
#define XFA_FXFA_XFA_FONTMGR_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_ext.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fxfa/fxfa.h"

class CPDF_Font;

struct XFA_FONTINFO {
  uint32_t dwFontNameHash;
  const FX_WCHAR* pPsName;
  const FX_WCHAR* pReplaceFont;
  uint16_t dwStyles;
  uint16_t wCodePage;
};

class CXFA_DefFontMgr {
 public:
  CXFA_DefFontMgr();
  ~CXFA_DefFontMgr();

  CFX_RetainPtr<CFGAS_GEFont> GetFont(CXFA_FFDoc* hDoc,
                                      const CFX_WideStringC& wsFontFamily,
                                      uint32_t dwFontStyles,
                                      uint16_t wCodePage = 0xFFFF);
  CFX_RetainPtr<CFGAS_GEFont> GetDefaultFont(
      CXFA_FFDoc* hDoc,
      const CFX_WideStringC& wsFontFamily,
      uint32_t dwFontStyles,
      uint16_t wCodePage = 0xFFFF);

 protected:
  std::vector<CFX_RetainPtr<CFGAS_GEFont>> m_CacheFonts;
};

class CXFA_PDFFontMgr {
 public:
  explicit CXFA_PDFFontMgr(CXFA_FFDoc* pDoc);
  ~CXFA_PDFFontMgr();

  CFX_RetainPtr<CFGAS_GEFont> GetFont(const CFX_WideStringC& wsFontFamily,
                                      uint32_t dwFontStyles,
                                      CPDF_Font** pPDFFont,
                                      bool bStrictMatch);
  bool GetCharWidth(const CFX_RetainPtr<CFGAS_GEFont>& pFont,
                    FX_WCHAR wUnicode,
                    bool bCharCode,
                    int32_t* pWidth);
  void SetFont(const CFX_RetainPtr<CFGAS_GEFont>& pFont, CPDF_Font* pPDFFont);

 protected:
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

  CXFA_FFDoc* const m_pDoc;
  std::map<CFX_RetainPtr<CFGAS_GEFont>, CPDF_Font*> m_FDE2PDFFont;
  std::map<CFX_ByteString, CFX_RetainPtr<CFGAS_GEFont>> m_FontMap;
};

class CXFA_FontMgr {
 public:
  CXFA_FontMgr();
  ~CXFA_FontMgr();

  CFX_RetainPtr<CFGAS_GEFont> GetFont(CXFA_FFDoc* hDoc,
                                      const CFX_WideStringC& wsFontFamily,
                                      uint32_t dwFontStyles,
                                      uint16_t wCodePage = 0xFFFF);
  void LoadDocFonts(CXFA_FFDoc* hDoc);
  void ReleaseDocFonts(CXFA_FFDoc* hDoc);
  void SetDefFontMgr(std::unique_ptr<CXFA_DefFontMgr> pFontMgr);

 protected:
  std::unique_ptr<CXFA_DefFontMgr> m_pDefFontMgr;
  std::map<CXFA_FFDoc*, std::unique_ptr<CXFA_PDFFontMgr>> m_PDFFontMgrMap;
  std::map<CFX_ByteString, CFX_RetainPtr<CFGAS_GEFont>> m_FontMap;
};

#endif  //  XFA_FXFA_XFA_FONTMGR_H_

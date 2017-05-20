// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_FONT_CFGAS_GEFONT_H_
#define XFA_FGAS_FONT_CFGAS_GEFONT_H_

#include <map>
#include <memory>
#include <vector>

#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/cfx_unowned_ptr.h"
#include "core/fxcrt/fx_memory.h"
#include "xfa/fgas/font/cfgas_fontmgr.h"
#include "xfa/fxfa/cxfa_pdffontmgr.h"

#define FXFONT_SUBST_ITALIC 0x02

class CFGAS_FontMgr;
class CFX_UnicodeEncoding;

class CFGAS_GEFont : public CFX_Retainable {
 public:
  template <typename T>
  friend class CFX_RetainPtr;
  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  static CFX_RetainPtr<CFGAS_GEFont> LoadFont(const wchar_t* pszFontFamily,
                                              uint32_t dwFontStyles,
                                              uint16_t wCodePage,
                                              CFGAS_FontMgr* pFontMgr);
  static CFX_RetainPtr<CFGAS_GEFont> LoadFont(CFX_Font* pExternalFont,
                                              CFGAS_FontMgr* pFontMgr);
  static CFX_RetainPtr<CFGAS_GEFont> LoadFont(
      std::unique_ptr<CFX_Font> pInternalFont,
      CFGAS_FontMgr* pFontMgr);

  CFX_RetainPtr<CFGAS_GEFont> Derive(uint32_t dwFontStyles,
                                     uint16_t wCodePage = 0);
  uint32_t GetFontStyles() const;
  bool GetCharWidth(wchar_t wUnicode, int32_t& iWidth, bool bCharCode);
  int32_t GetGlyphIndex(wchar_t wUnicode, bool bCharCode = false);
  int32_t GetAscent() const;
  int32_t GetDescent() const;
  bool GetCharBBox(wchar_t wUnicode, CFX_Rect* bbox, bool bCharCode = false);
  bool GetBBox(CFX_Rect* bbox);
  CFX_RetainPtr<CFGAS_GEFont> GetSubstFont(int32_t iGlyphIndex);
  CFX_Font* GetDevFont() const { return m_pFont; }
  void SetFontProvider(CXFA_PDFFontMgr* pProvider) {
    m_pProvider.Reset(pProvider);
  }
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  void SetLogicalFontStyle(uint32_t dwLogFontStyle) {
    m_bUseLogFontStyle = true;
    m_dwLogFontStyle = dwLogFontStyle;
  }
#endif

 private:
  explicit CFGAS_GEFont(CFGAS_FontMgr* pFontMgr);
  CFGAS_GEFont(const CFX_RetainPtr<CFGAS_GEFont>& src, uint32_t dwFontStyles);
  ~CFGAS_GEFont() override;

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
  bool LoadFontInternal(const wchar_t* pszFontFamily,
                        uint32_t dwFontStyles,
                        uint16_t wCodePage);
  bool LoadFontInternal(const uint8_t* pBuffer, int32_t length);
  bool LoadFontInternal(
      const CFX_RetainPtr<CFX_SeekableStreamProxy>& pFontStream,
      bool bSaveStream);
#endif
  bool LoadFontInternal(CFX_Font* pExternalFont);
  bool LoadFontInternal(std::unique_ptr<CFX_Font> pInternalFont);
  bool InitFont();
  bool GetCharBBoxInternal(wchar_t wUnicode,
                           CFX_Rect* bbox,
                           bool bRecursive,
                           bool bCharCode = false);
  bool GetCharWidthInternal(wchar_t wUnicode,
                            int32_t& iWidth,
                            bool bRecursive,
                            bool bCharCode);
  int32_t GetGlyphIndex(wchar_t wUnicode,
                        bool bRecursive,
                        CFX_RetainPtr<CFGAS_GEFont>* ppFont,
                        bool bCharCode = false);
  CFX_WideString GetFamilyName() const;

#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  bool m_bUseLogFontStyle;
  uint32_t m_dwLogFontStyle;
#endif
  CFX_Font* m_pFont;
  bool m_bExternalFont;
  CFX_RetainPtr<CFGAS_GEFont> m_pSrcFont;  // Only set by ctor, so no cycles.
  CFGAS_FontMgr::ObservedPtr m_pFontMgr;
  CXFA_PDFFontMgr::ObservedPtr m_pProvider;
  CFX_RetainPtr<CFX_SeekableStreamProxy> m_pStream;
  CFX_RetainPtr<IFX_SeekableReadStream> m_pFileRead;
  std::unique_ptr<CFX_UnicodeEncoding> m_pFontEncoding;
  std::map<wchar_t, int32_t> m_CharWidthMap;
  std::map<wchar_t, CFX_Rect> m_BBoxMap;
  std::vector<CFX_RetainPtr<CFGAS_GEFont>> m_SubstFonts;
  std::map<wchar_t, CFX_RetainPtr<CFGAS_GEFont>> m_FontMapper;
};

#endif  // XFA_FGAS_FONT_CFGAS_GEFONT_H_

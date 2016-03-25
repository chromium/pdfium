// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_FONT_FGAS_FONT_H_
#define XFA_FGAS_FONT_FGAS_FONT_H_

#include "core/include/fxge/fx_font.h"
#include "xfa/fgas/crt/fgas_stream.h"

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
#include "xfa/fgas/crt/fgas_memory.h"
#include "xfa/fgas/crt/fgas_utils.h"
#endif  // _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

class IFX_Font;
class IFX_FontMgr;

#define FX_FONTSTYLE_Normal 0x00
#define FX_FONTSTYLE_FixedPitch 0x01
#define FX_FONTSTYLE_Serif 0x02
#define FX_FONTSTYLE_Symbolic 0x04
#define FX_FONTSTYLE_Script 0x08
#define FX_FONTSTYLE_Italic 0x40
#define FX_FONTSTYLE_Bold 0x40000
#define FX_FONTSTYLE_BoldItalic (FX_FONTSTYLE_Bold | FX_FONTSTYLE_Italic)
#define FX_FONTSTYLE_ExactMatch 0x80000000
#define FX_FONTDECORATION_Underline 0x00000001
#define FX_FONTDECORATION_Strikeout 0x00000002
#define FX_FONTDECORATION_Overline 0x00000004
#define FX_FONTDECORATION_Emphasis 0x00000008
#define FX_FONTDECORATION_Superscript 0x00000010
#define FX_FONTDECORATION_Subscript 0x00000020
#define FX_FONTDECORATION_SmallCapital 0x00000040
#define FX_FONTDECORATION_Capital 0x00000080
#define FX_FONTDECORATION_Lowercase 0x000000C0
#define FX_FONTDECORATION_Raised 0x00000100
#define FX_FONTDECORATION_Sunken 0x00000200
#define FX_FONTDECORATION_Shadow 0x00000400
#define FX_FONTDECORATION_BoundingShape 0x20000000
#define FX_FONTDECORATION_Hide 0x40000000
#define FX_FONTDECORATION_StrokeFill 0x80000000
#define FX_BOUNDINGSHAPE_None 0
#define FX_BOUNDINGSHAPE_Circle 1
#define FX_BOUNDINGSHAPE_Square 2
#define FX_BOUNDINGSHAPE_Triangle 3
#define FX_BOUNDINGSHAPE_Diamond 4

class IFX_FontProvider {
 public:
  virtual ~IFX_FontProvider() {}
  virtual FX_BOOL GetCharWidth(IFX_Font* pFont,
                               FX_WCHAR wUnicode,
                               int32_t& iWidth,
                               FX_BOOL bCharCode = FALSE) = 0;
};

class IFX_Font {
 public:
  static IFX_Font* LoadFont(const FX_WCHAR* pszFontFamily,
                            uint32_t dwFontStyles,
                            uint16_t wCodePage,
                            IFX_FontMgr* pFontMgr);
  static IFX_Font* LoadFont(const uint8_t* pBuffer,
                            int32_t iLength,
                            IFX_FontMgr* pFontMgr);
  static IFX_Font* LoadFont(const FX_WCHAR* pszFileName, IFX_FontMgr* pFontMgr);
  static IFX_Font* LoadFont(IFX_Stream* pFontStream,
                            IFX_FontMgr* pFontMgr,
                            FX_BOOL bSaveStream = FALSE);
  static IFX_Font* LoadFont(CFX_Font* pExtFont,
                            IFX_FontMgr* pFontMgr,
                            FX_BOOL bTakeOver = FALSE);
  virtual ~IFX_Font() {}
  virtual void Release() = 0;
  virtual IFX_Font* Retain() = 0;
  virtual IFX_Font* Derive(uint32_t dwFontStyles, uint16_t wCodePage = 0) = 0;
  virtual void GetFamilyName(CFX_WideString& wsFamily) const = 0;
  virtual void GetPsName(CFX_WideString& wsName) const = 0;
  virtual uint32_t GetFontStyles() const = 0;
  virtual uint8_t GetCharSet() const = 0;
  virtual FX_BOOL GetCharWidth(FX_WCHAR wUnicode,
                               int32_t& iWidth,
                               FX_BOOL bCharCode = FALSE) = 0;
  virtual int32_t GetGlyphIndex(FX_WCHAR wUnicode,
                                FX_BOOL bCharCode = FALSE) = 0;
  virtual int32_t GetAscent() const = 0;
  virtual int32_t GetDescent() const = 0;
  virtual FX_BOOL GetCharBBox(FX_WCHAR wUnicode,
                              CFX_Rect& bbox,
                              FX_BOOL bCharCode = FALSE) = 0;
  virtual FX_BOOL GetBBox(CFX_Rect& bbox) = 0;
  virtual int32_t GetItalicAngle() const = 0;
  virtual void Reset() = 0;
  virtual IFX_Font* GetSubstFont(int32_t iGlyphIndex) const = 0;
  virtual void* GetDevFont() const = 0;
  virtual void SetFontProvider(IFX_FontProvider* pProvider) = 0;
#if _FXM_PLATFORM_ != _FXM_PLATFORM_WINDOWS_
  virtual void SetLogicalFontStyle(uint32_t dwLogFontStyle) = 0;
#endif
};
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
struct FX_FONTMATCHPARAMS {
  const FX_WCHAR* pwsFamily;
  uint32_t dwFontStyles;
  uint32_t dwUSB;
  uint32_t dwMatchFlags;
  FX_WCHAR wUnicode;
  uint16_t wCodePage;
};
typedef FX_FONTMATCHPARAMS* FX_LPFONTMATCHPARAMS;
typedef FX_FONTMATCHPARAMS const* FX_LPCFONTMATCHPARAMS;

struct FX_FONTSIGNATURE {
  uint32_t fsUsb[4];
  uint32_t fsCsb[2];
};
inline bool operator==(const FX_FONTSIGNATURE& left,
                       const FX_FONTSIGNATURE& right) {
  return left.fsUsb[0] == right.fsUsb[0] && left.fsUsb[1] == right.fsUsb[1] &&
         left.fsUsb[2] == right.fsUsb[2] && left.fsUsb[3] == right.fsUsb[3] &&
         left.fsCsb[0] == right.fsCsb[0] && left.fsCsb[1] == right.fsCsb[1];
}

struct FX_FONTDESCRIPTOR {
  FX_WCHAR wsFontFace[32];
  uint32_t dwFontStyles;
  uint8_t uCharSet;
  FX_FONTSIGNATURE FontSignature;
};
typedef FX_FONTDESCRIPTOR* FX_LPFONTDESCRIPTOR;
typedef FX_FONTDESCRIPTOR const* FX_LPCFONTDESCRIPTOR;
typedef CFX_MassArrayTemplate<FX_FONTDESCRIPTOR> CFX_FontDescriptors;
inline bool operator==(const FX_FONTDESCRIPTOR& left,
                       const FX_FONTDESCRIPTOR& right) {
  return left.uCharSet == right.uCharSet &&
         left.dwFontStyles == right.dwFontStyles &&
         left.FontSignature == right.FontSignature &&
         FXSYS_wcscmp(left.wsFontFace, right.wsFontFace) == 0;
}

#define FX_FONTMATCHPARA_MacthStyle 0x01
#define FX_FONTMATCHPARA_MacthFamily 0x02
#define FX_FONTMATCHPARA_MacthUnicode 0x04
typedef void (*FX_LPEnumAllFonts)(CFX_FontDescriptors& fonts,
                                  void* pUserData,
                                  const FX_WCHAR* pwsFaceName,
                                  FX_WCHAR wUnicode);
FX_LPEnumAllFonts FX_GetDefFontEnumerator();
typedef FX_LPCFONTDESCRIPTOR (*FX_LPMatchFont)(FX_LPFONTMATCHPARAMS pParams,
                                               const CFX_FontDescriptors& fonts,
                                               void* pUserData);
FX_LPMatchFont FX_GetDefFontMatchor();
class IFX_FontMgr {
 public:
  static IFX_FontMgr* Create(FX_LPEnumAllFonts pEnumerator,
                             FX_LPMatchFont pMatcher = NULL,
                             void* pUserData = NULL);
  virtual ~IFX_FontMgr() {}
  virtual void Release() = 0;
  virtual IFX_Font* GetDefFontByCodePage(
      uint16_t wCodePage,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByCharset(
      uint8_t nCharset,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByUnicode(
      FX_WCHAR wUnicode,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByLanguage(
      uint16_t wLanguage,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* LoadFont(const FX_WCHAR* pszFontFamily,
                             uint32_t dwFontStyles,
                             uint16_t wCodePage = 0xFFFF) = 0;
  virtual IFX_Font* LoadFont(const uint8_t* pBuffer, int32_t iLength) = 0;
  virtual IFX_Font* LoadFont(const FX_WCHAR* pszFileName) = 0;
  virtual IFX_Font* LoadFont(IFX_Stream* pFontStream,
                             const FX_WCHAR* pszFontAlias = NULL,
                             uint32_t dwFontStyles = 0,
                             uint16_t wCodePage = 0,
                             FX_BOOL bSaveStream = FALSE) = 0;
  virtual IFX_Font* LoadFont(IFX_Font* pSrcFont,
                             uint32_t dwFontStyles,
                             uint16_t wCodePage = 0xFFFF) = 0;
  virtual void ClearFontCache() = 0;
  virtual void RemoveFont(IFX_Font* pFont) = 0;
};
#else
class IFX_FontMgrDelegate {
 public:
  virtual ~IFX_FontMgrDelegate() {}
  virtual IFX_Font* GetDefFontByCodePage(
      IFX_FontMgr* pFontMgr,
      uint16_t wCodePage,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByCharset(
      IFX_FontMgr* pFontMgr,
      uint8_t nCharset,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByUnicode(
      IFX_FontMgr* pFontMgr,
      FX_WCHAR wUnicode,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByLanguage(
      IFX_FontMgr* pFontMgr,
      uint16_t wLanguage,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
};
class IFX_FontSourceEnum {
 public:
  virtual ~IFX_FontSourceEnum() {}
  virtual void Release() = 0;
  virtual FX_POSITION GetStartPosition(void* pUserData = NULL) = 0;
  virtual IFX_FileAccess* GetNext(FX_POSITION& pos, void* pUserData = NULL) = 0;
};
IFX_FontSourceEnum* FX_CreateDefaultFontSourceEnum();
class IFX_FontMgr {
 public:
  static IFX_FontMgr* Create(IFX_FontSourceEnum* pFontEnum,
                             IFX_FontMgrDelegate* pDelegate = NULL,
                             void* pUserData = NULL);
  virtual ~IFX_FontMgr() {}
  virtual void Release() = 0;
  virtual IFX_Font* GetDefFontByCodePage(
      uint16_t wCodePage,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByCharset(
      uint8_t nCharset,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByUnicode(
      FX_WCHAR wUnicode,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetDefFontByLanguage(
      uint16_t wLanguage,
      uint32_t dwFontStyles,
      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetFontByCodePage(uint16_t wCodePage,
                                      uint32_t dwFontStyles,
                                      const FX_WCHAR* pszFontFamily = NULL) = 0;
  inline IFX_Font* LoadFont(const FX_WCHAR* pszFontFamily,
                            uint32_t dwFontStyles,
                            uint16_t wCodePage) {
    return GetFontByCodePage(wCodePage, dwFontStyles, pszFontFamily);
  }
  virtual IFX_Font* GetFontByCharset(uint8_t nCharset,
                                     uint32_t dwFontStyles,
                                     const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetFontByUnicode(FX_WCHAR wUnicode,
                                     uint32_t dwFontStyles,
                                     const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* GetFontByLanguage(uint16_t wLanguage,
                                      uint32_t dwFontStyles,
                                      const FX_WCHAR* pszFontFamily = NULL) = 0;
  virtual IFX_Font* LoadFont(const uint8_t* pBuffer,
                             int32_t iLength,
                             int32_t iFaceIndex,
                             int32_t* pFaceCount = NULL) = 0;
  virtual IFX_Font* LoadFont(const FX_WCHAR* pszFileName,
                             int32_t iFaceIndex,
                             int32_t* pFaceCount = NULL) = 0;
  virtual IFX_Font* LoadFont(IFX_Stream* pFontStream,
                             int32_t iFaceIndex,
                             int32_t* pFaceCount = NULL,
                             FX_BOOL bSaveStream = FALSE) = 0;

  virtual void ClearFontCache() = 0;
  virtual void RemoveFont(IFX_Font* pFont) = 0;
};
#endif

#endif  // XFA_FGAS_FONT_FGAS_FONT_H_

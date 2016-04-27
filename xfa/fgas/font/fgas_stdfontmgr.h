// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FGAS_FONT_FGAS_STDFONTMGR_H_
#define XFA_FGAS_FONT_FGAS_STDFONTMGR_H_

#include "core/fxcrt/include/fx_ext.h"
#include "core/fxge/include/fx_freetype.h"
#include "core/fxge/include/fx_ge.h"
#include "third_party/freetype/include/freetype/fttypes.h"
#include "xfa/fgas/font/fgas_font.h"

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
int32_t FX_GetSimilarValue(FX_FONTDESCRIPTOR const* pFont,
                           uint32_t dwFontStyles);
FX_FONTDESCRIPTOR const* FX_DefFontMatcher(FX_LPFONTMATCHPARAMS pParams,
                                           const CFX_FontDescriptors& fonts);

class CFX_StdFontMgrImp : public IFX_FontMgr {
 public:
  CFX_StdFontMgrImp(FX_LPEnumAllFonts pEnumerator);
  ~CFX_StdFontMgrImp();
  virtual void Release() { delete this; }
  virtual IFX_Font* GetDefFontByCodePage(uint16_t wCodePage,
                                         uint32_t dwFontStyles,
                                         const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetDefFontByCharset(uint8_t nCharset,
                                        uint32_t dwFontStyles,
                                        const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetDefFontByUnicode(FX_WCHAR wUnicode,
                                        uint32_t dwFontStyles,
                                        const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetDefFontByLanguage(uint16_t wLanguage,
                                         uint32_t dwFontStyles,
                                         const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* LoadFont(const FX_WCHAR* pszFontFamily,
                             uint32_t dwFontStyles,
                             uint16_t wCodePage = 0xFFFF);
  virtual IFX_Font* LoadFont(const uint8_t* pBuffer, int32_t iLength);
  virtual IFX_Font* LoadFont(const FX_WCHAR* pszFileName);
  virtual IFX_Font* LoadFont(IFX_Stream* pFontStream,
                             const FX_WCHAR* pszFontAlias = NULL,
                             uint32_t dwFontStyles = 0,
                             uint16_t wCodePage = 0,
                             FX_BOOL bSaveStream = FALSE);
  virtual IFX_Font* LoadFont(IFX_Font* pSrcFont,
                             uint32_t dwFontStyles,
                             uint16_t wCodePage = 0xFFFF);
  virtual void ClearFontCache();
  virtual void RemoveFont(IFX_Font* pFont);

 protected:
  void RemoveFont(CFX_MapPtrToPtr& fontMap, IFX_Font* pFont);
  FX_FONTDESCRIPTOR const* FindFont(const FX_WCHAR* pszFontFamily,
                                    uint32_t dwFontStyles,
                                    uint32_t dwMatchFlags,
                                    uint16_t wCodePage,
                                    uint32_t dwUSB = 999,
                                    FX_WCHAR wUnicode = 0);
  IFX_Font* GetFont(FX_FONTDESCRIPTOR const* pFD, uint32_t dwFontStyles);

  FX_LPEnumAllFonts m_pEnumerator;
  CFX_FontDescriptors m_FontFaces;
  CFX_ArrayTemplate<IFX_Font*> m_Fonts;
  CFX_MapPtrToPtr m_CPFonts;
  CFX_MapPtrToPtr m_FamilyFonts;
  CFX_MapPtrToPtr m_UnicodeFonts;
  CFX_MapPtrToPtr m_BufferFonts;
  CFX_MapPtrToPtr m_FileFonts;
  CFX_MapPtrToPtr m_StreamFonts;
  CFX_MapPtrToPtr m_DeriveFonts;
};
uint32_t FX_GetGdiFontStyles(const LOGFONTW& lf);

#else

class CFX_FontDescriptor {
 public:
  CFX_FontDescriptor()
      : m_pFileAccess(NULL), m_nFaceIndex(0), m_dwFontStyles(0) {
    m_dwUsb[0] = m_dwUsb[1] = m_dwUsb[2] = m_dwUsb[3] = 0;
    m_dwCsb[0] = m_dwCsb[1] = 0;
  }
  ~CFX_FontDescriptor() {
    if (NULL != m_pFileAccess) {
      m_pFileAccess->Release();
    }
  }
  IFX_FileAccess* m_pFileAccess;
  int32_t m_nFaceIndex;
  CFX_WideString m_wsFaceName;
  CFX_WideStringArray m_wsFamilyNames;
  uint32_t m_dwFontStyles;
  uint32_t m_dwUsb[4];
  uint32_t m_dwCsb[2];
};
typedef CFX_ArrayTemplate<CFX_FontDescriptor*> CFX_FontDescriptors;

struct FX_FontDescriptorInfo {
 public:
  CFX_FontDescriptor* pFont;
  int32_t nPenalty;

  bool operator>(const FX_FontDescriptorInfo& other) const {
    return nPenalty > other.nPenalty;
  }
  bool operator<(const FX_FontDescriptorInfo& other) const {
    return nPenalty < other.nPenalty;
  }
  bool operator==(const FX_FontDescriptorInfo& other) const {
    return nPenalty == other.nPenalty;
  }
};
typedef CFX_ArrayTemplate<FX_FontDescriptorInfo> CFX_FontDescriptorInfos;

struct FX_HandleParentPath {
  FX_HandleParentPath() {}
  FX_HandleParentPath(const FX_HandleParentPath& x) {
    pFileHandle = x.pFileHandle;
    bsParentPath = x.bsParentPath;
  }
  void* pFileHandle;
  CFX_ByteString bsParentPath;
};

class CFX_FontSourceEnum_File {
 public:
  CFX_FontSourceEnum_File();

  void Release() { delete this; }
  FX_POSITION GetStartPosition();
  IFX_FileAccess* GetNext(FX_POSITION& pos);

 private:
  CFX_ByteString GetNextFile();

  CFX_WideString m_wsNext;
  CFX_ObjectArray<FX_HandleParentPath> m_FolderQueue;
  CFX_ByteStringArray m_FolderPaths;
};

typedef CFX_MapPtrTemplate<uint32_t, IFX_FileAccess*> CFX_HashFileMap;
typedef CFX_MapPtrTemplate<uint32_t, IFX_Font*> CFX_HashFontMap;
typedef CFX_MapPtrTemplate<uint32_t, CFX_FontDescriptorInfos*>
    CFX_HashFontDescsMap;
typedef CFX_MapPtrTemplate<uint32_t, CFX_ArrayTemplate<IFX_Font*>*>
    CFX_HashFontsMap;
typedef CFX_MapPtrTemplate<FX_WCHAR, IFX_Font*> CFX_UnicodeFontMap;
typedef CFX_MapPtrTemplate<IFX_FileAccess*, CFX_ArrayTemplate<IFX_Font*>*>
    CFX_FileFontMap;
typedef CFX_MapPtrTemplate<IFX_Font*, IFX_FileRead*> CFX_FonStreamtMap;

class CFX_FontMgrImp : public IFX_FontMgr {
 public:
  CFX_FontMgrImp(CFX_FontSourceEnum_File* pFontEnum);

  virtual void Release();
  virtual IFX_Font* GetDefFontByCodePage(uint16_t wCodePage,
                                         uint32_t dwFontStyles,
                                         const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetDefFontByCharset(uint8_t nCharset,
                                        uint32_t dwFontStyles,
                                        const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetDefFontByUnicode(FX_WCHAR wUnicode,
                                        uint32_t dwFontStyles,
                                        const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetDefFontByLanguage(uint16_t wLanguage,
                                         uint32_t dwFontStyles,
                                         const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetFontByCodePage(uint16_t wCodePage,
                                      uint32_t dwFontStyles,
                                      const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetFontByCharset(uint8_t nCharset,
                                     uint32_t dwFontStyles,
                                     const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetFontByUnicode(FX_WCHAR wUnicode,
                                     uint32_t dwFontStyles,
                                     const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* GetFontByLanguage(uint16_t wLanguage,
                                      uint32_t dwFontStyles,
                                      const FX_WCHAR* pszFontFamily = NULL);
  virtual IFX_Font* LoadFont(const uint8_t* pBuffer,
                             int32_t iLength,
                             int32_t iFaceIndex,
                             int32_t* pFaceCount);
  virtual IFX_Font* LoadFont(const FX_WCHAR* pszFileName,
                             int32_t iFaceIndex,
                             int32_t* pFaceCount);
  virtual IFX_Font* LoadFont(IFX_Stream* pFontStream,
                             int32_t iFaceIndex,
                             int32_t* pFaceCount,
                             FX_BOOL bSaveStream = FALSE);
  virtual IFX_Font* LoadFont(const CFX_WideString& wsFaceName,
                             int32_t iFaceIndex,
                             int32_t* pFaceCount);
  virtual void ClearFontCache();
  virtual void RemoveFont(IFX_Font* pFont);
  FX_BOOL EnumFonts();
  FX_BOOL EnumFontsFromFontMapper();
  FX_BOOL EnumFontsFromFiles();

 protected:
  void RegisterFace(FXFT_Face pFace,
                    CFX_FontDescriptors& Fonts,
                    const CFX_WideString* pFaceName,
                    IFX_FileAccess* pFontAccess);
  void RegisterFaces(IFX_FileRead* pFontStream,
                     const CFX_WideString* pFaceName);
  void GetNames(const uint8_t* name_table, CFX_WideStringArray& Names);
  void GetCharsets(FXFT_Face pFace, CFX_ArrayTemplate<uint16_t>& Charsets);
  void GetUSBCSB(FXFT_Face pFace, uint32_t* USB, uint32_t* CSB);
  uint32_t GetFlags(FXFT_Face pFace);
  CFX_FontDescriptors m_InstalledFonts;
  FX_BOOL VerifyUnicode(CFX_FontDescriptor* pDesc, FX_WCHAR wcUnicode);
  FX_BOOL VerifyUnicode(IFX_Font* pFont, FX_WCHAR wcUnicode);
  int32_t IsPartName(const CFX_WideString& Name1, const CFX_WideString& Name2);
  int32_t MatchFonts(CFX_FontDescriptorInfos& MatchedFonts,
                     uint16_t wCodePage,
                     uint32_t dwFontStyles,
                     const CFX_WideString& FontName,
                     FX_WCHAR wcUnicode = 0xFFFE);
  int32_t CalcPenalty(CFX_FontDescriptor* pInstalled,
                      uint16_t wCodePage,
                      uint32_t dwFontStyles,
                      const CFX_WideString& FontName,
                      FX_WCHAR wcUnicode = 0xFFFE);
  IFX_Font* LoadFont(IFX_FileAccess* pFontAccess,
                     int32_t iFaceIndex,
                     int32_t* pFaceCount,
                     FX_BOOL bWantCache = FALSE);
  FXFT_Face LoadFace(IFX_FileRead* pFontStream, int32_t iFaceIndex);
  IFX_FileRead* CreateFontStream(CFX_FontMapper* pFontMapper,
                                 IFX_SystemFontInfo* pSystemFontInfo,
                                 uint32_t index);
  IFX_FileRead* CreateFontStream(const CFX_ByteString& bsFaceName);

  CFX_HashFontDescsMap m_Hash2CandidateList;
  CFX_HashFontsMap m_Hash2Fonts;
  CFX_HashFileMap m_Hash2FileAccess;
  CFX_HashFontMap m_FileAccess2IFXFont;
  CFX_FonStreamtMap m_IFXFont2FileRead;
  CFX_UnicodeFontMap m_FailedUnicodes2NULL;
  CFX_FontSourceEnum_File* m_pFontSource;
};
#endif

#endif  // XFA_FGAS_FONT_FGAS_STDFONTMGR_H_

// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_FONTMAPPER_H_
#define CORE_FXGE_CFX_FONTMAPPER_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_string.h"
#include "core/fxge/fx_freetype.h"

class CFX_FontMgr;
class CFX_SubstFont;
class SystemFontInfoIface;

class CFX_FontMapper {
 public:
  explicit CFX_FontMapper(CFX_FontMgr* mgr);
  ~CFX_FontMapper();

  static int GetStandardFontName(ByteString* name);

  void SetSystemFontInfo(std::unique_ptr<SystemFontInfoIface> pFontInfo);
  SystemFontInfoIface* GetSystemFontInfo() { return m_pFontInfo.get(); }
  void AddInstalledFont(const ByteString& name, int charset);
  void LoadInstalledFonts();

  FXFT_FaceRec* FindSubstFont(const ByteString& face_name,
                              bool bTrueType,
                              uint32_t flags,
                              int weight,
                              int italic_angle,
                              int CharsetCP,
                              CFX_SubstFont* pSubstFont);

  bool IsBuiltinFace(const FXFT_FaceRec* face) const;
  int GetFaceSize() const;
  ByteString GetFaceName(int index) const { return m_FaceArray[index].name; }

  std::vector<ByteString> m_InstalledTTFonts;
  std::vector<std::pair<ByteString, ByteString>> m_LocalizedTTFonts;

 private:
  static const size_t MM_FACE_COUNT = 2;
  static const size_t FOXIT_FACE_COUNT = 14;

  ByteString GetPSNameFromTT(void* hFont);
  ByteString MatchInstalledFonts(const ByteString& norm_name);
  FXFT_FaceRec* UseInternalSubst(CFX_SubstFont* pSubstFont,
                                 int iBaseFont,
                                 int italic_angle,
                                 int weight,
                                 int picthfamily);
  FXFT_FaceRec* GetCachedTTCFace(void* hFont,
                                 const uint32_t tableTTCF,
                                 uint32_t ttc_size,
                                 uint32_t font_size);
  FXFT_FaceRec* GetCachedFace(void* hFont,
                              ByteString SubstName,
                              int weight,
                              bool bItalic,
                              uint32_t font_size);

  struct FaceData {
    ByteString name;
    uint32_t charset;
  };

  bool m_bListLoaded = false;
  ByteString m_LastFamily;
  std::vector<FaceData> m_FaceArray;
  std::unique_ptr<SystemFontInfoIface> m_pFontInfo;
  UnownedPtr<CFX_FontMgr> const m_pFontMgr;
  ScopedFXFTFaceRec m_MMFaces[MM_FACE_COUNT];
  ScopedFXFTFaceRec m_FoxitFaces[FOXIT_FACE_COUNT];
};

#endif  // CORE_FXGE_CFX_FONTMAPPER_H_

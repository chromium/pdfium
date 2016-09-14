// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_GE_CFX_FOLDERFONTINFO_H_
#define CORE_FXGE_GE_CFX_FOLDERFONTINFO_H_

#include <map>
#include <vector>

#include "core/fxge/include/cfx_fontmapper.h"
#include "core/fxge/include/fx_font.h"
#include "core/fxge/include/ifx_systemfontinfo.h"

class CFX_FolderFontInfo : public IFX_SystemFontInfo {
 public:
  CFX_FolderFontInfo();
  ~CFX_FolderFontInfo() override;

  void AddPath(const CFX_ByteStringC& path);

  // IFX_SytemFontInfo:
  FX_BOOL EnumFontList(CFX_FontMapper* pMapper) override;
  void* MapFont(int weight,
                FX_BOOL bItalic,
                int charset,
                int pitch_family,
                const FX_CHAR* face,
                int& bExact) override;
#ifdef PDF_ENABLE_XFA
  void* MapFontByUnicode(uint32_t dwUnicode,
                         int weight,
                         FX_BOOL bItalic,
                         int pitch_family) override;
#endif  // PDF_ENABLE_XFA
  void* GetFont(const FX_CHAR* face) override;
  uint32_t GetFontData(void* hFont,
                       uint32_t table,
                       uint8_t* buffer,
                       uint32_t size) override;
  void DeleteFont(void* hFont) override;
  FX_BOOL GetFaceName(void* hFont, CFX_ByteString& name) override;
  FX_BOOL GetFontCharset(void* hFont, int& charset) override;

 protected:
  void ScanPath(const CFX_ByteString& path);
  void ScanFile(const CFX_ByteString& path);
  void ReportFace(const CFX_ByteString& path,
                  FXSYS_FILE* pFile,
                  uint32_t filesize,
                  uint32_t offset);
  void* GetSubstFont(const CFX_ByteString& face);
  void* FindFont(int weight,
                 FX_BOOL bItalic,
                 int charset,
                 int pitch_family,
                 const FX_CHAR* family,
                 FX_BOOL bMatchName);

  std::map<CFX_ByteString, CFX_FontFaceInfo*> m_FontList;
  std::vector<CFX_ByteString> m_PathList;
  CFX_FontMapper* m_pMapper;
};

#endif  // CORE_FXGE_GE_CFX_FOLDERFONTINFO_H_

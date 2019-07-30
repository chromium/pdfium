// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_DOCPAGEDATA_H_
#define CORE_FPDFAPI_PAGE_CPDF_DOCPAGEDATA_H_

#include <map>
#include <memory>
#include <set>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/page/cpdf_colorspace.h"
#include "core/fpdfapi/parser/cpdf_document.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

class CFX_Font;
class CPDF_Dictionary;
class CPDF_FontEncoding;
class CPDF_IccProfile;
class CPDF_Image;
class CPDF_Object;
class CPDF_Pattern;
class CPDF_Stream;
class CPDF_StreamAcc;

class CPDF_DocPageData : public CPDF_Document::PageDataIface,
                         public CPDF_Font::FormFactoryIface {
 public:
  static CPDF_DocPageData* FromDocument(const CPDF_Document* pDoc);

  CPDF_DocPageData();
  ~CPDF_DocPageData() override;

  // CPDF_Document::PageDataIface:
  void ClearStockFont() override;
  RetainPtr<CPDF_StreamAcc> GetFontFileStreamAcc(
      const CPDF_Stream* pFontStream) override;
  void MaybePurgeFontFileStreamAcc(const CPDF_Stream* pFontStream) override;

  // CPDF_Font::FormFactoryIFace:
  std::unique_ptr<CPDF_Font::FormIface> CreateForm(
      CPDF_Document* pDocument,
      CPDF_Dictionary* pPageResources,
      CPDF_Stream* pFormStream) override;

  bool IsForceClear() const { return m_bForceClear; }

  RetainPtr<CPDF_Font> AddFont(std::unique_ptr<CFX_Font> pFont, int charset);
  RetainPtr<CPDF_Font> GetFont(CPDF_Dictionary* pFontDict);
  RetainPtr<CPDF_Font> AddStandardFont(const char* font,
                                       const CPDF_FontEncoding* pEncoding);
  RetainPtr<CPDF_Font> GetStandardFont(const ByteString& fontName,
                                       const CPDF_FontEncoding* pEncoding);
#if defined(OS_WIN)
  RetainPtr<CPDF_Font> AddWindowsFont(LOGFONTA* pLogFont);
#endif

  // Loads a colorspace.
  RetainPtr<CPDF_ColorSpace> GetColorSpace(const CPDF_Object* pCSObj,
                                           const CPDF_Dictionary* pResources);

  // Loads a colorspace in a context that might be while loading another
  // colorspace. |pVisited| is passed recursively to avoid circular calls
  // involving CPDF_ColorSpace::Load().
  RetainPtr<CPDF_ColorSpace> GetColorSpaceGuarded(
      const CPDF_Object* pCSObj,
      const CPDF_Dictionary* pResources,
      std::set<const CPDF_Object*>* pVisited);

  RetainPtr<CPDF_Pattern> GetPattern(CPDF_Object* pPatternObj,
                                     bool bShading,
                                     const CFX_Matrix& matrix);

  RetainPtr<CPDF_Image> GetImage(uint32_t dwStreamObjNum);
  void MaybePurgeImage(uint32_t dwStreamObjNum);

  RetainPtr<CPDF_IccProfile> GetIccProfile(const CPDF_Stream* pProfileStream);

 private:
  // Loads a colorspace in a context that might be while loading another
  // colorspace, or even in a recursive call from this method itself. |pVisited|
  // is passed recursively to avoid circular calls involving
  // CPDF_ColorSpace::Load() and |pVisitedInternal| is also passed recursively
  // to avoid circular calls with this method calling itself.
  RetainPtr<CPDF_ColorSpace> GetColorSpaceInternal(
      const CPDF_Object* pCSObj,
      const CPDF_Dictionary* pResources,
      std::set<const CPDF_Object*>* pVisited,
      std::set<const CPDF_Object*>* pVisitedInternal);

  size_t CalculateEncodingDict(int charset, CPDF_Dictionary* pBaseDict);
  CPDF_Dictionary* ProcessbCJK(
      CPDF_Dictionary* pBaseDict,
      int charset,
      ByteString basefont,
      std::function<void(wchar_t, wchar_t, CPDF_Array*)> Insert);
  void Clear(bool bForceRelease);

  bool m_bForceClear = false;

  // Specific destruction order may be required between maps.
  std::map<ByteString, RetainPtr<const CPDF_Stream>> m_HashProfileMap;
  std::map<const CPDF_Object*, ObservedPtr<CPDF_ColorSpace>> m_ColorSpaceMap;
  std::map<const CPDF_Stream*, RetainPtr<CPDF_StreamAcc>> m_FontFileMap;
  std::map<const CPDF_Stream*, ObservedPtr<CPDF_IccProfile>> m_IccProfileMap;
  std::map<const CPDF_Object*, ObservedPtr<CPDF_Pattern>> m_PatternMap;
  std::map<uint32_t, RetainPtr<CPDF_Image>> m_ImageMap;
  std::map<const CPDF_Dictionary*, ObservedPtr<CPDF_Font>> m_FontMap;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_DOCPAGEDATA_H_

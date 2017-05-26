// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_CMAP_H_
#define CORE_FPDFAPI_FONT_CPDF_CMAP_H_

#include <vector>

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fxcrt/cfx_retain_ptr.h"
#include "core/fxcrt/fx_basic.h"

class CPDF_CMapManager;
struct FXCMAP_CMap;

enum CIDCoding : uint8_t {
  CIDCODING_UNKNOWN = 0,
  CIDCODING_GB,
  CIDCODING_BIG5,
  CIDCODING_JIS,
  CIDCODING_KOREA,
  CIDCODING_UCS2,
  CIDCODING_CID,
  CIDCODING_UTF16,
};

class CPDF_CMap : public CFX_Retainable {
 public:
  enum CodingScheme : uint8_t {
    OneByte,
    TwoBytes,
    MixedTwoBytes,
    MixedFourBytes
  };

  struct CodeRange {
    int m_CharSize;
    uint8_t m_Lower[4];
    uint8_t m_Upper[4];
  };

  struct CIDRange {
    uint32_t m_StartCode;
    uint32_t m_EndCode;
    uint16_t m_StartCID;
  };

  template <typename T, typename... Args>
  friend CFX_RetainPtr<T> pdfium::MakeRetain(Args&&... args);

  void LoadPredefined(CPDF_CMapManager* pMgr,
                      const CFX_ByteString& name,
                      bool bPromptCJK);
  void LoadEmbedded(const uint8_t* pData, uint32_t dwSize);

  bool IsLoaded() const { return m_bLoaded; }
  bool IsVertWriting() const { return m_bVertical; }
  uint16_t CIDFromCharCode(uint32_t charcode) const;
  int GetCharSize(uint32_t charcode) const;
  uint32_t GetNextChar(const char* pString, int nStrLen, int& offset) const;
  int CountChar(const char* pString, int size) const;
  int AppendChar(char* str, uint32_t charcode) const;

 private:
  friend class CPDF_CMapParser;
  friend class CPDF_CIDFont;

  CPDF_CMap();
  ~CPDF_CMap() override;

  CFX_ByteString m_PredefinedCMap;
  bool m_bLoaded;
  bool m_bVertical;
  CIDSet m_Charset;
  CodingScheme m_CodingScheme;
  int m_Coding;
  std::vector<bool> m_MixedTwoByteLeadingBytes;
  std::vector<CodeRange> m_MixedFourByteLeadingRanges;
  std::vector<uint16_t> m_DirectCharcodeToCIDTable;
  std::vector<CIDRange> m_AdditionalCharcodeToCIDMappings;
  const FXCMAP_CMap* m_pEmbedMap;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_CMAP_H_

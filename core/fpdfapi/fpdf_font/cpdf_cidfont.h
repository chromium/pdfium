// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FPDF_FONT_CPDF_CIDFONT_H_
#define CORE_FPDFAPI_FPDF_FONT_CPDF_CIDFONT_H_

#include "core/fpdfapi/fpdf_font/include/cpdf_font.h"
#include "core/fxcrt/include/fx_string.h"
#include "core/fxcrt/include/fx_system.h"

enum CIDSet {
  CIDSET_UNKNOWN,
  CIDSET_GB1,
  CIDSET_CNS1,
  CIDSET_JAPAN1,
  CIDSET_KOREA1,
  CIDSET_UNICODE,
  CIDSET_NUM_SETS
};

class CFX_CTTGSUBTable;
class CPDF_Array;
class CPDF_CID2UnicodeMap;
class CPDF_CMap;
class CPDF_StreamAcc;

class CPDF_CIDFont : public CPDF_Font {
 public:
  CPDF_CIDFont();
  ~CPDF_CIDFont() override;

  static FX_FLOAT CIDTransformToFloat(uint8_t ch);

  // CPDF_Font:
  bool IsCIDFont() const override;
  const CPDF_CIDFont* AsCIDFont() const override;
  CPDF_CIDFont* AsCIDFont() override;
  int GlyphFromCharCode(FX_DWORD charcode, FX_BOOL* pVertGlyph = NULL) override;
  int GetCharWidthF(FX_DWORD charcode, int level = 0) override;
  FX_RECT GetCharBBox(FX_DWORD charcode, int level = 0) override;
  FX_DWORD GetNextChar(const FX_CHAR* pString,
                       int nStrLen,
                       int& offset) const override;
  int CountChar(const FX_CHAR* pString, int size) const override;
  int AppendChar(FX_CHAR* str, FX_DWORD charcode) const override;
  int GetCharSize(FX_DWORD charcode) const override;
  FX_BOOL IsVertWriting() const override;
  FX_BOOL IsUnicodeCompatible() const override;
  FX_BOOL Load() override;
  CFX_WideString UnicodeFromCharCode(FX_DWORD charcode) const override;
  FX_DWORD CharCodeFromUnicode(FX_WCHAR Unicode) const override;

  FX_BOOL LoadGB2312();
  uint16_t CIDFromCharCode(FX_DWORD charcode) const;
  const uint8_t* GetCIDTransform(uint16_t CID) const;
  short GetVertWidth(uint16_t CID) const;
  void GetVertOrigin(uint16_t CID, short& vx, short& vy) const;
  virtual FX_BOOL IsFontStyleFromCharCode(FX_DWORD charcode) const;

 protected:
  int GetGlyphIndex(FX_DWORD unicodeb, FX_BOOL* pVertGlyph);
  void LoadMetricsArray(CPDF_Array* pArray,
                        CFX_ArrayTemplate<FX_DWORD>& result,
                        int nElements);
  void LoadSubstFont();
  FX_WCHAR GetUnicodeFromCharCode(FX_DWORD charcode) const;

  CPDF_CMap* m_pCMap;
  CPDF_CMap* m_pAllocatedCMap;
  CPDF_CID2UnicodeMap* m_pCID2UnicodeMap;
  CIDSet m_Charset;
  FX_BOOL m_bType1;
  CPDF_StreamAcc* m_pCIDToGIDMap;
  FX_BOOL m_bCIDIsGID;
  uint16_t m_DefaultWidth;
  uint16_t* m_pAnsiWidths;
  FX_SMALL_RECT m_CharBBox[256];
  CFX_ArrayTemplate<FX_DWORD> m_WidthList;
  short m_DefaultVY;
  short m_DefaultW1;
  CFX_ArrayTemplate<FX_DWORD> m_VertMetrics;
  FX_BOOL m_bAdobeCourierStd;
  CFX_CTTGSUBTable* m_pTTGSUBTable;
};

#endif  // CORE_FPDFAPI_FPDF_FONT_CPDF_CIDFONT_H_

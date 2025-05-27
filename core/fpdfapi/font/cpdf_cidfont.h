// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_CIDFONT_H_
#define CORE_FPDFAPI_FONT_CPDF_CIDFONT_H_

#include <stdint.h>

#include <array>
#include <memory>
#include <vector>

#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/unowned_ptr.h"

enum CIDSet : uint8_t {
  CIDSET_UNKNOWN,
  CIDSET_GB1,
  CIDSET_CNS1,
  CIDSET_JAPAN1,
  CIDSET_KOREA1,
  CIDSET_UNICODE,
  CIDSET_NUM_SETS
};

struct CIDTransform {
  uint16_t cid;
  uint8_t a;
  uint8_t b;
  uint8_t c;
  uint8_t d;
  uint8_t e;
  uint8_t f;
};

class CFX_CTTGSUBTable;
class CPDF_CID2UnicodeMap;
class CPDF_CMap;
class CPDF_StreamAcc;

class CPDF_CIDFont final : public CPDF_Font {
 public:
  CONSTRUCT_VIA_MAKE_RETAIN;
  ~CPDF_CIDFont() override;

  static float CIDTransformToFloat(uint8_t ch);

  // CPDF_Font:
  bool IsCIDFont() const override;
  const CPDF_CIDFont* AsCIDFont() const override;
  CPDF_CIDFont* AsCIDFont() override;
  int GlyphFromCharCode(uint32_t charcode, bool* pVertGlyph) override;
  int GetCharWidthF(uint32_t charcode) override;
  FX_RECT GetCharBBox(uint32_t charcode) override;
  uint32_t GetNextChar(ByteStringView pString, size_t* pOffset) const override;
  size_t CountChar(ByteStringView pString) const override;
  void AppendChar(ByteString* str, uint32_t charcode) const override;
  bool IsVertWriting() const override;
  bool IsUnicodeCompatible() const override;
  bool Load() override;
  WideString UnicodeFromCharCode(uint32_t charcode) const override;
  uint32_t CharCodeFromUnicode(wchar_t Unicode) const override;

  uint16_t CIDFromCharCode(uint32_t charcode) const;
  const CIDTransform* GetCIDTransform(uint16_t cid) const;
  int16_t GetVertWidth(uint16_t cid) const;
  CFX_Point16 GetVertOrigin(uint16_t cid) const;
  int GetCharSize(uint32_t charcode) const;

 private:
  enum class CIDFontType : bool {
    kType1,    // CIDFontType0
    kTrueType  // CIDFontType2
  };

  CPDF_CIDFont(CPDF_Document* document, RetainPtr<CPDF_Dictionary> font_dict);

  void LoadGB2312();
  int GetGlyphIndex(uint32_t unicodeb, bool* pVertGlyph);
  int GetVerticalGlyph(int index, bool* pVertGlyph);
  void LoadSubstFont();
  wchar_t GetUnicodeFromCharCode(uint32_t charcode) const;

  RetainPtr<const CPDF_CMap> cmap_;
  UnownedPtr<const CPDF_CID2UnicodeMap> cid2unicode_map_;
  RetainPtr<CPDF_StreamAcc> stream_acc_;
  std::unique_ptr<CFX_CTTGSUBTable> ttg_subtable_;
  CIDFontType font_type_ = CIDFontType::kTrueType;
  bool cid_is_gid_ = false;
  bool ansi_widths_fixed_ = false;
  bool adobe_courier_std_ = false;
  CIDSet charset_ = CIDSET_UNKNOWN;
  int16_t default_width_ = 1000;
  int16_t default_vy_ = 880;
  int16_t default_w1_ = -1000;
  std::vector<int> width_list_;
  std::vector<int> vert_metrics_;
  std::array<FX_RECT, 256> char_bbox_;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_CIDFONT_H_

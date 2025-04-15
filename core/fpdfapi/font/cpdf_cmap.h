// Copyright 2017 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CPDF_CMAP_H_
#define CORE_FPDFAPI_FONT_CPDF_CMAP_H_

#include <stdint.h>

#include <array>
#include <vector>

#include "core/fpdfapi/font/cpdf_cidfont.h"
#include "core/fxcrt/fixed_size_data_vector.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxcrt/span.h"
#include "core/fxcrt/unowned_ptr.h"

namespace fxcmap {
struct CMap;
}

enum class CIDCoding : uint8_t {
  kUNKNOWN = 0,
  kGB,
  kBIG5,
  kJIS,
  kKOREA,
  kUCS2,
  kCID,
  kUTF16,
};

class CPDF_CMap final : public Retainable {
 public:
  static constexpr size_t kDirectMapTableSize = 65536;

  enum CodingScheme : uint8_t {
    OneByte,
    TwoBytes,
    MixedTwoBytes,
    MixedFourBytes
  };

  struct CodeRange {
    size_t char_size_;
    std::array<uint8_t, 4> lower_;
    std::array<uint8_t, 4> upper_;
  };

  struct CIDRange {
    uint32_t start_code_;
    uint32_t end_code_;
    uint16_t start_cid_;
  };

  CONSTRUCT_VIA_MAKE_RETAIN;

  bool IsLoaded() const { return loaded_; }
  bool IsVertWriting() const { return vertical_; }

  uint16_t CIDFromCharCode(uint32_t charcode) const;

  int GetCharSize(uint32_t charcode) const;
  uint32_t GetNextChar(ByteStringView pString, size_t* pOffset) const;
  size_t CountChar(ByteStringView pString) const;
  void AppendChar(ByteString* str, uint32_t charcode) const;

  void SetVertical(bool vert) { vertical_ = vert; }
  void SetCodingScheme(CodingScheme scheme) { coding_scheme_ = scheme; }
  void SetAdditionalMappings(std::vector<CIDRange> mappings);
  void SetMixedFourByteLeadingRanges(std::vector<CodeRange> ranges);

  CIDCoding GetCoding() const { return coding_; }
  const fxcmap::CMap* GetEmbedMap() const { return embed_map_; }
  CIDSet GetCharset() const { return charset_; }
  void SetCharset(CIDSet set) { charset_ = set; }

  void SetDirectCharcodeToCIDTableRange(uint32_t start_code,
                                        uint32_t end_code,
                                        uint16_t start_cid);
  bool IsDirectCharcodeToCIDTableIsEmpty() const {
    return direct_charcode_to_cidtable_.empty();
  }

 private:
  explicit CPDF_CMap(ByteStringView bsPredefinedName);
  explicit CPDF_CMap(pdfium::span<const uint8_t> spEmbeddedData);
  ~CPDF_CMap() override;

  bool loaded_ = false;
  bool vertical_ = false;
  CIDSet charset_ = CIDSET_UNKNOWN;
  CodingScheme coding_scheme_ = TwoBytes;
  CIDCoding coding_ = CIDCoding::kUNKNOWN;
  std::vector<bool> mixed_two_byte_leading_bytes_;
  std::vector<CodeRange> mixed_four_byte_leading_ranges_;
  FixedSizeDataVector<uint16_t> direct_charcode_to_cidtable_;
  std::vector<CIDRange> additional_charcode_to_cidmappings_;
  UnownedPtr<const fxcmap::CMap> embed_map_;
};

#endif  // CORE_FPDFAPI_FONT_CPDF_CMAP_H_

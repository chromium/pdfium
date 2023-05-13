// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_FONT_CFX_CTTGSUBTABLE_H_
#define CORE_FPDFAPI_FONT_CFX_CTTGSUBTABLE_H_

#include <stdint.h>

#include <memory>
#include <set>
#include <vector>

#include "core/fxcrt/data_vector.h"
#include "core/fxge/freetype/fx_freetype.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "third_party/abseil-cpp/absl/types/variant.h"

class CFX_CTTGSUBTable {
 public:
  explicit CFX_CTTGSUBTable(FT_Bytes gsub);
  ~CFX_CTTGSUBTable();

  uint32_t GetVerticalGlyph(uint32_t glyphnum) const;

 private:
  using FeatureIndices = DataVector<uint16_t>;
  using ScriptRecord = std::vector<FeatureIndices>;

  struct FeatureRecord {
    FeatureRecord();
    ~FeatureRecord();

    uint32_t feature_tag = 0;
    DataVector<uint16_t> lookup_list_indices;
  };

  struct TRangeRecord {
    TRangeRecord();

    uint16_t Start = 0;
    uint16_t End = 0;
    uint16_t StartCoverageIndex = 0;
  };

  struct TCoverageFormatBase {
    explicit TCoverageFormatBase(uint16_t format) : CoverageFormat(format) {}
    virtual ~TCoverageFormatBase() = default;

    const uint16_t CoverageFormat;
  };

  struct TCoverageFormat1 final : public TCoverageFormatBase {
    explicit TCoverageFormat1(size_t initial_size);
    ~TCoverageFormat1() override;

    DataVector<uint16_t> GlyphArray;
  };

  struct TCoverageFormat2 final : public TCoverageFormatBase {
    explicit TCoverageFormat2(size_t initial_size);
    ~TCoverageFormat2() override;

    std::vector<TRangeRecord> RangeRecords;
  };

  struct SubTable {
    SubTable();
    SubTable(const SubTable& that) = delete;
    SubTable& operator=(const SubTable& that) = delete;
    SubTable(SubTable&& that) noexcept;
    SubTable& operator=(SubTable&& that) noexcept;
    ~SubTable();

    std::unique_ptr<TCoverageFormatBase> coverage;
    // DeltaGlyphID for format 1.
    // Substitutes for format 2.
    absl::variant<absl::monostate, int16_t, DataVector<uint16_t>> table_data;
  };

  struct Lookup {
    using SubTables = std::vector<SubTable>;

    Lookup();
    Lookup(const Lookup& that) = delete;
    Lookup& operator=(const Lookup& that) = delete;
    Lookup(Lookup&& that) noexcept;
    Lookup& operator=(Lookup&& that) noexcept;
    ~Lookup();

    uint16_t lookup_type = 0;
    SubTables sub_tables;
  };

  bool LoadGSUBTable(FT_Bytes gsub);
  void Parse(FT_Bytes scriptlist, FT_Bytes featurelist, FT_Bytes lookuplist);
  void ParseScriptList(FT_Bytes raw);
  ScriptRecord ParseScript(FT_Bytes raw);
  FeatureIndices ParseLangSys(FT_Bytes raw);
  void ParseFeatureList(FT_Bytes raw);
  DataVector<uint16_t> ParseFeatureLookupListIndices(FT_Bytes raw);
  void ParseLookupList(FT_Bytes raw);
  Lookup ParseLookup(FT_Bytes raw);
  std::unique_ptr<TCoverageFormatBase> ParseCoverage(FT_Bytes raw);
  std::unique_ptr<TCoverageFormat1> ParseCoverageFormat1(FT_Bytes raw);
  std::unique_ptr<TCoverageFormat2> ParseCoverageFormat2(FT_Bytes raw);
  SubTable ParseSingleSubst(FT_Bytes raw);

  absl::optional<uint32_t> GetVerticalGlyphSub(const FeatureRecord& feature,
                                               uint32_t glyphnum) const;
  absl::optional<uint32_t> GetVerticalGlyphSub2(const Lookup& lookup,
                                                uint32_t glyphnum) const;
  int GetCoverageIndex(TCoverageFormatBase* Coverage, uint32_t g) const;

  uint8_t GetUInt8(FT_Bytes& p) const;
  int16_t GetInt16(FT_Bytes& p) const;
  uint16_t GetUInt16(FT_Bytes& p) const;
  int32_t GetInt32(FT_Bytes& p) const;
  uint32_t GetUInt32(FT_Bytes& p) const;

  std::set<uint32_t> feature_set_;
  std::vector<ScriptRecord> script_list_;
  std::vector<FeatureRecord> feature_list_;
  std::vector<Lookup> lookup_list_;
};

#endif  // CORE_FPDFAPI_FONT_CFX_CTTGSUBTABLE_H_

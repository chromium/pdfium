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

class CFX_CTTGSUBTable {
 public:
  explicit CFX_CTTGSUBTable(FT_Bytes gsub);
  ~CFX_CTTGSUBTable();

  uint32_t GetVerticalGlyph(uint32_t glyphnum) const;

 private:
  struct TLangSysRecord {
    TLangSysRecord();
    ~TLangSysRecord();

    uint32_t LangSysTag = 0;
    uint16_t LookupOrder = 0;
    uint16_t ReqFeatureIndex = 0;
    DataVector<uint16_t> FeatureIndices;
  };

  struct TScriptRecord {
    TScriptRecord();
    ~TScriptRecord();

    uint32_t ScriptTag = 0;
    uint16_t DefaultLangSys = 0;
    std::vector<TLangSysRecord> LangSysRecords;
  };

  struct TFeatureRecord {
    TFeatureRecord();
    ~TFeatureRecord();

    uint32_t FeatureTag = 0;
    uint16_t FeatureParams = 0;
    DataVector<uint16_t> LookupListIndices;
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

  struct TDevice {
    TDevice();

    uint16_t StartSize = 0;
    uint16_t EndSize = 0;
    uint16_t DeltaFormat = 0;
  };

  struct TSubTableBase {
    explicit TSubTableBase(uint16_t format);
    virtual ~TSubTableBase();

    const uint16_t SubstFormat;
    std::unique_ptr<TCoverageFormatBase> Coverage;
  };

  struct TSubTable1 final : public TSubTableBase {
    TSubTable1();
    ~TSubTable1() override;

    int16_t DeltaGlyphID = 0;
  };

  struct TSubTable2 final : public TSubTableBase {
    TSubTable2();
    ~TSubTable2() override;

    DataVector<uint16_t> Substitutes;
  };

  struct TLookup {
    TLookup();
    ~TLookup();

    uint16_t LookupType = 0;
    uint16_t LookupFlag = 0;
    std::vector<std::unique_ptr<TSubTableBase>> SubTables;
  };

  bool LoadGSUBTable(FT_Bytes gsub);
  bool Parse(FT_Bytes scriptlist, FT_Bytes featurelist, FT_Bytes lookuplist);
  void ParseScriptList(FT_Bytes raw);
  void ParseScript(FT_Bytes raw, TScriptRecord* rec);
  void ParseLangSys(FT_Bytes raw, TLangSysRecord* rec);
  void ParseFeatureList(FT_Bytes raw);
  void ParseFeature(FT_Bytes raw, TFeatureRecord* rec);
  void ParseLookupList(FT_Bytes raw);
  void ParseLookup(FT_Bytes raw, TLookup* rec);
  std::unique_ptr<TCoverageFormatBase> ParseCoverage(FT_Bytes raw);
  std::unique_ptr<TCoverageFormat1> ParseCoverageFormat1(FT_Bytes raw);
  std::unique_ptr<TCoverageFormat2> ParseCoverageFormat2(FT_Bytes raw);
  std::unique_ptr<TSubTableBase> ParseSingleSubst(FT_Bytes raw);
  std::unique_ptr<TSubTable1> ParseSingleSubstFormat1(FT_Bytes raw);
  std::unique_ptr<TSubTable2> ParseSingleSubstFormat2(FT_Bytes raw);

  absl::optional<uint32_t> GetVerticalGlyphSub(const TFeatureRecord& feature,
                                               uint32_t glyphnum) const;
  absl::optional<uint32_t> GetVerticalGlyphSub2(const TLookup& lookup,
                                                uint32_t glyphnum) const;
  int GetCoverageIndex(TCoverageFormatBase* Coverage, uint32_t g) const;

  uint8_t GetUInt8(FT_Bytes& p) const;
  int16_t GetInt16(FT_Bytes& p) const;
  uint16_t GetUInt16(FT_Bytes& p) const;
  int32_t GetInt32(FT_Bytes& p) const;
  uint32_t GetUInt32(FT_Bytes& p) const;

  std::set<uint32_t> m_featureSet;
  std::vector<TScriptRecord> ScriptList;
  std::vector<TFeatureRecord> FeatureList;
  std::vector<TLookup> LookupList;
};

#endif  // CORE_FPDFAPI_FONT_CFX_CTTGSUBTABLE_H_

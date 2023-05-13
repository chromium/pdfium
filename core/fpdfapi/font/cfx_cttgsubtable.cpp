// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/fpdfapi/font/cfx_cttgsubtable.h"

#include <stdint.h>

#include <utility>

#include "core/fxcrt/data_vector.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/stl_util.h"
#include "core/fxge/cfx_fontmapper.h"

namespace {

bool IsVerticalFeatureTag(uint32_t tag) {
  static constexpr uint32_t kTags[] = {
      CFX_FontMapper::MakeTag('v', 'r', 't', '2'),
      CFX_FontMapper::MakeTag('v', 'e', 'r', 't'),
  };
  return tag == kTags[0] || tag == kTags[1];
}

}  // namespace

CFX_CTTGSUBTable::CFX_CTTGSUBTable(FT_Bytes gsub) {
  if (!LoadGSUBTable(gsub))
    return;

  for (const auto& script : script_list_) {
    for (const auto& record : script) {
      for (uint16_t index : record) {
        if (IsVerticalFeatureTag(feature_list_[index].feature_tag)) {
          feature_set_.insert(index);
        }
      }
    }
  }
  if (!feature_set_.empty()) {
    return;
  }

  int i = 0;
  for (const FeatureRecord& feature : feature_list_) {
    if (IsVerticalFeatureTag(feature.feature_tag)) {
      feature_set_.insert(i);
    }
    ++i;
  }
}

CFX_CTTGSUBTable::~CFX_CTTGSUBTable() = default;

bool CFX_CTTGSUBTable::LoadGSUBTable(FT_Bytes gsub) {
  if (FXSYS_UINT32_GET_MSBFIRST(gsub) != 0x00010000)
    return false;

  Parse(&gsub[FXSYS_UINT16_GET_MSBFIRST(gsub + 4)],
        &gsub[FXSYS_UINT16_GET_MSBFIRST(gsub + 6)],
        &gsub[FXSYS_UINT16_GET_MSBFIRST(gsub + 8)]);
  return true;
}

uint32_t CFX_CTTGSUBTable::GetVerticalGlyph(uint32_t glyphnum) const {
  for (uint32_t item : feature_set_) {
    absl::optional<uint32_t> result =
        GetVerticalGlyphSub(feature_list_[item], glyphnum);
    if (result.has_value())
      return result.value();
  }
  return 0;
}

absl::optional<uint32_t> CFX_CTTGSUBTable::GetVerticalGlyphSub(
    const FeatureRecord& feature,
    uint32_t glyphnum) const {
  for (int index : feature.lookup_list_indices) {
    if (!fxcrt::IndexInBounds(lookup_list_, index)) {
      continue;
    }
    if (lookup_list_[index].lookup_type != 1) {
      continue;
    }
    absl::optional<uint32_t> result =
        GetVerticalGlyphSub2(lookup_list_[index], glyphnum);
    if (result.has_value())
      return result.value();
  }
  return absl::nullopt;
}

absl::optional<uint32_t> CFX_CTTGSUBTable::GetVerticalGlyphSub2(
    const Lookup& lookup,
    uint32_t glyphnum) const {
  for (const auto& sub_table : lookup.sub_tables) {
    if (absl::holds_alternative<absl::monostate>(sub_table.table_data)) {
      continue;
    }
    int index = GetCoverageIndex(sub_table.coverage, glyphnum);
    if (absl::holds_alternative<int16_t>(sub_table.table_data)) {
      if (index >= 0) {
        return glyphnum + absl::get<int16_t>(sub_table.table_data);
      }
    } else {
      const auto& substitutes =
          absl::get<DataVector<uint16_t>>(sub_table.table_data);
      if (fxcrt::IndexInBounds(substitutes, index)) {
        return substitutes[index];
      }
    }
  }
  return absl::nullopt;
}

int CFX_CTTGSUBTable::GetCoverageIndex(const CoverageFormat& coverage,
                                       uint32_t g) const {
  if (absl::holds_alternative<absl::monostate>(coverage)) {
    return -1;
  }

  if (absl::holds_alternative<DataVector<uint16_t>>(coverage)) {
    int i = 0;
    const auto& glyph_array = absl::get<DataVector<uint16_t>>(coverage);
    for (const auto& glyph : glyph_array) {
      if (static_cast<uint32_t>(glyph) == g) {
        return i;
      }
      ++i;
    }
    return -1;
  }

  const auto& range_records = absl::get<std::vector<TRangeRecord>>(coverage);
  for (const auto& rangeRec : range_records) {
    uint32_t s = rangeRec.Start;
    uint32_t e = rangeRec.End;
    uint32_t si = rangeRec.StartCoverageIndex;
    if (s <= g && g <= e) {
      return si + g - s;
    }
  }
  return -1;
}

uint8_t CFX_CTTGSUBTable::GetUInt8(FT_Bytes& p) const {
  uint8_t ret = p[0];
  p += 1;
  return ret;
}

int16_t CFX_CTTGSUBTable::GetInt16(FT_Bytes& p) const {
  uint16_t ret = FXSYS_UINT16_GET_MSBFIRST(p);
  p += 2;
  return *(int16_t*)&ret;
}

uint16_t CFX_CTTGSUBTable::GetUInt16(FT_Bytes& p) const {
  uint16_t ret = FXSYS_UINT16_GET_MSBFIRST(p);
  p += 2;
  return ret;
}

int32_t CFX_CTTGSUBTable::GetInt32(FT_Bytes& p) const {
  uint32_t ret = FXSYS_UINT32_GET_MSBFIRST(p);
  p += 4;
  return *(int32_t*)&ret;
}

uint32_t CFX_CTTGSUBTable::GetUInt32(FT_Bytes& p) const {
  uint32_t ret = FXSYS_UINT32_GET_MSBFIRST(p);
  p += 4;
  return ret;
}

void CFX_CTTGSUBTable::Parse(FT_Bytes scriptlist,
                             FT_Bytes featurelist,
                             FT_Bytes lookuplist) {
  ParseScriptList(scriptlist);
  ParseFeatureList(featurelist);
  ParseLookupList(lookuplist);
}

void CFX_CTTGSUBTable::ParseScriptList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  script_list_ = std::vector<ScriptRecord>(GetUInt16(sp));
  for (auto& script : script_list_) {
    // Skip over "ScriptTag" field.
    sp += 4;
    script = ParseScript(&raw[GetUInt16(sp)]);
  }
}

CFX_CTTGSUBTable::ScriptRecord CFX_CTTGSUBTable::ParseScript(FT_Bytes raw) {
  // Skip over "DefaultLangSys" field.
  FT_Bytes sp = raw + 2;
  ScriptRecord result(GetUInt16(sp));
  for (auto& record : result) {
    // Skip over "LangSysTag" field.
    sp += 4;
    record = ParseLangSys(&raw[GetUInt16(sp)]);
  }
  return result;
}

CFX_CTTGSUBTable::FeatureIndices CFX_CTTGSUBTable::ParseLangSys(FT_Bytes raw) {
  // Skip over "LookupOrder" and "ReqFeatureIndex" fields.
  FT_Bytes sp = raw + 4;
  FeatureIndices result(GetUInt16(sp));
  for (auto& element : result) {
    element = GetUInt16(sp);
  }
  return result;
}

void CFX_CTTGSUBTable::ParseFeatureList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  feature_list_ = std::vector<FeatureRecord>(GetUInt16(sp));
  for (auto& record : feature_list_) {
    record.feature_tag = GetUInt32(sp);
    record.lookup_list_indices =
        ParseFeatureLookupListIndices(&raw[GetUInt16(sp)]);
  }
}

DataVector<uint16_t> CFX_CTTGSUBTable::ParseFeatureLookupListIndices(
    FT_Bytes raw) {
  // Skip over "FeatureParams" field.
  FT_Bytes sp = raw + 2;
  DataVector<uint16_t> result(GetUInt16(sp));
  for (auto& index : result) {
    index = GetUInt16(sp);
  }
  return result;
}

void CFX_CTTGSUBTable::ParseLookupList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  lookup_list_ = std::vector<Lookup>(GetUInt16(sp));
  for (auto& lookup : lookup_list_) {
    lookup = ParseLookup(&raw[GetUInt16(sp)]);
  }
}

CFX_CTTGSUBTable::Lookup CFX_CTTGSUBTable::ParseLookup(FT_Bytes raw) {
  FT_Bytes sp = raw;
  CFX_CTTGSUBTable::Lookup result;
  result.lookup_type = GetUInt16(sp);
  // Skip over "LookupFlag" field.
  sp += 2;
  result.sub_tables = Lookup::SubTables(GetUInt16(sp));
  if (result.lookup_type != 1) {
    return result;
  }

  for (auto& sub_table : result.sub_tables) {
    sub_table = ParseSingleSubst(&raw[GetUInt16(sp)]);
  }
  return result;
}

CFX_CTTGSUBTable::CoverageFormat CFX_CTTGSUBTable::ParseCoverage(FT_Bytes raw) {
  FT_Bytes sp = raw;
  uint16_t format = GetUInt16(sp);
  if (format != 1 && format != 2) {
    return absl::monostate();
  }

  if (format == 1) {
    DataVector<uint16_t> glyph_array(GetUInt16(sp));
    for (auto& glyph : glyph_array) {
      glyph = GetUInt16(sp);
    }
    return glyph_array;
  }

  std::vector<TRangeRecord> range_records(GetUInt16(sp));
  for (auto& range_rec : range_records) {
    range_rec.Start = GetUInt16(sp);
    range_rec.End = GetUInt16(sp);
    range_rec.StartCoverageIndex = GetUInt16(sp);
  }
  return range_records;
}

CFX_CTTGSUBTable::SubTable CFX_CTTGSUBTable::ParseSingleSubst(FT_Bytes raw) {
  FT_Bytes sp = raw;
  uint16_t format = GetUInt16(sp);
  SubTable rec;
  if (format != 1 && format != 2) {
    return rec;
  }

  uint16_t offset = GetUInt16(sp);
  rec.coverage = ParseCoverage(&raw[offset]);
  if (format == 1) {
    rec.table_data = GetInt16(sp);
  } else {
    DataVector<uint16_t> table_data(GetUInt16(sp));
    for (auto& substitute : table_data) {
      substitute = GetUInt16(sp);
    }
    rec.table_data = std::move(table_data);
  }
  return rec;
}

CFX_CTTGSUBTable::FeatureRecord::FeatureRecord() = default;

CFX_CTTGSUBTable::FeatureRecord::~FeatureRecord() = default;

CFX_CTTGSUBTable::TRangeRecord::TRangeRecord() = default;

CFX_CTTGSUBTable::SubTable::SubTable() = default;

CFX_CTTGSUBTable::SubTable::SubTable(SubTable&& that) noexcept = default;

CFX_CTTGSUBTable::SubTable& CFX_CTTGSUBTable::SubTable::operator=(
    SubTable&& that) noexcept = default;

CFX_CTTGSUBTable::SubTable::~SubTable() = default;

CFX_CTTGSUBTable::Lookup::Lookup() = default;

CFX_CTTGSUBTable::Lookup::Lookup(Lookup&& that) noexcept = default;

CFX_CTTGSUBTable::Lookup& CFX_CTTGSUBTable::Lookup::operator=(
    Lookup&& that) noexcept = default;

CFX_CTTGSUBTable::Lookup::~Lookup() = default;

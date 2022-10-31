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

  for (const TScriptRecord& script : ScriptList) {
    for (const auto& record : script.LangSysRecords) {
      for (uint16_t index : record.FeatureIndices) {
        if (IsVerticalFeatureTag(FeatureList[index].FeatureTag))
          m_featureSet.insert(index);
      }
    }
  }
  if (!m_featureSet.empty())
    return;

  int i = 0;
  for (const TFeatureRecord& feature : FeatureList) {
    if (IsVerticalFeatureTag(feature.FeatureTag))
      m_featureSet.insert(i);
    ++i;
  }
}

CFX_CTTGSUBTable::~CFX_CTTGSUBTable() = default;

bool CFX_CTTGSUBTable::LoadGSUBTable(FT_Bytes gsub) {
  if (FXSYS_UINT32_GET_MSBFIRST(gsub) != 0x00010000)
    return false;

  return Parse(&gsub[FXSYS_UINT16_GET_MSBFIRST(gsub + 4)],
               &gsub[FXSYS_UINT16_GET_MSBFIRST(gsub + 6)],
               &gsub[FXSYS_UINT16_GET_MSBFIRST(gsub + 8)]);
}

uint32_t CFX_CTTGSUBTable::GetVerticalGlyph(uint32_t glyphnum) const {
  for (uint32_t item : m_featureSet) {
    absl::optional<uint32_t> result =
        GetVerticalGlyphSub(FeatureList[item], glyphnum);
    if (result.has_value())
      return result.value();
  }
  return 0;
}

absl::optional<uint32_t> CFX_CTTGSUBTable::GetVerticalGlyphSub(
    const TFeatureRecord& feature,
    uint32_t glyphnum) const {
  for (int index : feature.LookupListIndices) {
    if (!fxcrt::IndexInBounds(LookupList, index))
      continue;
    if (LookupList[index].LookupType != 1)
      continue;
    absl::optional<uint32_t> result =
        GetVerticalGlyphSub2(LookupList[index], glyphnum);
    if (result.has_value())
      return result.value();
  }
  return absl::nullopt;
}

absl::optional<uint32_t> CFX_CTTGSUBTable::GetVerticalGlyphSub2(
    const TLookup& lookup,
    uint32_t glyphnum) const {
  for (const auto& subTable : lookup.SubTables) {
    switch (subTable->SubstFormat) {
      case 1: {
        auto* tbl1 = static_cast<TSubTable1*>(subTable.get());
        if (GetCoverageIndex(tbl1->Coverage.get(), glyphnum) >= 0) {
          return glyphnum + tbl1->DeltaGlyphID;
        }
        break;
      }
      case 2: {
        auto* tbl2 = static_cast<TSubTable2*>(subTable.get());
        int index = GetCoverageIndex(tbl2->Coverage.get(), glyphnum);
        if (fxcrt::IndexInBounds(tbl2->Substitutes, index)) {
          return tbl2->Substitutes[index];
        }
        break;
      }
    }
  }
  return absl::nullopt;
}

int CFX_CTTGSUBTable::GetCoverageIndex(TCoverageFormatBase* Coverage,
                                       uint32_t g) const {
  if (!Coverage)
    return -1;

  switch (Coverage->CoverageFormat) {
    case 1: {
      int i = 0;
      TCoverageFormat1* c1 = static_cast<TCoverageFormat1*>(Coverage);
      for (const auto& glyph : c1->GlyphArray) {
        if (static_cast<uint32_t>(glyph) == g)
          return i;
        ++i;
      }
      return -1;
    }
    case 2: {
      TCoverageFormat2* c2 = static_cast<TCoverageFormat2*>(Coverage);
      for (const auto& rangeRec : c2->RangeRecords) {
        uint32_t s = rangeRec.Start;
        uint32_t e = rangeRec.End;
        uint32_t si = rangeRec.StartCoverageIndex;
        if (s <= g && g <= e)
          return si + g - s;
      }
      return -1;
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

bool CFX_CTTGSUBTable::Parse(FT_Bytes scriptlist,
                             FT_Bytes featurelist,
                             FT_Bytes lookuplist) {
  ParseScriptList(scriptlist);
  ParseFeatureList(featurelist);
  ParseLookupList(lookuplist);
  return true;
}

void CFX_CTTGSUBTable::ParseScriptList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  ScriptList = std::vector<TScriptRecord>(GetUInt16(sp));
  for (auto& scriptRec : ScriptList) {
    scriptRec.ScriptTag = GetUInt32(sp);
    ParseScript(&raw[GetUInt16(sp)], &scriptRec);
  }
}

void CFX_CTTGSUBTable::ParseScript(FT_Bytes raw, TScriptRecord* rec) {
  FT_Bytes sp = raw;
  rec->DefaultLangSys = GetUInt16(sp);
  rec->LangSysRecords = std::vector<TLangSysRecord>(GetUInt16(sp));
  for (auto& sysRecord : rec->LangSysRecords) {
    sysRecord.LangSysTag = GetUInt32(sp);
    ParseLangSys(&raw[GetUInt16(sp)], &sysRecord);
  }
}

void CFX_CTTGSUBTable::ParseLangSys(FT_Bytes raw, TLangSysRecord* rec) {
  FT_Bytes sp = raw;
  rec->LookupOrder = GetUInt16(sp);
  rec->ReqFeatureIndex = GetUInt16(sp);
  rec->FeatureIndices = DataVector<uint16_t>(GetUInt16(sp));
  for (auto& element : rec->FeatureIndices)
    element = GetUInt16(sp);
}

void CFX_CTTGSUBTable::ParseFeatureList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  FeatureList = std::vector<TFeatureRecord>(GetUInt16(sp));
  for (auto& featureRec : FeatureList) {
    featureRec.FeatureTag = GetUInt32(sp);
    ParseFeature(&raw[GetUInt16(sp)], &featureRec);
  }
}

void CFX_CTTGSUBTable::ParseFeature(FT_Bytes raw, TFeatureRecord* rec) {
  FT_Bytes sp = raw;
  rec->FeatureParams = GetUInt16(sp);
  rec->LookupListIndices = DataVector<uint16_t>(GetUInt16(sp));
  for (auto& listIndex : rec->LookupListIndices)
    listIndex = GetUInt16(sp);
}

void CFX_CTTGSUBTable::ParseLookupList(FT_Bytes raw) {
  FT_Bytes sp = raw;
  LookupList = std::vector<TLookup>(GetUInt16(sp));
  for (auto& lookup : LookupList)
    ParseLookup(&raw[GetUInt16(sp)], &lookup);
}

void CFX_CTTGSUBTable::ParseLookup(FT_Bytes raw, TLookup* rec) {
  FT_Bytes sp = raw;
  rec->LookupType = GetUInt16(sp);
  rec->LookupFlag = GetUInt16(sp);
  rec->SubTables = std::vector<std::unique_ptr<TSubTableBase>>(GetUInt16(sp));
  if (rec->LookupType != 1)
    return;

  for (auto& subTable : rec->SubTables)
    subTable = ParseSingleSubst(&raw[GetUInt16(sp)]);
}

std::unique_ptr<CFX_CTTGSUBTable::TCoverageFormatBase>
CFX_CTTGSUBTable::ParseCoverage(FT_Bytes raw) {
  FT_Bytes sp = raw;
  uint16_t format = GetUInt16(sp);
  if (format == 1)
    return ParseCoverageFormat1(raw);
  if (format == 2)
    return ParseCoverageFormat2(raw);
  return nullptr;
}

std::unique_ptr<CFX_CTTGSUBTable::TCoverageFormat1>
CFX_CTTGSUBTable::ParseCoverageFormat1(FT_Bytes raw) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  auto rec = std::make_unique<TCoverageFormat1>(GetUInt16(sp));
  for (auto& glyph : rec->GlyphArray)
    glyph = GetUInt16(sp);
  return rec;
}

std::unique_ptr<CFX_CTTGSUBTable::TCoverageFormat2>
CFX_CTTGSUBTable::ParseCoverageFormat2(FT_Bytes raw) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  auto rec = std::make_unique<TCoverageFormat2>(GetUInt16(sp));
  for (auto& rangeRec : rec->RangeRecords) {
    rangeRec.Start = GetUInt16(sp);
    rangeRec.End = GetUInt16(sp);
    rangeRec.StartCoverageIndex = GetUInt16(sp);
  }
  return rec;
}

std::unique_ptr<CFX_CTTGSUBTable::TSubTableBase>
CFX_CTTGSUBTable::ParseSingleSubst(FT_Bytes raw) {
  FT_Bytes sp = raw;
  uint16_t format = GetUInt16(sp);
  if (format == 1)
    return ParseSingleSubstFormat1(raw);
  if (format == 2)
    return ParseSingleSubstFormat2(raw);
  return nullptr;
}

std::unique_ptr<CFX_CTTGSUBTable::TSubTable1>
CFX_CTTGSUBTable::ParseSingleSubstFormat1(FT_Bytes raw) {
  FT_Bytes sp = raw;
  GetUInt16(sp);
  uint16_t offset = GetUInt16(sp);
  auto rec = std::make_unique<TSubTable1>();
  rec->Coverage = ParseCoverage(&raw[offset]);
  rec->DeltaGlyphID = GetInt16(sp);
  return rec;
}

std::unique_ptr<CFX_CTTGSUBTable::TSubTable2>
CFX_CTTGSUBTable::ParseSingleSubstFormat2(FT_Bytes raw) {
  FT_Bytes sp = raw;
  (void)GetUInt16(sp);
  uint16_t offset = GetUInt16(sp);
  auto rec = std::make_unique<TSubTable2>();
  rec->Coverage = ParseCoverage(&raw[offset]);
  rec->Substitutes = DataVector<uint16_t>(GetUInt16(sp));
  for (auto& substitute : rec->Substitutes)
    substitute = GetUInt16(sp);
  return rec;
}

CFX_CTTGSUBTable::TLangSysRecord::TLangSysRecord() = default;

CFX_CTTGSUBTable::TLangSysRecord::~TLangSysRecord() = default;

CFX_CTTGSUBTable::TScriptRecord::TScriptRecord() = default;

CFX_CTTGSUBTable::TScriptRecord::~TScriptRecord() = default;

CFX_CTTGSUBTable::TFeatureRecord::TFeatureRecord() = default;

CFX_CTTGSUBTable::TFeatureRecord::~TFeatureRecord() = default;

CFX_CTTGSUBTable::TRangeRecord::TRangeRecord() = default;

CFX_CTTGSUBTable::TCoverageFormat1::TCoverageFormat1(size_t initial_size)
    : TCoverageFormatBase(1), GlyphArray(initial_size) {}

CFX_CTTGSUBTable::TCoverageFormat1::~TCoverageFormat1() = default;

CFX_CTTGSUBTable::TCoverageFormat2::TCoverageFormat2(size_t initial_size)
    : TCoverageFormatBase(2), RangeRecords(initial_size) {}

CFX_CTTGSUBTable::TCoverageFormat2::~TCoverageFormat2() = default;

CFX_CTTGSUBTable::TDevice::TDevice() = default;

CFX_CTTGSUBTable::TSubTableBase::TSubTableBase(uint16_t format)
    : SubstFormat(format) {}

CFX_CTTGSUBTable::TSubTableBase::~TSubTableBase() = default;

CFX_CTTGSUBTable::TSubTable1::TSubTable1() : TSubTableBase(1) {}

CFX_CTTGSUBTable::TSubTable1::~TSubTable1() = default;

CFX_CTTGSUBTable::TSubTable2::TSubTable2() : TSubTableBase(2) {}

CFX_CTTGSUBTable::TSubTable2::~TSubTable2() = default;

CFX_CTTGSUBTable::TLookup::TLookup() = default;

CFX_CTTGSUBTable::TLookup::~TLookup() = default;

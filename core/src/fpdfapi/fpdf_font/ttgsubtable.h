// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFAPI_FPDF_FONT_TTGSUBTABLE_H_
#define CORE_SRC_FPDFAPI_FPDF_FONT_TTGSUBTABLE_H_

#include <stdint.h>

#include <map>

#include "core/include/fxcrt/fx_basic.h"
#include "core/include/fxge/fx_font.h"
#include "core/include/fxge/fx_freetype.h"

class CFX_GlyphMap {
 public:
  CFX_GlyphMap();
  ~CFX_GlyphMap();
  void SetAt(int key, int value);
  FX_BOOL Lookup(int key, int& value);

 protected:
  CFX_BinaryBuf m_Buffer;
};
class CFX_CTTGSUBTable {
 public:
  CFX_CTTGSUBTable(void) : m_bFeautureMapLoad(FALSE), loaded(false) {}
  CFX_CTTGSUBTable(FT_Bytes gsub) : m_bFeautureMapLoad(FALSE), loaded(false) {
    LoadGSUBTable(gsub);
  }
  virtual ~CFX_CTTGSUBTable() {}
  bool IsOk(void) const { return loaded; }
  bool LoadGSUBTable(FT_Bytes gsub);
  bool GetVerticalGlyph(uint32_t glyphnum, uint32_t* vglyphnum);

 private:
  struct tt_gsub_header {
    uint32_t Version;
    uint16_t ScriptList;
    uint16_t FeatureList;
    uint16_t LookupList;
  };
  struct TLangSys {
    uint16_t LookupOrder;
    uint16_t ReqFeatureIndex;
    uint16_t FeatureCount;
    uint16_t* FeatureIndex;
    TLangSys()
        : LookupOrder(0),
          ReqFeatureIndex(0),
          FeatureCount(0),
          FeatureIndex(NULL) {}
    ~TLangSys() { delete[] FeatureIndex; }

   private:
    TLangSys(const TLangSys&);
    TLangSys& operator=(const TLangSys&);
  };
  struct TLangSysRecord {
    uint32_t LangSysTag;
    struct TLangSys LangSys;
    TLangSysRecord() : LangSysTag(0) {}

   private:
    TLangSysRecord(const TLangSysRecord&);
    TLangSysRecord& operator=(const TLangSysRecord&);
  };
  struct TScript {
    uint16_t DefaultLangSys;
    uint16_t LangSysCount;
    struct TLangSysRecord* LangSysRecord;
    TScript() : DefaultLangSys(0), LangSysCount(0), LangSysRecord(NULL) {}
    ~TScript() { delete[] LangSysRecord; }

   private:
    TScript(const TScript&);
    TScript& operator=(const TScript&);
  };
  struct TScriptRecord {
    uint32_t ScriptTag;
    struct TScript Script;
    TScriptRecord() : ScriptTag(0) {}

   private:
    TScriptRecord(const TScriptRecord&);
    TScriptRecord& operator=(const TScriptRecord&);
  };
  struct TScriptList {
    uint16_t ScriptCount;
    struct TScriptRecord* ScriptRecord;
    TScriptList() : ScriptCount(0), ScriptRecord(NULL) {}
    ~TScriptList() { delete[] ScriptRecord; }

   private:
    TScriptList(const TScriptList&);
    TScriptList& operator=(const TScriptList&);
  };
  struct TFeature {
    uint16_t FeatureParams;
    int LookupCount;
    uint16_t* LookupListIndex;
    TFeature() : FeatureParams(0), LookupCount(0), LookupListIndex(NULL) {}
    ~TFeature() { delete[] LookupListIndex; }

   private:
    TFeature(const TFeature&);
    TFeature& operator=(const TFeature&);
  };
  struct TFeatureRecord {
    uint32_t FeatureTag;
    struct TFeature Feature;
    TFeatureRecord() : FeatureTag(0) {}

   private:
    TFeatureRecord(const TFeatureRecord&);
    TFeatureRecord& operator=(const TFeatureRecord&);
  };
  struct TFeatureList {
    int FeatureCount;
    struct TFeatureRecord* FeatureRecord;
    TFeatureList() : FeatureCount(0), FeatureRecord(NULL) {}
    ~TFeatureList() { delete[] FeatureRecord; }

   private:
    TFeatureList(const TFeatureList&);
    TFeatureList& operator=(const TFeatureList&);
  };
  enum TLookupFlag {
    LOOKUPFLAG_RightToLeft = 0x0001,
    LOOKUPFLAG_IgnoreBaseGlyphs = 0x0002,
    LOOKUPFLAG_IgnoreLigatures = 0x0004,
    LOOKUPFLAG_IgnoreMarks = 0x0008,
    LOOKUPFLAG_Reserved = 0x00F0,
    LOOKUPFLAG_MarkAttachmentType = 0xFF00,
  };
  struct TCoverageFormatBase {
    uint16_t CoverageFormat;
    CFX_GlyphMap m_glyphMap;
    TCoverageFormatBase() : CoverageFormat(0) {}
    virtual ~TCoverageFormatBase() {}

   private:
    TCoverageFormatBase(const TCoverageFormatBase&);
    TCoverageFormatBase& operator=(const TCoverageFormatBase&);
  };
  struct TCoverageFormat1 : public TCoverageFormatBase {
    uint16_t GlyphCount;
    uint16_t* GlyphArray;
    TCoverageFormat1() : GlyphCount(0), GlyphArray(NULL) { CoverageFormat = 1; }
    ~TCoverageFormat1() override { delete[] GlyphArray; }

   private:
    TCoverageFormat1(const TCoverageFormat1&);
    TCoverageFormat1& operator=(const TCoverageFormat1&);
  };
  struct TRangeRecord {
    uint16_t Start;
    uint16_t End;
    uint16_t StartCoverageIndex;
    TRangeRecord() : Start(0), End(0), StartCoverageIndex(0) {}
    friend bool operator>(const TRangeRecord& r1, const TRangeRecord& r2) {
      return r1.Start > r2.Start;
    }

   private:
    TRangeRecord(const TRangeRecord&);
  };
  struct TCoverageFormat2 : public TCoverageFormatBase {
    uint16_t RangeCount;
    struct TRangeRecord* RangeRecord;
    TCoverageFormat2() : RangeCount(0), RangeRecord(NULL) {
      CoverageFormat = 2;
    }
    ~TCoverageFormat2() override { delete[] RangeRecord; }

   private:
    TCoverageFormat2(const TCoverageFormat2&);
    TCoverageFormat2& operator=(const TCoverageFormat2&);
  };
  struct TClassDefFormatBase {
    uint16_t ClassFormat;
    TClassDefFormatBase() : ClassFormat(0) {}
    virtual ~TClassDefFormatBase() {}

   private:
    TClassDefFormatBase(const TClassDefFormatBase&);
    TClassDefFormatBase& operator=(const TClassDefFormatBase&);
  };
  struct TClassDefFormat1 : public TClassDefFormatBase {
    uint16_t StartGlyph;
    uint16_t GlyphCount;
    uint16_t* ClassValueArray;
    TClassDefFormat1() : StartGlyph(0), GlyphCount(0), ClassValueArray(NULL) {
      ClassFormat = 1;
    }
    ~TClassDefFormat1() override { delete[] ClassValueArray; }

   private:
    TClassDefFormat1(const TClassDefFormat1&);
    TClassDefFormat1& operator=(const TClassDefFormat1&);
  };
  struct TClassRangeRecord {
    uint16_t Start;
    uint16_t End;
    uint16_t Class;
    TClassRangeRecord() : Start(0), End(0), Class(0) {}

   private:
    TClassRangeRecord(const TClassRangeRecord&);
    TClassRangeRecord& operator=(const TClassRangeRecord&);
  };
  struct TClassDefFormat2 : public TClassDefFormatBase {
    uint16_t ClassRangeCount;
    struct TClassRangeRecord* ClassRangeRecord;
    TClassDefFormat2() : ClassRangeCount(0), ClassRangeRecord(NULL) {
      ClassFormat = 2;
    }
    ~TClassDefFormat2() override { delete[] ClassRangeRecord; }

   private:
    TClassDefFormat2(const TClassDefFormat2&);
    TClassDefFormat2& operator=(const TClassDefFormat2&);
  };
  struct TDevice {
    uint16_t StartSize;
    uint16_t EndSize;
    uint16_t DeltaFormat;
    TDevice() : StartSize(0), EndSize(0), DeltaFormat(0) {}

   private:
    TDevice(const TDevice&);
    TDevice& operator=(const TDevice&);
  };
  struct TSubTableBase {
    uint16_t SubstFormat;
    TSubTableBase() : SubstFormat(0) {}
    virtual ~TSubTableBase() {}

   private:
    TSubTableBase(const TSubTableBase&);
    TSubTableBase& operator=(const TSubTableBase&);
  };
  struct TSingleSubstFormat1 : public TSubTableBase {
    TCoverageFormatBase* Coverage;
    int16_t DeltaGlyphID;
    TSingleSubstFormat1() : Coverage(NULL), DeltaGlyphID(0) { SubstFormat = 1; }
    ~TSingleSubstFormat1() override { delete Coverage; }

   private:
    TSingleSubstFormat1(const TSingleSubstFormat1&);
    TSingleSubstFormat1& operator=(const TSingleSubstFormat1&);
  };
  struct TSingleSubstFormat2 : public TSubTableBase {
    TCoverageFormatBase* Coverage;
    uint16_t GlyphCount;
    uint16_t* Substitute;
    TSingleSubstFormat2() : Coverage(NULL), GlyphCount(0), Substitute(NULL) {
      SubstFormat = 2;
    }
    ~TSingleSubstFormat2() override {
      delete Coverage;
      delete[] Substitute;
    }

   private:
    TSingleSubstFormat2(const TSingleSubstFormat2&);
    TSingleSubstFormat2& operator=(const TSingleSubstFormat2&);
  };
  struct TLookup {
    uint16_t LookupType;
    uint16_t LookupFlag;
    uint16_t SubTableCount;
    struct TSubTableBase** SubTable;
    TLookup()
        : LookupType(0), LookupFlag(0), SubTableCount(0), SubTable(NULL) {}
    ~TLookup() {
      if (SubTable) {
        for (int i = 0; i < SubTableCount; ++i)
          delete SubTable[i];
        delete[] SubTable;
      }
    }

   private:
    TLookup(const TLookup&);
    TLookup& operator=(const TLookup&);
  };
  struct TLookupList {
    int LookupCount;
    struct TLookup* Lookup;
    TLookupList() : LookupCount(0), Lookup(NULL) {}
    ~TLookupList() { delete[] Lookup; }

   private:
    TLookupList(const TLookupList&);
    TLookupList& operator=(const TLookupList&);
  };
  bool Parse(FT_Bytes scriptlist, FT_Bytes featurelist, FT_Bytes lookuplist);
  void ParseScriptList(FT_Bytes raw, TScriptList* rec);
  void ParseScript(FT_Bytes raw, TScript* rec);
  void ParseLangSys(FT_Bytes raw, TLangSys* rec);
  void ParseFeatureList(FT_Bytes raw, TFeatureList* rec);
  void ParseFeature(FT_Bytes raw, TFeature* rec);
  void ParseLookupList(FT_Bytes raw, TLookupList* rec);
  void ParseLookup(FT_Bytes raw, TLookup* rec);
  void ParseCoverage(FT_Bytes raw, TCoverageFormatBase** rec);
  void ParseCoverageFormat1(FT_Bytes raw, TCoverageFormat1* rec);
  void ParseCoverageFormat2(FT_Bytes raw, TCoverageFormat2* rec);
  void ParseSingleSubst(FT_Bytes raw, TSubTableBase** rec);
  void ParseSingleSubstFormat1(FT_Bytes raw, TSingleSubstFormat1* rec);
  void ParseSingleSubstFormat2(FT_Bytes raw, TSingleSubstFormat2* rec);
  bool GetVerticalGlyphSub(uint32_t glyphnum,
                           uint32_t* vglyphnum,
                           struct TFeature* Feature);
  bool GetVerticalGlyphSub2(uint32_t glyphnum,
                            uint32_t* vglyphnum,
                            struct TLookup* Lookup);
  int GetCoverageIndex(struct TCoverageFormatBase* Coverage, uint32_t g);
  uint8_t GetUInt8(FT_Bytes& p) const {
    uint8_t ret = p[0];
    p += 1;
    return ret;
  }
  int16_t GetInt16(FT_Bytes& p) const {
    uint16_t ret = p[0] << 8 | p[1];
    p += 2;
    return *(int16_t*)&ret;
  }
  uint16_t GetUInt16(FT_Bytes& p) const {
    uint16_t ret = p[0] << 8 | p[1];
    p += 2;
    return ret;
  }
  int32_t GetInt32(FT_Bytes& p) const {
    uint32_t ret = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    p += 4;
    return *(int32_t*)&ret;
  }
  uint32_t GetUInt32(FT_Bytes& p) const {
    uint32_t ret = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
    p += 4;
    return ret;
  }
  std::map<FX_DWORD, FX_DWORD> m_featureMap;
  FX_BOOL m_bFeautureMapLoad;
  bool loaded;
  struct tt_gsub_header header;
  struct TScriptList ScriptList;
  struct TFeatureList FeatureList;
  struct TLookupList LookupList;
};
class CFX_GSUBTable final : public IFX_GSUBTable {
 public:
  ~CFX_GSUBTable() override {}
  FX_BOOL GetVerticalGlyph(FX_DWORD glyphnum, FX_DWORD* vglyphnum) override;

  CFX_CTTGSUBTable m_GsubImp;
};

#endif  // CORE_SRC_FPDFAPI_FPDF_FONT_TTGSUBTABLE_H_

// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef TTGSUBTable_H
#define TTGSUBTable_H
#include "../../../include/fxge/fx_freetype.h"
#include "../../../include/fxcrt/fx_basic.h"
#include "common.h"
class CFX_GlyphMap
{
public:
    CFX_GlyphMap();
    ~CFX_GlyphMap();
    void	SetAt(int key, int value);
    FX_BOOL	Lookup(int key, int &value);
protected:
    CFX_BinaryBuf	m_Buffer;
};
class CFX_CTTGSUBTable : public CFX_Object
{
public:
    CFX_CTTGSUBTable(void): m_bFeautureMapLoad(FALSE), loaded(false) {};
    CFX_CTTGSUBTable(FT_Bytes gsub): m_bFeautureMapLoad(FALSE), loaded(false)
    {
        LoadGSUBTable(gsub);
    }
    virtual ~CFX_CTTGSUBTable() {}
    bool IsOk(void) const
    {
        return loaded;
    }
    bool LoadGSUBTable(FT_Bytes gsub);
    bool GetVerticalGlyph(TT_uint32_t glyphnum, TT_uint32_t *vglyphnum);
private:
    struct tt_gsub_header {
        TT_uint32_t Version;
        TT_uint16_t ScriptList;
        TT_uint16_t FeatureList;
        TT_uint16_t LookupList;
    };
    struct TLangSys {
        TT_uint16_t LookupOrder;
        TT_uint16_t ReqFeatureIndex;
        TT_uint16_t FeatureCount;
        TT_uint16_t *FeatureIndex;
        TLangSys(): LookupOrder(0), ReqFeatureIndex(0), FeatureCount(0), FeatureIndex(NULL) {}
        ~TLangSys()
        {
            if(FeatureIndex) {
                delete[] FeatureIndex;
            }
        }
    private:
        TLangSys(const TLangSys&);
        TLangSys& operator=(const TLangSys&);
    };
    struct TLangSysRecord {
        TT_uint32_t LangSysTag;
        struct TLangSys LangSys;
        TLangSysRecord(): LangSysTag(0) {}
    private:
        TLangSysRecord(const TLangSysRecord&);
        TLangSysRecord& operator=(const TLangSysRecord&);
    };
    struct TScript {
        TT_uint16_t DefaultLangSys;
        TT_uint16_t LangSysCount;
        struct TLangSysRecord *LangSysRecord;
        TScript(): DefaultLangSys(0), LangSysCount(0), LangSysRecord(NULL) {}
        ~TScript()
        {
            if(LangSysRecord) {
                delete[] LangSysRecord;
            }
        }
    private:
        TScript(const TScript&);
        TScript& operator=(const TScript&);
    };
    struct TScriptRecord {
        TT_uint32_t ScriptTag;
        struct TScript Script;
        TScriptRecord(): ScriptTag(0) {}
    private:
        TScriptRecord(const TScriptRecord&);
        TScriptRecord& operator=(const TScriptRecord&);
    };
    struct TScriptList {
        TT_uint16_t ScriptCount;
        struct TScriptRecord *ScriptRecord;
        TScriptList(): ScriptCount(0), ScriptRecord(NULL) {}
        ~TScriptList()
        {
            if(ScriptRecord) {
                delete[] ScriptRecord;
            }
        }
    private:
        TScriptList(const TScriptList&);
        TScriptList& operator=(const TScriptList&);
    };
    struct TFeature {
        TT_uint16_t FeatureParams;
        int LookupCount;
        TT_uint16_t *LookupListIndex;
        TFeature(): FeatureParams(0), LookupCount(0), LookupListIndex(NULL) {}
        ~TFeature()
        {
            if(LookupListIndex) {
                delete[] LookupListIndex;
            }
        }
    private:
        TFeature(const TFeature&);
        TFeature& operator=(const TFeature&);
    };
    struct TFeatureRecord {
        TT_uint32_t FeatureTag;
        struct TFeature Feature;
        TFeatureRecord(): FeatureTag(0) {}
    private:
        TFeatureRecord(const TFeatureRecord&);
        TFeatureRecord& operator=(const TFeatureRecord&);
    };
    struct TFeatureList {
        int FeatureCount;
        struct TFeatureRecord *FeatureRecord;
        TFeatureList(): FeatureCount(0), FeatureRecord(NULL) {}
        ~TFeatureList()
        {
            if(FeatureRecord) {
                delete[] FeatureRecord;
            }
        }
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
        TT_uint16_t CoverageFormat;
        CFX_GlyphMap m_glyphMap;
        TCoverageFormatBase(): CoverageFormat(0) {}
        virtual ~TCoverageFormatBase() {}
    private:
        TCoverageFormatBase(const TCoverageFormatBase&);
        TCoverageFormatBase& operator=(const TCoverageFormatBase&);
    };
    struct TCoverageFormat1: public TCoverageFormatBase {
        TT_uint16_t GlyphCount;
        TT_uint16_t *GlyphArray;
        TCoverageFormat1(): GlyphCount(0), GlyphArray(NULL)
        {
            CoverageFormat = 1;
        }
        ~TCoverageFormat1()
        {
            if(GlyphArray) {
                delete[] GlyphArray;
            }
        }
    private:
        TCoverageFormat1(const TCoverageFormat1&);
        TCoverageFormat1& operator=(const TCoverageFormat1&);
    };
    struct TRangeRecord {
        TT_uint16_t Start;
        TT_uint16_t End;
        TT_uint16_t StartCoverageIndex;
        TRangeRecord(): Start(0), End(0), StartCoverageIndex(0) {}
        friend bool operator > (const TRangeRecord &r1, const TRangeRecord &r2)
        {
            return r1.Start > r2.Start;
        }
    private:
        TRangeRecord(const TRangeRecord&);
    };
    struct TCoverageFormat2: public TCoverageFormatBase {
        TT_uint16_t RangeCount;
        struct TRangeRecord *RangeRecord;
        TCoverageFormat2(): RangeCount(0), RangeRecord(NULL)
        {
            CoverageFormat = 2;
        }
        ~TCoverageFormat2()
        {
            if(RangeRecord) {
                delete[] RangeRecord;
            }
        }
    private:
        TCoverageFormat2(const TCoverageFormat2&);
        TCoverageFormat2& operator=(const TCoverageFormat2&);
    };
    struct TClassDefFormatBase {
        TT_uint16_t ClassFormat;
        TClassDefFormatBase(): ClassFormat(0) {}
        virtual ~TClassDefFormatBase() {}
    private:
        TClassDefFormatBase(const TClassDefFormatBase&);
        TClassDefFormatBase& operator=(const TClassDefFormatBase&);
    };
    struct TClassDefFormat1: public TClassDefFormatBase {
        TT_uint16_t StartGlyph;
        TT_uint16_t GlyphCount;
        TT_uint16_t *ClassValueArray;
        TClassDefFormat1(): StartGlyph(0), GlyphCount(0), ClassValueArray(NULL)
        {
            ClassFormat = 1;
        }
        ~TClassDefFormat1()
        {
            if(ClassValueArray) {
                delete[] ClassValueArray;
            }
        }
    private:
        TClassDefFormat1(const TClassDefFormat1&);
        TClassDefFormat1& operator=(const TClassDefFormat1&);
    };
    struct TClassRangeRecord {
        TT_uint16_t Start;
        TT_uint16_t End;
        TT_uint16_t Class;
        TClassRangeRecord(): Start(0), End(0), Class(0) {}
    private:
        TClassRangeRecord(const TClassRangeRecord&);
        TClassRangeRecord& operator=(const TClassRangeRecord&);
    };
    struct TClassDefFormat2: public TClassDefFormatBase {
        TT_uint16_t ClassRangeCount;
        struct TClassRangeRecord *ClassRangeRecord;
        TClassDefFormat2(): ClassRangeCount(0), ClassRangeRecord(NULL)
        {
            ClassFormat = 2;
        }
        ~TClassDefFormat2()
        {
            if(ClassRangeRecord) {
                delete[] ClassRangeRecord;
            }
        }
    private:
        TClassDefFormat2(const TClassDefFormat2&);
        TClassDefFormat2& operator=(const TClassDefFormat2&);
    };
    struct TDevice {
        TT_uint16_t StartSize;
        TT_uint16_t EndSize;
        TT_uint16_t DeltaFormat;
        TDevice(): StartSize(0), EndSize(0), DeltaFormat(0) {}
    private:
        TDevice(const TDevice&);
        TDevice& operator=(const TDevice&);
    };
    struct TSubTableBase {
        TT_uint16_t SubstFormat;
        TSubTableBase(): SubstFormat(0) {}
        virtual ~TSubTableBase() {}
    private:
        TSubTableBase(const TSubTableBase&);
        TSubTableBase& operator=(const TSubTableBase&);
    };
    struct TSingleSubstFormat1: public TSubTableBase {
        TCoverageFormatBase *Coverage;
        TT_int16_t DeltaGlyphID;
        TSingleSubstFormat1(): Coverage(NULL), DeltaGlyphID(0)
        {
            SubstFormat = 1;
        }
        ~TSingleSubstFormat1()
        {
            if(Coverage) {
                delete Coverage;
            }
        }
    private:
        TSingleSubstFormat1(const TSingleSubstFormat1&);
        TSingleSubstFormat1& operator=(const TSingleSubstFormat1&);
    };
    struct TSingleSubstFormat2: public TSubTableBase {
        TCoverageFormatBase *Coverage;
        TT_uint16_t GlyphCount;
        TT_uint16_t *Substitute;
        TSingleSubstFormat2(): Coverage(NULL), GlyphCount(0), Substitute(NULL)
        {
            SubstFormat = 2;
        }
        ~TSingleSubstFormat2()
        {
            if(Coverage) {
                delete Coverage;
            }
            if(Substitute) {
                delete[] Substitute;
            }
        }
    private:
        TSingleSubstFormat2(const TSingleSubstFormat2&);
        TSingleSubstFormat2& operator=(const TSingleSubstFormat2&);
    };
    struct TLookup {
        TT_uint16_t LookupType;
        TT_uint16_t LookupFlag;
        TT_uint16_t SubTableCount;
        struct TSubTableBase **SubTable;
        TLookup(): LookupType(0), LookupFlag(0), SubTableCount(0), SubTable(NULL) {}
        ~TLookup()
        {
            if(SubTableCount > 0 && SubTable != NULL) {
                for(int i = 0; i < SubTableCount; i++) {
                    delete SubTable[i];
                }
                delete[] SubTable;
            }
        }
    private:
        TLookup(const TLookup&);
        TLookup& operator=(const TLookup&);
    };
    struct TLookupList {
        int LookupCount;
        struct TLookup *Lookup;
        TLookupList(): LookupCount(0), Lookup(NULL) {}
        ~TLookupList()
        {
            if(Lookup) {
                delete[] Lookup;
            }
        }
    private:
        TLookupList(const TLookupList&);
        TLookupList& operator=(const TLookupList&);
    };
    bool Parse(
        FT_Bytes scriptlist,
        FT_Bytes featurelist,
        FT_Bytes lookuplist);
    void ParseScriptList(FT_Bytes raw, TScriptList *rec);
    void ParseScript(FT_Bytes raw, TScript *rec);
    void ParseLangSys(FT_Bytes raw, TLangSys *rec);
    void ParseFeatureList(FT_Bytes raw, TFeatureList *rec);
    void ParseFeature(FT_Bytes raw, TFeature *rec);
    void ParseLookupList(FT_Bytes raw, TLookupList *rec);
    void ParseLookup(FT_Bytes raw, TLookup *rec);
    void ParseCoverage(FT_Bytes raw, TCoverageFormatBase **rec);
    void ParseCoverageFormat1(FT_Bytes raw, TCoverageFormat1 *rec);
    void ParseCoverageFormat2(FT_Bytes raw, TCoverageFormat2 *rec);
    void ParseSingleSubst(FT_Bytes raw, TSubTableBase **rec);
    void ParseSingleSubstFormat1(FT_Bytes raw, TSingleSubstFormat1 *rec);
    void ParseSingleSubstFormat2(FT_Bytes raw, TSingleSubstFormat2 *rec);
    bool GetVerticalGlyphSub(
        TT_uint32_t glyphnum,
        TT_uint32_t *vglyphnum,
        struct TFeature *Feature);
    bool GetVerticalGlyphSub2(
        TT_uint32_t glyphnum,
        TT_uint32_t *vglyphnum,
        struct TLookup *Lookup);
    int GetCoverageIndex(struct TCoverageFormatBase *Coverage, TT_uint32_t g);
    TT_uint8_t GetUInt8(FT_Bytes& p) const
    {
        TT_uint8_t ret = p[0];
        p += 1;
        return ret;
    }
    TT_int16_t GetInt16(FT_Bytes& p) const
    {
        TT_uint16_t ret = p[0] << 8 | p[1];
        p += 2;
        return *(TT_int16_t*)&ret;
    }
    TT_uint16_t GetUInt16(FT_Bytes& p) const
    {
        TT_uint16_t ret = p[0] << 8 | p[1];
        p += 2;
        return ret;
    }
    TT_int32_t GetInt32(FT_Bytes& p) const
    {
        TT_uint32_t ret = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
        p += 4;
        return *(TT_int32_t*)&ret;
    }
    TT_uint32_t GetUInt32(FT_Bytes& p) const
    {
        TT_uint32_t ret = p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3];
        p += 4;
        return ret;
    }
    CFX_CMapDWordToDWord m_featureMap;
    FX_BOOL	m_bFeautureMapLoad;
    bool loaded;
    struct tt_gsub_header header;
    struct TScriptList ScriptList;
    struct TFeatureList FeatureList;
    struct TLookupList LookupList;
};
class CFX_GSUBTable FX_FINAL : public IFX_GSUBTable, public CFX_Object
{
public:
    virtual void	Release() FX_OVERRIDE
    {
        delete this;
    }
    virtual FX_BOOL GetVerticalGlyph(FX_DWORD glyphnum, FX_DWORD* vglyphnum) FX_OVERRIDE;

    CFX_CTTGSUBTable m_GsubImp;

private:
    ~CFX_GSUBTable() { }
};
#endif


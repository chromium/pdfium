// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxge/fx_freetype.h"
#include "ttgsubtable.h"
CFX_GlyphMap::CFX_GlyphMap()
{
}
CFX_GlyphMap::~CFX_GlyphMap()
{
}
extern "C" {
    static int _CompareInt(const void* p1, const void* p2)
    {
        return (*(FX_DWORD*)p1) - (*(FX_DWORD*)p2);
    }
};
struct _IntPair {
    FX_INT32 key;
    FX_INT32 value;
};
void CFX_GlyphMap::SetAt(int key, int value)
{
    FX_DWORD count = m_Buffer.GetSize() / sizeof(_IntPair);
    _IntPair* buf = (_IntPair*)m_Buffer.GetBuffer();
    _IntPair pair = {key, value};
    if (count == 0 || key > buf[count - 1].key) {
        m_Buffer.AppendBlock(&pair, sizeof(_IntPair));
        return;
    }
    int low = 0, high = count - 1;
    while (low <= high) {
        int mid = (low + high) / 2;
        if (buf[mid].key < key) {
            low = mid + 1;
        } else if (buf[mid].key > key) {
            high = mid - 1;
        } else {
            buf[mid].value = value;
            return;
        }
    }
    m_Buffer.InsertBlock(low * sizeof(_IntPair), &pair, sizeof(_IntPair));
}
FX_BOOL CFX_GlyphMap::Lookup(int key, int &value)
{
    FX_LPVOID pResult = FXSYS_bsearch(&key, m_Buffer.GetBuffer(), m_Buffer.GetSize() / sizeof(_IntPair),
                                      sizeof(_IntPair), _CompareInt);
    if (pResult == NULL) {
        return FALSE;
    }
    value = ((FX_DWORD*)pResult)[1];
    return TRUE;
}
bool CFX_CTTGSUBTable::LoadGSUBTable(FT_Bytes gsub)
{
    header.Version = gsub[0] << 24 | gsub[1] << 16 | gsub[2] << 8 | gsub[3];
    if(header.Version != 0x00010000) {
        return false;
    }
    header.ScriptList  = gsub[4] << 8 | gsub[5];
    header.FeatureList = gsub[6] << 8 | gsub[7];
    header.LookupList  = gsub[8] << 8 | gsub[9];
    return Parse(
               &gsub[header.ScriptList],
               &gsub[header.FeatureList],
               &gsub[header.LookupList]);
}
bool CFX_CTTGSUBTable::GetVerticalGlyph(TT_uint32_t glyphnum, TT_uint32_t *vglyphnum)
{
    TT_uint32_t tag[] = {
        (TT_uint8_t)'v' << 24 |
        (TT_uint8_t)'r' << 16 |
        (TT_uint8_t)'t' <<  8 |
        (TT_uint8_t)'2',
        (TT_uint8_t)'v' << 24 |
        (TT_uint8_t)'e' << 16 |
        (TT_uint8_t)'r' <<  8 |
        (TT_uint8_t)'t',
    };
    if (!m_bFeautureMapLoad) {
        for (int i = 0; i < ScriptList.ScriptCount; i++) {
            for (int j = 0; j <	(ScriptList.ScriptRecord + i)->Script.LangSysCount; ++j) {
                for (int k = 0; k < ((ScriptList.ScriptRecord + i)->Script.LangSysRecord + j)->LangSys.FeatureCount; ++k) {
                    FX_DWORD index = *(((ScriptList.ScriptRecord + i)->Script.LangSysRecord + j)->LangSys.FeatureIndex + k);
                    if (FeatureList.FeatureRecord[index].FeatureTag == tag[0] || FeatureList.FeatureRecord[index].FeatureTag == tag[1]) {
                        FX_DWORD value;
                        if (!m_featureMap.Lookup(index, value)) {
                            m_featureMap.SetAt(index, index);
                        }
                    }
                }
            }
        }
        if (!m_featureMap.GetStartPosition()) {
            for (int i = 0; i < FeatureList.FeatureCount; i ++) {
                if (FeatureList.FeatureRecord[i].FeatureTag == tag[0] || FeatureList.FeatureRecord[i].FeatureTag == tag[1]) {
                    FX_DWORD value;
                    if (!m_featureMap.Lookup(i, value)) {
                        m_featureMap.SetAt(i, i);
                    }
                }
            }
        }
        m_bFeautureMapLoad = TRUE;
    }
    FX_POSITION pos = m_featureMap.GetStartPosition();
    while (pos) {
        FX_DWORD index, value;
        m_featureMap.GetNextAssoc(pos, index, value);
        if(GetVerticalGlyphSub(glyphnum, vglyphnum,	&FeatureList.FeatureRecord[value].Feature)) {
            return true;
        }
    }
    return false;
}
bool CFX_CTTGSUBTable::GetVerticalGlyphSub(
    TT_uint32_t glyphnum,
    TT_uint32_t *vglyphnum,
    struct TFeature *Feature)
{
    for(int i = 0; i < Feature->LookupCount; i++) {
        int index = Feature->LookupListIndex[i];
        if(index < 0 || LookupList.LookupCount < index) {
            continue;
        }
        if(LookupList.Lookup[index].LookupType == 1) {
            if(GetVerticalGlyphSub2(
                        glyphnum,
                        vglyphnum,
                        &LookupList.Lookup[index])) {
                return true;
            }
        }
    }
    return false;
}
bool CFX_CTTGSUBTable::GetVerticalGlyphSub2(
    TT_uint32_t glyphnum,
    TT_uint32_t *vglyphnum,
    struct TLookup *Lookup)
{
    for(int i = 0; i < Lookup->SubTableCount; i++) {
        switch(Lookup->SubTable[i]->SubstFormat) {
            case 1: {
                    TSingleSubstFormat1 *tbl1 = (TSingleSubstFormat1*)Lookup->SubTable[i];
                    if(GetCoverageIndex(tbl1->Coverage, glyphnum) >= 0) {
                        *vglyphnum = glyphnum + tbl1->DeltaGlyphID;
                        return true;
                    }
                    break;
                }
            case 2: {
                    TSingleSubstFormat2 *tbl2 = (TSingleSubstFormat2*)Lookup->SubTable[i];
                    int index = -1;
                    index = GetCoverageIndex(tbl2->Coverage, glyphnum);
                    if(0 <= index && index < tbl2->GlyphCount) {
                        *vglyphnum = tbl2->Substitute[index];
                        return true;
                    }
                    break;
                }
        }
    }
    return false;
}
int CFX_CTTGSUBTable::GetCoverageIndex(struct TCoverageFormatBase *Coverage, TT_uint32_t g)
{
    int i = 0;
    if(Coverage == NULL) {
        return -1;
    }
    switch(Coverage->CoverageFormat) {
        case 1: {
                TCoverageFormat1 *c1 = (TCoverageFormat1*)Coverage;
                for(i = 0; i < c1->GlyphCount; i++) {
                    if((TT_uint32_t)c1->GlyphArray[i] == g) {
                        return i;
                    }
                }
                return -1;
            }
        case 2: {
                TCoverageFormat2 *c2 = (TCoverageFormat2*)Coverage;
                for(i = 0; i < c2->RangeCount; i++) {
                    TT_uint32_t s = c2->RangeRecord[i].Start;
                    TT_uint32_t e = c2->RangeRecord[i].End;
                    TT_uint32_t si = c2->RangeRecord[i].StartCoverageIndex;
                    if (s <= g && g <= e) {
                        return si + g - s;
                    }
                }
                return -1;
            }
    }
    return -1;
}
bool CFX_CTTGSUBTable::Parse(
    FT_Bytes scriptlist,
    FT_Bytes featurelist,
    FT_Bytes lookuplist)
{
    ParseScriptList(scriptlist, &ScriptList);
    ParseFeatureList(featurelist, &FeatureList);
    ParseLookupList(lookuplist, &LookupList);
    return true;
}
void CFX_CTTGSUBTable::ParseScriptList(FT_Bytes raw, struct TScriptList *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->ScriptCount = GetUInt16(sp);
    if(rec->ScriptCount <= 0) {
        return;
    }
    rec->ScriptRecord = new struct TScriptRecord[rec->ScriptCount];
    for(i = 0; i < rec->ScriptCount; i++) {
        rec->ScriptRecord[i].ScriptTag = GetUInt32(sp);
        TT_uint16_t offset = GetUInt16(sp);
        ParseScript(
            &raw[offset],
            &rec->ScriptRecord[i].Script);
    }
}
void CFX_CTTGSUBTable::ParseScript(FT_Bytes raw, struct TScript *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->DefaultLangSys = GetUInt16(sp);
    rec->LangSysCount = GetUInt16(sp);
    if(rec->LangSysCount <= 0) {
        return;
    }
    rec->LangSysRecord = new struct TLangSysRecord[rec->LangSysCount];
    for(i = 0; i < rec->LangSysCount; i++) {
        rec->LangSysRecord[i].LangSysTag = GetUInt32(sp);
        TT_uint16_t offset = GetUInt16(sp);
        ParseLangSys(
            &raw[offset],
            &rec->LangSysRecord[i].LangSys);
    }
}
void CFX_CTTGSUBTable::ParseLangSys(FT_Bytes raw, struct TLangSys *rec)
{
    FT_Bytes sp = raw;
    rec->LookupOrder = GetUInt16(sp);
    rec->ReqFeatureIndex = GetUInt16(sp);
    rec->FeatureCount = GetUInt16(sp);
    if(rec->FeatureCount <= 0) {
        return;
    }
    rec->FeatureIndex = new TT_uint16_t[rec->FeatureCount];
    FXSYS_memset32(rec->FeatureIndex, 0, sizeof(TT_uint16_t) * rec->FeatureCount);
    for (int i = 0; i < rec->FeatureCount; ++i) {
        rec->FeatureIndex[i] = GetUInt16(sp);
    }
}
void CFX_CTTGSUBTable::ParseFeatureList(FT_Bytes raw, TFeatureList *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->FeatureCount = GetUInt16(sp);
    if(rec->FeatureCount <= 0) {
        return;
    }
    rec->FeatureRecord = new struct TFeatureRecord[rec->FeatureCount];
    for(i = 0; i < rec->FeatureCount; i++) {
        rec->FeatureRecord[i].FeatureTag = GetUInt32(sp);
        TT_uint16_t offset = GetUInt16(sp);
        ParseFeature(
            &raw[offset],
            &rec->FeatureRecord[i].Feature);
    }
}
void CFX_CTTGSUBTable::ParseFeature(FT_Bytes raw, TFeature *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->FeatureParams = GetUInt16(sp);
    rec->LookupCount = GetUInt16(sp);
    if(rec->LookupCount <= 0) {
        return;
    }
    rec->LookupListIndex = new TT_uint16_t[rec->LookupCount];
    for(i = 0; i < rec->LookupCount; i++) {
        rec->LookupListIndex[i] = GetUInt16(sp);
    }
}
void CFX_CTTGSUBTable::ParseLookupList(FT_Bytes raw, TLookupList *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->LookupCount = GetUInt16(sp);
    if(rec->LookupCount <= 0) {
        return;
    }
    rec->Lookup = new struct TLookup[rec->LookupCount];
    for(i = 0; i < rec->LookupCount; i++) {
        TT_uint16_t offset = GetUInt16(sp);
        ParseLookup(
            &raw[offset],
            &rec->Lookup[i]);
    }
}
void CFX_CTTGSUBTable::ParseLookup(FT_Bytes raw, TLookup *rec)
{
    int i;
    FT_Bytes sp = raw;
    rec->LookupType = GetUInt16(sp);
    rec->LookupFlag = GetUInt16(sp);
    rec->SubTableCount = GetUInt16(sp);
    if(rec->SubTableCount <= 0) {
        return;
    }
    rec->SubTable = new struct TSubTableBase*[rec->SubTableCount];
    for(i = 0; i < rec->SubTableCount; i++) {
        rec->SubTable[i] = NULL;
    }
    if(rec->LookupType != 1) {
        return;
    }
    for(i = 0; i < rec->SubTableCount; i++) {
        TT_uint16_t offset = GetUInt16(sp);
        ParseSingleSubst(
            &raw[offset],
            &rec->SubTable[i]);
    }
}
void CFX_CTTGSUBTable::ParseCoverage(FT_Bytes raw, TCoverageFormatBase **rec)
{
    FT_Bytes sp = raw;
    TT_uint16_t Format = GetUInt16(sp);
    switch(Format) {
        case 1:
            *rec = new TCoverageFormat1();
            ParseCoverageFormat1(raw, (TCoverageFormat1*)*rec);
            break;
        case 2:
            *rec = new TCoverageFormat2();
            ParseCoverageFormat2(raw, (TCoverageFormat2*)*rec);
            break;
    }
}
void CFX_CTTGSUBTable::ParseCoverageFormat1(FT_Bytes raw, TCoverageFormat1 *rec)
{
    int i;
    FT_Bytes sp = raw;
    GetUInt16(sp);
    rec->GlyphCount = GetUInt16(sp);
    if(rec->GlyphCount <= 0) {
        return;
    }
    rec->GlyphArray = new TT_uint16_t[rec->GlyphCount];
    for(i = 0; i < rec->GlyphCount; i++) {
        rec->GlyphArray[i] = GetUInt16(sp);
    }
}
void CFX_CTTGSUBTable::ParseCoverageFormat2(FT_Bytes raw, TCoverageFormat2 *rec)
{
    int i;
    FT_Bytes sp = raw;
    GetUInt16(sp);
    rec->RangeCount = GetUInt16(sp);
    if(rec->RangeCount <= 0) {
        return;
    }
    rec->RangeRecord = new TRangeRecord[rec->RangeCount];
    for(i = 0; i < rec->RangeCount; i++) {
        rec->RangeRecord[i].Start = GetUInt16(sp);
        rec->RangeRecord[i].End = GetUInt16(sp);
        rec->RangeRecord[i].StartCoverageIndex = GetUInt16(sp);
    }
}
void CFX_CTTGSUBTable::ParseSingleSubst(FT_Bytes raw, TSubTableBase **rec)
{
    FT_Bytes sp = raw;
    TT_uint16_t Format = GetUInt16(sp);
    switch(Format) {
        case 1:
            *rec = new TSingleSubstFormat1();
            ParseSingleSubstFormat1(raw, (TSingleSubstFormat1*)*rec);
            break;
        case 2:
            *rec = new TSingleSubstFormat2();
            ParseSingleSubstFormat2(raw, (TSingleSubstFormat2*)*rec);
            break;
    }
}
void CFX_CTTGSUBTable::ParseSingleSubstFormat1(FT_Bytes raw, TSingleSubstFormat1 *rec)
{
    FT_Bytes sp = raw;
    GetUInt16(sp);
    TT_uint16_t offset = GetUInt16(sp);
    ParseCoverage(
        &raw[offset],
        &rec->Coverage);
    rec->DeltaGlyphID = GetInt16(sp);
}
void CFX_CTTGSUBTable::ParseSingleSubstFormat2(FT_Bytes raw, TSingleSubstFormat2 *rec)
{
    int i;
    FT_Bytes sp = raw;
    GetUInt16(sp);
    TT_uint16_t offset = GetUInt16(sp);
    ParseCoverage(
        &raw[offset],
        &rec->Coverage);
    rec->GlyphCount = GetUInt16(sp);
    if(rec->GlyphCount <= 0) {
        return;
    }
    rec->Substitute = new TT_uint16_t[rec->GlyphCount];
    for(i = 0; i < rec->GlyphCount; i++) {
        rec->Substitute[i] = GetUInt16(sp);
    }
}
FX_BOOL CFX_GSUBTable::GetVerticalGlyph(FX_DWORD glyphnum, FX_DWORD* vglyphnum)
{
    return m_GsubImp.GetVerticalGlyph(glyphnum, vglyphnum);
}
IFX_GSUBTable* FXGE_CreateGSUBTable(CFX_Font* pFont)
{
    if (!pFont) {
        return NULL;
    }
    if (NULL == pFont->m_pGsubData) {
        unsigned long length = 0;
        int error = FXFT_Load_Sfnt_Table(pFont->m_Face, FT_MAKE_TAG('G', 'S', 'U', 'B'), 0, NULL, &length);
        if (!error) {
            pFont->m_pGsubData = (unsigned char*)FX_Alloc(FX_BYTE, length);
        }
        if (!pFont->m_pGsubData) {
            return NULL;
        }
    }
    int error = FXFT_Load_Sfnt_Table(pFont->m_Face, FT_MAKE_TAG('G', 'S', 'U', 'B'), 0, pFont->m_pGsubData, NULL);
    if (!error && pFont->m_pGsubData) {
        CFX_GSUBTable* pGsubTable = new CFX_GSUBTable;
        if (pGsubTable->m_GsubImp.LoadGSUBTable((FT_Bytes)pFont->m_pGsubData)) {
            return pGsubTable;
        }
        pGsubTable->Release();
    }
    return NULL;
}

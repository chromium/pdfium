// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_page.h"
#include "font_int.h"
#include "../fpdf_cmaps/cmap_int.h"
#include "../../../include/fxge/fx_ge.h"
#include "../../../include/fxge/fx_freetype.h"
extern FX_DWORD FT_CharCodeFromUnicode(int encoding, FX_WCHAR unicode);
extern short TT2PDF(int m, FXFT_Face face);
extern FX_BOOL FT_UseTTCharmap(FXFT_Face face, int platform_id, int encoding_id);
extern FX_LPCSTR GetAdobeCharName(int iBaseEncoding, const CFX_ByteString* pCharNames, int charcode);
CPDF_CMapManager::CPDF_CMapManager()
{
    m_bPrompted = FALSE;
    FXSYS_memset32(m_CID2UnicodeMaps, 0, sizeof m_CID2UnicodeMaps);
}
CPDF_CMapManager::~CPDF_CMapManager()
{
    DropAll(FALSE);
}
CPDF_CMap* CPDF_CMapManager::GetPredefinedCMap(const CFX_ByteString& name, FX_BOOL bPromptCJK)
{
    CPDF_CMap* pCMap;
    if (m_CMaps.Lookup(name, (FX_LPVOID&)pCMap)) {
        return pCMap;
    }
    pCMap = LoadPredefinedCMap(name, bPromptCJK);
    if (name.IsEmpty()) {
        return pCMap;
    }
    m_CMaps.SetAt(name, pCMap);
    return pCMap;
}
CPDF_CMap* CPDF_CMapManager::LoadPredefinedCMap(const CFX_ByteString& name, FX_BOOL bPromptCJK)
{
    CPDF_CMap* pCMap = new CPDF_CMap;
    FX_LPCSTR pname = name;
    if (*pname == '/') {
        pname ++;
    }
    pCMap->LoadPredefined(this, pname, bPromptCJK);
    return pCMap;
}
static const FX_LPCSTR g_CharsetNames[NUMBER_OF_CIDSETS] = {NULL, "GB1", "CNS1", "Japan1", "Korea1", "UCS" };
static const int g_CharsetCPs[NUMBER_OF_CIDSETS] = {0, 936, 950, 932, 949, 1200 };
int _CharsetFromOrdering(const CFX_ByteString& Ordering)
{
	for (int charset = 1; charset < NUMBER_OF_CIDSETS; charset++) {
		if (Ordering == CFX_ByteStringC(g_CharsetNames[charset]))
			return charset;
	}
	return CIDSET_UNKNOWN;
}
void CPDF_CMapManager::ReloadAll()
{
    DropAll(TRUE);
}
void CPDF_CMapManager::DropAll(FX_BOOL bReload)
{
    FX_POSITION pos = m_CMaps.GetStartPosition();
    while (pos) {
        CFX_ByteString name;
        CPDF_CMap* pCMap;
        m_CMaps.GetNextAssoc(pos, name, (FX_LPVOID&)pCMap);
        if (pCMap == NULL) {
            continue;
        }
        if (bReload) {
            pCMap->LoadPredefined(this, name, FALSE);
        } else {
            delete pCMap;
        }
    }
    for (int i = 0; i < sizeof m_CID2UnicodeMaps / sizeof(CPDF_CID2UnicodeMap*); i ++) {
        CPDF_CID2UnicodeMap* pMap = m_CID2UnicodeMaps[i];
        if (pMap == NULL) {
            continue;
        }
        if (bReload) {
            pMap->Load(this, i, FALSE);
        } else {
            delete pMap;
        }
    }
}
CPDF_CID2UnicodeMap* CPDF_CMapManager::GetCID2UnicodeMap(int charset, FX_BOOL bPromptCJK)
{
    if (m_CID2UnicodeMaps[charset] == NULL) {
        m_CID2UnicodeMaps[charset] = LoadCID2UnicodeMap(charset, bPromptCJK);
    }
    return m_CID2UnicodeMaps[charset];
}
CPDF_CID2UnicodeMap* CPDF_CMapManager::LoadCID2UnicodeMap(int charset, FX_BOOL bPromptCJK)
{
    CPDF_CID2UnicodeMap* pMap = new CPDF_CID2UnicodeMap();
    if (!pMap->Initialize()) {
        delete pMap;
        return NULL;
    }
    pMap->Load(this, charset, bPromptCJK);
    return pMap;
}
CPDF_CMapParser::CPDF_CMapParser()
{
    m_pCMap = NULL;
    m_Status = 0;
    m_CodeSeq = 0;
}
FX_BOOL	CPDF_CMapParser::Initialize(CPDF_CMap* pCMap)
{
    m_pCMap = pCMap;
    m_Status = 0;
    m_CodeSeq = 0;
    m_AddMaps.EstimateSize(0, 10240);
    return TRUE;
}
static FX_DWORD CMap_GetCode(FX_BSTR word)
{
    int num = 0;
    if (word.GetAt(0) == '<') {
        for (int i = 1; i < word.GetLength(); i ++) {
            FX_BYTE digit = word.GetAt(i);
            if (digit >= '0' && digit <= '9') {
                digit = digit - '0';
            } else if (digit >= 'a' && digit <= 'f') {
                digit = digit - 'a' + 10;
            } else if (digit >= 'A' && digit <= 'F') {
                digit = digit - 'A' + 10;
            } else {
                return num;
            }
            num = num * 16 + digit;
        }
    } else {
        for (int i = 0; i < word.GetLength(); i ++) {
            if (word.GetAt(i) < '0' || word.GetAt(i) > '9') {
                return num;
            }
            num = num * 10 + word.GetAt(i) - '0';
        }
    }
    return num;
}
static FX_BOOL _CMap_GetCodeRange(_CMap_CodeRange& range, FX_BSTR first, FX_BSTR second)
{
    if (first.GetLength() == 0 || first.GetAt(0) != '<') {
        return FALSE;
    }
    int i;
    for (i = 1; i < first.GetLength(); i ++)
        if (first.GetAt(i) == '>') {
            break;
        }
    range.m_CharSize = (i - 1) / 2;
    if (range.m_CharSize > 4) {
        return FALSE;
    }
    for (i = 0; i < range.m_CharSize; i ++) {
        FX_BYTE digit1 = first.GetAt(i * 2 + 1);
        FX_BYTE digit2 = first.GetAt(i * 2 + 2);
        FX_BYTE byte = (digit1 >= '0' && digit1 <= '9') ? (digit1 - '0') : ((digit1 & 0xdf) - 'A' + 10);
        byte = byte * 16 + ((digit2 >= '0' && digit2 <= '9') ? (digit2 - '0') : ((digit2 & 0xdf) - 'A' + 10));
        range.m_Lower[i] = byte;
    }
    FX_DWORD size = second.GetLength();
    for (i = 0; i < range.m_CharSize; i ++) {
        FX_BYTE digit1 = ((FX_DWORD)i * 2 + 1 < size) ? second.GetAt((FX_STRSIZE)i * 2 + 1) : 0;
        FX_BYTE digit2 = ((FX_DWORD)i * 2 + 2 < size) ? second.GetAt((FX_STRSIZE)i * 2 + 2) : 0;
        FX_BYTE byte = (digit1 >= '0' && digit1 <= '9') ? (digit1 - '0') : ((digit1 & 0xdf) - 'A' + 10);
        byte = byte * 16 + ((digit2 >= '0' && digit2 <= '9') ? (digit2 - '0') : ((digit2 & 0xdf) - 'A' + 10));
        range.m_Upper[i] = byte;
    }
    return TRUE;
}
static CFX_ByteString CMap_GetString(FX_BSTR word)
{
    return word.Mid(1, word.GetLength() - 2);
}
void CPDF_CMapParser::ParseWord(FX_BSTR word)
{
    if (word.IsEmpty()) {
        return;
    }
    if (word == FX_BSTRC("begincidchar")) {
        m_Status = 1;
        m_CodeSeq = 0;
    } else if (word == FX_BSTRC("begincidrange")) {
        m_Status = 2;
        m_CodeSeq = 0;
    } else if (word == FX_BSTRC("endcidrange") || word == FX_BSTRC("endcidchar")) {
        m_Status = 0;
    } else if (word == FX_BSTRC("/WMode")) {
        m_Status = 6;
    } else if (word == FX_BSTRC("/Registry")) {
        m_Status = 3;
    } else if (word == FX_BSTRC("/Ordering")) {
        m_Status = 4;
    } else if (word == FX_BSTRC("/Supplement")) {
        m_Status = 5;
    } else if (word == FX_BSTRC("begincodespacerange")) {
        m_Status = 7;
        m_CodeSeq = 0;
    } else if (word == FX_BSTRC("usecmap")) {
    } else if (m_Status == 1 || m_Status == 2) {
        m_CodePoints[m_CodeSeq] = CMap_GetCode(word);
        m_CodeSeq ++;
        FX_DWORD StartCode, EndCode;
        FX_WORD StartCID;
        if (m_Status == 1) {
            if (m_CodeSeq < 2) {
                return;
            }
            EndCode = StartCode = m_CodePoints[0];
            StartCID = (FX_WORD)m_CodePoints[1];
        } else {
            if (m_CodeSeq < 3) {
                return;
            }
            StartCode = m_CodePoints[0];
            EndCode = m_CodePoints[1];
            StartCID = (FX_WORD)m_CodePoints[2];
        }
        if (EndCode < 0x10000) {
            for (FX_DWORD code = StartCode; code <= EndCode; code ++) {
                m_pCMap->m_pMapping[code] = (FX_WORD)(StartCID + code - StartCode);
            }
        } else {
            FX_DWORD buf[2];
            buf[0] = StartCode;
            buf[1] = ((EndCode - StartCode) << 16) + StartCID;
            m_AddMaps.AppendBlock(buf, sizeof buf);
        }
        m_CodeSeq = 0;
    } else if (m_Status == 3) {
        CMap_GetString(word);
        m_Status = 0;
    } else if (m_Status == 4) {
        m_pCMap->m_Charset = _CharsetFromOrdering(CMap_GetString(word));
        m_Status = 0;
    } else if (m_Status == 5) {
        CMap_GetCode(word);
        m_Status = 0;
    } else if (m_Status == 6) {
        m_pCMap->m_bVertical = CMap_GetCode(word);
        m_Status = 0;
    } else if (m_Status == 7) {
        if (word == FX_BSTRC("endcodespacerange")) {
            int nSegs = m_CodeRanges.GetSize();
            if (nSegs > 1) {
                m_pCMap->m_CodingScheme = CPDF_CMap::MixedFourBytes;
                m_pCMap->m_nCodeRanges = nSegs;
                m_pCMap->m_pLeadingBytes = FX_Alloc2D(FX_BYTE, nSegs, sizeof(_CMap_CodeRange));
                FXSYS_memcpy32(m_pCMap->m_pLeadingBytes, m_CodeRanges.GetData(), nSegs * sizeof(_CMap_CodeRange));
            } else if (nSegs == 1) {
                m_pCMap->m_CodingScheme = (m_CodeRanges[0].m_CharSize == 2) ? CPDF_CMap::TwoBytes : CPDF_CMap::OneByte;
            }
            m_Status = 0;
        } else {
            if (word.GetLength() == 0 || word.GetAt(0) != '<') {
                return;
            }
            if (m_CodeSeq % 2) {
                _CMap_CodeRange range;
                if (_CMap_GetCodeRange(range, m_LastWord, word)) {
                    m_CodeRanges.Add(range);
                }
            }
            m_CodeSeq ++;
        }
    }
    m_LastWord = word;
}
CPDF_CMap::CPDF_CMap()
{
    m_Charset = CIDSET_UNKNOWN;
    m_Coding = CIDCODING_UNKNOWN;
    m_CodingScheme = TwoBytes;
    m_bVertical = 0;
    m_bLoaded = FALSE;
    m_pMapping = NULL;
    m_pLeadingBytes = NULL;
    m_pAddMapping = NULL;
    m_pEmbedMap = NULL;
    m_pUseMap = NULL;
    m_nCodeRanges = 0;
}
CPDF_CMap::~CPDF_CMap()
{
    if (m_pMapping) {
        FX_Free(m_pMapping);
    }
    if (m_pAddMapping) {
        FX_Free(m_pAddMapping);
    }
    if (m_pLeadingBytes) {
        FX_Free(m_pLeadingBytes);
    }
    if (m_pUseMap) {
        delete m_pUseMap;
    }
}
void CPDF_CMap::Release()
{
    if (m_PredefinedCMap.IsEmpty()) {
        delete this;
    }
}
const CPDF_PredefinedCMap g_PredefinedCMaps[] = {
    { "GB-EUC", CIDSET_GB1, CIDCODING_GB, CPDF_CMap::MixedTwoBytes, 1, {0xa1, 0xfe} },
    { "GBpc-EUC", CIDSET_GB1, CIDCODING_GB, CPDF_CMap::MixedTwoBytes, 1, {0xa1, 0xfc} },
    { "GBK-EUC", CIDSET_GB1, CIDCODING_GB, CPDF_CMap::MixedTwoBytes, 1, {0x81, 0xfe} },
    { "GBKp-EUC", CIDSET_GB1, CIDCODING_GB, CPDF_CMap::MixedTwoBytes, 1, {0x81, 0xfe} },
    { "GBK2K-EUC", CIDSET_GB1, CIDCODING_GB, CPDF_CMap::MixedTwoBytes, 1, {0x81, 0xfe} },
    { "GBK2K", CIDSET_GB1, CIDCODING_GB, CPDF_CMap::MixedTwoBytes, 1, {0x81, 0xfe} },
    { "UniGB-UCS2", CIDSET_GB1, CIDCODING_UCS2, CPDF_CMap::TwoBytes },
    { "UniGB-UTF16", CIDSET_GB1, CIDCODING_UTF16, CPDF_CMap::TwoBytes },
    { "B5pc", CIDSET_CNS1, CIDCODING_BIG5, CPDF_CMap::MixedTwoBytes, 1, {0xa1, 0xfc} },
    { "HKscs-B5", CIDSET_CNS1, CIDCODING_BIG5, CPDF_CMap::MixedTwoBytes, 1, {0x88, 0xfe} },
    { "ETen-B5", CIDSET_CNS1, CIDCODING_BIG5, CPDF_CMap::MixedTwoBytes, 1, {0xa1, 0xfe} },
    { "ETenms-B5", CIDSET_CNS1, CIDCODING_BIG5, CPDF_CMap::MixedTwoBytes, 1, {0xa1, 0xfe} },
    { "UniCNS-UCS2", CIDSET_CNS1, CIDCODING_UCS2, CPDF_CMap::TwoBytes },
    { "UniCNS-UTF16", CIDSET_CNS1, CIDCODING_UTF16, CPDF_CMap::TwoBytes },
    { "83pv-RKSJ", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::MixedTwoBytes, 2, {0x81, 0x9f, 0xe0, 0xfc} },
    { "90ms-RKSJ", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::MixedTwoBytes, 2, {0x81, 0x9f, 0xe0, 0xfc} },
    { "90msp-RKSJ", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::MixedTwoBytes, 2, {0x81, 0x9f, 0xe0, 0xfc} },
    { "90pv-RKSJ", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::MixedTwoBytes, 2, {0x81, 0x9f, 0xe0, 0xfc} },
    { "Add-RKSJ", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::MixedTwoBytes, 2, {0x81, 0x9f, 0xe0, 0xfc} },
    { "EUC", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::MixedTwoBytes, 2, {0x8e, 0x8e, 0xa1, 0xfe} },
    { "H", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::TwoBytes, 1, {0x21, 0x7e} },
    { "V", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::TwoBytes, 1, {0x21, 0x7e} },
    { "Ext-RKSJ", CIDSET_JAPAN1, CIDCODING_JIS, CPDF_CMap::MixedTwoBytes, 2, {0x81, 0x9f, 0xe0, 0xfc} },
    { "UniJIS-UCS2", CIDSET_JAPAN1, CIDCODING_UCS2, CPDF_CMap::TwoBytes },
    { "UniJIS-UCS2-HW", CIDSET_JAPAN1, CIDCODING_UCS2, CPDF_CMap::TwoBytes },
    { "UniJIS-UTF16", CIDSET_JAPAN1, CIDCODING_UTF16, CPDF_CMap::TwoBytes },
    { "KSC-EUC", CIDSET_KOREA1, CIDCODING_KOREA, CPDF_CMap::MixedTwoBytes, 1, {0xa1, 0xfe} },
    { "KSCms-UHC", CIDSET_KOREA1, CIDCODING_KOREA, CPDF_CMap::MixedTwoBytes, 1, {0x81, 0xfe} },
    { "KSCms-UHC-HW", CIDSET_KOREA1, CIDCODING_KOREA, CPDF_CMap::MixedTwoBytes, 1, {0x81, 0xfe} },
    { "KSCpc-EUC", CIDSET_KOREA1, CIDCODING_KOREA, CPDF_CMap::MixedTwoBytes, 1, {0xa1, 0xfd} },
    { "UniKS-UCS2", CIDSET_KOREA1, CIDCODING_UCS2, CPDF_CMap::TwoBytes },
    { "UniKS-UTF16", CIDSET_KOREA1, CIDCODING_UTF16, CPDF_CMap::TwoBytes },
    { NULL, 0, 0 }
};
extern void FPDFAPI_FindEmbeddedCMap(const char* name, int charset, int coding, const FXCMAP_CMap*& pMap);
extern FX_WORD FPDFAPI_CIDFromCharCode(const FXCMAP_CMap* pMap, FX_DWORD charcode);
FX_BOOL CPDF_CMap::LoadPredefined(CPDF_CMapManager* pMgr, FX_LPCSTR pName, FX_BOOL bPromptCJK)
{
    m_PredefinedCMap = pName;
    if (m_PredefinedCMap == FX_BSTRC("Identity-H") || m_PredefinedCMap == FX_BSTRC("Identity-V")) {
        m_Coding = CIDCODING_CID;
        m_bVertical = pName[9] == 'V';
        m_bLoaded = TRUE;
        return TRUE;
    }
    CFX_ByteString cmapid = m_PredefinedCMap;
    m_bVertical = cmapid.Right(1) == FX_BSTRC("V");
    if (cmapid.GetLength() > 2) {
        cmapid = cmapid.Left(cmapid.GetLength() - 2);
    }
    int index = 0;
    while (1) {
        if (g_PredefinedCMaps[index].m_pName == NULL) {
            return FALSE;
        }
        if (cmapid == CFX_ByteStringC(g_PredefinedCMaps[index].m_pName)) {
            break;
        }
        index ++;
    }
    const CPDF_PredefinedCMap& map = g_PredefinedCMaps[index];
    m_Charset = map.m_Charset;
    m_Coding = map.m_Coding;
    m_CodingScheme = map.m_CodingScheme;
    if (m_CodingScheme == MixedTwoBytes) {
        m_pLeadingBytes = FX_Alloc(FX_BYTE, 256);
        for (FX_DWORD i = 0; i < map.m_LeadingSegCount; i ++) {
            for (int b = map.m_LeadingSegs[i * 2]; b <= map.m_LeadingSegs[i * 2 + 1]; b ++) {
                m_pLeadingBytes[b] = 1;
            }
        }
    }
    FPDFAPI_FindEmbeddedCMap(pName, m_Charset, m_Coding, m_pEmbedMap);
    if (m_pEmbedMap) {
        m_bLoaded = TRUE;
        return TRUE;
    }
    return FALSE;
}
extern "C" {
    static int compare_dword(const void* data1, const void* data2)
    {
        return (*(FX_DWORD*)data1) - (*(FX_DWORD*)data2);
    }
};
FX_BOOL CPDF_CMap::LoadEmbedded(FX_LPCBYTE pData, FX_DWORD size)
{
    m_pMapping = FX_Alloc(FX_WORD, 65536);
    CPDF_CMapParser parser;
    parser.Initialize(this);
    CPDF_SimpleParser syntax(pData, size);
    while (1) {
        CFX_ByteStringC word = syntax.GetWord();
        if (word.IsEmpty()) {
            break;
        }
        parser.ParseWord(word);
    }
    if (m_CodingScheme == MixedFourBytes && parser.m_AddMaps.GetSize()) {
        m_pAddMapping = FX_Alloc(FX_BYTE, parser.m_AddMaps.GetSize() + 4);
        *(FX_DWORD*)m_pAddMapping = parser.m_AddMaps.GetSize() / 8;
        FXSYS_memcpy32(m_pAddMapping + 4, parser.m_AddMaps.GetBuffer(), parser.m_AddMaps.GetSize());
        FXSYS_qsort(m_pAddMapping + 4, parser.m_AddMaps.GetSize() / 8, 8, compare_dword);
    }
    return TRUE;
}
extern "C" {
    static int compareCID(const void* key, const void* element)
    {
        if ((*(FX_DWORD*)key) < (*(FX_DWORD*)element)) {
            return -1;
        }
        if ((*(FX_DWORD*)key) > (*(FX_DWORD*)element) + ((FX_DWORD*)element)[1] / 65536) {
            return 1;
        }
        return 0;
    }
};
FX_WORD CPDF_CMap::CIDFromCharCode(FX_DWORD charcode) const
{
    if (m_Coding == CIDCODING_CID) {
        return (FX_WORD)charcode;
    }
    if (m_pEmbedMap) {
        return FPDFAPI_CIDFromCharCode(m_pEmbedMap, charcode);
    }
    if (m_pMapping == NULL) {
        return (FX_WORD)charcode;
    }
    if (charcode >> 16) {
        if (m_pAddMapping) {
            void* found = FXSYS_bsearch(&charcode, m_pAddMapping + 4, *(FX_DWORD*)m_pAddMapping, 8, compareCID);
            if (found == NULL) {
                if (m_pUseMap) {
                    return m_pUseMap->CIDFromCharCode(charcode);
                }
                return 0;
            }
            return (FX_WORD)(((FX_DWORD*)found)[1] % 65536 + charcode - * (FX_DWORD*)found);
        }
        if (m_pUseMap) {
            return m_pUseMap->CIDFromCharCode(charcode);
        }
        return 0;
    }
    FX_DWORD CID = m_pMapping[charcode];
    if (!CID && m_pUseMap) {
        return m_pUseMap->CIDFromCharCode(charcode);
    }
    return (FX_WORD)CID;
}
static int _CheckCodeRange(FX_LPBYTE codes, int size, _CMap_CodeRange* pRanges, int nRanges)
{
    int iSeg = nRanges - 1;
    while (iSeg >= 0) {
        if (pRanges[iSeg].m_CharSize < size) {
            iSeg --;
            continue;
        }
        int iChar = 0;
        while (iChar < size) {
            if (codes[iChar] < pRanges[iSeg].m_Lower[iChar] ||
                    codes[iChar] > pRanges[iSeg].m_Upper[iChar]) {
                break;
            }
            iChar ++;
        }
        if (iChar == pRanges[iSeg].m_CharSize) {
            return 2;
        }
        if (iChar) {
            if (size == pRanges[iSeg].m_CharSize) {
                return 2;
            }
            return 1;
        }
        iSeg --;
    }
    return 0;
}
FX_DWORD CPDF_CMap::GetNextChar(FX_LPCSTR pString, int nStrLen, int& offset) const
{
    switch (m_CodingScheme) {
        case OneByte:
            return ((FX_LPBYTE)pString)[offset++];
        case TwoBytes:
            offset += 2;
            return ((FX_LPBYTE)pString)[offset - 2] * 256 + ((FX_LPBYTE)pString)[offset - 1];
        case MixedTwoBytes: {
                FX_BYTE byte1 = ((FX_LPBYTE)pString)[offset++];
                if (!m_pLeadingBytes[byte1]) {
                    return byte1;
                }
                FX_BYTE byte2 = ((FX_LPBYTE)pString)[offset++];
                return byte1 * 256 + byte2;
            }
        case MixedFourBytes: {
                FX_BYTE codes[4];
                int char_size = 1;
                codes[0] = ((FX_LPBYTE)pString)[offset++];
                _CMap_CodeRange* pRanges = (_CMap_CodeRange*)m_pLeadingBytes;
                while (1) {
                    int ret = _CheckCodeRange(codes, char_size, pRanges, m_nCodeRanges);
                    if (ret == 0) {
                        return 0;
                    }
                    if (ret == 2) {
                        FX_DWORD charcode = 0;
                        for (int i = 0; i < char_size; i ++) {
                            charcode = (charcode << 8) + codes[i];
                        }
                        return charcode;
                    }
                    if (char_size == 4 || offset == nStrLen) {
                        return 0;
                    }
                    codes[char_size ++] = ((FX_LPBYTE)pString)[offset++];
                }
                break;
            }
    }
    return 0;
}
int CPDF_CMap::GetCharSize(FX_DWORD charcode) const
{
    switch (m_CodingScheme) {
        case OneByte:
            return 1;
        case TwoBytes:
            return 2;
        case MixedTwoBytes:
        case MixedFourBytes:
            if (charcode < 0x100) {
                return 1;
            }
            if (charcode < 0x10000) {
                return 2;
            }
            if (charcode < 0x1000000) {
                return 3;
            }
            return 4;
    }
    return 1;
}
int CPDF_CMap::CountChar(FX_LPCSTR pString, int size) const
{
    switch (m_CodingScheme) {
        case OneByte:
            return size;
        case TwoBytes:
            return (size + 1) / 2;
        case MixedTwoBytes: {
                int count = 0;
                for (int i = 0; i < size; i ++) {
                    count ++;
                    if (m_pLeadingBytes[((FX_LPBYTE)pString)[i]]) {
                        i ++;
                    }
                }
                return count;
            }
        case MixedFourBytes: {
                int count = 0, offset = 0;
                while (offset < size) {
                    GetNextChar(pString, size, offset);
                    count ++;
                }
                return count;
            }
    }
    return size;
}
int _GetCharSize(FX_DWORD charcode, _CMap_CodeRange* pRanges, int iRangesSize)
{
    if (!iRangesSize) {
        return 1;
    }
    FX_BYTE codes[4];
    codes[0] = codes[1] = 0x00;
    codes[2] = (FX_BYTE)(charcode >> 8 & 0xFF);
    codes[3] = (FX_BYTE)charcode;
    int offset = 0, size = 4;
    for (int i = 0; i < 4; ++i) {
        int iSeg = iRangesSize - 1;
        while (iSeg >= 0) {
            if (pRanges[iSeg].m_CharSize < size) {
                iSeg --;
                continue;
            }
            int iChar = 0;
            while (iChar < size) {
                if (codes[offset + iChar] < pRanges[iSeg].m_Lower[iChar] ||
                        codes[offset + iChar] > pRanges[iSeg].m_Upper[iChar]) {
                    break;
                }
                iChar ++;
            }
            if (iChar == pRanges[iSeg].m_CharSize) {
                return size;
            }
            iSeg --;
        }
        size --;
        offset ++;
    }
    return 1;
}
int CPDF_CMap::AppendChar(FX_LPSTR str, FX_DWORD charcode) const
{
    switch (m_CodingScheme) {
        case OneByte:
            str[0] = (FX_BYTE)charcode;
            return 1;
        case TwoBytes:
            str[0] = (FX_BYTE)(charcode / 256);
            str[1] = (FX_BYTE)(charcode % 256);
            return 2;
        case MixedTwoBytes:
        case MixedFourBytes:
            if (charcode < 0x100) {
                _CMap_CodeRange* pRanges = (_CMap_CodeRange*)m_pLeadingBytes;
                int iSize = _GetCharSize(charcode, pRanges, m_nCodeRanges);
                if (iSize == 0) {
                    iSize = 1;
                }
                if (iSize > 1) {
                    FXSYS_memset32(str, 0, sizeof(FX_BYTE) * iSize);
                }
                str[iSize - 1] = (FX_BYTE)charcode;
                return iSize;
            } else if (charcode < 0x10000) {
                str[0] = (FX_BYTE)(charcode >> 8);
                str[1] = (FX_BYTE)charcode;
                return 2;
            } else if (charcode < 0x1000000) {
                str[0] = (FX_BYTE)(charcode >> 16);
                str[1] = (FX_BYTE)(charcode >> 8);
                str[2] = (FX_BYTE)charcode;
                return 3;
            } else {
                str[0] = (FX_BYTE)(charcode >> 24);
                str[1] = (FX_BYTE)(charcode >> 16);
                str[2] = (FX_BYTE)(charcode >> 8);
                str[3] = (FX_BYTE)charcode;
                return 4;
            }
    }
    return 0;
}
CPDF_CID2UnicodeMap::CPDF_CID2UnicodeMap()
{
    m_EmbeddedCount = 0;
}
CPDF_CID2UnicodeMap::~CPDF_CID2UnicodeMap()
{
}
FX_BOOL CPDF_CID2UnicodeMap::Initialize()
{
    return TRUE;
}
FX_BOOL CPDF_CID2UnicodeMap::IsLoaded()
{
    return m_EmbeddedCount != 0;
}
FX_WCHAR CPDF_CID2UnicodeMap::UnicodeFromCID(FX_WORD CID)
{
    if (m_Charset == CIDSET_UNICODE) {
        return CID;
    }
    if (CID < m_EmbeddedCount) {
        return m_pEmbeddedMap[CID];
    }
    return 0;
}
void FPDFAPI_LoadCID2UnicodeMap(int charset, const FX_WORD*& pMap, FX_DWORD& count);
void CPDF_CID2UnicodeMap::Load(CPDF_CMapManager* pMgr, int charset, FX_BOOL bPromptCJK)
{
    m_Charset = charset;
    FPDFAPI_LoadCID2UnicodeMap(charset, m_pEmbeddedMap, m_EmbeddedCount);
}
#include "ttgsubtable.h"
CPDF_CIDFont::CPDF_CIDFont() : CPDF_Font(PDFFONT_CIDFONT)
{
    m_pCMap = NULL;
    m_pAllocatedCMap = NULL;
    m_pCID2UnicodeMap = NULL;
    m_pAnsiWidths = NULL;
    m_pCIDToGIDMap = NULL;
    m_bCIDIsGID = FALSE;
    m_bAdobeCourierStd = FALSE;
    m_pTTGSUBTable = NULL;
    FXSYS_memset8(m_CharBBox, 0xff, 256 * sizeof(FX_SMALL_RECT));
}
CPDF_CIDFont::~CPDF_CIDFont()
{
    if (m_pAnsiWidths) {
        FX_Free(m_pAnsiWidths);
    }
    if (m_pAllocatedCMap) {
        delete m_pAllocatedCMap;
    }
    if (m_pCIDToGIDMap) {
        delete m_pCIDToGIDMap;
    }
    if (m_pTTGSUBTable) {
        delete m_pTTGSUBTable;
    }
}
FX_WORD CPDF_CIDFont::CIDFromCharCode(FX_DWORD charcode) const
{
    if (m_pCMap == NULL) {
        return (FX_WORD)charcode;
    }
    return m_pCMap->CIDFromCharCode(charcode);
}
FX_BOOL CPDF_CIDFont::IsVertWriting() const
{
    return m_pCMap ? m_pCMap->IsVertWriting() : FALSE;
}
extern FX_DWORD FPDFAPI_CharCodeFromCID(const FXCMAP_CMap* pMap, FX_WORD cid);
static FX_DWORD _EmbeddedCharcodeFromUnicode(const FXCMAP_CMap* pEmbedMap, int charset, FX_WCHAR unicode)
{
    if (charset <= 0 || charset > 4) {
        return 0;
    }
    CPDF_FontGlobals* pFontGlobals = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
    const FX_WORD* pCodes = pFontGlobals->m_EmbeddedToUnicodes[charset].m_pMap;
    if (pCodes == NULL) {
        return 0;
    }
    int nCodes = pFontGlobals->m_EmbeddedToUnicodes[charset].m_Count;
    for (int i = 0; i < nCodes; i++) {
        if (pCodes[i] == unicode) {
            FX_DWORD CharCode = FPDFAPI_CharCodeFromCID(pEmbedMap, i);
            if (CharCode == 0) {
                continue;
            }
            return CharCode;
        }
    }
    return 0;
}
static FX_WCHAR _EmbeddedUnicodeFromCharcode(const FXCMAP_CMap* pEmbedMap, int charset, FX_DWORD charcode)
{
    if (charset <= 0 || charset > 4) {
        return 0;
    }
    FX_WORD cid = FPDFAPI_CIDFromCharCode(pEmbedMap, charcode);
    if (cid == 0) {
        return 0;
    }
    CPDF_FontGlobals* pFontGlobals = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals();
    const FX_WORD* pCodes = pFontGlobals->m_EmbeddedToUnicodes[charset].m_pMap;
    if (pCodes == NULL) {
        return 0;
    }
    if (cid < pFontGlobals->m_EmbeddedToUnicodes[charset].m_Count) {
        return pCodes[cid];
    }
    return 0;
}
FX_WCHAR CPDF_CIDFont::_UnicodeFromCharCode(FX_DWORD charcode) const
{
    switch (m_pCMap->m_Coding) {
        case CIDCODING_UCS2:
        case CIDCODING_UTF16:
            return (FX_WCHAR)charcode;
        case CIDCODING_CID:
            if (m_pCID2UnicodeMap == NULL || !m_pCID2UnicodeMap->IsLoaded()) {
                return 0;
            }
            return m_pCID2UnicodeMap->UnicodeFromCID((FX_WORD)charcode);
    }
    if (!m_pCMap->IsLoaded() || m_pCID2UnicodeMap == NULL || !m_pCID2UnicodeMap->IsLoaded()) {
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
        FX_WCHAR unicode;
        int charsize = 1;
        if (charcode > 255) {
            charcode = (charcode % 256) * 256 + (charcode / 256);
            charsize = 2;
        }
        int ret = FXSYS_MultiByteToWideChar(g_CharsetCPs[m_pCMap->m_Coding], 0, (FX_LPCSTR)&charcode, charsize, &unicode, 1);
        if (ret != 1) {
            return 0;
        }
        return unicode;
#endif
        if (m_pCMap->m_pEmbedMap) {
            return _EmbeddedUnicodeFromCharcode(m_pCMap->m_pEmbedMap, m_pCMap->m_Charset, charcode);
        } else {
            return 0;
        }
    }
    return m_pCID2UnicodeMap->UnicodeFromCID(CIDFromCharCode(charcode));
}
FX_DWORD CPDF_CIDFont::_CharCodeFromUnicode(FX_WCHAR unicode) const
{
    switch (m_pCMap->m_Coding) {
        case CIDCODING_UNKNOWN:
            return 0;
        case CIDCODING_UCS2:
        case CIDCODING_UTF16:
            return unicode;
        case CIDCODING_CID: {
                if (m_pCID2UnicodeMap == NULL || !m_pCID2UnicodeMap->IsLoaded()) {
                    return 0;
                }
                FX_DWORD CID = 0;
                while (CID < 65536) {
                    FX_WCHAR this_unicode = m_pCID2UnicodeMap->UnicodeFromCID((FX_WORD)CID);
                    if (this_unicode == unicode) {
                        return CID;
                    }
                    CID ++;
                }
                break;
            }
    }
	
    if (unicode < 0x80) {
        return static_cast<FX_DWORD>(unicode);
    } else if (m_pCMap->m_Coding == CIDCODING_CID) {
        return 0;
    }
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
    FX_BYTE buffer[32];
    int ret = FXSYS_WideCharToMultiByte(g_CharsetCPs[m_pCMap->m_Coding], 0, &unicode, 1, (char*)buffer, 4, NULL, NULL);
    if (ret == 1) {
        return buffer[0];
    } else if (ret == 2) {
        return buffer[0] * 256 + buffer[1];
    }
    return 0;
#endif
    if (m_pCMap->m_pEmbedMap) {
        return _EmbeddedCharcodeFromUnicode(m_pCMap->m_pEmbedMap, m_pCMap->m_Charset, unicode);
    }
    return 0;
}
static void FT_UseCIDCharmap(FXFT_Face face, int coding)
{
    int encoding;
    switch (coding) {
        case CIDCODING_GB:
            encoding = FXFT_ENCODING_GB2312;
            break;
        case CIDCODING_BIG5:
            encoding = FXFT_ENCODING_BIG5;
            break;
        case CIDCODING_JIS:
            encoding = FXFT_ENCODING_SJIS;
            break;
        case CIDCODING_KOREA:
            encoding = FXFT_ENCODING_JOHAB;
            break;
        default:
            encoding = FXFT_ENCODING_UNICODE;
    }
    int err = FXFT_Select_Charmap(face, encoding);
    if (err) {
        err = FXFT_Select_Charmap(face, FXFT_ENCODING_UNICODE);
    }
    if (err && FXFT_Get_Face_Charmaps(face)) {
        FXFT_Set_Charmap(face, *FXFT_Get_Face_Charmaps(face));
    }
}
FX_BOOL CPDF_CIDFont::_Load()
{
    if (m_pFontDict->GetString(FX_BSTRC("Subtype")) == FX_BSTRC("TrueType")) {
        return LoadGB2312();
    }
    CPDF_Array* pFonts = m_pFontDict->GetArray(FX_BSTRC("DescendantFonts"));
    if (pFonts == NULL) {
        return FALSE;
    }
    if (pFonts->GetCount() != 1) {
        return FALSE;
    }
    CPDF_Dictionary* pCIDFontDict = pFonts->GetDict(0);
    if (pCIDFontDict == NULL) {
        return FALSE;
    }
    m_BaseFont = pCIDFontDict->GetString(FX_BSTRC("BaseFont"));
    if ((m_BaseFont.Compare("CourierStd") == 0 || m_BaseFont.Compare("CourierStd-Bold") == 0
            || m_BaseFont.Compare("CourierStd-BoldOblique") == 0 || m_BaseFont.Compare("CourierStd-Oblique") == 0)
            && !IsEmbedded()) {
        m_bAdobeCourierStd = TRUE;
    }
    CPDF_Dictionary* pFontDesc = pCIDFontDict->GetDict(FX_BSTRC("FontDescriptor"));
    if (pFontDesc) {
        LoadFontDescriptor(pFontDesc);
    }
    CPDF_Object* pEncoding = m_pFontDict->GetElementValue(FX_BSTRC("Encoding"));
    if (pEncoding == NULL) {
        return FALSE;
    }
    CFX_ByteString subtype = pCIDFontDict->GetString(FX_BSTRC("Subtype"));
    m_bType1 = FALSE;
    if (subtype == FX_BSTRC("CIDFontType0")) {
        m_bType1 = TRUE;
    }
    if (pEncoding->GetType() == PDFOBJ_NAME) {
        CFX_ByteString cmap = pEncoding->GetString();
        m_pCMap = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals()->m_CMapManager.GetPredefinedCMap(cmap,
                  m_pFontFile && m_bType1);
    } else if (pEncoding->GetType() == PDFOBJ_STREAM) {
        m_pAllocatedCMap = m_pCMap = new CPDF_CMap;
        CPDF_Stream* pStream = (CPDF_Stream*)pEncoding;
        CPDF_StreamAcc acc;
        acc.LoadAllData(pStream, FALSE);
        m_pCMap->LoadEmbedded(acc.GetData(), acc.GetSize());
    } else {
        return FALSE;
    }
    if (m_pCMap == NULL) {
        return FALSE;
    }
    m_Charset = m_pCMap->m_Charset;
    if (m_Charset == CIDSET_UNKNOWN) {
        CPDF_Dictionary* pCIDInfo = pCIDFontDict->GetDict(FX_BSTRC("CIDSystemInfo"));
        if (pCIDInfo) {
            m_Charset = _CharsetFromOrdering(pCIDInfo->GetString(FX_BSTRC("Ordering")));
        }
    }
    if (m_Charset != CIDSET_UNKNOWN)
        m_pCID2UnicodeMap = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals()->m_CMapManager.GetCID2UnicodeMap(m_Charset,
                            m_pFontFile == NULL && (m_pCMap->m_Coding == CIDCODING_CID || pCIDFontDict->KeyExist(FX_BSTRC("W"))));
    if (m_Font.GetFace()) {
        if (m_bType1) {
            FXFT_Select_Charmap(m_Font.GetFace(), FXFT_ENCODING_UNICODE);
        } else {
            FT_UseCIDCharmap(m_Font.GetFace(), m_pCMap->m_Coding);
        }
    }
    m_DefaultWidth = pCIDFontDict->GetInteger(FX_BSTRC("DW"), 1000);
    CPDF_Array* pWidthArray = pCIDFontDict->GetArray(FX_BSTRC("W"));
    if (pWidthArray) {
        LoadMetricsArray(pWidthArray, m_WidthList, 1);
    }
    if (!IsEmbedded()) {
        LoadSubstFont();
    }
    if (1) {
        if (m_pFontFile || (GetSubstFont()->m_SubstFlags & FXFONT_SUBST_EXACT)) {
            CPDF_Object* pmap = pCIDFontDict->GetElementValue(FX_BSTRC("CIDToGIDMap"));
            if (pmap) {
                if (pmap->GetType() == PDFOBJ_STREAM) {
                    m_pCIDToGIDMap = new CPDF_StreamAcc;
                    m_pCIDToGIDMap->LoadAllData((CPDF_Stream*)pmap, FALSE);
                } else if (pmap->GetString() == FX_BSTRC("Identity")) {
#if _FXM_PLATFORM_  == _FXM_PLATFORM_APPLE_
                    if (m_pFontFile) {
                        m_bCIDIsGID = TRUE;
                    }
#else
                    m_bCIDIsGID = TRUE;
#endif
                }
            }
        }
    }
    CheckFontMetrics();
    if (IsVertWriting()) {
        pWidthArray = pCIDFontDict->GetArray(FX_BSTRC("W2"));
        if (pWidthArray) {
            LoadMetricsArray(pWidthArray, m_VertMetrics, 3);
        }
        CPDF_Array* pDefaultArray = pCIDFontDict->GetArray(FX_BSTRC("DW2"));
        if (pDefaultArray) {
            m_DefaultVY = pDefaultArray->GetInteger(0);
            m_DefaultW1 = pDefaultArray->GetInteger(1);
        } else {
            m_DefaultVY = 880;
            m_DefaultW1 = -1000;
        }
    }
    return TRUE;
}
FX_FLOAT _CIDTransformToFloat(FX_BYTE ch)
{
    if (ch < 128) {
        return ch * 1.0f / 127;
    }
    return (-255 + ch) * 1.0f / 127;
}
void CPDF_CIDFont::GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level)
{
    if (charcode < 256 && m_CharBBox[charcode].Right != -1) {
        rect.bottom = m_CharBBox[charcode].Bottom;
        rect.left = m_CharBBox[charcode].Left;
        rect.right = m_CharBBox[charcode].Right;
        rect.top = m_CharBBox[charcode].Top;
        return;
    }
    FX_BOOL bVert = FALSE;
    int glyph_index = GlyphFromCharCode(charcode, &bVert);
    if (m_Font.m_Face == NULL) {
        rect = FX_RECT(0, 0, 0, 0);
    } else {
        rect.left = rect.bottom = rect.right = rect.top = 0;
        FXFT_Face face = m_Font.m_Face;
        if (FXFT_Is_Face_Tricky(face)) {
            int err = FXFT_Load_Glyph(face, glyph_index, FXFT_LOAD_IGNORE_GLOBAL_ADVANCE_WIDTH);
            if (!err) {
                FXFT_BBox cbox;
                FXFT_Glyph glyph;
                err = FXFT_Get_Glyph(((FXFT_Face)face)->glyph, &glyph);
                if (!err) {
                    FXFT_Glyph_Get_CBox(glyph, FXFT_GLYPH_BBOX_PIXELS, &cbox);
                    int pixel_size_x = ((FXFT_Face)face)->size->metrics.x_ppem;
                    int	pixel_size_y = ((FXFT_Face)face)->size->metrics.y_ppem;
                    if (pixel_size_x == 0 || pixel_size_y == 0) {
                        rect.left = cbox.xMin;
                        rect.right = cbox.xMax;
                        rect.top = cbox.yMax;
                        rect.bottom = cbox.yMin;
                    } else {
                        rect.left = cbox.xMin * 1000 / pixel_size_x;
                        rect.right = cbox.xMax * 1000 / pixel_size_x;
                        rect.top = cbox.yMax * 1000 / pixel_size_y;
                        rect.bottom = cbox.yMin * 1000 / pixel_size_y;
                    }
                    if (rect.top > FXFT_Get_Face_Ascender(face)) {
                        rect.top = FXFT_Get_Face_Ascender(face);
                    }
                    if (rect.bottom < FXFT_Get_Face_Descender(face)) {
                        rect.bottom = FXFT_Get_Face_Descender(face);
                    }
                    FXFT_Done_Glyph(glyph);
                }
            }
        } else {
            int err = FXFT_Load_Glyph(face, glyph_index, FXFT_LOAD_NO_SCALE);
            if (err == 0) {
                rect.left = TT2PDF(FXFT_Get_Glyph_HoriBearingX(face), face);
                rect.right = TT2PDF(FXFT_Get_Glyph_HoriBearingX(face) + FXFT_Get_Glyph_Width(face), face);
                rect.top = TT2PDF(FXFT_Get_Glyph_HoriBearingY(face), face);
                rect.top += rect.top / 64;
                rect.bottom = TT2PDF(FXFT_Get_Glyph_HoriBearingY(face) - FXFT_Get_Glyph_Height(face), face);
            }
        }
    }
    if (m_pFontFile == NULL && m_Charset == CIDSET_JAPAN1) {
        FX_WORD CID = CIDFromCharCode(charcode);
        FX_LPCBYTE pTransform = GetCIDTransform(CID);
        if (pTransform && !bVert) {
            CFX_AffineMatrix matrix(_CIDTransformToFloat(pTransform[0]), _CIDTransformToFloat(pTransform[1]),
                                    _CIDTransformToFloat(pTransform[2]), _CIDTransformToFloat(pTransform[3]),
                                    _CIDTransformToFloat(pTransform[4]) * 1000 , _CIDTransformToFloat(pTransform[5]) * 1000);
            CFX_FloatRect rect_f(rect);
            rect_f.Transform(&matrix);
            rect = rect_f.GetOutterRect();
        }
    }
    if (charcode < 256) {
        m_CharBBox[charcode].Bottom = (short)rect.bottom;
        m_CharBBox[charcode].Left = (short)rect.left;
        m_CharBBox[charcode].Right = (short)rect.right;
        m_CharBBox[charcode].Top = (short)rect.top;
    }
}
int CPDF_CIDFont::GetCharWidthF(FX_DWORD charcode, int level)
{
    if (m_pAnsiWidths && charcode < 0x80) {
        return m_pAnsiWidths[charcode];
    }
    FX_WORD cid = CIDFromCharCode(charcode);
    int size = m_WidthList.GetSize();
    FX_DWORD* list = m_WidthList.GetData();
    for (int i = 0; i < size; i += 3) {
        if (cid >= list[i] && cid <= list[i + 1]) {
            return (int)list[i + 2];
        }
    }
    return m_DefaultWidth;
}
short CPDF_CIDFont::GetVertWidth(FX_WORD CID) const
{
    FX_DWORD vertsize = m_VertMetrics.GetSize() / 5;
    if (vertsize == 0) {
        return m_DefaultW1;
    }
    const FX_DWORD* pTable = m_VertMetrics.GetData();
    for (FX_DWORD i = 0; i < vertsize; i ++)
        if (pTable[i * 5] <= CID && pTable[i * 5 + 1] >= CID) {
            return (short)(int)pTable[i * 5 + 2];
        }
    return m_DefaultW1;
}
void CPDF_CIDFont::GetVertOrigin(FX_WORD CID, short& vx, short &vy) const
{
    FX_DWORD vertsize = m_VertMetrics.GetSize() / 5;
    if (vertsize) {
        const FX_DWORD* pTable = m_VertMetrics.GetData();
        for (FX_DWORD i = 0; i < vertsize; i ++)
            if (pTable[i * 5] <= CID && pTable[i * 5 + 1] >= CID) {
                vx = (short)(int)pTable[i * 5 + 3];
                vy = (short)(int)pTable[i * 5 + 4];
                return;
            }
    }
    FX_DWORD dwWidth = m_DefaultWidth;
    int size = m_WidthList.GetSize();
    const FX_DWORD* list = m_WidthList.GetData();
    for (int i = 0; i < size; i += 3) {
        if (CID >= list[i] && CID <= list[i + 1]) {
            dwWidth = (FX_WORD)list[i + 2];
            break;
        }
    }
    vx = (short)dwWidth / 2;
    vy = (short)m_DefaultVY;
}
int	CPDF_CIDFont::GetGlyphIndex(FX_DWORD unicode, FX_BOOL *pVertGlyph)
{
    if (pVertGlyph) {
        *pVertGlyph = FALSE;
    }
    int index = FXFT_Get_Char_Index(m_Font.m_Face, unicode );
    if (unicode == 0x2502) {
        return index;
    }
    if (index && IsVertWriting()) {
        if (m_pTTGSUBTable) {
            TT_uint32_t vindex = 0;
            m_pTTGSUBTable->GetVerticalGlyph(index, &vindex);
            if (vindex) {
                index = vindex;
                if (pVertGlyph) {
                    *pVertGlyph = TRUE;
                }
            }
            return index;
        }
        if (NULL == m_Font.m_pGsubData) {
            unsigned long length = 0;
            int error = FXFT_Load_Sfnt_Table( m_Font.m_Face, FT_MAKE_TAG('G', 'S', 'U', 'B'), 0, NULL, &length);
            if (!error) {
                m_Font.m_pGsubData = (unsigned char*)FX_Alloc(FX_BYTE, length);
            }
        }
        int error = FXFT_Load_Sfnt_Table( m_Font.m_Face, FT_MAKE_TAG('G', 'S', 'U', 'B'), 0, m_Font.m_pGsubData, NULL);
        if (!error && m_Font.m_pGsubData) {
            m_pTTGSUBTable = new CFX_CTTGSUBTable;
            m_pTTGSUBTable->LoadGSUBTable((FT_Bytes)m_Font.m_pGsubData);
            TT_uint32_t vindex = 0;
            m_pTTGSUBTable->GetVerticalGlyph(index, &vindex);
            if (vindex) {
                index = vindex;
                if (pVertGlyph) {
                    *pVertGlyph = TRUE;
                }
            }
        }
        return index;
    }
    if (pVertGlyph) {
        *pVertGlyph = FALSE;
    }
    return index;
}
int CPDF_CIDFont::GlyphFromCharCode(FX_DWORD charcode, FX_BOOL *pVertGlyph)
{
    if (pVertGlyph) {
        *pVertGlyph = FALSE;
    }
    if (m_pFontFile == NULL && m_pCIDToGIDMap == NULL) {
        FX_WORD cid = CIDFromCharCode(charcode);
        FX_WCHAR unicode = 0;
        if (m_bCIDIsGID) {
#if _FXM_PLATFORM_ != _FXM_PLATFORM_APPLE_
            return cid;
#else
            if (m_Flags & PDFFONT_SYMBOLIC) {
                return cid;
            }
            CFX_WideString uni_str = UnicodeFromCharCode(charcode);
            if (uni_str.IsEmpty()) {
                return cid;
            }
            unicode = uni_str.GetAt(0);
#endif
        } else {
            if (cid && m_pCID2UnicodeMap && m_pCID2UnicodeMap->IsLoaded()) {
                unicode = m_pCID2UnicodeMap->UnicodeFromCID(cid);
            }
            if (unicode == 0) {
                unicode = _UnicodeFromCharCode(charcode);
            }
            if (unicode == 0 && !(m_Flags & PDFFONT_SYMBOLIC)) {
                unicode = UnicodeFromCharCode(charcode).GetAt(0);
            }
        }
        if (unicode == 0) {
            if (!m_bAdobeCourierStd) {
                return charcode == 0 ? -1 : (int)charcode;
            }
            charcode += 31;
            int index = 0, iBaseEncoding;
            FX_BOOL bMSUnicode = FT_UseTTCharmap(m_Font.m_Face, 3, 1);
            FX_BOOL bMacRoman = FALSE;
            if (!bMSUnicode) {
                bMacRoman = FT_UseTTCharmap(m_Font.m_Face, 1, 0);
            }
            iBaseEncoding = PDFFONT_ENCODING_STANDARD;
            if (bMSUnicode) {
                iBaseEncoding = PDFFONT_ENCODING_WINANSI;
            } else if (bMacRoman) {
                iBaseEncoding = PDFFONT_ENCODING_MACROMAN;
            }
            FX_LPCSTR name = GetAdobeCharName(iBaseEncoding, NULL, charcode);
            if (name == NULL) {
                return charcode == 0 ? -1 : (int)charcode;
            }
            FX_WORD unicode = PDF_UnicodeFromAdobeName(name);
            if (unicode) {
                if (bMSUnicode) {
                    index = FXFT_Get_Char_Index(m_Font.m_Face, unicode);
                } else if (bMacRoman) {
                    FX_DWORD maccode = FT_CharCodeFromUnicode(FXFT_ENCODING_APPLE_ROMAN, unicode);
                    index = !maccode ? FXFT_Get_Name_Index(m_Font.m_Face, (char *)name) : FXFT_Get_Char_Index(m_Font.m_Face, maccode);
                } else {
                    return FXFT_Get_Char_Index(m_Font.m_Face, unicode);
                }
            } else {
                return charcode == 0 ? -1 : (int)charcode;
            }
            if (index == 0 || index == 0xffff) {
                return charcode == 0 ? -1 : (int)charcode;
            } else {
                return index;
            }
        }
        if (m_Charset == CIDSET_JAPAN1) {
            if (unicode == '\\') {
                unicode = '/';
            }
#if _FXM_PLATFORM_ != _FXM_PLATFORM_APPLE_
            else if (unicode == 0xa5) {
                unicode = 0x5c;
            }
#endif
        }
        if (m_Font.m_Face == NULL) {
            return unicode;
        }
        int err = FXFT_Select_Charmap(m_Font.m_Face, FXFT_ENCODING_UNICODE);
        if (err != 0) {
            int i;
            for (i = 0; i < FXFT_Get_Face_CharmapCount(m_Font.m_Face); i ++) {
                FX_DWORD ret = FT_CharCodeFromUnicode(FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmaps(m_Font.m_Face)[i]), (FX_WCHAR)charcode);
                if (ret == 0) {
                    continue;
                }
                FXFT_Set_Charmap(m_Font.m_Face, FXFT_Get_Face_Charmaps(m_Font.m_Face)[i]);
                unicode = (FX_WCHAR)ret;
                break;
            }
            if (i == FXFT_Get_Face_CharmapCount(m_Font.m_Face) && i) {
                FXFT_Set_Charmap(m_Font.m_Face, FXFT_Get_Face_Charmaps(m_Font.m_Face)[0]);
                unicode = (FX_WCHAR)charcode;
            }
        }
        if (FXFT_Get_Face_Charmap(m_Font.m_Face)) {
            int index = GetGlyphIndex(unicode, pVertGlyph);
            if (index == 0) {
                return -1;
            }
            return index;
        }
        return unicode ;
    }
    if (m_Font.m_Face == NULL) {
        return -1;
    }
    FX_WORD cid = CIDFromCharCode(charcode);
    if (m_bType1) {
        if (NULL == m_pCIDToGIDMap) {
            return cid;
        }
    } else {
        if (m_pCIDToGIDMap == NULL) {
            if (m_pFontFile && m_pCMap->m_pMapping == NULL) {
                return cid;
            }
            if (m_pCMap->m_Coding == CIDCODING_UNKNOWN || FXFT_Get_Face_Charmap(m_Font.m_Face) == NULL) {
                return cid;
            }
            if (FXFT_Get_Charmap_Encoding(FXFT_Get_Face_Charmap(m_Font.m_Face)) == FXFT_ENCODING_UNICODE) {
                CFX_WideString unicode_str = UnicodeFromCharCode(charcode);
                if (unicode_str.IsEmpty()) {
                    return -1;
                }
                charcode = unicode_str.GetAt(0);
            }
            return GetGlyphIndex(charcode, pVertGlyph);
        }
    }
    FX_DWORD byte_pos = cid * 2;
    if (byte_pos + 2 > m_pCIDToGIDMap->GetSize()) {
        return -1;
    }
    FX_LPCBYTE pdata = m_pCIDToGIDMap->GetData() + byte_pos;
    return pdata[0] * 256 + pdata[1];
}
FX_DWORD CPDF_CIDFont::GetNextChar(FX_LPCSTR pString, int nStrLen, int& offset) const
{
    return m_pCMap->GetNextChar(pString, nStrLen, offset);
}
int CPDF_CIDFont::GetCharSize(FX_DWORD charcode) const
{
    return m_pCMap->GetCharSize(charcode);
}
int CPDF_CIDFont::CountChar(FX_LPCSTR pString, int size) const
{
    return m_pCMap->CountChar(pString, size);
}
int CPDF_CIDFont::AppendChar(FX_LPSTR str, FX_DWORD charcode) const
{
    return m_pCMap->AppendChar(str, charcode);
}
FX_BOOL CPDF_CIDFont::IsUnicodeCompatible() const
{
    if (!m_pCMap->IsLoaded() || m_pCID2UnicodeMap == NULL || !m_pCID2UnicodeMap->IsLoaded()) {
        return m_pCMap->m_Coding != CIDCODING_UNKNOWN;
    }
    return TRUE;
}
FX_BOOL CPDF_CIDFont::IsFontStyleFromCharCode(FX_DWORD charcode) const
{
    return TRUE;
}
void CPDF_CIDFont::LoadSubstFont()
{
    m_Font.LoadSubst(m_BaseFont, !m_bType1, m_Flags, m_StemV * 5, m_ItalicAngle, g_CharsetCPs[m_Charset], IsVertWriting());
}
void CPDF_CIDFont::LoadMetricsArray(CPDF_Array* pArray, CFX_DWordArray& result, int nElements)
{
    int width_status = 0;
    int iCurElement = 0;
    int first_code = 0, last_code;
    FX_DWORD count = pArray->GetCount();
    for (FX_DWORD i = 0; i < count; i ++) {
        CPDF_Object* pObj = pArray->GetElementValue(i);
        if (pObj == NULL) {
            continue;
        }
        if (pObj->GetType() == PDFOBJ_ARRAY) {
            if (width_status != 1) {
                return;
            }
            CPDF_Array* pArray = (CPDF_Array*)pObj;
            FX_DWORD count = pArray->GetCount();
            for (FX_DWORD j = 0; j < count; j += nElements) {
                result.Add(first_code);
                result.Add(first_code);
                for (int k = 0; k < nElements; k ++) {
                    result.Add(pArray->GetInteger(j + k));
                }
                first_code ++;
            }
            width_status = 0;
        } else {
            if (width_status == 0) {
                first_code = pObj->GetInteger();
                width_status = 1;
            } else if (width_status == 1) {
                last_code = pObj->GetInteger();
                width_status = 2;
                iCurElement = 0;
            } else {
                if (!iCurElement) {
                    result.Add(first_code);
                    result.Add(last_code);
                }
                result.Add(pObj->GetInteger());
                iCurElement ++;
                if (iCurElement == nElements) {
                    width_status = 0;
                }
            }
        }
    }
}
FX_BOOL CPDF_CIDFont::LoadGB2312()
{
    m_BaseFont = m_pFontDict->GetString(FX_BSTRC("BaseFont"));
    CPDF_Dictionary* pFontDesc = m_pFontDict->GetDict(FX_BSTRC("FontDescriptor"));
    if (pFontDesc) {
        LoadFontDescriptor(pFontDesc);
    }
    m_Charset = CIDSET_GB1;
    m_bType1 = FALSE;
    m_pCMap = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals()->m_CMapManager.GetPredefinedCMap(
                  FX_BSTRC("GBK-EUC-H"), FALSE);
    m_pCID2UnicodeMap = CPDF_ModuleMgr::Get()->GetPageModule()->GetFontGlobals()->m_CMapManager.GetCID2UnicodeMap(m_Charset, FALSE);
    if (!IsEmbedded()) {
        LoadSubstFont();
    }
    CheckFontMetrics();
    m_DefaultWidth = 1000;
    m_pAnsiWidths = FX_Alloc(FX_WORD, 128);
    for (int i = 32; i < 127; i ++) {
        m_pAnsiWidths[i] = 500;
    }
    return TRUE;
}
const struct _CIDTransform {
    FX_WORD		CID;
    FX_BYTE		a, b, c, d, e, f;
}
Japan1_VertCIDs[] = {
    {97, 129, 0, 0, 127, 55, 0},
    {7887, 127, 0, 0, 127, 76, 89},
    {7888, 127, 0, 0, 127, 79, 94},
    {7889, 0, 129, 127, 0, 17, 127},
    {7890, 0, 129, 127, 0, 17, 127},
    {7891, 0, 129, 127, 0, 17, 127},
    {7892, 0, 129, 127, 0, 17, 127},
    {7893, 0, 129, 127, 0, 17, 127},
    {7894, 0, 129, 127, 0, 17, 127},
    {7895, 0, 129, 127, 0, 17, 127},
    {7896, 0, 129, 127, 0, 17, 127},
    {7897, 0, 129, 127, 0, 17, 127},
    {7898, 0, 129, 127, 0, 17, 127},
    {7899, 0, 129, 127, 0, 17, 104},
    {7900, 0, 129, 127, 0, 17, 127},
    {7901, 0, 129, 127, 0, 17, 104},
    {7902, 0, 129, 127, 0, 17, 127},
    {7903, 0, 129, 127, 0, 17, 127},
    {7904, 0, 129, 127, 0, 17, 127},
    {7905, 0, 129, 127, 0, 17, 114},
    {7906, 0, 129, 127, 0, 17, 127},
    {7907, 0, 129, 127, 0, 17, 127},
    {7908, 0, 129, 127, 0, 17, 127},
    {7909, 0, 129, 127, 0, 17, 127},
    {7910, 0, 129, 127, 0, 17, 127},
    {7911, 0, 129, 127, 0, 17, 127},
    {7912, 0, 129, 127, 0, 17, 127},
    {7913, 0, 129, 127, 0, 17, 127},
    {7914, 0, 129, 127, 0, 17, 127},
    {7915, 0, 129, 127, 0, 17, 114},
    {7916, 0, 129, 127, 0, 17, 127},
    {7917, 0, 129, 127, 0, 17, 127},
    {7918, 127, 0, 0, 127, 18, 25},
    {7919, 127, 0, 0, 127, 18, 25},
    {7920, 127, 0, 0, 127, 18, 25},
    {7921, 127, 0, 0, 127, 18, 25},
    {7922, 127, 0, 0, 127, 18, 25},
    {7923, 127, 0, 0, 127, 18, 25},
    {7924, 127, 0, 0, 127, 18, 25},
    {7925, 127, 0, 0, 127, 18, 25},
    {7926, 127, 0, 0, 127, 18, 25},
    {7927, 127, 0, 0, 127, 18, 25},
    {7928, 127, 0, 0, 127, 18, 25},
    {7929, 127, 0, 0, 127, 18, 25},
    {7930, 127, 0, 0, 127, 18, 25},
    {7931, 127, 0, 0, 127, 18, 25},
    {7932, 127, 0, 0, 127, 18, 25},
    {7933, 127, 0, 0, 127, 18, 25},
    {7934, 127, 0, 0, 127, 18, 25},
    {7935, 127, 0, 0, 127, 18, 25},
    {7936, 127, 0, 0, 127, 18, 25},
    {7937, 127, 0, 0, 127, 18, 25},
    {7938, 127, 0, 0, 127, 18, 25},
    {7939, 127, 0, 0, 127, 18, 25},
    {8720, 0, 129, 127, 0, 19, 102},
    {8721, 0, 129, 127, 0, 13, 127},
    {8722, 0, 129, 127, 0, 19, 108},
    {8723, 0, 129, 127, 0, 19, 102},
    {8724, 0, 129, 127, 0, 19, 102},
    {8725, 0, 129, 127, 0, 19, 102},
    {8726, 0, 129, 127, 0, 19, 102},
    {8727, 0, 129, 127, 0, 19, 102},
    {8728, 0, 129, 127, 0, 19, 114},
    {8729, 0, 129, 127, 0, 19, 114},
    {8730, 0, 129, 127, 0, 38, 108},
    {8731, 0, 129, 127, 0, 13, 108},
    {8732, 0, 129, 127, 0, 19, 108},
    {8733, 0, 129, 127, 0, 19, 108},
    {8734, 0, 129, 127, 0, 19, 108},
    {8735, 0, 129, 127, 0, 19, 108},
    {8736, 0, 129, 127, 0, 19, 102},
    {8737, 0, 129, 127, 0, 19, 102},
    {8738, 0, 129, 127, 0, 19, 102},
    {8739, 0, 129, 127, 0, 19, 102},
    {8740, 0, 129, 127, 0, 19, 102},
    {8741, 0, 129, 127, 0, 19, 102},
    {8742, 0, 129, 127, 0, 19, 102},
    {8743, 0, 129, 127, 0, 19, 102},
    {8744, 0, 129, 127, 0, 19, 102},
    {8745, 0, 129, 127, 0, 19, 102},
    {8746, 0, 129, 127, 0, 19, 114},
    {8747, 0, 129, 127, 0, 19, 114},
    {8748, 0, 129, 127, 0, 19, 102},
    {8749, 0, 129, 127, 0, 19, 102},
    {8750, 0, 129, 127, 0, 19, 102},
    {8751, 0, 129, 127, 0, 19, 102},
    {8752, 0, 129, 127, 0, 19, 102},
    {8753, 0, 129, 127, 0, 19, 102},
    {8754, 0, 129, 127, 0, 19, 102},
    {8755, 0, 129, 127, 0, 19, 102},
    {8756, 0, 129, 127, 0, 19, 102},
    {8757, 0, 129, 127, 0, 19, 102},
    {8758, 0, 129, 127, 0, 19, 102},
    {8759, 0, 129, 127, 0, 19, 102},
    {8760, 0, 129, 127, 0, 19, 102},
    {8761, 0, 129, 127, 0, 19, 102},
    {8762, 0, 129, 127, 0, 19, 102},
    {8763, 0, 129, 127, 0, 19, 102},
    {8764, 0, 129, 127, 0, 19, 102},
    {8765, 0, 129, 127, 0, 19, 102},
    {8766, 0, 129, 127, 0, 19, 102},
    {8767, 0, 129, 127, 0, 19, 102},
    {8768, 0, 129, 127, 0, 19, 102},
    {8769, 0, 129, 127, 0, 19, 102},
    {8770, 0, 129, 127, 0, 19, 102},
    {8771, 0, 129, 127, 0, 19, 102},
    {8772, 0, 129, 127, 0, 19, 102},
    {8773, 0, 129, 127, 0, 19, 102},
    {8774, 0, 129, 127, 0, 19, 102},
    {8775, 0, 129, 127, 0, 19, 102},
    {8776, 0, 129, 127, 0, 19, 102},
    {8777, 0, 129, 127, 0, 19, 102},
    {8778, 0, 129, 127, 0, 19, 102},
    {8779, 0, 129, 127, 0, 19, 114},
    {8780, 0, 129, 127, 0, 19, 108},
    {8781, 0, 129, 127, 0, 19, 114},
    {8782, 0, 129, 127, 0, 13, 114},
    {8783, 0, 129, 127, 0, 19, 108},
    {8784, 0, 129, 127, 0, 13, 114},
    {8785, 0, 129, 127, 0, 19, 108},
    {8786, 0, 129, 127, 0, 19, 108},
    {8787, 0, 129, 127, 0, 19, 108},
    {8788, 0, 129, 127, 0, 19, 108},
    {8789, 0, 129, 127, 0, 19, 108},
    {8790, 0, 129, 127, 0, 19, 108},
    {8791, 0, 129, 127, 0, 19, 108},
    {8792, 0, 129, 127, 0, 19, 108},
    {8793, 0, 129, 127, 0, 19, 108},
    {8794, 0, 129, 127, 0, 19, 108},
    {8795, 0, 129, 127, 0, 19, 108},
    {8796, 0, 129, 127, 0, 19, 108},
    {8797, 0, 129, 127, 0, 19, 108},
    {8798, 0, 129, 127, 0, 19, 108},
    {8799, 0, 129, 127, 0, 19, 108},
    {8800, 0, 129, 127, 0, 19, 108},
    {8801, 0, 129, 127, 0, 19, 108},
    {8802, 0, 129, 127, 0, 19, 108},
    {8803, 0, 129, 127, 0, 19, 108},
    {8804, 0, 129, 127, 0, 19, 108},
    {8805, 0, 129, 127, 0, 19, 108},
    {8806, 0, 129, 127, 0, 19, 108},
    {8807, 0, 129, 127, 0, 19, 108},
    {8808, 0, 129, 127, 0, 19, 108},
    {8809, 0, 129, 127, 0, 19, 108},
    {8810, 0, 129, 127, 0, 19, 108},
    {8811, 0, 129, 127, 0, 19, 114},
    {8812, 0, 129, 127, 0, 19, 102},
    {8813, 0, 129, 127, 0, 19, 114},
    {8814, 0, 129, 127, 0, 76, 102},
    {8815, 0, 129, 127, 0, 13, 121},
    {8816, 0, 129, 127, 0, 19, 114},
    {8817, 0, 129, 127, 0, 19, 127},
    {8818, 0, 129, 127, 0, 19, 114},
    {8819, 0, 129, 127, 0, 218, 108},
};
FX_LPCBYTE CPDF_CIDFont::GetCIDTransform(FX_WORD CID) const
{
    if (m_Charset != CIDSET_JAPAN1 || m_pFontFile != NULL) {
        return NULL;
    }
    int begin = 0;
    int end = sizeof Japan1_VertCIDs / sizeof(struct _CIDTransform) - 1;
    while (begin <= end) {
        int middle = (begin + end) / 2;
        FX_WORD middlecode = Japan1_VertCIDs[middle].CID;
        if (middlecode > CID) {
            end = middle - 1;
        } else if (middlecode < CID) {
            begin = middle + 1;
        } else {
            return &Japan1_VertCIDs[middle].a;
        }
    }
    return NULL;
}

// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

typedef void* FXFT_Library;
class CPDF_CMapManager : public CFX_Object
{
public:
    CPDF_CMapManager();
    ~CPDF_CMapManager();
    FX_LPVOID				GetPackage(FX_BOOL bPrompt);
    CPDF_CMap*				GetPredefinedCMap(const CFX_ByteString& name, FX_BOOL bPrompt);
    CPDF_CID2UnicodeMap*	GetCID2UnicodeMap(int charset, FX_BOOL bPrompt);
    void					ReloadAll();
private:
    CPDF_CMap*				LoadPredefinedCMap(const CFX_ByteString& name,  FX_BOOL bPrompt);
    CPDF_CID2UnicodeMap*	LoadCID2UnicodeMap(int charset, FX_BOOL bPrompt);
    void					DropAll(FX_BOOL bReload);
    FX_BOOL					m_bPrompted;
    CFX_MapByteStringToPtr	m_CMaps;
    CPDF_CID2UnicodeMap*	m_CID2UnicodeMaps[6];
};
class CPDF_FontGlobals : public CFX_Object
{
public:
    CPDF_FontGlobals();
    ~CPDF_FontGlobals();
    void				ClearAll();
    void				Clear(void* key);
    CPDF_Font*			Find(void* key, int index);
    void				Set(void* key, int index, CPDF_Font* pFont);
    CFX_MapPtrToPtr		m_pStockMap;
    CPDF_CMapManager	m_CMapManager;
    struct {
        const struct FXCMAP_CMap*	m_pMapList;
        int				m_Count;
    } m_EmbeddedCharsets[NUMBER_OF_CIDSETS];
    struct {
        const FX_WORD*	m_pMap;
        int				m_Count;
    } m_EmbeddedToUnicodes[NUMBER_OF_CIDSETS];
    FX_LPBYTE			m_pContrastRamps;
};
struct _CMap_CodeRange {
    int			m_CharSize;
    FX_BYTE		m_Lower[4];
    FX_BYTE		m_Upper[4];
};
class CPDF_CMapParser : public CFX_Object
{
public:
    CPDF_CMapParser();
    ~CPDF_CMapParser() {}
    FX_BOOL	Initialize(CPDF_CMap*);
    void	ParseWord(FX_BSTR str);
    CFX_BinaryBuf	m_AddMaps;
private:
    CPDF_CMap*	m_pCMap;
    int				m_Status;
    int				m_CodeSeq;
    FX_DWORD		m_CodePoints[4];
    CFX_ArrayTemplate<_CMap_CodeRange>	m_CodeRanges;
    CFX_ByteString	m_Registry, m_Ordering, m_Supplement;
    CFX_ByteString	m_LastWord;
};
#define CIDCODING_UNKNOWN	0
#define CIDCODING_GB		1
#define CIDCODING_BIG5		2
#define CIDCODING_JIS		3
#define CIDCODING_KOREA		4
#define CIDCODING_UCS2		5
#define CIDCODING_CID		6
#define CIDCODING_UTF16		7
class CPDF_CMap : public CFX_Object
{
public:
    CPDF_CMap();
    FX_BOOL					LoadPredefined(CPDF_CMapManager* pMgr, const FX_CHAR* name, FX_BOOL bPromptCJK);
    FX_BOOL					LoadEmbedded(FX_LPCBYTE pData, FX_DWORD dwSize);
    void					Release();
    FX_BOOL					IsLoaded() const
    {
        return m_bLoaded;
    }
    int						GetCharset()
    {
        return m_Charset;
    }
    FX_BOOL					IsVertWriting() const
    {
        return m_bVertical;
    }
    FX_WORD					CIDFromCharCode(FX_DWORD charcode) const;
    FX_DWORD				CharCodeFromCID(FX_WORD CID) const;
    int						GetCharSize(FX_DWORD charcode) const;
    FX_DWORD				GetNextChar(const FX_CHAR* pString, int& offset) const;
    int						CountChar(const FX_CHAR* pString, int size) const;
    int						AppendChar(FX_LPSTR str, FX_DWORD charcode) const;
    typedef enum {OneByte, TwoBytes, MixedTwoBytes, MixedFourBytes} CodingScheme;
protected:
    ~CPDF_CMap();
    friend class			CPDF_CMapParser;
    friend class			CPDF_CMapManager;
    friend class			CPDF_CIDFont;
protected:
    CFX_ByteString			m_PredefinedCMap;
    FX_BOOL					m_bVertical;
    int						m_Charset, m_Coding;
    CodingScheme			m_CodingScheme;
    int						m_nCodeRanges;
    FX_BYTE*				m_pLeadingBytes;
    FX_WORD*				m_pMapping;
    FX_LPBYTE				m_pAddMapping;
    FX_BOOL					m_bLoaded;
    const FXCMAP_CMap*		m_pEmbedMap;
    CPDF_CMap*				m_pUseMap;
};
class CPDF_PredefinedCMap
{
public:
    FX_LPCSTR	m_pName;
    int			m_Charset;
    int			m_Coding;
    CPDF_CMap::CodingScheme	m_CodingScheme;
    FX_DWORD	m_LeadingSegCount;
    FX_BYTE		m_LeadingSegs[4];
};
typedef struct _FileHeader {
    FX_BYTE		btTag[4];
    FX_BYTE		btVersion;
    FX_BYTE		btFormat;
    FX_BYTE		btReserved1[2];
    FX_DWORD	dwStartIndex;
    FX_DWORD	dwEndIndex;
    FX_DWORD	dwDataSize;
    FX_DWORD	dwDataOffset;
    FX_DWORD	dwRecordSize;
} FXMP_FILEHEADER;
class CPDF_CID2UnicodeMap : public CFX_Object
{
public:
    CPDF_CID2UnicodeMap();
    ~CPDF_CID2UnicodeMap();
    FX_BOOL		Initialize();
    FX_BOOL		IsLoaded();
    void		Load(CPDF_CMapManager* pMgr, int charset, FX_BOOL bPromptCJK);
    FX_WCHAR	UnicodeFromCID(FX_WORD CID);
protected:
    int			m_Charset;
    const FX_WORD*	m_pEmbeddedMap;
    FX_DWORD	m_EmbeddedCount;
};
class CPDF_ToUnicodeMap : public CFX_Object
{
public:
    void					Load(CPDF_Stream* pStream);
    CFX_WideString			Lookup(FX_DWORD charcode);
    FX_DWORD				ReverseLookup(FX_WCHAR unicode);
protected:
    CFX_CMapDWordToDWord	m_Map;
    CPDF_CID2UnicodeMap*	m_pBaseMap;
    CFX_WideTextBuf			m_MultiCharBuf;
};
class CPDF_FontCharMap : public CFX_CharMap, public CFX_Object
{
public:
    CPDF_FontCharMap(CPDF_Font* pFont);
    CPDF_Font*		m_pFont;
};

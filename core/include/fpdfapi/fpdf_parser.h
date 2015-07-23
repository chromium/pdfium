// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_PARSER_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_PARSER_H_

#include "../fxcrt/fx_system.h"
#include "fpdf_objects.h"

class CPDF_Document;
class CPDF_Parser;
class CPDF_SecurityHandler;
class CPDF_StandardSecurityHandler;
class CPDF_CryptoHandler;
class CPDF_Object;
class IFX_FileRead;
class CFDF_Document;
class CFDF_Parser;
class CFX_Font;
class CFX_AffineMatrix;
class CFX_FloatRect;
class CPDF_Point;
class CPDF_DocPageData;
class CPDF_DocRenderData;
class CPDF_ModuleMgr;
class CFX_DIBSource;
class CPDF_Font;
class CPDF_Image;
class CPDF_ColorSpace;
class CPDF_Pattern;
class CPDF_FontEncoding;
class CPDF_IccProfile;
class CFX_PrivateData;
#define FPDFPERM_PRINT			0x0004
#define FPDFPERM_MODIFY			0x0008
#define FPDFPERM_EXTRACT		0x0010
#define FPDFPERM_ANNOT_FORM		0x0020
#define FPDFPERM_FILL_FORM		0x0100
#define FPDFPERM_EXTRACT_ACCESS	0x0200
#define FPDFPERM_ASSEMBLE		0x0400
#define FPDFPERM_PRINT_HIGH		0x0800
#define FPDF_PAGE_MAX_NUM		0xFFFFF

// Indexed by 8-bit character code, contains either:
//   'W' - for whitespace: NUL, TAB, CR, LF, FF, 0x80, 0xff
//   'N' - for numeric: 0123456789+-.
//   'D' - for delimiter: %()/<>[]{}
//   'R' - otherwise.
extern const char PDF_CharType[256];

class CPDF_Document : public CFX_PrivateData, public CPDF_IndirectObjects
{
public:
    CPDF_Document();
    explicit CPDF_Document(CPDF_Parser* pParser);

    ~CPDF_Document();

    CPDF_Parser*			GetParser() const
    {
        return m_pParser;
    }

    CPDF_Dictionary*		GetRoot() const
    {
        return m_pRootDict;
    }

    CPDF_Dictionary*		GetInfo() const
    {
        return m_pInfoDict;
    }

    void					GetID(CFX_ByteString& id1, CFX_ByteString& id2) const
    {
        id1 = m_ID1;
        id2 = m_ID2;
    }

    int						GetPageCount() const;

    CPDF_Dictionary*		GetPage(int iPage);

    int						GetPageIndex(FX_DWORD objnum);

    FX_DWORD				GetUserPermissions(bool bCheckRevision = false) const;

    bool					IsOwner() const;



    CPDF_DocPageData*		GetPageData()
    {
        return GetValidatePageData();
    }

    void					ClearPageData();

    void					RemoveColorSpaceFromPageData(CPDF_Object* pObject);


    CPDF_DocRenderData*		GetRenderData()
    {
        return GetValidateRenderData();
    }

    void					ClearRenderData();

    void					ClearRenderFont();


    bool					IsFormStream(FX_DWORD objnum, bool& bForm) const;

    // |pFontDict| must not be null.
    CPDF_Font* LoadFont(CPDF_Dictionary* pFontDict);

    CPDF_ColorSpace*		LoadColorSpace(CPDF_Object* pCSObj, CPDF_Dictionary* pResources = NULL);

    CPDF_Pattern*			LoadPattern(CPDF_Object* pObj, bool bShading, const CFX_AffineMatrix* matrix = NULL);

    CPDF_Image*				LoadImageF(CPDF_Object* pObj);

    CPDF_StreamAcc*			LoadFontFile(CPDF_Stream* pStream);

    CPDF_IccProfile*		LoadIccProfile(CPDF_Stream* pStream);

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

    CPDF_Font*				AddWindowsFont(LOGFONTA* pLogFont, bool bVert, bool bTranslateName = false);
    CPDF_Font*				AddWindowsFont(LOGFONTW* pLogFont, bool bVert, bool bTranslateName = false);
#endif
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
    CPDF_Font*              AddMacFont(CTFontRef pFont, bool bVert, bool bTranslateName = false);
#endif

    CPDF_Font*				AddStandardFont(const FX_CHAR* font, CPDF_FontEncoding* pEncoding);


    CPDF_Font*				AddFont(CFX_Font* pFont, int charset, bool bVert);

    void					CreateNewDoc();

    CPDF_Dictionary*		CreateNewPage(int iPage);

    void					DeletePage(int iPage);

    void					LoadDoc();
    void					LoadAsynDoc(CPDF_Dictionary *pLinearized);
    void					LoadPages();
protected:

    CPDF_Dictionary*		m_pRootDict;

    CPDF_Dictionary*		m_pInfoDict;

    CFX_ByteString			m_ID1;

    CFX_ByteString			m_ID2;


    bool					m_bLinearized;

    FX_DWORD				m_dwFirstPageNo;

    FX_DWORD				m_dwFirstPageObjNum;

    CFX_DWordArray			m_PageList;

    int						_GetPageCount() const;
    CPDF_Dictionary*		_FindPDFPage(CPDF_Dictionary* pPages, int iPage, int nPagesToGo, int level);
    int						_FindPageIndex(CPDF_Dictionary* pNode, FX_DWORD& skip_count, FX_DWORD objnum, int& index, int level = 0);
    bool					IsContentUsedElsewhere(FX_DWORD objnum, CPDF_Dictionary* pPageDict);
    bool					CheckOCGVisible(CPDF_Dictionary* pOCG, bool bPrinting);
    CPDF_DocPageData*		GetValidatePageData();
    CPDF_DocRenderData*		GetValidateRenderData();
    friend class			CPDF_Creator;
    friend class			CPDF_Parser;
    friend class			CPDF_DataAvail;
    friend class			CPDF_OCContext;



    CPDF_DocPageData*		m_pDocPage;

    CPDF_DocRenderData*		m_pDocRender;

};

#define PDFWORD_EOF			0
#define PDFWORD_NUMBER		1
#define PDFWORD_TEXT		2
#define PDFWORD_DELIMITER	3
#define PDFWORD_NAME		4
class CPDF_SimpleParser
{
public:

    CPDF_SimpleParser(const uint8_t* pData, FX_DWORD dwSize);

    CPDF_SimpleParser(const CFX_ByteStringC& str);

    CFX_ByteStringC		GetWord();

    bool				SearchToken(const CFX_ByteStringC& token);

    bool				SkipWord(const CFX_ByteStringC& token);

    bool				FindTagPair(const CFX_ByteStringC& start_token, const CFX_ByteStringC& end_token,
                                    FX_DWORD& start_pos, FX_DWORD& end_pos);

    bool				FindTagParam(const CFX_ByteStringC& token, int nParams);

    FX_DWORD			GetPos()
    {
        return m_dwCurPos;
    }

    void				SetPos(FX_DWORD pos)
    {
        ASSERT(pos <= m_dwSize);
        m_dwCurPos = pos;
    }
private:

    void				ParseWord(const uint8_t*& pStart, FX_DWORD& dwSize, int& type);

    const uint8_t*			m_pData;

    FX_DWORD			m_dwSize;

    FX_DWORD			m_dwCurPos;
};
class CPDF_SyntaxParser
{
public:

    CPDF_SyntaxParser();
    virtual ~CPDF_SyntaxParser();

    void				InitParser(IFX_FileRead* pFileAccess, FX_DWORD HeaderOffset);

    FX_FILESIZE			SavePos()
    {
        return m_Pos;
    }

    void				RestorePos(FX_FILESIZE pos)
    {
        m_Pos = pos;
    }

    CPDF_Object*		GetObject(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, FX_DWORD gennum, struct PARSE_CONTEXT* pContext = NULL, bool bDecrypt = true);


    CPDF_Object*		GetObjectByStrict(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, FX_DWORD gennum, struct PARSE_CONTEXT* pContext = NULL);

    int					GetDirectNum();

    CFX_ByteString		GetString(FX_DWORD objnum, FX_DWORD gennum);

    CFX_ByteString		GetName();

    CFX_ByteString		GetKeyword();

    void				GetBinary(uint8_t* buffer, FX_DWORD size);

    void				ToNextLine();

    void				ToNextWord();

    bool				SearchWord(const CFX_ByteStringC& word, bool bWholeWord, bool bForward, FX_FILESIZE limit);

    int					SearchMultiWord(const CFX_ByteStringC& words, bool bWholeWord, FX_FILESIZE limit);

    FX_FILESIZE			FindTag(const CFX_ByteStringC& tag, FX_FILESIZE limit);

    void				SetEncrypt(CPDF_CryptoHandler* pCryptoHandler)
    {
        m_pCryptoHandler = pCryptoHandler;
    }

    bool				IsEncrypted()
    {
        return m_pCryptoHandler != NULL;
    }

    bool				GetCharAt(FX_FILESIZE pos, uint8_t& ch);

    bool				ReadBlock(uint8_t* pBuf, FX_DWORD size);

    CFX_ByteString		GetNextWord(bool& bIsNumber);
protected:
    static const int kParserMaxRecursionDepth = 64;
    static int s_CurrentRecursionDepth;

    virtual bool				GetNextChar(uint8_t& ch);

    bool				GetCharAtBackward(FX_FILESIZE pos, uint8_t& ch);

    void				GetNextWord();

    bool				IsWholeWord(FX_FILESIZE startpos, FX_FILESIZE limit, const uint8_t* tag, FX_DWORD taglen);

    CFX_ByteString		ReadString();

    CFX_ByteString		ReadHexString();

    CPDF_Stream*		ReadStream(CPDF_Dictionary* pDict, PARSE_CONTEXT* pContext, FX_DWORD objnum, FX_DWORD gennum);

    FX_FILESIZE			m_Pos;

    bool				m_bFileStream;

    int					m_MetadataObjnum;

    IFX_FileRead*		m_pFileAccess;

    FX_DWORD			m_HeaderOffset;

    FX_FILESIZE			m_FileLen;

    uint8_t*			m_pFileBuf;

    FX_DWORD			m_BufSize;

    FX_FILESIZE			m_BufOffset;

    CPDF_CryptoHandler*	m_pCryptoHandler;

    uint8_t				m_WordBuffer[257];

    FX_DWORD			m_WordSize;

    bool				m_bIsNumber;

    FX_FILESIZE			m_dwWordPos;
    friend class		CPDF_Parser;
    friend class		CPDF_DataAvail;
};

#define PDFPARSE_TYPEONLY	1
#define PDFPARSE_NOSTREAM	2
struct PARSE_CONTEXT {

    bool		m_Flags;

    FX_FILESIZE	m_DictStart;

    FX_FILESIZE	m_DictEnd;

    FX_FILESIZE	m_DataStart;

    FX_FILESIZE	m_DataEnd;
};

#define PDFPARSE_ERROR_SUCCESS		0
#define PDFPARSE_ERROR_FILE			1
#define PDFPARSE_ERROR_FORMAT		2
#define PDFPARSE_ERROR_PASSWORD		3
#define PDFPARSE_ERROR_HANDLER		4
#define PDFPARSE_ERROR_CERT			5

class CPDF_Parser
{
public:
    CPDF_Parser();
    ~CPDF_Parser();

    FX_DWORD			StartParse(const FX_CHAR* filename, bool bReParse = false);
    FX_DWORD			StartParse(const FX_WCHAR* filename, bool bReParse = false);
    FX_DWORD			StartParse(IFX_FileRead* pFile, bool bReParse = false, bool bOwnFileRead = true);

    void				CloseParser(bool bReParse = false);

    FX_DWORD	GetPermissions(bool bCheckRevision = false);

    bool		IsOwner();

    void				SetPassword(const FX_CHAR* password)
    {
        m_Password = password;
    }

    CFX_ByteString		GetPassword()
    {
        return m_Password;
    }

    CPDF_SecurityHandler* GetSecurityHandler()
    {
        return m_pSecurityHandler;
    }

    CPDF_CryptoHandler*	GetCryptoHandler()
    {
        return m_Syntax.m_pCryptoHandler;
    }

    void				SetSecurityHandler(CPDF_SecurityHandler* pSecurityHandler, bool bForced = false);

    CFX_ByteString		GetRecipient()
    {
        return m_bsRecipient;
    }

    CPDF_Dictionary*	GetTrailer()
    {
        return m_pTrailer;
    }

    FX_FILESIZE			GetLastXRefOffset()
    {
        return m_LastXRefOffset;
    }

    CPDF_Document*		GetDocument()
    {
        return m_pDocument;
    }

    CFX_ArrayTemplate<CPDF_Dictionary*>* GetOtherTrailers()
    {
        return &m_Trailers;
    }

    FX_DWORD	GetRootObjNum();
    FX_DWORD	GetInfoObjNum() ;
    CPDF_Array*	GetIDArray() ;

    CPDF_Dictionary*	GetEncryptDict()
    {
        return m_pEncryptDict;
    }

    bool				IsEncrypted()
    {
        return GetEncryptDict() != NULL;
    }


    CPDF_Object*		ParseIndirectObject(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, PARSE_CONTEXT* pContext = NULL) ;
    FX_DWORD			GetLastObjNum();
    bool				IsFormStream(FX_DWORD objnum, bool& bForm);

    FX_FILESIZE			GetObjectOffset(FX_DWORD objnum);

    FX_FILESIZE			GetObjectSize(FX_DWORD objnum);

    int					GetObjectVersion(FX_DWORD objnum)
    {
        return m_ObjVersion[objnum];
    }

    void				GetIndirectBinary(FX_DWORD objnum, uint8_t*& pBuffer, FX_DWORD& size);

    bool				GetFileStreamOption()
    {
        return m_Syntax.m_bFileStream;
    }

    void				SetFileStreamOption(bool b)
    {
        m_Syntax.m_bFileStream = b;
    }

    IFX_FileRead*		GetFileAccess() const
    {
        return m_Syntax.m_pFileAccess;
    }

    int					GetFileVersion() const
    {
        return m_FileVersion;
    }

    bool				IsXRefStream() const
    {
        return m_bXRefStream;
    }
    CPDF_Object*		ParseIndirectObjectAt(CPDF_IndirectObjects* pObjList, FX_FILESIZE pos, FX_DWORD objnum,
            struct PARSE_CONTEXT* pContext);

    CPDF_Object*		ParseIndirectObjectAtByStrict(CPDF_IndirectObjects* pObjList, FX_FILESIZE pos, FX_DWORD objnum,
            struct PARSE_CONTEXT* pContext, FX_FILESIZE *pResultPos);

    FX_DWORD			StartAsynParse(IFX_FileRead* pFile, bool bReParse = false, bool bOwnFileRead = true);

    FX_DWORD			GetFirstPageNo()
    {
        return m_dwFirstPageNo;
    }
protected:
    CPDF_Document*		m_pDocument;

    CPDF_SyntaxParser	m_Syntax;
    bool				m_bOwnFileRead;
    CPDF_Object*		ParseDirect(CPDF_Object* pObj);

    bool				LoadAllCrossRefV4(FX_FILESIZE pos);

    bool				LoadAllCrossRefV5(FX_FILESIZE pos);

    bool				LoadCrossRefV4(FX_FILESIZE pos, FX_FILESIZE streampos, bool bSkip, bool bFirst);

    bool				LoadCrossRefV5(FX_FILESIZE pos, FX_FILESIZE& prev, bool bMainXRef);

    CPDF_Dictionary*	LoadTrailerV4();

    bool				RebuildCrossRef();

    FX_DWORD			SetEncryptHandler();

    void				ReleaseEncryptHandler();

    bool				LoadLinearizedAllCrossRefV4(FX_FILESIZE pos, FX_DWORD dwObjCount);

    bool				LoadLinearizedCrossRefV4(FX_FILESIZE pos, FX_DWORD dwObjCount);

    bool				LoadLinearizedAllCrossRefV5(FX_FILESIZE pos);

    FX_DWORD			LoadLinearizedMainXRefTable();

    CFX_MapPtrToPtr		m_ObjectStreamMap;

    CPDF_StreamAcc*		GetObjectStream(FX_DWORD number);

    bool				IsLinearizedFile(IFX_FileRead* pFileAccess, FX_DWORD offset);



    int					m_FileVersion;

    CPDF_Dictionary*	m_pTrailer;

    CPDF_Dictionary*	m_pEncryptDict;
    void SetEncryptDictionary(CPDF_Dictionary* pDict);

    FX_FILESIZE			m_LastXRefOffset;

    bool				m_bXRefStream;


    CPDF_SecurityHandler*	m_pSecurityHandler;

    bool					m_bForceUseSecurityHandler;

    CFX_ByteString			m_bsRecipient;

    CFX_ByteString		m_FilePath;

    CFX_ByteString		m_Password;

    CFX_FileSizeArray	m_CrossRef;

    CFX_ByteArray		m_V5Type;

    CFX_FileSizeArray	m_SortedOffset;

    CFX_WordArray		m_ObjVersion;
    CFX_ArrayTemplate<CPDF_Dictionary *>	m_Trailers;

    bool				m_bVersionUpdated;

    CPDF_Object*		m_pLinearized;

    FX_DWORD			m_dwFirstPageNo;

    FX_DWORD			m_dwXrefStartObjNum;
    friend class		CPDF_Creator;
    friend class		CPDF_DataAvail;
};
#define FXCIPHER_NONE	0
#define FXCIPHER_RC4	1
#define FXCIPHER_AES	2
#define FXCIPHER_AES2   3
class CPDF_SecurityHandler
{
public:

    virtual ~CPDF_SecurityHandler() {}

    virtual bool		OnInit(CPDF_Parser* pParser, CPDF_Dictionary* pEncryptDict) = 0;

    virtual FX_DWORD	GetPermissions() = 0;

    virtual bool		IsOwner() = 0;

    virtual bool		GetCryptInfo(int& cipher, const uint8_t*& buffer, int& keylen) = 0;

    virtual bool		IsMetadataEncrypted()
    {
        return true;
    }

    virtual CPDF_CryptoHandler*	CreateCryptoHandler() = 0;

    virtual CPDF_StandardSecurityHandler* GetStandardHandler()
    {
        return NULL;
    }
};
#define PDF_ENCRYPT_CONTENT				0
class CPDF_StandardSecurityHandler : public CPDF_SecurityHandler
{
public:
    CPDF_StandardSecurityHandler();

    virtual ~CPDF_StandardSecurityHandler();
    virtual bool		OnInit(CPDF_Parser* pParser, CPDF_Dictionary* pEncryptDict);
    virtual FX_DWORD	GetPermissions();
    virtual bool		IsOwner()
    {
        return m_bOwner;
    }
    virtual bool		GetCryptInfo(int& cipher, const uint8_t*& buffer, int& keylen);
    virtual bool		IsMetadataEncrypted();
    virtual CPDF_CryptoHandler*	CreateCryptoHandler();
    virtual CPDF_StandardSecurityHandler* GetStandardHandler()
    {
        return this;
    }

    void				OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray,
                                 const uint8_t* user_pass, FX_DWORD user_size,
                                 const uint8_t* owner_pass, FX_DWORD owner_size, FX_DWORD type = PDF_ENCRYPT_CONTENT);

    void				OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray,
                                 const uint8_t* user_pass, FX_DWORD user_size, FX_DWORD type = PDF_ENCRYPT_CONTENT);

    CFX_ByteString		GetUserPassword(const uint8_t* owner_pass, FX_DWORD pass_size);
    CFX_ByteString		GetUserPassword(const uint8_t* owner_pass, FX_DWORD pass_size, int32_t key_len);
    int					GetVersion()
    {
        return m_Version;
    }
    int					GetRevision()
    {
        return m_Revision;
    }

    int					CheckPassword(const uint8_t* password, FX_DWORD pass_size, bool bOwner, uint8_t* key);
    int					CheckPassword(const uint8_t* password, FX_DWORD pass_size, bool bOwner, uint8_t* key, int key_len);
private:

    int					m_Version;

    int					m_Revision;

    CPDF_Parser*		m_pParser;

    CPDF_Dictionary*	m_pEncryptDict;

    bool				LoadDict(CPDF_Dictionary* pEncryptDict);
    bool				LoadDict(CPDF_Dictionary* pEncryptDict, FX_DWORD type, int& cipher, int& key_len);

    bool				CheckUserPassword(const uint8_t* password, FX_DWORD pass_size,
                                          bool bIgnoreEncryptMeta, uint8_t* key, int32_t key_len);

    bool				CheckOwnerPassword(const uint8_t* password, FX_DWORD pass_size, uint8_t* key, int32_t key_len);
    bool				AES256_CheckPassword(const uint8_t* password, FX_DWORD size, bool bOwner, uint8_t* key);
    void				AES256_SetPassword(CPDF_Dictionary* pEncryptDict, const uint8_t* password, FX_DWORD size, bool bOwner, const uint8_t* key);
    void				AES256_SetPerms(CPDF_Dictionary* pEncryptDict, FX_DWORD permission, bool bEncryptMetadata, const uint8_t* key);
    void				OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray,
                                 const uint8_t* user_pass, FX_DWORD user_size,
                                 const uint8_t* owner_pass, FX_DWORD owner_size, bool bDefault, FX_DWORD type);
    bool				CheckSecurity(int32_t key_len);

    bool				m_bOwner;

    FX_DWORD			m_Permissions;

    int					m_Cipher;

    uint8_t				m_EncryptKey[32];

    int					m_KeyLen;
};
class CPDF_CryptoHandler
{
public:

    virtual ~CPDF_CryptoHandler() {}

    virtual bool		Init(CPDF_Dictionary* pEncryptDict, CPDF_SecurityHandler* pSecurityHandler) = 0;

    virtual FX_DWORD	DecryptGetSize(FX_DWORD src_size) = 0;

    virtual void*	DecryptStart(FX_DWORD objnum, FX_DWORD gennum) = 0;

    virtual bool		DecryptStream(void* context, const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf) = 0;

    virtual bool		DecryptFinish(void* context, CFX_BinaryBuf& dest_buf) = 0;


    virtual FX_DWORD	EncryptGetSize(FX_DWORD objnum, FX_DWORD version, const uint8_t* src_buf, FX_DWORD src_size) = 0;

    virtual bool		EncryptContent(FX_DWORD objnum, FX_DWORD version, const uint8_t* src_buf, FX_DWORD src_size,
                                       uint8_t* dest_buf, FX_DWORD& dest_size) = 0;

    void				Decrypt(FX_DWORD objnum, FX_DWORD version, CFX_ByteString& str);
};
class CPDF_StandardCryptoHandler : public CPDF_CryptoHandler
{
public:

    CPDF_StandardCryptoHandler();

    virtual ~CPDF_StandardCryptoHandler();

    bool				Init(int cipher, const uint8_t* key, int keylen);
    virtual bool		Init(CPDF_Dictionary* pEncryptDict, CPDF_SecurityHandler* pSecurityHandler);
    virtual FX_DWORD	DecryptGetSize(FX_DWORD src_size);
    virtual void*	DecryptStart(FX_DWORD objnum, FX_DWORD gennum);
    virtual bool		DecryptStream(void* context, const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual bool		DecryptFinish(void* context, CFX_BinaryBuf& dest_buf);
    virtual FX_DWORD	EncryptGetSize(FX_DWORD objnum, FX_DWORD version, const uint8_t* src_buf, FX_DWORD src_size);
    virtual bool		EncryptContent(FX_DWORD objnum, FX_DWORD version, const uint8_t* src_buf, FX_DWORD src_size,
                                       uint8_t* dest_buf, FX_DWORD& dest_size);
protected:

    virtual void		CryptBlock(bool bEncrypt, FX_DWORD objnum, FX_DWORD gennum, const uint8_t* src_buf, FX_DWORD src_size,
                                   uint8_t* dest_buf, FX_DWORD& dest_size);
    virtual void*	CryptStart(FX_DWORD objnum, FX_DWORD gennum, bool bEncrypt);
    virtual bool		CryptStream(void* context, const uint8_t* src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf, bool bEncrypt);
    virtual bool		CryptFinish(void* context, CFX_BinaryBuf& dest_buf, bool bEncrypt);

    uint8_t				m_EncryptKey[32];

    int					m_KeyLen;

    int					m_Cipher;

    uint8_t*			m_pAESContext;
};
class CPDF_Point
{
public:

    CPDF_Point(FX_FLOAT xx, FX_FLOAT yy)
    {
        x = xx;
        y = yy;
    }

    FX_FLOAT			x;

    FX_FLOAT			y;
};

#define CPDF_Rect		CFX_FloatRect
#define CPDF_Matrix		CFX_AffineMatrix
CFX_ByteString PDF_NameDecode(const CFX_ByteStringC& orig);
CFX_ByteString PDF_NameDecode(const CFX_ByteString& orig);
CFX_ByteString PDF_NameEncode(const CFX_ByteString& orig);
CFX_ByteString PDF_EncodeString(const CFX_ByteString& src, bool bHex = false);
CFX_WideString PDF_DecodeText(const uint8_t* pData, FX_DWORD size, CFX_CharMap* pCharMap = NULL);
inline CFX_WideString PDF_DecodeText(const CFX_ByteString& bstr, CFX_CharMap* pCharMap = NULL) {
    return PDF_DecodeText((const uint8_t*)bstr.c_str(), bstr.GetLength(), pCharMap);
}
CFX_ByteString PDF_EncodeText(const FX_WCHAR* pString, int len = -1, CFX_CharMap* pCharMap = NULL);
inline CFX_ByteString PDF_EncodeText(const CFX_WideString& str, CFX_CharMap* pCharMap = NULL) {
    return PDF_EncodeText(str.c_str(), str.GetLength(), pCharMap);
}
FX_FLOAT PDF_ClipFloat(FX_FLOAT f);
class CFDF_Document : public CPDF_IndirectObjects
{
public:
    static CFDF_Document* CreateNewDoc();
    static CFDF_Document* ParseFile(IFX_FileRead *pFile, bool bOwnFile = false);
    static CFDF_Document* ParseMemory(const uint8_t* pData, FX_DWORD size);

    ~CFDF_Document();

    bool					WriteBuf(CFX_ByteTextBuf& buf) const;

    CPDF_Dictionary*		GetRoot() const
    {
        return m_pRootDict;
    }

    CFX_WideString			GetWin32Path() const;
protected:

    CFDF_Document();
    void	ParseStream(IFX_FileRead *pFile, bool bOwnFile);
    CPDF_Dictionary*		m_pRootDict;
    IFX_FileRead*			m_pFile;
    bool					m_bOwnFile;
};

CFX_WideString	FPDF_FileSpec_GetWin32Path(const CPDF_Object* pFileSpec);
void			FPDF_FileSpec_SetWin32Path(CPDF_Object* pFileSpec, const CFX_WideString& fullpath);

void FlateEncode(const uint8_t* src_buf, FX_DWORD src_size, uint8_t*& dest_buf, FX_DWORD& dest_size);
FX_DWORD FlateDecode(const uint8_t* src_buf, FX_DWORD src_size, uint8_t*& dest_buf, FX_DWORD& dest_size);
FX_DWORD RunLengthDecode(const uint8_t* src_buf, FX_DWORD src_size, uint8_t*& dest_buf, FX_DWORD& dest_size);
class CPDF_NumberTree
{
public:

    CPDF_NumberTree(CPDF_Dictionary* pRoot)
    {
        m_pRoot = pRoot;
    }

    CPDF_Object*		LookupValue(int num);
protected:

    CPDF_Dictionary*	m_pRoot;
};

class IFX_FileAvail
{
public:
    virtual ~IFX_FileAvail() { }
    virtual bool			IsDataAvail( FX_FILESIZE offset, FX_DWORD size) = 0;
};
class IFX_DownloadHints
{
public:
    virtual ~IFX_DownloadHints() { }
    virtual void			AddSegment(FX_FILESIZE offset, FX_DWORD size) = 0;
};
#define PDF_IS_LINEARIZED			1
#define PDF_NOT_LINEARIZED			0
#define PDF_UNKNOW_LINEARIZED		-1
#define PDFFORM_NOTAVAIL		0
#define PDFFORM_AVAIL			1
#define PDFFORM_NOTEXIST		2
class IPDF_DataAvail
{
public:
    static IPDF_DataAvail* Create(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead);
    virtual ~IPDF_DataAvail() { }

    IFX_FileAvail* GetFileAvail() const { return m_pFileAvail; }
    IFX_FileRead* GetFileRead() const { return m_pFileRead; }

    virtual bool			IsDocAvail(IFX_DownloadHints* pHints) = 0;
    virtual void			SetDocument(CPDF_Document* pDoc) = 0;
    virtual bool			IsPageAvail(int iPage, IFX_DownloadHints* pHints) = 0;
    virtual bool			IsLinearized() = 0;
    virtual int32_t		IsFormAvail(IFX_DownloadHints *pHints) = 0;
    virtual int32_t		IsLinearizedPDF() = 0;
    virtual void				GetLinearizedMainXRefInfo(FX_FILESIZE *pPos, FX_DWORD *pSize) = 0;

protected:
    IPDF_DataAvail(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead);

    IFX_FileAvail* m_pFileAvail;
    IFX_FileRead* m_pFileRead;
};
class CPDF_SortObjNumArray
{
public:

    void AddObjNum(FX_DWORD dwObjNum);

    bool Find(FX_DWORD dwObjNum);

    void RemoveAll()
    {
        m_number_array.RemoveAll();
    }
protected:

    bool BinarySearch(FX_DWORD value, int &iNext);
protected:

    CFX_DWordArray			m_number_array;
};
enum PDF_PAGENODE_TYPE {
    PDF_PAGENODE_UNKOWN = 0,
    PDF_PAGENODE_PAGE,
    PDF_PAGENODE_PAGES,
    PDF_PAGENODE_ARRAY,
};
class CPDF_PageNode
{
public:
    CPDF_PageNode() : m_type(PDF_PAGENODE_UNKOWN) {}
    ~CPDF_PageNode();
    PDF_PAGENODE_TYPE	m_type;
    FX_DWORD			m_dwPageNo;
    CFX_PtrArray		m_childNode;
};
enum PDF_DATAAVAIL_STATUS {
    PDF_DATAAVAIL_HEADER = 0,
    PDF_DATAAVAIL_FIRSTPAGE,
    PDF_DATAAVAIL_FIRSTPAGE_PREPARE,
    PDF_DATAAVAIL_END,
    PDF_DATAAVAIL_CROSSREF,
    PDF_DATAAVAIL_CROSSREF_ITEM,
    PDF_DATAAVAIL_CROSSREF_STREAM,
    PDF_DATAAVAIL_TRAILER,
    PDF_DATAAVAIL_LOADALLCRSOSSREF,
    PDF_DATAAVAIL_ROOT,
    PDF_DATAAVAIL_INFO,
    PDF_DATAAVAIL_ACROFORM,
    PDF_DATAAVAIL_ACROFORM_SUBOBJECT,
    PDF_DATAAVAIL_PAGETREE,
    PDF_DATAAVAIL_PAGE,
    PDF_DATAAVAIL_PAGE_LATERLOAD,
    PDF_DATAAVAIL_RESOURCES,
    PDF_DATAAVAIL_DONE,
    PDF_DATAAVAIL_ERROR,
    PDF_DATAAVAIL_LOADALLFILE,
    PDF_DATAAVAIL_TRAILER_APPEND
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_PARSER_H_

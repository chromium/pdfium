// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDF_PARSER_
#define _FPDF_PARSER_
#ifndef _FX_BASIC_H_
#include "../fxcrt/fx_ext.h"
#endif
#ifndef _FPDF_OBJECTS_
#include "fpdf_objects.h"
#endif
class CPDF_Document;
class IPDF_DocParser;
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

class IPDF_EnumPageHandler
{
public:
    virtual ~IPDF_EnumPageHandler() { }
    virtual FX_BOOL EnumPage(CPDF_Dictionary* pPageDict) = 0;
};
class CPDF_Document : public CFX_PrivateData, public CPDF_IndirectObjects
{
public:

    CPDF_Document(IPDF_DocParser* pParser);

    CPDF_Document();

    ~CPDF_Document();

    IPDF_DocParser*			GetParser() const
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

    void					EnumPages(IPDF_EnumPageHandler* pHandler);

    FX_DWORD				GetUserPermissions(FX_BOOL bCheckRevision = FALSE) const;

    FX_BOOL					IsOwner() const;



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


    FX_BOOL					IsFormStream(FX_DWORD objnum, FX_BOOL& bForm) const;




    CPDF_Font*				LoadFont(CPDF_Dictionary* pFontDict);

    CPDF_Font*				FindFont(CPDF_Dictionary* pFontDict);

    CPDF_ColorSpace*		LoadColorSpace(CPDF_Object* pCSObj, CPDF_Dictionary* pResources = NULL);

    CPDF_Pattern*			LoadPattern(CPDF_Object* pObj, FX_BOOL bShading, const CFX_AffineMatrix* matrix = NULL);

    CPDF_Image*				LoadImageF(CPDF_Object* pObj);

    CPDF_StreamAcc*			LoadFontFile(CPDF_Stream* pStream);

    CPDF_IccProfile*		LoadIccProfile(CPDF_Stream* pStream);

#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_

    CPDF_Font*				AddWindowsFont(LOGFONTA* pLogFont, FX_BOOL bVert, FX_BOOL bTranslateName = FALSE);
    CPDF_Font*				AddWindowsFont(LOGFONTW* pLogFont, FX_BOOL bVert, FX_BOOL bTranslateName = FALSE);
#endif
#if _FXM_PLATFORM_ == _FXM_PLATFORM_APPLE_
    CPDF_Font*              AddMacFont(CTFontRef pFont, FX_BOOL bVert, FX_BOOL bTranslateName = FALSE);
#endif

    CPDF_Font*				AddStandardFont(const FX_CHAR* font, CPDF_FontEncoding* pEncoding);


    CPDF_Font*				AddFont(CFX_Font* pFont, int charset, FX_BOOL bVert);

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


    FX_BOOL					m_bLinearized;

    FX_DWORD				m_dwFirstPageNo;

    FX_DWORD				m_dwFirstPageObjNum;

    CFX_DWordArray			m_PageList;

    int						_GetPageCount() const;
    CPDF_Dictionary*		_FindPDFPage(CPDF_Dictionary* pPages, int iPage, int nPagesToGo, int level);
    int						_FindPageIndex(CPDF_Dictionary* pNode, FX_DWORD& skip_count, FX_DWORD objnum, int& index, int level = 0);
    FX_BOOL					IsContentUsedElsewhere(FX_DWORD objnum, CPDF_Dictionary* pPageDict);
    FX_BOOL					CheckOCGVisible(CPDF_Dictionary* pOCG, FX_BOOL bPrinting);
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
class CPDF_SimpleParser : public CFX_Object
{
public:

    CPDF_SimpleParser(FX_LPCBYTE pData, FX_DWORD dwSize);

    CPDF_SimpleParser(FX_BSTR str);

    CFX_ByteStringC		GetWord();

    FX_BOOL				SearchToken(FX_BSTR token);

    FX_BOOL				SkipWord(FX_BSTR token);

    FX_BOOL				FindTagPair(FX_BSTR start_token, FX_BSTR end_token,
                                    FX_DWORD& start_pos, FX_DWORD& end_pos);

    FX_BOOL				FindTagParam(FX_BSTR token, int nParams);

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

    void				ParseWord(FX_LPCBYTE& pStart, FX_DWORD& dwSize, int& type);

    FX_LPCBYTE			m_pData;

    FX_DWORD			m_dwSize;

    FX_DWORD			m_dwCurPos;
};
class CPDF_SyntaxParser : public CFX_Object
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

    CPDF_Object*		GetObject(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, FX_DWORD gennum, struct PARSE_CONTEXT* pContext = NULL, FX_BOOL bDecrypt = TRUE);


    CPDF_Object*		GetObjectByStrict(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, FX_DWORD gennum, struct PARSE_CONTEXT* pContext = NULL);

    int					GetDirectNum();

    CFX_ByteString		GetString(FX_DWORD objnum, FX_DWORD gennum);

    CFX_ByteString		GetName();

    CFX_ByteString		GetKeyword();

    void				GetBinary(FX_BYTE* buffer, FX_DWORD size);

    void				ToNextLine();

    void				ToNextWord();

    FX_BOOL				SearchWord(FX_BSTR word, FX_BOOL bWholeWord, FX_BOOL bForward, FX_FILESIZE limit);

    int					SearchMultiWord(FX_BSTR words, FX_BOOL bWholeWord, FX_FILESIZE limit);

    FX_FILESIZE			FindTag(FX_BSTR tag, FX_FILESIZE limit);

    void				SetEncrypt(CPDF_CryptoHandler* pCryptoHandler)
    {
        m_pCryptoHandler = pCryptoHandler;
    }

    FX_BOOL				IsEncrypted()
    {
        return m_pCryptoHandler != NULL;
    }

    FX_BOOL				GetCharAt(FX_FILESIZE pos, FX_BYTE& ch);

    FX_BOOL				ReadBlock(FX_BYTE* pBuf, FX_DWORD size);

    CFX_ByteString		GetNextWord(FX_BOOL& bIsNumber);
protected:
    static const int kParserMaxRecursionDepth = 64;
    static int s_CurrentRecursionDepth;

    virtual FX_BOOL				GetNextChar(FX_BYTE& ch);

    FX_BOOL				GetCharAtBackward(FX_FILESIZE pos, FX_BYTE& ch);

    void				GetNextWord();

    FX_BOOL				IsWholeWord(FX_FILESIZE startpos, FX_FILESIZE limit, FX_LPCBYTE tag, FX_DWORD taglen);

    CFX_ByteString		ReadString();

    CFX_ByteString		ReadHexString();

    CPDF_Stream*		ReadStream(CPDF_Dictionary* pDict, PARSE_CONTEXT* pContext, FX_DWORD objnum, FX_DWORD gennum);

    FX_FILESIZE			m_Pos;

    FX_BOOL				m_bFileStream;

    int					m_MetadataObjnum;

    IFX_FileRead*		m_pFileAccess;

    FX_DWORD			m_HeaderOffset;

    FX_FILESIZE			m_FileLen;

    FX_BYTE*			m_pFileBuf;

    FX_DWORD			m_BufSize;

    FX_FILESIZE			m_BufOffset;

    CPDF_CryptoHandler*	m_pCryptoHandler;

    FX_BYTE				m_WordBuffer[257];

    FX_DWORD			m_WordSize;

    FX_BOOL				m_bIsNumber;

    FX_FILESIZE			m_dwWordPos;
    friend class		CPDF_Parser;
    friend class		CPDF_DataAvail;
};

#define PDFPARSE_TYPEONLY	1
#define PDFPARSE_NOSTREAM	2
struct PARSE_CONTEXT {

    FX_BOOL		m_Flags;

    FX_FILESIZE	m_DictStart;

    FX_FILESIZE	m_DictEnd;

    FX_FILESIZE	m_DataStart;

    FX_FILESIZE	m_DataEnd;
};
class IPDF_DocParser : public CFX_Object
{
public:
    virtual ~IPDF_DocParser() { }
    virtual FX_DWORD	GetRootObjNum() = 0;

    virtual FX_DWORD	GetInfoObjNum() = 0;

    virtual CPDF_Object*	ParseIndirectObject(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, PARSE_CONTEXT* pContext = NULL) = 0;

    virtual FX_DWORD	GetLastObjNum() = 0;

    virtual CPDF_Array*	GetIDArray() = 0;

    virtual CPDF_Dictionary*	GetEncryptDict() = 0;

    FX_BOOL				IsEncrypted()
    {
        return GetEncryptDict() != NULL;
    }

    virtual FX_DWORD	GetPermissions(FX_BOOL bCheckRevision = FALSE) = 0;

    virtual FX_BOOL		IsOwner() = 0;

    virtual FX_BOOL		IsFormStream(FX_DWORD objnum, FX_BOOL& bForm) = 0;
};

#define PDFPARSE_ERROR_SUCCESS		0
#define PDFPARSE_ERROR_FILE			1
#define PDFPARSE_ERROR_FORMAT		2
#define PDFPARSE_ERROR_PASSWORD		3
#define PDFPARSE_ERROR_HANDLER		4
#define PDFPARSE_ERROR_CERT			5
class CPDF_Parser FX_FINAL : public IPDF_DocParser
{
public:

    CPDF_Parser();
    ~CPDF_Parser() override;

    FX_DWORD			StartParse(FX_LPCSTR filename, FX_BOOL bReParse = FALSE);

    FX_DWORD			StartParse(FX_LPCWSTR filename, FX_BOOL bReParse = FALSE);

    FX_DWORD			StartParse(IFX_FileRead* pFile, FX_BOOL bReParse = FALSE, FX_BOOL bOwnFileRead = TRUE);

    void				CloseParser(FX_BOOL bReParse = FALSE);

    virtual FX_DWORD	GetPermissions(FX_BOOL bCheckRevision = FALSE) FX_OVERRIDE;

    virtual FX_BOOL		IsOwner() FX_OVERRIDE;

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

    void				SetSecurityHandler(CPDF_SecurityHandler* pSecurityHandler, FX_BOOL bForced = FALSE);

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
    CFX_ArrayTemplate<CPDF_Dictionary *> * GetOtherTrailers()
    {
        return &m_Trailers;
    }

    virtual FX_DWORD	GetRootObjNum() FX_OVERRIDE;
    virtual FX_DWORD	GetInfoObjNum()  FX_OVERRIDE;
    virtual CPDF_Array*	GetIDArray()  FX_OVERRIDE;
    virtual CPDF_Dictionary*	GetEncryptDict()  FX_OVERRIDE
    {
        return m_pEncryptDict;
    }
    virtual CPDF_Object*		ParseIndirectObject(CPDF_IndirectObjects* pObjList, FX_DWORD objnum, PARSE_CONTEXT* pContext = NULL)  FX_OVERRIDE;
    virtual FX_DWORD	GetLastObjNum() FX_OVERRIDE;
    virtual FX_BOOL		IsFormStream(FX_DWORD objnum, FX_BOOL& bForm)  FX_OVERRIDE;

    FX_FILESIZE			GetObjectOffset(FX_DWORD objnum);

    FX_FILESIZE			GetObjectSize(FX_DWORD objnum);

    int					GetObjectVersion(FX_DWORD objnum)
    {
        return m_ObjVersion[objnum];
    }

    void				GetIndirectBinary(FX_DWORD objnum, FX_BYTE*& pBuffer, FX_DWORD& size);

    FX_BOOL				GetFileStreamOption()
    {
        return m_Syntax.m_bFileStream;
    }

    void				SetFileStreamOption(FX_BOOL b)
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

    FX_BOOL				IsXRefStream() const
    {
        return m_bXRefStream;
    }
    CPDF_Object*		ParseIndirectObjectAt(CPDF_IndirectObjects* pObjList, FX_FILESIZE pos, FX_DWORD objnum,
            struct PARSE_CONTEXT* pContext);

    CPDF_Object*		ParseIndirectObjectAtByStrict(CPDF_IndirectObjects* pObjList, FX_FILESIZE pos, FX_DWORD objnum,
            struct PARSE_CONTEXT* pContext, FX_FILESIZE *pResultPos);

    FX_DWORD			StartAsynParse(IFX_FileRead* pFile, FX_BOOL bReParse = FALSE, FX_BOOL bOwnFileRead = TRUE);

    FX_DWORD			GetFirstPageNo()
    {
        return m_dwFirstPageNo;
    }
protected:
    CPDF_Document*		m_pDocument;

    CPDF_SyntaxParser	m_Syntax;
    FX_BOOL				m_bOwnFileRead;
    CPDF_Object*		ParseDirect(CPDF_Object* pObj);

    FX_BOOL				LoadAllCrossRefV4(FX_FILESIZE pos);

    FX_BOOL				LoadAllCrossRefV5(FX_FILESIZE pos);

    FX_BOOL				LoadCrossRefV4(FX_FILESIZE pos, FX_FILESIZE streampos, FX_BOOL bSkip, FX_BOOL bFirst);

    FX_BOOL				LoadCrossRefV5(FX_FILESIZE pos, FX_FILESIZE& prev, FX_BOOL bMainXRef);

    CPDF_Dictionary*	LoadTrailerV4();

    FX_BOOL				RebuildCrossRef();

    FX_DWORD			SetEncryptHandler();

    void				ReleaseEncryptHandler();

    FX_BOOL				LoadLinearizedAllCrossRefV4(FX_FILESIZE pos, FX_DWORD dwObjCount);

    FX_BOOL				LoadLinearizedCrossRefV4(FX_FILESIZE pos, FX_DWORD dwObjCount);

    FX_BOOL				LoadLinearizedAllCrossRefV5(FX_FILESIZE pos);

    FX_DWORD			LoadLinearizedMainXRefTable();

    CFX_MapPtrToPtr		m_ObjectStreamMap;

    CPDF_StreamAcc*		GetObjectStream(FX_DWORD number);

    FX_BOOL				IsLinearizedFile(IFX_FileRead* pFileAccess, FX_DWORD offset);



    int					m_FileVersion;

    CPDF_Dictionary*	m_pTrailer;

    CPDF_Dictionary*	m_pEncryptDict;
    void SetEncryptDictionary(CPDF_Dictionary* pDict);

    FX_FILESIZE			m_LastXRefOffset;

    FX_BOOL				m_bXRefStream;


    CPDF_SecurityHandler*	m_pSecurityHandler;

    FX_BOOL					m_bForceUseSecurityHandler;

    CFX_ByteString			m_bsRecipient;

    CFX_ByteString		m_FilePath;

    CFX_ByteString		m_Password;

    CFX_FileSizeArray	m_CrossRef;

    CFX_ByteArray		m_V5Type;

    CFX_FileSizeArray	m_SortedOffset;

    CFX_WordArray		m_ObjVersion;
    CFX_ArrayTemplate<CPDF_Dictionary *>	m_Trailers;

    FX_BOOL				m_bVersionUpdated;

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
class CPDF_SecurityHandler : public CFX_Object
{
public:

    virtual ~CPDF_SecurityHandler() {}

    virtual FX_BOOL		OnInit(CPDF_Parser* pParser, CPDF_Dictionary* pEncryptDict) = 0;

    virtual FX_DWORD	GetPermissions() = 0;

    virtual FX_BOOL		IsOwner() = 0;

    virtual FX_BOOL		GetCryptInfo(int& cipher, FX_LPCBYTE& buffer, int& keylen) = 0;

    virtual FX_BOOL		IsMetadataEncrypted()
    {
        return TRUE;
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
    virtual FX_BOOL		OnInit(CPDF_Parser* pParser, CPDF_Dictionary* pEncryptDict);
    virtual FX_DWORD	GetPermissions();
    virtual FX_BOOL		IsOwner()
    {
        return m_bOwner;
    }
    virtual FX_BOOL		GetCryptInfo(int& cipher, FX_LPCBYTE& buffer, int& keylen);
    virtual FX_BOOL		IsMetadataEncrypted();
    virtual CPDF_CryptoHandler*	CreateCryptoHandler();
    virtual CPDF_StandardSecurityHandler* GetStandardHandler()
    {
        return this;
    }

    void				OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray,
                                 FX_LPCBYTE user_pass, FX_DWORD user_size,
                                 FX_LPCBYTE owner_pass, FX_DWORD owner_size, FX_DWORD type = PDF_ENCRYPT_CONTENT);

    void				OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray,
                                 FX_LPCBYTE user_pass, FX_DWORD user_size, FX_DWORD type = PDF_ENCRYPT_CONTENT);

    CFX_ByteString		GetUserPassword(FX_LPCBYTE owner_pass, FX_DWORD pass_size);
    CFX_ByteString		GetUserPassword(FX_LPCBYTE owner_pass, FX_DWORD pass_size, FX_INT32 key_len);
    int					GetVersion()
    {
        return m_Version;
    }
    int					GetRevision()
    {
        return m_Revision;
    }

    int					CheckPassword(FX_LPCBYTE password, FX_DWORD pass_size, FX_BOOL bOwner, FX_LPBYTE key);
    int					CheckPassword(FX_LPCBYTE password, FX_DWORD pass_size, FX_BOOL bOwner, FX_LPBYTE key, int key_len);
private:

    int					m_Version;

    int					m_Revision;

    CPDF_Parser*		m_pParser;

    CPDF_Dictionary*	m_pEncryptDict;

    FX_BOOL				LoadDict(CPDF_Dictionary* pEncryptDict);
    FX_BOOL				LoadDict(CPDF_Dictionary* pEncryptDict, FX_DWORD type, int& cipher, int& key_len);

    FX_BOOL				CheckUserPassword(FX_LPCBYTE password, FX_DWORD pass_size,
                                          FX_BOOL bIgnoreEncryptMeta, FX_LPBYTE key, FX_INT32 key_len);

    FX_BOOL				CheckOwnerPassword(FX_LPCBYTE password, FX_DWORD pass_size, FX_LPBYTE key, FX_INT32 key_len);
    FX_BOOL				AES256_CheckPassword(FX_LPCBYTE password, FX_DWORD size, FX_BOOL bOwner, FX_LPBYTE key);
    void				AES256_SetPassword(CPDF_Dictionary* pEncryptDict, FX_LPCBYTE password, FX_DWORD size, FX_BOOL bOwner, FX_LPCBYTE key);
    void				AES256_SetPerms(CPDF_Dictionary* pEncryptDict, FX_DWORD permission, FX_BOOL bEncryptMetadata, FX_LPCBYTE key);
    void				OnCreate(CPDF_Dictionary* pEncryptDict, CPDF_Array* pIdArray,
                                 FX_LPCBYTE user_pass, FX_DWORD user_size,
                                 FX_LPCBYTE owner_pass, FX_DWORD owner_size, FX_BOOL bDefault, FX_DWORD type);
    FX_BOOL				CheckSecurity(FX_INT32 key_len);

    FX_BOOL				m_bOwner;

    FX_DWORD			m_Permissions;

    int					m_Cipher;

    FX_BYTE				m_EncryptKey[32];

    int					m_KeyLen;
};
class CPDF_CryptoHandler : public CFX_Object
{
public:

    virtual ~CPDF_CryptoHandler() {}

    virtual FX_BOOL		Init(CPDF_Dictionary* pEncryptDict, CPDF_SecurityHandler* pSecurityHandler) = 0;

    virtual FX_DWORD	DecryptGetSize(FX_DWORD src_size) = 0;

    virtual FX_LPVOID	DecryptStart(FX_DWORD objnum, FX_DWORD gennum) = 0;

    virtual FX_BOOL		DecryptStream(FX_LPVOID context, FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf) = 0;

    virtual FX_BOOL		DecryptFinish(FX_LPVOID context, CFX_BinaryBuf& dest_buf) = 0;


    virtual FX_DWORD	EncryptGetSize(FX_DWORD objnum, FX_DWORD version, FX_LPCBYTE src_buf, FX_DWORD src_size) = 0;

    virtual FX_BOOL		EncryptContent(FX_DWORD objnum, FX_DWORD version, FX_LPCBYTE src_buf, FX_DWORD src_size,
                                       FX_LPBYTE dest_buf, FX_DWORD& dest_size) = 0;

    void				Decrypt(FX_DWORD objnum, FX_DWORD version, CFX_ByteString& str);
};
class CPDF_StandardCryptoHandler : public CPDF_CryptoHandler
{
public:

    CPDF_StandardCryptoHandler();

    virtual ~CPDF_StandardCryptoHandler();

    FX_BOOL				Init(int cipher, FX_LPCBYTE key, int keylen);
    virtual FX_BOOL		Init(CPDF_Dictionary* pEncryptDict, CPDF_SecurityHandler* pSecurityHandler);
    virtual FX_DWORD	DecryptGetSize(FX_DWORD src_size);
    virtual FX_LPVOID	DecryptStart(FX_DWORD objnum, FX_DWORD gennum);
    virtual FX_BOOL		DecryptStream(FX_LPVOID context, FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf);
    virtual FX_BOOL		DecryptFinish(FX_LPVOID context, CFX_BinaryBuf& dest_buf);
    virtual FX_DWORD	EncryptGetSize(FX_DWORD objnum, FX_DWORD version, FX_LPCBYTE src_buf, FX_DWORD src_size);
    virtual FX_BOOL		EncryptContent(FX_DWORD objnum, FX_DWORD version, FX_LPCBYTE src_buf, FX_DWORD src_size,
                                       FX_LPBYTE dest_buf, FX_DWORD& dest_size);
protected:

    virtual void		CryptBlock(FX_BOOL bEncrypt, FX_DWORD objnum, FX_DWORD gennum, FX_LPCBYTE src_buf, FX_DWORD src_size,
                                   FX_LPBYTE dest_buf, FX_DWORD& dest_size);
    virtual FX_LPVOID	CryptStart(FX_DWORD objnum, FX_DWORD gennum, FX_BOOL bEncrypt);
    virtual FX_BOOL		CryptStream(FX_LPVOID context, FX_LPCBYTE src_buf, FX_DWORD src_size, CFX_BinaryBuf& dest_buf, FX_BOOL bEncrypt);
    virtual FX_BOOL		CryptFinish(FX_LPVOID context, CFX_BinaryBuf& dest_buf, FX_BOOL bEncrypt);

    FX_BYTE				m_EncryptKey[32];

    int					m_KeyLen;

    int					m_Cipher;

    FX_LPBYTE			m_pAESContext;
};
class CPDF_Point : public CFX_Object
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
CFX_ByteString PDF_NameDecode(FX_BSTR orig);
CFX_ByteString PDF_NameDecode(const CFX_ByteString& orig);
CFX_ByteString PDF_NameEncode(const CFX_ByteString& orig);
CFX_ByteString PDF_EncodeString(const CFX_ByteString& src, FX_BOOL bHex = FALSE);
CFX_WideString PDF_DecodeText(const CFX_ByteString& str, CFX_CharMap* pCharMap = NULL);
CFX_WideString PDF_DecodeText(FX_LPCBYTE pData, FX_DWORD size, CFX_CharMap* pCharMap = NULL);
CFX_ByteString PDF_EncodeText(FX_LPCWSTR pString, int len = -1, CFX_CharMap* pCharMap = NULL);
FX_FLOAT PDF_ClipFloat(FX_FLOAT f);
class CFDF_Document : public CPDF_IndirectObjects
{
public:
    static CFDF_Document* CreateNewDoc();
    static CFDF_Document* ParseFile(IFX_FileRead *pFile, FX_BOOL bOwnFile = FALSE);
    static CFDF_Document* ParseMemory(FX_LPCBYTE pData, FX_DWORD size);

    ~CFDF_Document();

    FX_BOOL					WriteBuf(CFX_ByteTextBuf& buf) const;

    CPDF_Dictionary*		GetRoot() const
    {
        return m_pRootDict;
    }

    CFX_WideString			GetWin32Path() const;
protected:

    CFDF_Document();
    void	ParseStream(IFX_FileRead *pFile, FX_BOOL bOwnFile);
    CPDF_Dictionary*		m_pRootDict;
    IFX_FileRead*			m_pFile;
    FX_BOOL					m_bOwnFile;
};

CFX_WideString	FPDF_FileSpec_GetWin32Path(const CPDF_Object* pFileSpec);
void			FPDF_FileSpec_SetWin32Path(CPDF_Object* pFileSpec, const CFX_WideString& fullpath);

void FlateEncode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
FX_DWORD FlateDecode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
FX_DWORD RunLengthDecode(const FX_BYTE* src_buf, FX_DWORD src_size, FX_LPBYTE& dest_buf, FX_DWORD& dest_size);
class CPDF_NumberTree : public CFX_Object
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
    virtual FX_BOOL			IsDataAvail( FX_FILESIZE offset, FX_DWORD size) = 0;
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

    virtual FX_BOOL			IsDocAvail(IFX_DownloadHints* pHints) = 0;
    virtual void			SetDocument(CPDF_Document* pDoc) = 0;
    virtual FX_BOOL			IsPageAvail(int iPage, IFX_DownloadHints* pHints) = 0;
    virtual FX_BOOL			IsLinearized() = 0;
    virtual FX_INT32		IsFormAvail(IFX_DownloadHints *pHints) = 0;
    virtual FX_INT32		IsLinearizedPDF() = 0;
    virtual void				GetLinearizedMainXRefInfo(FX_FILESIZE *pPos, FX_DWORD *pSize) = 0;

protected:
    IPDF_DataAvail(IFX_FileAvail* pFileAvail, IFX_FileRead* pFileRead);

    IFX_FileAvail* m_pFileAvail;
    IFX_FileRead* m_pFileRead;
};
class CPDF_SortObjNumArray : public CFX_Object
{
public:

    void AddObjNum(FX_DWORD dwObjNum);

    FX_BOOL Find(FX_DWORD dwObjNum);

    void RemoveAll()
    {
        m_number_array.RemoveAll();
    }
protected:

    FX_BOOL BinarySearch(FX_DWORD value, int &iNext);
protected:

    CFX_DWordArray			m_number_array;
};
enum PDF_PAGENODE_TYPE {
    PDF_PAGENODE_UNKOWN = 0,
    PDF_PAGENODE_PAGE,
    PDF_PAGENODE_PAGES,
    PDF_PAGENODE_ARRAY,
};
class CPDF_PageNode : public CFX_Object
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
#endif

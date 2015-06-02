// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_OBJECTS_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_OBJECTS_H_

#include "../fxcrt/fx_ext.h"

class CPDF_Array;
class CPDF_Boolean;
class CPDF_CryptoHandler;
class CPDF_Dictionary;
class CPDF_Document;
class CPDF_IndirectObjects;
class CPDF_Null;
class CPDF_Number;
class CPDF_Parser;
class CPDF_Reference;
class CPDF_Stream;
class CPDF_StreamAcc;
class CPDF_StreamFilter;
class CPDF_String;
class IFX_FileRead;

#define PDFOBJ_INVALID		0
#define	PDFOBJ_BOOLEAN		1
#define PDFOBJ_NUMBER		2
#define PDFOBJ_STRING		3
#define PDFOBJ_NAME			4
#define PDFOBJ_ARRAY		5
#define PDFOBJ_DICTIONARY	6
#define PDFOBJ_STREAM		7
#define PDFOBJ_NULL			8
#define PDFOBJ_REFERENCE	9

typedef IFX_FileStream* (*FPDF_LPFCloneStreamCallback)(CPDF_Stream *pStream, FX_LPVOID pUserData);
class CPDF_Object 
{
public:

    int                                 GetType() const
    {
        return m_Type;
    }

    FX_DWORD                            GetObjNum() const
    {
        return m_ObjNum;
    }

    FX_DWORD                            GetGenNum() const
    {
        return m_GenNum;
    }

    FX_BOOL                             IsIdentical(CPDF_Object* pObj) const;

    CPDF_Object*                        Clone(FX_BOOL bDirect = FALSE) const;

    CPDF_Object*                        CloneRef(CPDF_IndirectObjects* pObjs) const;

    CPDF_Object*                        GetDirect() const;

    void                                Release();

    CFX_ByteString                      GetString() const;

    CFX_ByteStringC                     GetConstString() const;

    CFX_WideString                      GetUnicodeText(CFX_CharMap* pCharMap = NULL) const; 
    FX_FLOAT                            GetNumber() const;

    FX_FLOAT                            GetNumber16() const;

    int                                 GetInteger() const;

    CPDF_Dictionary*                    GetDict() const;

    CPDF_Array*                         GetArray() const;

    void                                SetString(const CFX_ByteString& str);

    void                                SetUnicodeText(FX_LPCWSTR pUnicodes, int len = -1);

    int                                 GetDirectType() const;

    FX_BOOL                             IsModified() const
    {
        return FALSE;
    }
protected:
    CPDF_Object(FX_DWORD type) : m_Type(type), m_ObjNum(0), m_GenNum(0) { }
    ~CPDF_Object() { }
    void                                Destroy();

    static const int                    OBJECT_REF_MAX_DEPTH = 128;
    static int                          s_nCurRefDepth;
    FX_DWORD                            m_Type;
    FX_DWORD                            m_ObjNum;
    FX_DWORD                            m_GenNum;

    friend class			CPDF_IndirectObjects;
    friend class			CPDF_Parser;
    friend class			CPDF_SyntaxParser;
private:
    CPDF_Object(const CPDF_Object& src) {}
    CPDF_Object* CloneInternal(FX_BOOL bDirect, CFX_MapPtrToPtr* visited) const;
};
class CPDF_Boolean : public CPDF_Object
{
public:

    static CPDF_Boolean*	Create(FX_BOOL value)
    {
        return new CPDF_Boolean(value);
    }

    CPDF_Boolean() : CPDF_Object(PDFOBJ_BOOLEAN), m_bValue(false) { }
    CPDF_Boolean(FX_BOOL value) : CPDF_Object(PDFOBJ_BOOLEAN), m_bValue(value) { }

    FX_BOOL					Identical(CPDF_Boolean* pOther) const
    {
        return m_bValue == pOther->m_bValue;
    }
protected:

    FX_BOOL					m_bValue;
    friend class			CPDF_Object;
};
class CPDF_Number : public CPDF_Object
{
public:

    static CPDF_Number*		Create(int value)
    {
        return new CPDF_Number(value);
    }

    static CPDF_Number*		Create(FX_FLOAT value)
    {
        return new CPDF_Number(value);
    }

    static CPDF_Number*		Create(FX_BSTR str)
    {
        return new CPDF_Number(str);
    }

    static CPDF_Number*		Create(FX_BOOL bInteger, void* pData)
    {
        return new CPDF_Number(bInteger, pData);
    }

    CPDF_Number() : CPDF_Object(PDFOBJ_NUMBER), m_bInteger(false), m_Integer(0) { }

    CPDF_Number(FX_BOOL bInteger, void* pData);

    CPDF_Number(int value);

    CPDF_Number(FX_FLOAT value);

    CPDF_Number(FX_BSTR str);

    FX_BOOL					Identical(CPDF_Number* pOther) const;

    CFX_ByteString			GetString() const;

    void					SetString(FX_BSTR str);

    FX_BOOL					IsInteger() const
    {
        return m_bInteger;
    }

    int						GetInteger() const
    {
        return m_bInteger ? m_Integer : (int)m_Float;
    }

    FX_FLOAT				GetNumber() const
    {
        return m_bInteger ? (FX_FLOAT)m_Integer : m_Float;
    }

    void					SetNumber(FX_FLOAT value);

    FX_FLOAT			GetNumber16() const
    {
        return GetNumber();
    }

    FX_FLOAT				GetFloat() const
    {
        return m_bInteger ? (FX_FLOAT)m_Integer : m_Float;
    }
protected:

    FX_BOOL					m_bInteger;

    union {

        int					m_Integer;

        FX_FLOAT			m_Float;
    };
    friend class			CPDF_Object;
};
class CPDF_String : public CPDF_Object
{
public:

    static CPDF_String*		Create(const CFX_ByteString& str, FX_BOOL bHex = FALSE)
    {
        return new CPDF_String(str, bHex);
    }

    static CPDF_String*		Create(const CFX_WideString& str)
    {
        return new CPDF_String(str);
    }

    CPDF_String() : CPDF_Object(PDFOBJ_STRING), m_bHex(FALSE) { }

    CPDF_String(const CFX_ByteString& str, FX_BOOL bHex = FALSE)
        : CPDF_Object(PDFOBJ_STRING), m_String(str), m_bHex(bHex) {
    }

    CPDF_String(const CFX_WideString& str);

    CFX_ByteString&			GetString()
    {
        return m_String;
    }

    FX_BOOL					Identical(CPDF_String* pOther) const
    {
        return m_String == pOther->m_String;
    }

    FX_BOOL					IsHex() const
    {
        return m_bHex;
    }
protected:

    CFX_ByteString			m_String;

    FX_BOOL					m_bHex;
    friend class			CPDF_Object;
};
class CPDF_Name : public CPDF_Object
{
public:

    static CPDF_Name*		Create(const CFX_ByteString& str)
    {
        return new CPDF_Name(str);
    }

    static CPDF_Name*		Create(FX_BSTR str)
    {
        return new CPDF_Name(str);
    }

    static CPDF_Name*		Create(FX_LPCSTR str)
    {
        return new CPDF_Name(str);
    }

    CPDF_Name(const CFX_ByteString& str) : CPDF_Object(PDFOBJ_NAME), m_Name(str) { }
    CPDF_Name(FX_BSTR str) : CPDF_Object(PDFOBJ_NAME), m_Name(str) { }
    CPDF_Name(FX_LPCSTR str) : CPDF_Object(PDFOBJ_NAME), m_Name(str) { }

    CFX_ByteString&			GetString()
    {
        return m_Name;
    }

    FX_BOOL					Identical(CPDF_Name* pOther) const
    {
        return m_Name == pOther->m_Name;
    }
protected:

    CFX_ByteString			m_Name;
    friend class			CPDF_Object;
};
class CPDF_Array : public CPDF_Object
{
public:

    static CPDF_Array*		Create()
    {
        return new CPDF_Array();
    }

    CPDF_Array() : CPDF_Object(PDFOBJ_ARRAY) { }

    FX_DWORD				GetCount() const
    {
        return m_Objects.GetSize();
    }

    CPDF_Object*			GetElement(FX_DWORD index) const;

    CPDF_Object*			GetElementValue(FX_DWORD index) const;



    CFX_AffineMatrix		GetMatrix();

    CFX_FloatRect			GetRect();




    CFX_ByteString			GetString(FX_DWORD index) const;

    CFX_ByteStringC			GetConstString(FX_DWORD index) const;

    int						GetInteger(FX_DWORD index) const;

    FX_FLOAT				GetNumber(FX_DWORD index) const;

    CPDF_Dictionary*		GetDict(FX_DWORD index) const;

    CPDF_Stream*			GetStream(FX_DWORD index) const;

    CPDF_Array*				GetArray(FX_DWORD index) const;

    FX_FLOAT				GetFloat(FX_DWORD index) const
    {
        return GetNumber(index);
    }




    void					SetAt(FX_DWORD index, CPDF_Object* pObj, CPDF_IndirectObjects* pObjs = NULL);


    void					InsertAt(FX_DWORD index, CPDF_Object* pObj, CPDF_IndirectObjects* pObjs = NULL);

    void					RemoveAt(FX_DWORD index);


    void					Add(CPDF_Object* pObj, CPDF_IndirectObjects* pObjs = NULL);



    void					AddNumber(FX_FLOAT f);

    void					AddInteger(int i);

    void					AddString(const CFX_ByteString& str);

    void					AddName(const CFX_ByteString& str);

    void					AddReference(CPDF_IndirectObjects* pDoc, FX_DWORD objnum);

    void					AddReference(CPDF_IndirectObjects* pDoc, CPDF_Object* obj)
    {
        AddReference(pDoc, obj->GetObjNum());
    }


    FX_FLOAT			GetNumber16(FX_DWORD index) const
    {
        return GetNumber(index);
    }

    void					AddNumber16(FX_FLOAT value)
    {
        AddNumber(value);
    }

    FX_BOOL					Identical(CPDF_Array* pOther) const;
protected:

    ~CPDF_Array();

    CFX_PtrArray			m_Objects;
    friend class			CPDF_Object;
};
class CPDF_Dictionary : public CPDF_Object
{
public:

    static CPDF_Dictionary*	Create()
    {
        return new CPDF_Dictionary();
    }

    CPDF_Dictionary() : CPDF_Object(PDFOBJ_DICTIONARY) { }

    CPDF_Object*			GetElement(FX_BSTR key) const;

    CPDF_Object*			GetElementValue(FX_BSTR key) const;





    CFX_ByteString			GetString(FX_BSTR key) const;

    CFX_ByteStringC			GetConstString(FX_BSTR key) const;

    CFX_ByteString			GetString(FX_BSTR key, FX_BSTR default_str) const;

    CFX_ByteStringC			GetConstString(FX_BSTR key, FX_BSTR default_str) const;

    CFX_WideString			GetUnicodeText(FX_BSTR key, CFX_CharMap* pCharMap = NULL) const;

    int						GetInteger(FX_BSTR key) const;

    int						GetInteger(FX_BSTR key, int default_int) const;

    FX_BOOL					GetBoolean(FX_BSTR key, FX_BOOL bDefault = FALSE) const;

    FX_FLOAT				GetNumber(FX_BSTR key) const;

    CPDF_Dictionary*		GetDict(FX_BSTR key) const;

    CPDF_Stream*			GetStream(FX_BSTR key) const;

    CPDF_Array*				GetArray(FX_BSTR key) const;

    CFX_FloatRect			GetRect(FX_BSTR key) const;

    CFX_AffineMatrix		GetMatrix(FX_BSTR key) const;

    FX_FLOAT				GetFloat(FX_BSTR key) const
    {
        return GetNumber(key);
    }


    FX_BOOL					KeyExist(FX_BSTR key) const;

    FX_POSITION				GetStartPos() const;

    CPDF_Object*			GetNextElement(FX_POSITION& pos, CFX_ByteString& key) const;

    void					SetAt(FX_BSTR key, CPDF_Object* pObj, CPDF_IndirectObjects* pObjs = NULL);



    void					SetAtName(FX_BSTR key, const CFX_ByteString& name);


    void					SetAtString(FX_BSTR key, const CFX_ByteString& string);


    void					SetAtInteger(FX_BSTR key, int i);


    void					SetAtNumber(FX_BSTR key, FX_FLOAT f);

    void					SetAtReference(FX_BSTR key, CPDF_IndirectObjects* pDoc, FX_DWORD objnum);

    void					SetAtReference(FX_BSTR key, CPDF_IndirectObjects* pDoc, CPDF_Object* obj)
    {
        SetAtReference(key, pDoc, obj->GetObjNum());
    }

    void					AddReference(FX_BSTR key, CPDF_IndirectObjects* pDoc, FX_DWORD objnum);

    void					AddReference(FX_BSTR key, CPDF_IndirectObjects* pDoc, CPDF_Object* obj)
    {
        AddReference(key, pDoc, obj->GetObjNum());
    }

    void					SetAtRect(FX_BSTR key, const CFX_FloatRect& rect);

    void					SetAtMatrix(FX_BSTR key, const CFX_AffineMatrix& matrix);

    void					SetAtBoolean(FX_BSTR key, FX_BOOL bValue);



    void					RemoveAt(FX_BSTR key);


    void					ReplaceKey(FX_BSTR oldkey, FX_BSTR newkey);

    FX_BOOL					Identical(CPDF_Dictionary* pDict) const;

    int						GetCount() const
    {
        return m_Map.GetCount();
    }

    void					AddValue(FX_BSTR key, CPDF_Object* pObj);
protected:

    ~CPDF_Dictionary();

    CFX_CMapByteStringToPtr	m_Map;

    friend class			CPDF_Object;
};
class CPDF_Stream : public CPDF_Object
{
public:

    static CPDF_Stream*		Create(FX_LPBYTE pData, FX_DWORD size, CPDF_Dictionary* pDict)
    {
        return new CPDF_Stream(pData, size, pDict);
    }

    CPDF_Stream(FX_LPBYTE pData, FX_DWORD size, CPDF_Dictionary* pDict);

    CPDF_Dictionary*		GetDict() const
    {
        return m_pDict;
    }

    void					SetData(FX_LPCBYTE pData, FX_DWORD size, FX_BOOL bCompressed, FX_BOOL bKeepBuf);

    void					InitStream(FX_BYTE* pData, FX_DWORD size, CPDF_Dictionary* pDict);

    void					InitStream(IFX_FileRead *pFile, CPDF_Dictionary* pDict);

    FX_BOOL					Identical(CPDF_Stream* pOther) const;

    CPDF_StreamFilter*		GetStreamFilter(FX_BOOL bRaw = FALSE) const;



    FX_DWORD				GetRawSize() const
    {
        return m_dwSize;
    }

    FX_BOOL					ReadRawData(FX_FILESIZE start_pos, FX_LPBYTE pBuf, FX_DWORD buf_size) const;


    FX_BOOL					IsMemoryBased() const
    {
        return m_GenNum == (FX_DWORD) - 1;
    }

    CPDF_Stream*			Clone(FX_BOOL bDirect, FPDF_LPFCloneStreamCallback lpfCallback, FX_LPVOID pUserData) const;
protected:

    ~CPDF_Stream();

    CPDF_Dictionary*		m_pDict;

    FX_DWORD				m_dwSize;

    FX_DWORD				m_GenNum;

    union {

        FX_LPBYTE			m_pDataBuf;

        IFX_FileRead*		m_pFile;
    };

    FX_FILESIZE				m_FileOffset;

    CPDF_CryptoHandler*		m_pCryptoHandler;

    void					InitStream(CPDF_Dictionary* pDict);
    friend class			CPDF_Object;
    friend class			CPDF_StreamAcc;
    friend class			CPDF_AttachmentAcc;
};
class CPDF_StreamAcc 
{
public:

    CPDF_StreamAcc();

    ~CPDF_StreamAcc();

    void					LoadAllData(const CPDF_Stream* pStream, FX_BOOL bRawAccess = FALSE,
                                        FX_DWORD estimated_size = 0, FX_BOOL bImageAcc = FALSE);

    const CPDF_Stream*		GetStream() const
    {
        return m_pStream;
    }

    CPDF_Dictionary*		GetDict() const
    {
        return m_pStream? m_pStream->GetDict() : NULL;
    }

    FX_LPCBYTE				GetData() const;

    FX_DWORD				GetSize() const;

    FX_LPBYTE				DetachData();

    const CFX_ByteString&	GetImageDecoder()
    {
        return m_ImageDecoder;
    }

    const CPDF_Dictionary*	GetImageParam()
    {
        return m_pImageParam;
    }
protected:

    FX_LPBYTE				m_pData;

    FX_DWORD				m_dwSize;

    FX_BOOL					m_bNewBuf;

    CFX_ByteString			m_ImageDecoder;

    CPDF_Dictionary*		m_pImageParam;

    const CPDF_Stream*		m_pStream;

    FX_LPBYTE				m_pSrcData;
};
CFX_DataFilter* FPDF_CreateFilter(FX_BSTR name, const CPDF_Dictionary* pParam, int width = 0, int height = 0);
#define FPDF_FILTER_BUFFER_SIZE		20480
class CPDF_StreamFilter 
{
public:

    ~CPDF_StreamFilter();

    FX_DWORD			ReadBlock(FX_LPBYTE buffer, FX_DWORD size);

    FX_DWORD			GetSrcPos()
    {
        return m_SrcOffset;
    }

    const CPDF_Stream*	GetStream()
    {
        return m_pStream;
    }
protected:

    CPDF_StreamFilter() {}

    FX_DWORD			ReadLeftOver(FX_LPBYTE buffer, FX_DWORD buf_size);

    const CPDF_Stream*	m_pStream;

    CFX_DataFilter*		m_pFilter;

    CFX_BinaryBuf*		m_pBuffer;

    FX_DWORD			m_BufOffset;

    FX_DWORD			m_SrcOffset;

    FX_BYTE				m_SrcBuffer[FPDF_FILTER_BUFFER_SIZE];
    friend class CPDF_Stream;
};
class CPDF_Null : public CPDF_Object
{
public:

    static CPDF_Null*		Create()
    {
        return new CPDF_Null();
    }

    CPDF_Null() : CPDF_Object(PDFOBJ_NULL) { }
};
class CPDF_Reference : public CPDF_Object
{
public:

    static CPDF_Reference*	Create(CPDF_IndirectObjects* pDoc, int objnum)
    {
        return new CPDF_Reference(pDoc, objnum);
    }

    CPDF_Reference(CPDF_IndirectObjects* pDoc, int objnum)
        : CPDF_Object(PDFOBJ_REFERENCE), m_pObjList(pDoc), m_RefObjNum(objnum) {
    }

    CPDF_IndirectObjects*	GetObjList() const
    {
        return m_pObjList;
    }

    FX_DWORD				GetRefObjNum() const
    {
        return m_RefObjNum;
    }

    void					SetRef(CPDF_IndirectObjects* pDoc, FX_DWORD objnum);

    FX_BOOL					Identical(CPDF_Reference* pOther) const
    {
        return m_RefObjNum == pOther->m_RefObjNum;
    }
protected:

    CPDF_IndirectObjects*	m_pObjList;

    FX_DWORD				m_RefObjNum;
    friend class			CPDF_Object;
};
class CPDF_IndirectObjects 
{
public:

    CPDF_IndirectObjects(CPDF_Parser* pParser);

    ~CPDF_IndirectObjects();

    CPDF_Object*			GetIndirectObject(FX_DWORD objnum, struct PARSE_CONTEXT* pContext = NULL);

    int						GetIndirectType(FX_DWORD objnum);

    FX_DWORD				AddIndirectObject(CPDF_Object* pObj);

    void					ReleaseIndirectObject(FX_DWORD objnum);

    void					InsertIndirectObject(FX_DWORD objnum, CPDF_Object* pObj);

    FX_DWORD				GetLastObjNum() const;

    FX_POSITION				GetStartPosition() const
    {
        return m_IndirectObjs.GetStartPosition();
    }

    void					GetNextAssoc(FX_POSITION& rPos, FX_DWORD& objnum, CPDF_Object*& pObject) const
    {
        m_IndirectObjs.GetNextAssoc(rPos, (void*&)objnum, (void*&)pObject);
    }
protected:

    CFX_MapPtrToPtr			m_IndirectObjs;

    CPDF_Parser*			m_pParser;

    FX_DWORD				m_LastObjNum;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_OBJECTS_H_

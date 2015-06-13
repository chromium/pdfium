// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_SERIAL_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_SERIAL_H_

#include "fpdf_page.h"
#include "fpdf_pageobj.h"

class CPDF_ObjectStream;
class CPDF_XRefStream;
CFX_ByteTextBuf& operator << (CFX_ByteTextBuf& buf, const CPDF_Object* pObj);
class CPDF_ObjArchiveSaver : public CFX_ArchiveSaver
{
public:

    friend CPDF_ObjArchiveSaver&	operator << (CPDF_ObjArchiveSaver& ar, const CPDF_Object* pObj);
protected:

    CFX_MapPtrToPtr			m_ObjectMap;
};
class CPDF_ObjArchiveLoader : public CFX_ArchiveLoader
{
public:

    CPDF_ObjArchiveLoader(const uint8_t* pData, FX_DWORD dwSize) : CFX_ArchiveLoader(pData, dwSize),
        m_IndirectObjects(NULL) {}

    friend CPDF_ObjArchiveLoader&	operator >> (CPDF_ObjArchiveLoader& ar, CPDF_Object*& pObj);
protected:

    CPDF_IndirectObjects		m_IndirectObjects;
};
class CPDF_PageArchiveSaver : public CPDF_ObjArchiveSaver
{
public:

    CPDF_PageArchiveSaver(CPDF_PageObjects* pPageObjs);

    friend CPDF_PageArchiveSaver&	operator << (CPDF_PageArchiveSaver& ar, CPDF_PageObject* pObj);



    friend CPDF_PageArchiveSaver&	operator << (CPDF_PageArchiveSaver& ar, CPDF_ClipPath clip_path);

    friend CPDF_PageArchiveSaver&	operator << (CPDF_PageArchiveSaver& ar, CPDF_GraphState graph_state);

    friend CPDF_PageArchiveSaver&	operator << (CPDF_PageArchiveSaver& ar, CPDF_TextState text_state);

    friend CPDF_PageArchiveSaver&	operator << (CPDF_PageArchiveSaver& ar, CPDF_ColorState color_state);

    friend CPDF_PageArchiveSaver&	operator << (CPDF_PageArchiveSaver& ar, CPDF_GeneralState general_state);

protected:

    CPDF_ClipPath		m_LastClipPath;

    CPDF_GraphState		m_LastGraphState;

    CPDF_ColorState		m_LastColorState;

    CPDF_TextState		m_LastTextState;

    CPDF_GeneralState	m_LastGeneralState;

    CPDF_PageObjects*	m_pCurPage;
};
class CPDF_PageArchiveLoader : public CPDF_ObjArchiveLoader
{
public:

    CPDF_PageArchiveLoader(CPDF_PageObjects* pPageObjs, const uint8_t* pData, FX_DWORD dwSize);

    friend CPDF_PageArchiveLoader&	operator >> (CPDF_PageArchiveLoader& ar, CPDF_PageObject*& pObj);



    friend CPDF_PageArchiveLoader&	operator >> (CPDF_PageArchiveLoader& ar, CPDF_ClipPath& clip_path);

    friend CPDF_PageArchiveLoader&	operator >> (CPDF_PageArchiveLoader& ar, CPDF_GraphState& graph_state);

    friend CPDF_PageArchiveLoader&	operator >> (CPDF_PageArchiveLoader& ar, CPDF_TextState& text_state);

    friend CPDF_PageArchiveLoader&	operator >> (CPDF_PageArchiveLoader& ar, CPDF_ColorState& color_state);

    friend CPDF_PageArchiveLoader&	operator >> (CPDF_PageArchiveLoader& ar, CPDF_GeneralState& general_state);

protected:
    void				PostProcColor(CPDF_Color& color);

    CPDF_Object*		AddResource(CPDF_Object* pSrcObj, const FX_CHAR* type);

    CPDF_ClipPath		m_LastClipPath;

    CPDF_GraphState		m_LastGraphState;

    CPDF_ColorState		m_LastColorState;

    CPDF_TextState		m_LastTextState;

    CPDF_GeneralState	m_LastGeneralState;

    CPDF_PageObjects*	m_pCurPage;

    CFX_MapPtrToPtr		m_ObjectMap;
};
#define FPDFCREATE_INCREMENTAL		1
#define FPDFCREATE_NO_ORIGINAL		2
#define FPDFCREATE_PROGRESSIVE		4
#define FPDFCREATE_OBJECTSTREAM		8
class CPDF_Creator
{
public:

    CPDF_Creator(CPDF_Document* pDoc);

    ~CPDF_Creator();

    void				RemoveSecurity();

    FX_BOOL				Create(const FX_WCHAR* filename, FX_DWORD flags = 0);

    FX_BOOL				Create(const FX_CHAR* filename, FX_DWORD flags = 0);

    FX_BOOL				Create(IFX_StreamWrite* pFile, FX_DWORD flags = 0);

    int32_t			Continue(IFX_Pause *pPause = NULL);

    FX_BOOL				SetFileVersion(int32_t fileVersion = 17);
protected:

    CPDF_Document*		m_pDocument;

    CPDF_Parser*		m_pParser;

    FX_BOOL				m_bCompress;

    FX_BOOL				m_bSecurityChanged;

    CPDF_Dictionary*	m_pEncryptDict;
    FX_DWORD			m_dwEnryptObjNum;
    FX_BOOL				m_bEncryptCloned;

    FX_BOOL				m_bStandardSecurity;

    CPDF_CryptoHandler*	m_pCryptoHandler;
    FX_BOOL				m_bNewCrypto;

    FX_BOOL				m_bEncryptMetadata;

    CPDF_Object*		m_pMetadata;

    CPDF_XRefStream*	m_pXRefStream;

    int32_t			m_ObjectStreamSize;

    FX_DWORD			m_dwLastObjNum;
    FX_BOOL				Create(FX_DWORD flags);
    void				ResetStandardSecurity();
    void				Clear();
    int32_t			WriteDoc_Stage1(IFX_Pause *pPause);
    int32_t			WriteDoc_Stage2(IFX_Pause *pPause);
    int32_t			WriteDoc_Stage3(IFX_Pause *pPause);
    int32_t			WriteDoc_Stage4(IFX_Pause *pPause);

    CFX_FileBufferArchive	m_File;

    FX_FILESIZE			m_Offset;
    void				InitOldObjNumOffsets();
    void				InitNewObjNumOffsets();
    void				AppendNewObjNum(FX_DWORD objbum);
    int32_t			WriteOldIndirectObject(FX_DWORD objnum);
    int32_t			WriteOldObjs(IFX_Pause *pPause);
    int32_t			WriteNewObjs(FX_BOOL bIncremental, IFX_Pause *pPause);
    int32_t			WriteIndirectObj(const CPDF_Object* pObj);
    int32_t			WriteDirectObj(FX_DWORD objnum, const CPDF_Object* pObj, FX_BOOL bEncrypt = TRUE);
    int32_t			WriteIndirectObjectToStream(const CPDF_Object* pObj);
    int32_t			WriteIndirectObj(FX_DWORD objnum, const CPDF_Object* pObj);
    int32_t			WriteIndirectObjectToStream(FX_DWORD objnum, const uint8_t* pBuffer, FX_DWORD dwSize);
    int32_t			AppendObjectNumberToXRef(FX_DWORD objnum);
    void				InitID(FX_BOOL bDefault = TRUE);
    int32_t			WriteStream(const CPDF_Object* pStream, FX_DWORD objnum, CPDF_CryptoHandler* pCrypto);

    int32_t			m_iStage;
    FX_DWORD			m_dwFlags;
    FX_POSITION			m_Pos;
    FX_FILESIZE			m_XrefStart;

    CFX_FileSizeListArray	m_ObjectOffset;

    CFX_DWordListArray		m_ObjectSize;
    CFX_DWordArray		m_NewObjNumArray;

    CPDF_Array*			m_pIDArray;

    int32_t			m_FileVersion;
    friend class CPDF_ObjectStream;
    friend class CPDF_XRefStream;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_SERIAL_H_

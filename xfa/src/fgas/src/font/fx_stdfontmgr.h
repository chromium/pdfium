// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FX_FONTMGR_IMP
#define _FX_FONTMGR_IMP
#if _FXM_PLATFORM_ == _FXM_PLATFORM_WINDOWS_
FX_INT32 FX_GetSimilarValue(FX_LPCFONTDESCRIPTOR pFont, FX_DWORD dwFontStyles);
FX_LPCFONTDESCRIPTOR FX_DefFontMatcher(FX_LPFONTMATCHPARAMS pParams, const CFX_FontDescriptors &fonts, FX_LPVOID pUserData);
class CFX_StdFontMgrImp : public IFX_FontMgr, public CFX_Object
{
public:
    CFX_StdFontMgrImp(FX_LPEnumAllFonts pEnumerator, FX_LPMatchFont pMatcher, FX_LPVOID pUserData);
    ~CFX_StdFontMgrImp();
    virtual void			Release()
    {
        delete this;
    }
    virtual IFX_Font*		GetDefFontByCodePage(FX_WORD wCodePage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetDefFontByCharset(FX_BYTE nCharset, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetDefFontByUnicode(FX_WCHAR wUnicode, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetDefFontByLanguage(FX_WORD wLanguage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		LoadFont(FX_LPCWSTR pszFontFamily, FX_DWORD dwFontStyles, FX_WORD wCodePage = 0xFFFF);
    virtual IFX_Font*		LoadFont(FX_LPCBYTE pBuffer, FX_INT32 iLength);
    virtual IFX_Font*		LoadFont(FX_LPCWSTR pszFileName);
    virtual IFX_Font*		LoadFont(IFX_Stream *pFontStream, FX_LPCWSTR pszFontAlias = NULL, FX_DWORD dwFontStyles = 0, FX_WORD wCodePage = 0, FX_BOOL bSaveStream = FALSE);
    virtual IFX_Font*		LoadFont(IFX_Font *pSrcFont, FX_DWORD dwFontStyles, FX_WORD wCodePage = 0xFFFF);
    virtual void			ClearFontCache();
    virtual void			RemoveFont(IFX_Font *pFont);
protected:
    FX_LPMatchFont			m_pMatcher;
    FX_LPEnumAllFonts		m_pEnumerator;
    CFX_FontDescriptors		m_FontFaces;
    CFX_PtrArray			m_Fonts;
    CFX_MapPtrToPtr			m_CPFonts;
    CFX_MapPtrToPtr			m_FamilyFonts;
    CFX_MapPtrToPtr			m_UnicodeFonts;
    CFX_MapPtrToPtr			m_BufferFonts;
    CFX_MapPtrToPtr			m_FileFonts;
    CFX_MapPtrToPtr			m_StreamFonts;
    CFX_MapPtrToPtr			m_DeriveFonts;
    FX_LPVOID				m_pUserData;
    void					RemoveFont(CFX_MapPtrToPtr &fontMap, IFX_Font *pFont);
    FX_LPCFONTDESCRIPTOR	FindFont(FX_LPCWSTR pszFontFamily, FX_DWORD dwFontStyles, FX_DWORD dwMatchFlags, FX_WORD wCodePage, FX_DWORD dwUSB = 999, FX_WCHAR wUnicode = 0);
    IFX_Font*				GetFont(FX_LPCFONTDESCRIPTOR pFD, FX_DWORD dwFontStyles);
};
FX_DWORD FX_GetGdiFontStyles(const LOGFONTW &lf);
#else
class CFX_FontDescriptor : public CFX_Object
{
public:
    CFX_FontDescriptor()
        : m_pFileAccess(NULL)
        , m_nFaceIndex(0)
        , m_dwFontStyles(0)
    {
        m_dwUsb[0] = m_dwUsb[1] = m_dwUsb[2] = m_dwUsb[3] = 0;
        m_dwCsb[0] = m_dwCsb[1] = 0;
    }
    ~CFX_FontDescriptor()
    {
        if (NULL != m_pFileAccess) {
            m_pFileAccess->Release();
        }
    }
    IFX_FileAccess*			m_pFileAccess;
    FX_INT32				m_nFaceIndex;
    CFX_WideString			m_wsFaceName;
    CFX_WideStringArray		m_wsFamilyNames;
    FX_DWORD				m_dwFontStyles;
    FX_DWORD				m_dwUsb[4];
    FX_DWORD				m_dwCsb[2];
};
typedef CFX_ArrayTemplate<CFX_FontDescriptor*> CFX_FontDescriptors;
struct FX_FontDescriptorInfo {
public:
    CFX_FontDescriptor* pFont;
    FX_INT32 nPenalty;
    FX_BOOL operator > (const FX_FontDescriptorInfo& x)
    {
        return this->nPenalty > x.nPenalty;
    };
    FX_BOOL operator < (const FX_FontDescriptorInfo& x)
    {
        return this->nPenalty < x.nPenalty;
    };
    FX_BOOL operator == (const FX_FontDescriptorInfo& x)
    {
        return this->nPenalty == x.nPenalty;
    };
};
typedef CFX_ArrayTemplate<FX_FontDescriptorInfo> CFX_FontDescriptorInfos;
struct FX_HandleParentPath : public CFX_Object {
    FX_HandleParentPath()
    {
    }
    FX_HandleParentPath(const FX_HandleParentPath& x)
    {
        pFileHandle = x.pFileHandle;
        bsParentPath = x.bsParentPath;
    }
    void* pFileHandle;
    CFX_ByteString bsParentPath;
};
class CFX_FontSourceEnum_File : public IFX_FontSourceEnum, public CFX_Object
{
public:
    CFX_FontSourceEnum_File();
    virtual void			Release()
    {
        delete this;
    };
    virtual FX_POSITION		GetStartPosition(FX_LPVOID pUserData = NULL);
    virtual IFX_FileAccess*	GetNext(FX_POSITION& pos, FX_LPVOID pUserData = NULL);
private:
    CFX_ByteString			GetNextFile();
    CFX_WideString			m_wsNext;
    CFX_ObjectArray<FX_HandleParentPath>	m_FolderQueue;
    CFX_ByteStringArray		m_FolderPaths;
};
typedef CFX_MapPtrTemplate<FX_DWORD, IFX_FileAccess*> CFX_HashFileMap;
typedef CFX_MapPtrTemplate<FX_DWORD, IFX_Font*> CFX_HashFontMap;
typedef CFX_MapPtrTemplate<FX_DWORD, CFX_FontDescriptorInfos*> CFX_HashFontDescsMap;
typedef CFX_MapPtrTemplate<FX_DWORD, CFX_ArrayTemplate<IFX_Font*>* > CFX_HashFontsMap;
typedef CFX_MapPtrTemplate<FX_WCHAR, IFX_Font*> CFX_UnicodeFontMap;
typedef CFX_MapPtrTemplate<IFX_FileAccess*, CFX_ArrayTemplate<IFX_Font*>* > CFX_FileFontMap;
typedef CFX_MapPtrTemplate<IFX_Font*, IFX_FileRead*> CFX_FonStreamtMap;
class CFX_FontMgrImp : public IFX_FontMgr, public CFX_Object
{
public:
    CFX_FontMgrImp(IFX_FontSourceEnum* pFontEnum, IFX_FontMgrDelegate* pDelegate = NULL, FX_LPVOID pUserData = NULL);
    virtual void			Release();
    virtual IFX_Font*		GetDefFontByCodePage(FX_WORD wCodePage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetDefFontByCharset(FX_BYTE nCharset, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetDefFontByUnicode(FX_WCHAR wUnicode, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetDefFontByLanguage(FX_WORD wLanguage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetFontByCodePage(FX_WORD wCodePage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetFontByCharset(FX_BYTE nCharset, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetFontByUnicode(FX_WCHAR wUnicode, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		GetFontByLanguage(FX_WORD wLanguage, FX_DWORD dwFontStyles, FX_LPCWSTR pszFontFamily = NULL);
    virtual IFX_Font*		LoadFont(FX_LPCBYTE pBuffer, FX_INT32 iLength, FX_INT32 iFaceIndex, FX_INT32* pFaceCount);
    virtual IFX_Font*		LoadFont(FX_LPCWSTR pszFileName, FX_INT32 iFaceIndex, FX_INT32* pFaceCount);
    virtual IFX_Font*		LoadFont(IFX_Stream* pFontStream, FX_INT32 iFaceIndex, FX_INT32* pFaceCount, FX_BOOL bSaveStream = FALSE);
    virtual void			ClearFontCache();
    virtual void			RemoveFont(IFX_Font* pFont);
    FX_BOOL					EnumFonts();
protected:
    void					ReportFace(FXFT_Face pFace, CFX_FontDescriptors& Fonts, IFX_FileAccess* pFontAccess);
    void					GetNames(FX_LPCBYTE name_table, CFX_WideStringArray& Names);
    void					GetCharsets(FXFT_Face pFace, CFX_WordArray& Charsets);
    void					GetUSBCSB(FXFT_Face pFace, FX_DWORD* USB, FX_DWORD* CSB);
    FX_DWORD				GetFlags(FXFT_Face pFace);
    CFX_FontDescriptors		m_InstalledFonts;
    FX_BOOL					VerifyUnicode(CFX_FontDescriptor* pDesc, FX_WCHAR wcUnicode);
    FX_BOOL					VerifyUnicode(IFX_Font* pFont, FX_WCHAR wcUnicode);
    void					NormalizeFontName( CFX_WideString& FontName );
    FX_INT32				IsPartName(const CFX_WideString& Name1, const CFX_WideString& Name2);
    FX_INT32				MatchFonts(CFX_FontDescriptorInfos& MatchedFonts, FX_WORD wCodePage, FX_DWORD dwFontStyles, const CFX_WideString& FontName, FX_WCHAR wcUnicode = 0xFFFE);
    FX_INT32				CalcPenalty(CFX_FontDescriptor* pInstalled, FX_WORD wCodePage, FX_DWORD dwFontStyles, const CFX_WideString& FontName, FX_WCHAR wcUnicode = 0xFFFE);
    IFX_Font*				LoadFont(IFX_FileAccess* pFontAccess, FX_INT32 iFaceIndex, FX_INT32* pFaceCount, FX_BOOL bWantCache = FALSE);
    FXFT_Face				LoadFace(IFX_FileRead* pFontStream, FX_INT32 iFaceIndex);
    CFX_HashFontDescsMap	m_Hash2CandidateList;
    CFX_HashFontsMap		m_Hash2Fonts;
    CFX_HashFileMap			m_Hash2FileAccess;
    CFX_HashFontMap			m_FileAccess2IFXFont;
    CFX_FonStreamtMap		m_IFXFont2FileRead;
    CFX_UnicodeFontMap		m_FailedUnicodes2NULL;
    IFX_FontSourceEnum*		m_pFontSource;
    IFX_FontMgrDelegate*	m_pDelegate;
    FX_LPVOID				m_pUserData;
};
#endif
#endif

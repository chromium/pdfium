// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FXGE_ANDROID_FPF_SKIAFONTMGR_H_
#define CORE_SRC_FXGE_ANDROID_FPF_SKIAFONTMGR_H_

#if _FX_OS_ == _FX_ANDROID_
#define FPF_SKIAFONTTYPE_Unknown	0
#define	FPF_SKIAFONTTYPE_Path		1
#define FPF_SKIAFONTTYPE_File		2
#define FPF_SKIAFONTTYPE_Buffer		3
class CFPF_SkiaFontDescriptor 
{
public:
    CFPF_SkiaFontDescriptor() : m_pFamily(NULL), m_dwStyle(0), m_iFaceIndex(0), m_dwCharsets(0), m_iGlyphNum(0) {}
    virtual ~CFPF_SkiaFontDescriptor()
    {
        if (m_pFamily) {
            FX_Free(m_pFamily);
        }
    }
    virtual	FX_INT32	GetType() const
    {
        return FPF_SKIAFONTTYPE_Unknown;
    }
    void				SetFamily(FX_LPCSTR pFamily)
    {
        if (m_pFamily) {
            FX_Free(m_pFamily);
        }
        FX_INT32 iSize = FXSYS_strlen(pFamily);
        m_pFamily = FX_Alloc(FX_CHAR, iSize + 1);
        FXSYS_memcpy32(m_pFamily, pFamily, iSize * sizeof(FX_CHAR));
        m_pFamily[iSize] = 0;
    }
    FX_LPSTR		m_pFamily;
    FX_DWORD		m_dwStyle;
    FX_INT32		m_iFaceIndex;
    FX_DWORD		m_dwCharsets;
    FX_INT32		m_iGlyphNum;
};
class CFPF_SkiaPathFont : public CFPF_SkiaFontDescriptor
{
public:
    CFPF_SkiaPathFont() : m_pPath(NULL) {}
    virtual ~CFPF_SkiaPathFont()
    {
        if (m_pPath) {
            FX_Free(m_pPath);
        }
    }
    virtual	FX_INT32	GetType() const
    {
        return FPF_SKIAFONTTYPE_Path;
    }
    void				SetPath(FX_LPCSTR pPath)
    {
        if (m_pPath) {
            FX_Free(m_pPath);
        }
        FX_INT32 iSize = FXSYS_strlen(pPath);
        m_pPath = FX_Alloc(FX_CHAR, iSize + 1);
        FXSYS_memcpy32(m_pPath, pPath, iSize * sizeof(FX_CHAR));
        m_pPath[iSize] = 0;
    }
    FX_LPSTR		m_pPath;
};
class CFPF_SkiaFileFont : public CFPF_SkiaFontDescriptor
{
public:
    CFPF_SkiaFileFont() : m_pFile(NULL) {}
    virtual FX_INT32	GetType() const
    {
        return FPF_SKIAFONTTYPE_File;
    }
    IFX_FileRead		*m_pFile;
};
class CFPF_SkiaBufferFont : public CFPF_SkiaFontDescriptor
{
public:
    CFPF_SkiaBufferFont() : m_pBuffer(NULL), m_szBuffer(0) {}
    virtual FX_INT32	GetType() const
    {
        return FPF_SKIAFONTTYPE_Buffer;
    }
    FX_LPVOID			m_pBuffer;
    size_t				m_szBuffer;
};
class CFPF_SkiaFontMgr : public IFPF_FontMgr
{
public:
    CFPF_SkiaFontMgr();
    virtual ~CFPF_SkiaFontMgr();
    FX_BOOL					InitFTLibrary();
    virtual void			LoadSystemFonts();
    virtual void			LoadPrivateFont(IFX_FileRead* pFontFile);
    virtual void			LoadPrivateFont(FX_BSTR bsFileName);
    virtual void			LoadPrivateFont(FX_LPVOID pBuffer, size_t szBuffer);

    virtual IFPF_Font*		CreateFont(FX_BSTR bsFamilyname, FX_BYTE uCharset, FX_DWORD dwStyle, FX_DWORD dwMatch = 0);
    FXFT_Face				GetFontFace(IFX_FileRead *pFileRead, FX_INT32 iFaceIndex = 0);
    FXFT_Face				GetFontFace(FX_BSTR bsFile, FX_INT32 iFaceIndex = 0);
    FXFT_Face				GetFontFace(FX_LPCBYTE pBuffer, size_t szBuffer, FX_INT32 iFaceIndex = 0);
protected:
    void				ScanPath(FX_BSTR path);
    void				ScanFile(FX_BSTR file);
    void				ReportFace(FXFT_Face face, CFPF_SkiaFontDescriptor *pFontDesc);
    void				OutputSystemFonts();
    FX_BOOL				m_bLoaded;
    CFX_PtrArray		m_FontFaces;
    FXFT_Library		m_FTLibrary;
    CFX_MapPtrToPtr		m_FamilyFonts;
};
#endif

#endif  // CORE_SRC_FXGE_ANDROID_FPF_SKIAFONTMGR_H_

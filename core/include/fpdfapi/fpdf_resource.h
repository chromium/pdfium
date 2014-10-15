// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FPDF_RESOURCE_
#define _FPDF_RESOURCE_
#ifndef _FPDF_PARSER_
#include "fpdf_parser.h"
#endif
#ifndef _FX_FONT_H_
#include "../fxge/fx_font.h"
#endif
class CPDF_Font;
class CPDF_Type1Font;
class CPDF_TrueTypeFont;
class CPDF_CIDFont;
class CPDF_Type3Font;
class CPDF_FontEncoding;
class CPDF_CMap;
class CPDF_CID2UnicodeMap;
class CPDF_ColorSpace;
class CPDF_Color;
class CPDF_Function;
class CPDF_Pattern;
class CPDF_TilingPattern;
class CPDF_ShadingPattern;
class CPDF_Image;
class CPDF_Face;
class CPDF_ToUnicodeMap;
class CFX_SubstFont;
class CFX_Font;
class CPDF_RenderContext;
class CPDF_Form;
class CPDF_ImageObject;
class CFX_DIBitmap;
typedef struct FT_FaceRec_* FXFT_Face;
class CFX_CTTGSUBTable;
class CPDF_Page;

template <class ObjClass> class CPDF_CountedObject : public CFX_Object
{
public:
    ObjClass	m_Obj;
    FX_DWORD	m_nCount;
};
typedef CPDF_CountedObject<CPDF_Font*>          CPDF_CountedFont;
typedef CPDF_CountedObject<CPDF_ColorSpace*>    CPDF_CountedColorSpace;
typedef CPDF_CountedObject<CPDF_Pattern*>       CPDF_CountedPattern;
typedef CPDF_CountedObject<CPDF_Image*>         CPDF_CountedImage;
typedef CPDF_CountedObject<CPDF_IccProfile*>    CPDF_CountedICCProfile;
typedef CPDF_CountedObject<CPDF_StreamAcc*>     CPDF_CountedStreamAcc;


typedef CFX_MapPtrTemplate<CPDF_Dictionary*, CPDF_CountedFont*>     CPDF_FontMap;
typedef CFX_MapPtrTemplate<CPDF_Object*, CPDF_CountedColorSpace*>   CPDF_ColorSpaceMap;
typedef CFX_MapPtrTemplate<CPDF_Object*, CPDF_CountedPattern*>      CPDF_PatternMap;
typedef CFX_MapPtrTemplate<FX_DWORD, CPDF_CountedImage*>            CPDF_ImageMap;
typedef CFX_MapPtrTemplate<CPDF_Stream*, CPDF_CountedICCProfile*>   CPDF_IccProfileMap;
typedef CFX_MapPtrTemplate<CPDF_Stream*, CPDF_CountedStreamAcc*>    CPDF_FontFileMap;

#define PDFFONT_TYPE1			1
#define PDFFONT_TRUETYPE		2
#define PDFFONT_TYPE3			3
#define PDFFONT_CIDFONT			4
#define PDFFONT_FIXEDPITCH		1
#define PDFFONT_SERIF			2
#define PDFFONT_SYMBOLIC		4
#define PDFFONT_SCRIPT			8
#define PDFFONT_NONSYMBOLIC		32
#define PDFFONT_ITALIC			64
#define PDFFONT_ALLCAP			0x10000
#define PDFFONT_SMALLCAP		0x20000
#define PDFFONT_FORCEBOLD		0x40000
#define PDFFONT_USEEXTERNATTR	0x80000
FX_WCHAR PDF_UnicodeFromAdobeName(const FX_CHAR* name);
CFX_ByteString PDF_AdobeNameFromUnicode(FX_WCHAR unicode);
class CPDF_Font : public CFX_Object
{
public:

    static CPDF_Font*		CreateFontF(CPDF_Document* pDoc, CPDF_Dictionary* pFontDict);

    static CPDF_Font*		GetStockFont(CPDF_Document* pDoc, FX_BSTR fontname);

    virtual ~CPDF_Font();




    int						GetFontType() const
    {
        return m_FontType;
    }

    CFX_ByteString			GetFontTypeName() const;

    const CFX_ByteString&	GetBaseFont() const
    {
        return m_BaseFont;
    }

    const CFX_SubstFont*	GetSubstFont() const
    {
        return m_Font.GetSubstFont();
    }

    FX_DWORD				GetFlags() const
    {
        return m_Flags;
    }

    virtual FX_BOOL			IsVertWriting()const;




    CPDF_Type1Font*			GetType1Font() const
    {
        return m_FontType == PDFFONT_TYPE1 ? (CPDF_Type1Font*)(void*)this : NULL;
    }

    CPDF_TrueTypeFont*		GetTrueTypeFont() const
    {
        return m_FontType == PDFFONT_TRUETYPE ? (CPDF_TrueTypeFont*)(void*)this : NULL;
    }

    CPDF_CIDFont*			GetCIDFont() const
    {
        return (m_FontType == PDFFONT_CIDFONT) ? (CPDF_CIDFont*)(void*)this : NULL;
    }

    CPDF_Type3Font*			GetType3Font() const
    {
        return (m_FontType == PDFFONT_TYPE3) ? (CPDF_Type3Font*)(void*)this : NULL;
    }


    FX_BOOL					IsEmbedded() const
    {
        return m_FontType == PDFFONT_TYPE3 || m_pFontFile != NULL;
    }

    virtual FX_BOOL			IsUnicodeCompatible() const
    {
        return FALSE;
    }

    CPDF_StreamAcc*			GetFontFile() const
    {
        return m_pFontFile;
    }

    CPDF_Dictionary*		GetFontDict() const
    {
        return m_pFontDict;
    }

    FX_BOOL					IsStandardFont() const;

    FXFT_Face				GetFace() const
    {
        return m_Font.GetFace();
    }



    virtual FX_DWORD		GetNextChar(FX_LPCSTR pString, int& offset) const
    {
        return (FX_BYTE)pString[offset++];
    }

    virtual int				CountChar(FX_LPCSTR pString, int size) const
    {
        return size;
    }

    void					AppendChar(CFX_ByteString& str, FX_DWORD charcode) const;

    virtual int				AppendChar(FX_LPSTR buf, FX_DWORD charcode) const
    {
        *buf = (FX_CHAR)charcode;
        return 1;
    }

    virtual int				GetCharSize(FX_DWORD charcode) const
    {
        return 1;
    }




    virtual int				GlyphFromCharCode(FX_DWORD charcode, FX_BOOL *pVertGlyph = NULL) = 0;
    virtual int				GlyphFromCharCodeExt(FX_DWORD charcode)
    {
        return GlyphFromCharCode(charcode);
    }

    CFX_WideString			UnicodeFromCharCode(FX_DWORD charcode) const;

    FX_DWORD				CharCodeFromUnicode(FX_WCHAR Unicode) const;

    CFX_CharMap*			GetCharMap()
    {
        return m_pCharMap;
    }

    CFX_ByteString			EncodeString(const CFX_WideString& str) const;

    CFX_WideString			DecodeString(const CFX_ByteString& str) const;




    void					GetFontBBox(FX_RECT& rect) const
    {
        rect = m_FontBBox;
    }

    int						GetTypeAscent() const
    {
        return m_Ascent;
    }

    int						GetTypeDescent() const
    {
        return m_Descent;
    }

    int						GetItalicAngle() const
    {
        return m_ItalicAngle;
    }

    int						GetStemV() const
    {
        return m_StemV;
    }

    int						GetStringWidth(const FX_CHAR* pString, int size);

    virtual int				GetCharWidthF(FX_DWORD charcode, int level = 0) = 0;

    virtual int				GetCharTypeWidth(FX_DWORD charcode);

    virtual void			GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level = 0) = 0;



    CPDF_Document*			m_pDocument;

    class CFX_PathData*		LoadGlyphPath(FX_DWORD charcode, int dest_width = 0);

    CFX_Font				m_Font;
protected:

    CPDF_Font();

    FX_BOOL					Initialize();

    FX_BOOL					Load();

    virtual FX_BOOL			_Load() = 0;

    virtual FX_WCHAR		_UnicodeFromCharCode(FX_DWORD charcode) const = 0;

    virtual FX_DWORD		_CharCodeFromUnicode(FX_WCHAR Unicode) const = 0;

    void					LoadUnicodeMap();

    void					LoadPDFEncoding(CPDF_Object* pEncoding, int& iBaseEncoding,
                                            CFX_ByteString*& pCharNames, FX_BOOL bEmbedded, FX_BOOL bTrueType);

    void					LoadFontDescriptor(CPDF_Dictionary*);

    void					LoadCharWidths(FX_WORD* pWidths);

    void					CheckFontMetrics();

    CFX_CharMap*			m_pCharMap;



    int						m_FontType;

    CFX_ByteString			m_BaseFont;

    CPDF_StreamAcc*			m_pFontFile;

    CPDF_Dictionary*		m_pFontDict;

    CPDF_ToUnicodeMap*		m_pToUnicodeMap;

    FX_BOOL					m_bToUnicodeLoaded;




    int						m_Flags;

    FX_RECT					m_FontBBox;

    int						m_StemV;

    int						m_Ascent;

    int						m_Descent;

    int						m_ItalicAngle;

};
#define PDFFONT_ENCODING_BUILTIN		0
#define PDFFONT_ENCODING_WINANSI		1
#define PDFFONT_ENCODING_MACROMAN		2
#define PDFFONT_ENCODING_MACEXPERT		3
#define PDFFONT_ENCODING_STANDARD		4
#define PDFFONT_ENCODING_ADOBE_SYMBOL	5
#define PDFFONT_ENCODING_ZAPFDINGBATS	6
#define PDFFONT_ENCODING_PDFDOC			7
#define PDFFONT_ENCODING_MS_SYMBOL		8
#define PDFFONT_ENCODING_UNICODE		9
class CPDF_FontEncoding : public CFX_Object
{
public:

    CPDF_FontEncoding();

    CPDF_FontEncoding(int PredefinedEncoding);

    void					LoadEncoding(CPDF_Object* pEncoding);

    FX_BOOL					IsIdentical(CPDF_FontEncoding* pAnother) const;

    FX_WCHAR				UnicodeFromCharCode(FX_BYTE charcode) const
    {
        return m_Unicodes[charcode];
    }

    int						CharCodeFromUnicode(FX_WCHAR unicode) const;

    void					SetUnicode(FX_BYTE charcode, FX_WCHAR unicode)
    {
        m_Unicodes[charcode] = unicode;
    }

    CPDF_Object*			Realize();
public:

    FX_WCHAR				m_Unicodes[256];
};
class CPDF_SimpleFont : public CPDF_Font
{
public:

    CPDF_SimpleFont();

    virtual ~CPDF_SimpleFont();

    CPDF_FontEncoding*		GetEncoding()
    {
        return &m_Encoding;
    }
    virtual int				GetCharWidthF(FX_DWORD charcode, int level = 0);
    virtual void			GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level = 0);
    virtual int				GlyphFromCharCode(FX_DWORD charcode, FX_BOOL *pVertGlyph = NULL);
    virtual FX_BOOL			IsUnicodeCompatible() const;
protected:

    FX_BOOL					LoadCommon();

    void					LoadSubstFont();

    void					LoadFaceMetrics();

    virtual void			LoadGlyphMap() = 0;
    virtual FX_WCHAR		_UnicodeFromCharCode(FX_DWORD charcode) const
    {
        return m_Encoding.UnicodeFromCharCode((FX_BYTE)charcode);
    }
    virtual FX_DWORD		_CharCodeFromUnicode(FX_WCHAR Unicode) const
    {
        return m_Encoding.CharCodeFromUnicode(Unicode);
    }



    CPDF_FontEncoding		m_Encoding;

    FX_WORD					m_GlyphIndex[256];
    FX_WORD					m_ExtGID[256];




    CFX_ByteString*			m_pCharNames;

    int						m_BaseEncoding;




    FX_WORD					m_CharWidth[256];

    FX_SMALL_RECT			m_CharBBox[256];

    FX_BOOL					m_bUseFontWidth;

    void					LoadCharMetrics(int charcode);

};
class CPDF_Type1Font : public CPDF_SimpleFont
{
public:

    CPDF_Type1Font();

    int						GetBase14Font()
    {
        return m_Base14Font;
    }
    virtual int				GlyphFromCharCodeExt(FX_DWORD charcode);
protected:
    virtual FX_BOOL			_Load();

    int						m_Base14Font;
    virtual void			LoadGlyphMap();
};
class CPDF_TrueTypeFont : public CPDF_SimpleFont
{
public:

    CPDF_TrueTypeFont();
protected:
    virtual FX_BOOL			_Load();
    virtual void			LoadGlyphMap();
};
class CPDF_Type3Char : public CFX_Object
{
public:

    CPDF_Type3Char();

    ~CPDF_Type3Char();

    FX_BOOL LoadBitmap(CPDF_RenderContext* pContext);

    FX_BOOL					m_bColored;

    FX_BOOL					m_bPageRequired;



    CPDF_Form*				m_pForm;

    CFX_AffineMatrix		m_ImageMatrix;

    CFX_DIBitmap*			m_pBitmap;


    int						m_Width;

    FX_RECT					m_BBox;
};
class CPDF_Type3Font : public CPDF_SimpleFont
{
public:
    CPDF_Type3Font();
    virtual ~CPDF_Type3Font();
    void					SetPageResources(CPDF_Dictionary* pResources)
    {
        m_pPageResources = pResources;
    }
    CPDF_Type3Char*			LoadChar(FX_DWORD charcode, int level = 0);
    virtual int				GetCharWidthF(FX_DWORD charcode, int level = 0);
    virtual int				GetCharTypeWidth(FX_DWORD charcode)
    {
        return GetCharWidthF(charcode);
    }
    virtual void			GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level = 0);
    CFX_AffineMatrix&		GetFontMatrix()
    {
        return m_FontMatrix;
    }
    void					CheckType3FontMetrics();
private:
    virtual FX_BOOL			_Load();
    virtual void			LoadGlyphMap() {}
    int						m_CharWidthL[256];
    CPDF_Dictionary*		m_pCharProcs;
    CPDF_Dictionary*		m_pPageResources;
    CPDF_Dictionary*		m_pFontResources;
    CFX_MapPtrToPtr			m_CacheMap;
    CFX_MapPtrToPtr			m_DeletedMap;
protected:
    CFX_AffineMatrix		m_FontMatrix;
};
#define CIDSET_UNKNOWN		0
#define CIDSET_GB1			1
#define CIDSET_CNS1			2
#define CIDSET_JAPAN1		3
#define CIDSET_KOREA1		4
#define CIDSET_UNICODE		5
#define NUMBER_OF_CIDSETS   6

class CPDF_CIDFont : public CPDF_Font
{
public:

    CPDF_CIDFont();

    virtual ~CPDF_CIDFont();

    FX_BOOL					LoadGB2312();
    virtual int				GlyphFromCharCode(FX_DWORD charcode, FX_BOOL *pVertGlyph = NULL);
    virtual int				GetCharWidthF(FX_DWORD charcode, int level = 0);
    virtual void			GetCharBBox(FX_DWORD charcode, FX_RECT& rect, int level = 0);

    FX_WORD					CIDFromCharCode(FX_DWORD charcode) const;

    FX_BOOL					IsTrueType()
    {
        return !m_bType1;
    }


    virtual FX_DWORD		GetNextChar(const FX_CHAR* pString, int& offset) const;
    virtual int				CountChar(const FX_CHAR* pString, int size) const;
    virtual int				AppendChar(FX_LPSTR str, FX_DWORD charcode) const;
    virtual int				GetCharSize(FX_DWORD charcode) const;


    int						GetCharset() const
    {
        return m_Charset;
    }

    FX_LPCBYTE				GetCIDTransform(FX_WORD CID) const;



    virtual FX_BOOL			IsVertWriting() const;

    short					GetVertWidth(FX_WORD CID) const;

    void					GetVertOrigin(FX_WORD CID, short& vx, short& vy) const;

    virtual FX_BOOL			IsUnicodeCompatible() const;
    virtual FX_BOOL			IsFontStyleFromCharCode(FX_DWORD charcode) const;
protected:
    friend class			CPDF_Font;
    virtual FX_BOOL			_Load();
    virtual FX_WCHAR		_UnicodeFromCharCode(FX_DWORD charcode) const;
    virtual FX_DWORD		_CharCodeFromUnicode(FX_WCHAR Unicode) const;
    int				GetGlyphIndex(FX_DWORD unicodeb, FX_BOOL *pVertGlyph);

    CPDF_CMap*				m_pCMap;

    CPDF_CMap*				m_pAllocatedCMap;

    CPDF_CID2UnicodeMap*	m_pCID2UnicodeMap;

    int						m_Charset;

    FX_BOOL					m_bType1;

    CPDF_StreamAcc*			m_pCIDToGIDMap;
    FX_BOOL					m_bCIDIsGID;



    FX_WORD					m_DefaultWidth;

    FX_WORD*				m_pAnsiWidths;

    FX_SMALL_RECT			m_CharBBox[256];

    CFX_DWordArray			m_WidthList;

    short					m_DefaultVY;

    short					m_DefaultW1;

    CFX_DWordArray			m_VertMetrics;


    void					LoadMetricsArray(CPDF_Array* pArray, CFX_DWordArray& result, int nElements);

    void					LoadSubstFont();

    FX_BOOL					m_bAdobeCourierStd;

    CFX_CTTGSUBTable*			m_pTTGSUBTable;
};
#define PDFCS_DEVICEGRAY		1

#define PDFCS_DEVICERGB			2
#define PDFCS_DEVICECMYK		3
#define PDFCS_CALGRAY			4
#define PDFCS_CALRGB			5
#define PDFCS_LAB				6
#define PDFCS_ICCBASED			7
#define PDFCS_SEPARATION		8
#define PDFCS_DEVICEN			9
#define PDFCS_INDEXED			10
#define PDFCS_PATTERN			11
class CPDF_ColorSpace : public CFX_Object
{
public:

    static CPDF_ColorSpace* GetStockCS(int Family);

    static CPDF_ColorSpace*	Load(CPDF_Document* pDoc, CPDF_Object* pCSObj);

    void					ReleaseCS();

    int						GetBufSize() const;

    FX_FLOAT*			CreateBuf();

    void					GetDefaultColor(FX_FLOAT* buf) const;

    int						CountComponents() const
    {
        return m_nComponents;
    }

    int						GetFamily() const
    {
        return m_Family;
    }

    virtual void			GetDefaultValue(int iComponent, FX_FLOAT& value, FX_FLOAT& min, FX_FLOAT& max) const
    {
        value = 0;
        min = 0;
        max = 1.0f;
    }

    FX_BOOL					sRGB() const;



    virtual FX_BOOL			GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const = 0;

    virtual FX_BOOL			SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const
    {
        return FALSE;
    }




    FX_BOOL					GetCMYK(FX_FLOAT* pBuf, FX_FLOAT& c, FX_FLOAT& m, FX_FLOAT& y, FX_FLOAT& k) const;

    FX_BOOL					SetCMYK(FX_FLOAT* pBuf, FX_FLOAT c, FX_FLOAT m, FX_FLOAT y, FX_FLOAT k) const;


    virtual void			TranslateImageLine(FX_LPBYTE dest_buf, FX_LPCBYTE src_buf, int pixels,
            int image_width, int image_height, FX_BOOL bTransMask = FALSE) const;

    CPDF_Array*&			GetArray()
    {
        return m_pArray;
    }

    int						GetMaxIndex() const;

    virtual CPDF_ColorSpace*	GetBaseCS() const
    {
        return NULL;
    }

    virtual void			EnableStdConversion(FX_BOOL bEnabled);
    CPDF_Document*			m_pDocument;
protected:

    CPDF_ColorSpace();

    virtual ~CPDF_ColorSpace() {}
    virtual FX_BOOL			v_Load(CPDF_Document* pDoc, CPDF_Array* pArray)
    {
        return TRUE;
    }
    virtual FX_BOOL			v_GetCMYK(FX_FLOAT* pBuf, FX_FLOAT& c, FX_FLOAT& m, FX_FLOAT& y, FX_FLOAT& k) const
    {
        return FALSE;
    }
    virtual FX_BOOL			v_SetCMYK(FX_FLOAT* pBuf, FX_FLOAT c, FX_FLOAT m, FX_FLOAT y, FX_FLOAT k) const
    {
        return FALSE;
    }

    int						m_Family;

    int						m_nComponents;

    CPDF_Array*				m_pArray;

    FX_DWORD				m_dwStdConversion;
};
class CPDF_Color : public CFX_Object
{
public:

    CPDF_Color() :m_pCS(NULL), m_pBuffer(NULL)
    {
    }

    CPDF_Color(int family);

    ~CPDF_Color();

    FX_BOOL					IsNull() const
    {
        return m_pBuffer == NULL;
    }

    FX_BOOL					IsEqual(const CPDF_Color& other) const;

    FX_BOOL					IsPattern() const
    {
        return m_pCS && m_pCS->GetFamily() == PDFCS_PATTERN;
    }

    void					Copy(const CPDF_Color* pSrc);

    void					SetColorSpace(CPDF_ColorSpace* pCS);

    void					SetValue(FX_FLOAT* comp);

    void					SetValue(CPDF_Pattern* pPattern, FX_FLOAT* comp, int ncomps);

    FX_BOOL					GetRGB(int& R, int& G, int& B) const;

    CPDF_Pattern*			GetPattern() const;

    CPDF_ColorSpace*		GetPatternCS() const;

    FX_FLOAT*			GetPatternColor() const;

    CPDF_ColorSpace*		m_pCS;

protected:
    void	ReleaseBuffer();
    void	ReleaseColorSpace();
    FX_FLOAT*			    m_pBuffer;
};
#define PATTERN_TILING		1
#define PATTERN_SHADING		2
class CPDF_Pattern : public CFX_Object
{
public:
   
    virtual ~CPDF_Pattern();
    void    SetForceClear(FX_BOOL bForceClear) { m_bForceClear = bForceClear; }

    CPDF_Object*                m_pPatternObj;

    int                         m_PatternType;

    CFX_AffineMatrix            m_Pattern2Form;
    CFX_AffineMatrix            m_ParentMatrix;

    CPDF_Document*              m_pDocument;

protected:
    CPDF_Pattern(const CFX_AffineMatrix* pParentMatrix);
    FX_BOOL     m_bForceClear;
};

class CPDF_TilingPattern : public CPDF_Pattern
{
public:

    CPDF_TilingPattern(CPDF_Document* pDoc, CPDF_Object* pPatternObj, const CFX_AffineMatrix* parentMatrix);

    virtual ~CPDF_TilingPattern();

    FX_BOOL				Load();



    FX_BOOL				m_bColored;

    CFX_FloatRect		m_BBox;

    FX_FLOAT			m_XStep;

    FX_FLOAT			m_YStep;



    CPDF_Form*			m_pForm;
};
class CPDF_ShadingPattern : public CPDF_Pattern
{
public:

    CPDF_ShadingPattern(CPDF_Document* pDoc, CPDF_Object* pPatternObj, FX_BOOL bShading, const CFX_AffineMatrix* parentMatrix);

    virtual ~CPDF_ShadingPattern();

    CPDF_Object*		m_pShadingObj;

    FX_BOOL				m_bShadingObj;

    FX_BOOL				Load();

    FX_BOOL				Reload();

    int					m_ShadingType;

    CPDF_ColorSpace*	m_pCS; // Still keep m_pCS as some CPDF_ColorSpace (name object) are not managed as counted objects. Refer to CPDF_DocPageData::GetColorSpace.

    CPDF_CountedColorSpace*	m_pCountedCS;

    CPDF_Function*		m_pFunctions[4];

    int					m_nFuncs;
protected:
    void	Clear();
};
struct CPDF_MeshVertex {
    FX_FLOAT x, y;
    FX_FLOAT r, g, b;
};
class CPDF_MeshStream : public CFX_Object
{
public:

    FX_BOOL				Load(CPDF_Stream* pShadingStream, CPDF_Function** pFuncs, int nFuncs, CPDF_ColorSpace* pCS);

    FX_DWORD			GetFlag();

    void				GetCoords(FX_FLOAT& x, FX_FLOAT& y);

    void				GetColor(FX_FLOAT& r, FX_FLOAT& g, FX_FLOAT& b);

    FX_DWORD 			GetVertex(CPDF_MeshVertex& vertex, CFX_AffineMatrix* pObject2Bitmap);

    FX_BOOL				GetVertexRow(CPDF_MeshVertex* vertex, int count, CFX_AffineMatrix* pObject2Bitmap);
    CPDF_Function**	m_pFuncs;
    CPDF_ColorSpace*	m_pCS;
    FX_DWORD			m_nFuncs, m_nCoordBits, m_nCompBits, m_nFlagBits, m_nComps;
    FX_DWORD			m_CoordMax, m_CompMax;
    FX_FLOAT			m_xmin, m_xmax, m_ymin, m_ymax;
    FX_FLOAT			m_ColorMin[8], m_ColorMax[8];
    CPDF_StreamAcc		m_Stream;
    CFX_BitStream		m_BitStream;
};
#define PDF_IMAGE_NO_COMPRESS				0x0000
#define PDF_IMAGE_LOSSY_COMPRESS			0x0001
#define PDF_IMAGE_LOSSLESS_COMPRESS			0x0002
#define PDF_IMAGE_MASK_LOSSY_COMPRESS		0x0004
#define PDF_IMAGE_MASK_LOSSLESS_COMPRESS	0x0008
class CPDF_ImageSetParam
{
public:
    CPDF_ImageSetParam()
        : pMatteColor(NULL)
        , nQuality(80)
    {
    }
    FX_ARGB* pMatteColor;
    FX_INT32 nQuality;
};
class CPDF_Image : public CFX_Object
{
public:

    CPDF_Image(CPDF_Document* pDoc);

    ~CPDF_Image();

    FX_BOOL					LoadImageF(CPDF_Stream* pImageStream, FX_BOOL bInline);

    void					Release();

    CPDF_Image*				Clone();

    FX_BOOL					IsInline()
    {
        return m_bInline;
    }

    void					SetInlineDict(CPDF_Dictionary* pDict)
    {
        m_pInlineDict = pDict;
    }

    CPDF_Dictionary*		GetInlineDict() const
    {
        return m_pInlineDict;
    }

    CPDF_Stream*			GetStream() const
    {
        return m_pStream;
    }

    CPDF_Dictionary*		GetDict() const
    {
        return m_pStream? m_pStream->GetDict(): NULL;
    }

    CPDF_Dictionary*		GetOC() const
    {
        return m_pOC;
    }

    CPDF_Document*			GetDocument() const
    {
        return m_pDocument;
    }



    FX_INT32				GetPixelHeight() const
    {
        return m_Height;
    }

    FX_INT32				GetPixelWidth() const
    {
        return m_Width;
    }


    FX_BOOL					IsMask() const
    {
        return m_bIsMask;
    }

    FX_BOOL					IsInterpol() const
    {
        return m_bInterpolate;
    }

    CFX_DIBSource*			LoadDIBSource(CFX_DIBSource** ppMask = NULL, FX_DWORD* pMatteColor = NULL, FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE) const;



    void					SetImage(const CFX_DIBitmap* pDIBitmap, FX_INT32 iCompress, IFX_FileWrite *pFileWrite = NULL, IFX_FileRead *pFileRead = NULL, const CFX_DIBitmap* pMask = NULL, const CPDF_ImageSetParam* pParam = NULL);

    void					SetJpegImage(FX_BYTE* pImageData, FX_DWORD size);

    void					SetJpegImage(IFX_FileRead *pFile);

    void					ResetCache(CPDF_Page* pPage, const CFX_DIBitmap* pDIBitmap);

public:
    FX_BOOL					StartLoadDIBSource(CPDF_Dictionary* pFormResource, CPDF_Dictionary* pPageResource, FX_BOOL bStdCS = FALSE, FX_DWORD GroupFamily = 0, FX_BOOL bLoadMask = FALSE);
    FX_BOOL					Continue(IFX_Pause* pPause);
    CFX_DIBSource*			DetachBitmap();
    CFX_DIBSource*			DetachMask();
    CFX_DIBSource*			m_pDIBSource;
    CFX_DIBSource*			m_pMask;
    FX_DWORD				m_MatteColor;
private:

    CPDF_Stream*			m_pStream;
    FX_BOOL					m_bInline;
    CPDF_Dictionary*		m_pInlineDict;

    FX_INT32				m_Height;

    FX_INT32				m_Width;

    FX_BOOL					m_bIsMask;

    FX_BOOL					m_bInterpolate;

    CPDF_Document*			m_pDocument;

    CPDF_Dictionary*		m_pOC;
    CPDF_Dictionary*	InitJPEG(FX_LPBYTE pData, FX_DWORD size);
};
#endif

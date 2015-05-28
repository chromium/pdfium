// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_FPDFAPI_FPDF_PAGE_PAGEINT_H_
#define CORE_SRC_FPDFAPI_FPDF_PAGE_PAGEINT_H_

#include "../../../include/fpdfapi/fpdf_pageobj.h"

#define PARSE_STEP_LIMIT		100
#define STREAM_PARSE_BUFSIZE	20480
class CPDF_QuickFontCache;
class CPDF_StreamParser 
{
public:

    CPDF_StreamParser(const FX_BYTE* pData, FX_DWORD dwSize);
    ~CPDF_StreamParser();

    CPDF_Stream*		ReadInlineStream(CPDF_Document* pDoc, CPDF_Dictionary* pDict, CPDF_Object* pCSObj, FX_BOOL bDecode);
    typedef enum { EndOfData, Number, Keyword, Name, Others } SyntaxType;

    SyntaxType			ParseNextElement();
    FX_LPBYTE			GetWordBuf()
    {
        return m_WordBuffer;
    }
    FX_DWORD			GetWordSize()
    {
        return m_WordSize;
    }
    CPDF_Object*		GetObject()
    {
        CPDF_Object* pObj = m_pLastObj;
        m_pLastObj = NULL;
        return pObj;
    }
    FX_DWORD			GetPos()
    {
        return m_Pos;
    }
    void				SetPos(FX_DWORD pos)
    {
        m_Pos = pos;
    }

    CPDF_Object*		ReadNextObject(FX_BOOL bAllowNestedArray = FALSE, FX_BOOL bInArray = FALSE);
    void				SkipPathObject();
protected:
    void				GetNextWord(FX_BOOL& bIsNumber);
    CFX_ByteString		ReadString();
    CFX_ByteString		ReadHexString();
    const FX_BYTE*		m_pBuf;
    FX_DWORD			m_Size;
    FX_DWORD			m_Pos;
    FX_BYTE				m_WordBuffer[256];
    FX_DWORD			m_WordSize;
    CPDF_Object*		m_pLastObj;
};
typedef enum {
    PDFOP_CloseFillStrokePath = 0, PDFOP_FillStrokePath,
    PDFOP_CloseEOFillStrokePath, PDFOP_EOFillStrokePath,
    PDFOP_BeginMarkedContent_Dictionary, PDFOP_BeginImage,
    PDFOP_BeginMarkedContent, PDFOP_BeginText,
    PDFOP_BeginSectionUndefined, PDFOP_CurveTo_123,
    PDFOP_ConcatMatrix, PDFOP_SetColorSpace_Fill,
    PDFOP_SetColorSpace_Stroke, PDFOP_SetDash,
    PDFOP_SetCharWidth, PDFOP_SetCachedDevice,
    PDFOP_ExecuteXObject, PDFOP_MarkPlace_Dictionary,
    PDFOP_EndImage, PDFOP_EndMarkedContent,
    PDFOP_EndText, PDFOP_EndSectionUndefined,
    PDFOP_FillPath, PDFOP_FillPathOld,
    PDFOP_EOFillPath, PDFOP_SetGray_Fill,
    PDFOP_SetGray_Stroke, PDFOP_SetExtendGraphState,
    PDFOP_ClosePath, PDFOP_SetFlat,
    PDFOP_BeginImageData, PDFOP_SetLineJoin,
    PDFOP_SetLineCap, PDFOP_SetCMYKColor_Fill,
    PDFOP_SetCMYKColor_Stroke, PDFOP_LineTo,
    PDFOP_MoveTo, PDFOP_SetMiterLimit,
    PDFOP_MarkPlace, PDFOP_EndPath,
    PDFOP_SaveGraphState, PDFOP_RestoreGraphState,
    PDFOP_Rectangle, PDFOP_SetRGBColor_Fill,
    PDFOP_SetRGBColor_Stroke, PDFOP_SetRenderIntent,
    PDFOP_CloseStrokePath, PDFOP_StrokePath,
    PDFOP_SetColor_Fill, PDFOP_SetColor_Stroke,
    PDFOP_SetColorPS_Fill, PDFOP_SetColorPS_Stroke,
    PDFOP_ShadeFill, PDFOP_SetCharSpace,
    PDFOP_MoveTextPoint, PDFOP_MoveTextPoint_SetLeading,
    PDFOP_SetFont, PDFOP_ShowText,
    PDFOP_ShowText_Positioning, PDFOP_SetTextLeading,
    PDFOP_SetTextMatrix, PDFOP_SetTextRenderMode,
    PDFOP_SetTextRise, PDFOP_SetWordSpace,
    PDFOP_SetHorzScale, PDFOP_MoveToNextLine,
    PDFOP_CurveTo_23, PDFOP_SetLineWidth,
    PDFOP_Clip, PDFOP_EOClip,
    PDFOP_CurveTo_13, PDFOP_NextLineShowText,
    PDFOP_NextLineShowText_Space, PDFOP_Invalid
} PDFOP;
#define PARAM_BUF_SIZE	16
typedef struct {
    int			m_Type;
    union {
        struct {
            FX_BOOL		m_bInteger;
            union {
                int		m_Integer;
                FX_FLOAT m_Float;
            };
        } m_Number;
        CPDF_Object*	m_pObject;
        struct {
            int			m_Len;
            char		m_Buffer[32];
        } m_Name;
    };
} _ContentParam;
#define _FPDF_MAX_FORM_LEVEL_		30
#define _FPDF_MAX_TYPE3_FORM_LEVEL_	4
#define _FPDF_MAX_OBJECT_STACK_SIZE_ 512
class CPDF_StreamContentParser 
{
public:
    CPDF_StreamContentParser();
    ~CPDF_StreamContentParser();
    FX_BOOL Initialize();
    void	PrepareParse(CPDF_Document* pDoc, CPDF_Dictionary* pPageResources, CPDF_Dictionary* pParentResources,
                         CFX_AffineMatrix* pmtContentToUser,
                         CPDF_PageObjects* pObjList, CPDF_Dictionary* pResources,
                         CFX_FloatRect* pBBox, CPDF_ParseOptions* pOptions,
                         CPDF_AllStates* pAllStates, int level);
    CPDF_Document*		m_pDocument;
    CPDF_Dictionary*	m_pPageResources;
    CPDF_Dictionary*	m_pParentResources;
    CPDF_PageObjects*	m_pObjectList;
    CPDF_Dictionary*	m_pResources;
    int					m_Level;
    CFX_AffineMatrix	m_mtContentToUser;
    CFX_FloatRect		m_BBox;
    CPDF_ParseOptions	m_Options;
    _ContentParam		m_ParamBuf1[PARAM_BUF_SIZE];
    FX_DWORD			m_ParamStartPos;
    FX_DWORD			m_ParamCount;
    void				AddNumberParam(FX_LPCSTR str, int len);
    void				AddObjectParam(CPDF_Object* pObj);
    void				AddNameParam(FX_LPCSTR name, int size);
    int					GetNextParamPos();
    void				ClearAllParams();
    CPDF_Object*		GetObject(FX_DWORD index);
    CFX_ByteString		GetString(FX_DWORD index);
    FX_FLOAT			GetNumber(FX_DWORD index);
    FX_FLOAT		GetNumber16(FX_DWORD index);
    int					GetInteger(FX_DWORD index)
    {
        return (FX_INT32)(GetNumber(index));
    }
    FX_BOOL				OnOperator(FX_LPCSTR op);
    void				BigCaseCaller(int index);
    FX_BOOL				m_bAbort;
    CPDF_StreamParser*	m_pSyntax;
    FX_DWORD			GetParsePos()
    {
        return m_pSyntax->GetPos();
    }
    CPDF_AllStates*		m_pCurStates;
    CPDF_ContentMark	m_CurContentMark;
    CFX_PtrArray		m_ClipTextList;
    CPDF_TextObject*	m_pLastTextObject;
    FX_FLOAT			m_DefFontSize;
    void				AddTextObject(CFX_ByteString* pText, FX_FLOAT fInitKerning, FX_FLOAT* pKerning, int count);

    void				ConvertUserSpace(FX_FLOAT& x, FX_FLOAT& y);
    void				ConvertTextSpace(FX_FLOAT& x, FX_FLOAT& y);
    void				OnChangeTextMatrix();
    FX_DWORD			Parse(FX_LPCBYTE pData, FX_DWORD dwSize, FX_DWORD max_cost);
    void				ParsePathObject();
    int					m_CompatCount;
    FX_PATHPOINT*		m_pPathPoints;
    int					m_PathPointCount;
    int					m_PathAllocSize;
    FX_FLOAT			m_PathStartX, m_PathStartY;
    FX_FLOAT			m_PathCurrentX, m_PathCurrentY;
    int					m_PathClipType;
    void				AddPathPoint(FX_FLOAT x, FX_FLOAT y, int flag);
    void				AddPathRect(FX_FLOAT x, FX_FLOAT y, FX_FLOAT w, FX_FLOAT h);
    void				AddPathObject(int FillType, FX_BOOL bStroke);
    CPDF_ImageObject*	AddImage(CPDF_Stream* pStream, CPDF_Image* pImage, FX_BOOL bInline);
    void				AddDuplicateImage();
    void				AddForm(CPDF_Stream*);
    CFX_ByteString		m_LastImageName;
    CPDF_Image*			m_pLastImage;
    CFX_BinaryBuf		m_LastImageDict, m_LastImageData;
    CPDF_Dictionary*	m_pLastImageDict;
    CPDF_Dictionary*    m_pLastCloneImageDict;
    FX_BOOL				m_bReleaseLastDict;
    FX_BOOL				m_bSameLastDict;
    void				SetGraphicStates(CPDF_PageObject* pObj, FX_BOOL bColor, FX_BOOL bText, FX_BOOL bGraph);
    FX_BOOL				m_bColored;
    FX_FLOAT			m_Type3Data[6];
    FX_BOOL				m_bResourceMissing;
    CFX_PtrArray		m_StateStack;
    void				SaveStates(CPDF_AllStates*);
    void				RestoreStates(CPDF_AllStates*);
    CPDF_Font*			FindFont(const CFX_ByteString& name);
    CPDF_ColorSpace*	FindColorSpace(const CFX_ByteString& name);
    CPDF_Pattern*		FindPattern(const CFX_ByteString& name, FX_BOOL bShading);
    CPDF_Object*		FindResourceObj(FX_BSTR type, const CFX_ByteString& name);
    void Handle_CloseFillStrokePath();
    void Handle_FillStrokePath();
    void Handle_CloseEOFillStrokePath();
    void Handle_EOFillStrokePath();
    void Handle_BeginMarkedContent_Dictionary();
    void Handle_BeginImage();
    void Handle_BeginMarkedContent();
    void Handle_BeginText();
    void Handle_BeginSectionUndefined();
    void Handle_CurveTo_123();
    void Handle_ConcatMatrix();
    void Handle_SetColorSpace_Fill();
    void Handle_SetColorSpace_Stroke();
    void Handle_SetDash();
    void Handle_SetCharWidth();
    void Handle_SetCachedDevice();
    void Handle_ExecuteXObject();
    void Handle_MarkPlace_Dictionary();
    void Handle_EndImage();
    void Handle_EndMarkedContent();
    void Handle_EndText();
    void Handle_EndSectionUndefined();
    void Handle_FillPath();
    void Handle_FillPathOld();
    void Handle_EOFillPath();
    void Handle_SetGray_Fill();
    void Handle_SetGray_Stroke();
    void Handle_SetExtendGraphState();
    void Handle_ClosePath();
    void Handle_SetFlat();
    void Handle_BeginImageData();
    void Handle_SetLineJoin();
    void Handle_SetLineCap();
    void Handle_SetCMYKColor_Fill();
    void Handle_SetCMYKColor_Stroke();
    void Handle_LineTo();
    void Handle_MoveTo();
    void Handle_SetMiterLimit();
    void Handle_MarkPlace();
    void Handle_EndPath();
    void Handle_SaveGraphState();
    void Handle_RestoreGraphState();
    void Handle_Rectangle();
    void Handle_SetRGBColor_Fill();
    void Handle_SetRGBColor_Stroke();
    void Handle_SetRenderIntent();
    void Handle_CloseStrokePath();
    void Handle_StrokePath();
    void Handle_SetColor_Fill();
    void Handle_SetColor_Stroke();
    void Handle_SetColorPS_Fill();
    void Handle_SetColorPS_Stroke();
    void Handle_ShadeFill();
    void Handle_SetCharSpace();
    void Handle_MoveTextPoint();
    void Handle_MoveTextPoint_SetLeading();
    void Handle_SetFont();
    void Handle_ShowText();
    void Handle_ShowText_Positioning();
    void Handle_SetTextLeading();
    void Handle_SetTextMatrix();
    void Handle_SetTextRenderMode();
    void Handle_SetTextRise();
    void Handle_SetWordSpace();
    void Handle_SetHorzScale();
    void Handle_MoveToNextLine();
    void Handle_CurveTo_23();
    void Handle_SetLineWidth();
    void Handle_Clip();
    void Handle_EOClip();
    void Handle_CurveTo_13();
    void Handle_NextLineShowText();
    void Handle_NextLineShowText_Space();
    void Handle_Invalid();
};
class CPDF_ContentParser 
{
public:
    CPDF_ContentParser();
    ~CPDF_ContentParser();
    typedef enum { Ready, ToBeContinued, Done } ParseStatus;
    ParseStatus			GetStatus()
    {
        return m_Status;
    }
    void				Start(CPDF_Page* pPage, CPDF_ParseOptions* pOptions);
    void				Start(CPDF_Form* pForm, CPDF_AllStates* pGraphicStates, CFX_AffineMatrix* pParentMatrix,
                              CPDF_Type3Char* pType3Char, CPDF_ParseOptions* pOptions, int level);
    void				Continue(IFX_Pause* pPause);
    int					EstimateProgress();
protected:
    void				Clear();
    ParseStatus			m_Status;
    CPDF_PageObjects*	m_pObjects;
    FX_BOOL				m_bForm;
    CPDF_ParseOptions	m_Options;
    CPDF_Type3Char*		m_pType3Char;
    int					m_InternalStage;
    CPDF_StreamAcc*		m_pSingleStream;
    CPDF_StreamAcc**	m_pStreamArray;
    FX_DWORD			m_nStreams;
    FX_LPBYTE			m_pData;
    FX_DWORD			m_Size;
    class CPDF_StreamContentParser*	m_pParser;
    FX_DWORD			m_CurrentOffset;
    CPDF_StreamFilter*	m_pStreamFilter;
};
class CPDF_AllStates : public CPDF_GraphicStates
{
public:
    CPDF_AllStates();
    ~CPDF_AllStates();
    void	Copy(const CPDF_AllStates& src);
    void	ProcessExtGS(CPDF_Dictionary* pGS, CPDF_StreamContentParser* pParser);
    void	SetLineDash(CPDF_Array*, FX_FLOAT, FX_FLOAT scale);
    CFX_AffineMatrix		m_TextMatrix, m_CTM, m_ParentMatrix;
    FX_FLOAT				m_TextX, m_TextY, m_TextLineX, m_TextLineY;
    FX_FLOAT				m_TextLeading, m_TextRise, m_TextHorzScale;
};

template <class KeyType, class ValueType>
KeyType PDF_DocPageData_FindValue(const CFX_MapPtrTemplate<KeyType, CPDF_CountedObject<ValueType>*> &map, ValueType findValue, CPDF_CountedObject<ValueType>*& findData)
{
    FX_POSITION pos = map.GetStartPosition();
    while (pos) {
        KeyType findKey;
        map.GetNextAssoc(pos, findKey, findData);
        if (findData->m_Obj == findValue) {
            return findKey;
        }
    }
    findData = NULL;
    return (KeyType)(FX_UINTPTR)NULL;
}
template <class KeyType, class ValueType>
FX_BOOL PDF_DocPageData_Release(CFX_MapPtrTemplate<KeyType, CPDF_CountedObject<ValueType>*> &map, KeyType findKey, ValueType findValue, FX_BOOL bForce = FALSE)
{
    if (!findKey && !findValue) {
        return FALSE;
    }
    CPDF_CountedObject<ValueType>* findData = NULL;
    if (!findKey) {
        findKey = PDF_DocPageData_FindValue<KeyType, ValueType>(map, findValue, findData);
    } else if (!map.Lookup(findKey, findData)) {
        return FALSE;
    }
    if (findData && ((-- findData->m_nCount) == 0 || bForce)) {
        delete findData->m_Obj;
        delete findData;
        map.RemoveKey(findKey);
        return TRUE;
    }
    return FALSE;
}
class CPDF_DocPageData 
{
public:
    CPDF_DocPageData(CPDF_Document *pPDFDoc);
    ~CPDF_DocPageData();
    void                        Clear(FX_BOOL bRelease = FALSE);
    CPDF_Font*                  GetFont(CPDF_Dictionary* pFontDict, FX_BOOL findOnly);
    CPDF_Font*                  GetStandardFont(FX_BSTR fontName, CPDF_FontEncoding* pEncoding);
    void                        ReleaseFont(CPDF_Dictionary* pFontDict);
    CPDF_ColorSpace*            GetColorSpace(CPDF_Object* pCSObj, CPDF_Dictionary* pResources);
    CPDF_ColorSpace*            GetCopiedColorSpace(CPDF_Object* pCSObj);
    void                        ReleaseColorSpace(CPDF_Object* pColorSpace);
    CPDF_Pattern*               GetPattern(CPDF_Object* pPatternObj, FX_BOOL bShading, const CFX_AffineMatrix* matrix);
    void                        ReleasePattern(CPDF_Object* pPatternObj);
    CPDF_Image*                 GetImage(CPDF_Object* pImageStream);
    void                        ReleaseImage(CPDF_Object* pImageStream);
    CPDF_IccProfile*            GetIccProfile(CPDF_Stream* pIccProfileStream);
    void                        ReleaseIccProfile(CPDF_Stream* pIccProfileStream, CPDF_IccProfile* pIccProfile);
    CPDF_StreamAcc*             GetFontFileStreamAcc(CPDF_Stream* pFontStream);
    void                        ReleaseFontFileStreamAcc(CPDF_Stream* pFontStream, FX_BOOL bForce = FALSE);
    FX_BOOL                     IsForceClear() const {return m_bForceClear;}
    CPDF_CountedColorSpace*     FindColorSpacePtr(CPDF_Object* pCSObj) const;
    CPDF_CountedPattern*        FindPatternPtr(CPDF_Object* pPatternObj) const;

    CPDF_Document*              m_pPDFDoc;
    CPDF_FontMap                m_FontMap;
    CPDF_ColorSpaceMap          m_ColorSpaceMap;
    CPDF_PatternMap             m_PatternMap;
    CPDF_ImageMap               m_ImageMap;
    CPDF_IccProfileMap          m_IccProfileMap;
    CFX_MapByteStringToPtr      m_HashProfileMap;
    CPDF_FontFileMap            m_FontFileMap;
    FX_BOOL                     m_bForceClear;
};
class CPDF_Function 
{
public:
    static CPDF_Function*	Load(CPDF_Object* pFuncObj);
    virtual ~CPDF_Function();
    FX_BOOL		Call(FX_FLOAT* inputs, int ninputs, FX_FLOAT* results, int& nresults) const;
    int			CountInputs()
    {
        return m_nInputs;
    }
    int			CountOutputs()
    {
        return m_nOutputs;
    }
protected:
    CPDF_Function();
    int			m_nInputs, m_nOutputs;
    FX_FLOAT*	m_pDomains;
    FX_FLOAT*	m_pRanges;
    FX_BOOL		Init(CPDF_Object* pObj);
    virtual FX_BOOL	v_Init(CPDF_Object* pObj) = 0;
    virtual FX_BOOL	v_Call(FX_FLOAT* inputs, FX_FLOAT* results) const = 0;
};
class CPDF_IccProfile 
{
public:
    CPDF_IccProfile(FX_LPCBYTE pData, FX_DWORD dwSize);
    ~CPDF_IccProfile();
    FX_INT32 GetComponents() const { return m_nSrcComponents; }
    FX_BOOL					m_bsRGB;
    FX_LPVOID				m_pTransform;
private:
    FX_INT32                m_nSrcComponents;
};
class CPDF_DeviceCS : public CPDF_ColorSpace
{
public:
    CPDF_DeviceCS(int family);
    virtual FX_BOOL	GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    FX_BOOL	SetRGB(FX_FLOAT* pBuf, FX_FLOAT R, FX_FLOAT G, FX_FLOAT B) const;
    FX_BOOL	v_GetCMYK(FX_FLOAT* pBuf, FX_FLOAT& c, FX_FLOAT& m, FX_FLOAT& y, FX_FLOAT& k) const;
    FX_BOOL	v_SetCMYK(FX_FLOAT* pBuf, FX_FLOAT c, FX_FLOAT m, FX_FLOAT y, FX_FLOAT k) const;
    virtual void	TranslateImageLine(FX_LPBYTE pDestBuf, FX_LPCBYTE pSrcBuf, int pixels, int image_width, int image_height, FX_BOOL bTransMask = FALSE) const;
};
class CPDF_PatternCS : public CPDF_ColorSpace
{
public:
    CPDF_PatternCS();
    ~CPDF_PatternCS();
    virtual FX_BOOL		v_Load(CPDF_Document* pDoc, CPDF_Array* pArray);
    virtual FX_BOOL		GetRGB(FX_FLOAT* pBuf, FX_FLOAT& R, FX_FLOAT& G, FX_FLOAT& B) const;
    virtual CPDF_ColorSpace*	GetBaseCS() const
    {
        return m_pBaseCS;
    }
    CPDF_ColorSpace*	m_pBaseCS;
    CPDF_CountedColorSpace*	m_pCountedBaseCS;
};
#define	MAX_PAGE_OBJECTS_UNIFY_NAMING				4096
class CPDF_ResourceNaming 
{
public:
    struct _NamingState  {
        CFX_ByteString	m_Prefix;
        int				m_nIndex;
    };
    ~CPDF_ResourceNaming();
    CFX_ByteString		GetName(const CPDF_Dictionary* pResList, FX_LPCSTR szType);
protected:
    CFX_MapByteStringToPtr	m_NamingCache;
};

#endif  // CORE_SRC_FPDFAPI_FPDF_PAGE_PAGEINT_H_

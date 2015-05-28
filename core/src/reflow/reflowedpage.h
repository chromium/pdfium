// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_SRC_REFLOW_REFLOWEDPAGE_H_
#define CORE_SRC_REFLOW_REFLOWEDPAGE_H_

#include "../../include/reflow/reflowengine.h"

#define GET_SIGNED(a) ( (a)>0 ? a/a : (a==0 ? 0 : -a/a) )
class CRF_Data;
class CRF_LineData;
class CRF_CharData;
class CRF_PathData;
class CRF_ImageData;
class CRF_Table;
class CRF_AttrOperation;
class CRF_OperationDate;
class CPDF_ReflowedPage;
class CPDF_Rect;
typedef CFX_SegmentedArray<CRF_Data*> CRF_DataPtrArray;
class CRF_CharState;
typedef CFX_SegmentedArray<CRF_CharState> CRF_CharStateArray;
#define SST_GE		1
#define SST_BLSE	2
#define SST_ILSE	3
#define SST_IE		4
class CPDF_LayoutProcessor_Reflow : public IPDF_LayoutProcessor
{
public:
    CPDF_LayoutProcessor_Reflow();
    ~CPDF_LayoutProcessor_Reflow();
    void Init(FX_FLOAT TopIndent, FX_FLOAT fWidth, FX_FLOAT fHeight, CPDF_ReflowedPage* pReflowedPage, int flags, FX_FLOAT lineSpace);

    LayoutStatus	StartProcess(IPDF_LayoutElement* pElement, IFX_Pause* pPause, const CFX_AffineMatrix* pPDFMatrix = NULL);
    LayoutStatus	Continue();
    int				GetPosition();
protected:
    void	FitPageMode();
    void	ProcessElement(IPDF_LayoutElement* pElement, FX_FLOAT reflowWidth);
    FX_FLOAT GetElmWidth(IPDF_LayoutElement* pElement);
    CFX_FloatRect GetElmBBox(IPDF_LayoutElement* pElement);
    void	ProcessTable(FX_FLOAT dx);
    void	ProcessObjs(IPDF_LayoutElement* pElement, FX_FLOAT reflowWidth);
    void	ProcessObject(CPDF_PageObject* pObj, FX_FLOAT reflowWidth, CFX_AffineMatrix objMatrix);
    void	ProcessTextObject(CPDF_TextObject *pObj, FX_FLOAT reflowWidth, CFX_AffineMatrix objMatrix);
    void	ProcessPathObject(CPDF_PathObject *pObj, FX_FLOAT reflowWidth);
    void	ProcessUnitaryObjs(CPDF_PageObjects *pObjs, FX_FLOAT reflowWidth, CFX_AffineMatrix objMatrix);
    FX_INT32 LogicPreObj(CPDF_TextObject* pObj);
    int ProcessInsertObject(CPDF_TextObject* pObj, CFX_AffineMatrix formMatrix);
    FX_WCHAR GetPreChar();
    FX_BOOL IsSameTextObject(CPDF_TextObject* pTextObj1, CPDF_TextObject* pTextObj2);
    int GetCharWidth(FX_DWORD charCode, CPDF_Font* pFont) const;
    FX_BOOL	IsCanBreakAfter(FX_DWORD unicode);
    FX_BOOL	IsCanBreakBefore(FX_DWORD unicode);
    FX_INT32 GetElementTypes(LayoutType layoutType);
    void				CreateRFData(CPDF_PageObject* pObj, CFX_AffineMatrix* pMatrix = NULL);
    CRF_CharState*		GetCharState(CPDF_TextObject* pObj, CPDF_Font* pFont, FX_FLOAT fHeight, FX_ARGB color);
    FX_FLOAT		ConverWidth(FX_FLOAT width);
    void	AddData2CurrLine(CRF_Data* pData);
    void	AddTemp2CurrLine(int begin, int count );
    void	Transform(const CFX_AffineMatrix* pMatrix, CRF_Data* pData);
    void	Transform(const CFX_AffineMatrix* pMatrix, CRF_DataPtrArray* pDataArray, int beginPos, int count = 0);
    FX_FLOAT GetDatasWidth( int beginPos, int endpos);
    void	UpdateCurrLine();
    FX_BOOL	FinishedCurrLine();
    int m_flags;
    CFX_AffineMatrix m_PDFMatrix;
    LayoutStatus	m_Status;
    CPDF_TextObject* m_pPreObj;
    CFX_AffineMatrix m_perMatrix;
    IPDF_LayoutElement*	m_pLayoutElement;
    IPDF_LayoutElement* m_pRootElement;
    FX_FLOAT		m_CurrRefWidth;
    IFX_Pause*		m_pPause;
    LayoutEnum		m_CurrWritingMode;
    CPDF_ReflowedPage*	m_pReflowedPage;
    FX_FLOAT		m_fRefWidth;
    FX_FLOAT		m_TopIndent;
    FX_FLOAT		m_fLineSpace;
    FX_FLOAT		m_fScreenHeight;
    FX_FLOAT		m_fCurrMaxWidth;
    FX_FLOAT		m_fCurrLineWidth;
    FX_FLOAT		m_fCurrLineHeight;
    CRF_DataPtrArray*	m_pCurrLine;
    CRF_DataPtrArray*	m_pTempLine;
    FX_BOOL			m_bIllustration;
    FX_FLOAT		m_fLineHeight;
    LayoutEnum		m_TextAlign;
    FX_FLOAT		m_StartIndent;
    CFX_ArrayTemplate<CRF_Table*> m_TableArray;
    int				m_PausePosition;
};
struct RF_TableCell {
    int			m_BeginPos;
    int			m_EndPos;
    FX_FLOAT m_MaxWidth;
    FX_FLOAT m_PosX;
    FX_FLOAT	m_PosY;
    FX_FLOAT	m_CellWidth;
    FX_FLOAT m_CellHeight;
    int			m_RowSpan;
    int			m_ColSpan;
    LayoutEnum	m_BlockAlign;
    LayoutEnum	m_InlineAlign;
};
typedef CFX_ArrayTemplate<RF_TableCell*> CRF_TableCellArray;
class CRF_Table 
{
public:
    CRF_Table()
    {
        m_TableWidth = 0;
        m_nCol = 0;
    }
    CRF_TableCellArray  m_pCellArray;
    CFX_WordArray		m_nCell;
    int					m_nCol;
    FX_FLOAT			m_TableWidth;
    FX_FLOAT			m_ReflowPageHeight;
};
class CRF_CharState 
{
public:
    CPDF_Font*	m_pFont;
    FX_ARGB		m_Color;
    FX_BOOL		m_bVert;
    FX_FLOAT m_fFontSize;
    FX_FLOAT m_fAscent;
    FX_FLOAT m_fDescent;

    CPDF_TextObject*	m_pTextObj;
};
class CRF_PageInfo 
{
public:
    CRF_PageInfo(CPDF_PageObject* pPageObj, CRF_PageInfo* pParent = NULL)
        : m_pPageObj(pPageObj) , m_pParent(pParent)
    {
    }
    CPDF_PageObject* GetPageObj()
    {
        return m_pPageObj;
    }
    CPDF_Dictionary* GetFormDict()
    {
        if (NULL == m_pParent) {
            return NULL;
        }
        CPDF_PageObject* pParentObj = m_pParent->GetPageObj();
        if (NULL == pParentObj || PDFPAGE_FORM != pParentObj->m_Type) {
            return NULL;
        }
        return ((CPDF_FormObject*)pParentObj)->m_pForm->m_pResources;
    }
protected:
    CPDF_PageObject*		m_pPageObj;
    CRF_PageInfo*			m_pParent;
};
class CPDF_ReflowedPage : public IPDF_ReflowedPage, public CFX_PrivateData
{
public:

    CPDF_ReflowedPage(CFX_GrowOnlyPool*	pMemoryPool);
    ~CPDF_ReflowedPage();
    CFX_PrivateData*	GetPrivateDataCtrl()
    {
        return this;
    };
    void		GetDisplayMatrix(CFX_AffineMatrix& matrix, FX_INT32 xPos, FX_INT32 yPos, FX_INT32 xSize, FX_INT32 ySize, FX_INT32 iRotate, const CFX_AffineMatrix* pPageMatrix);

    FX_FLOAT	GetPageHeight() ;
    FX_FLOAT	GetPageWidth()
    {
        return m_PageWidth;
    };
    void		FocusGetData(const CFX_AffineMatrix matrix, FX_INT32 x, FX_INT32 y, CFX_ByteString& str);
    FX_BOOL		FocusGetPosition(const CFX_AffineMatrix matrix, CFX_ByteString str, FX_INT32& x, FX_INT32& y);
    CRF_DataPtrArray*	m_pReflowed;
    FX_FLOAT			m_PageWidth;
    FX_FLOAT			m_PageHeight;
    FX_BOOL				m_bWaiting;
    CRF_CharStateArray*	m_pCharState;
    CFX_GrowOnlyPool*	m_pMemoryPool;
    FX_BOOL				m_bCreateMemoryPool;
    CPDF_Page*			m_pPDFPage;
    FX_BOOL					RetainPageObjsMemberShip();
    void					MarkPageObjMemberShip(CPDF_PageObject* pObj, CRF_PageInfo* pParent);
    void					ReleasePageObjsMemberShip();
    CPDF_Dictionary*		GetFormResDict(CPDF_PageObject* pObj);

    CFX_MapPtrToPtr*		m_pPageInfos;
};
class CPDF_ProgressiveReflowPageParser : public IPDF_ProgressiveReflowPageParser
{
public:
    CPDF_ProgressiveReflowPageParser();
    ~CPDF_ProgressiveReflowPageParser() ;
    void			Init();

    ParseStatus		GetStatus()
    {
        return m_Status;
    };

    void			SetParserStyle(RF_ParseStyle style)
    {
        m_ParseStyle = style;
    };
    void			Start(IPDF_ReflowedPage* pReflowPage, CPDF_Page* pPage, FX_FLOAT TopIndent, FX_FLOAT fWidth, FX_FLOAT fHeight, IFX_Pause* pPause, int flags);
    void			Continue(IFX_Pause* pPause);
    int				GetPosition() ;

    void			Clear();
    ParseStatus		m_Status;
protected:
    RF_ParseStyle		m_ParseStyle;
    CPDF_Page*			m_pPDFPage;
    IFX_Pause*			m_pPause;
    CPDF_ReflowedPage*	m_pReflowPage;
    FX_FLOAT			m_TopIndent;
    FX_FLOAT			m_ReflowedWidth;
    FX_FLOAT			m_fScreenHeight;
    IPDF_LayoutProvider*	m_pProvider;
    IPDF_LayoutProcessor*	m_pReflowEngine;
    int					m_nObjProcessed;
    int m_flags;
};
class CPDF_ProgressiveReflowPageRender : public IPDF_ProgressiveReflowPageRender
{
public:
    CPDF_ProgressiveReflowPageRender();
    ~CPDF_ProgressiveReflowPageRender() ;

    RenderStatus			GetStatus()
    {
        return m_Status;
    };


    void		SetDisplayColor(FX_COLORREF color);
    void		Start(IPDF_ReflowedPage* pReflowPage, CFX_RenderDevice* pDevice, const CFX_AffineMatrix* pMatrix, IFX_Pause* pPause, int DitherBits);
    void		Continue(IFX_Pause* pPause);
    int			GetPosition();


    void				Clear();
protected:
    void				Display(IFX_Pause* pPause);
    RenderStatus m_Status;
    CPDF_ReflowedPage*	m_pReflowPage;
    CFX_AffineMatrix*	m_pDisplayMatrix;
    int					m_CurrNum;
    IFX_FontEncoding*	m_pFontEncoding;
    CFX_RenderDevice*	m_pFXDevice;
    int					m_DitherBits;
    FX_COLORREF			m_DisplayColor;
    typedef struct CRF_TextDataAtt {
        CRF_TextDataAtt()
        {
            pFont = NULL;
            fFontSize = 0.0f;
            Color = 0;
        }
        CRF_TextDataAtt(CPDF_Font* font, FX_FLOAT fontSize, FX_ARGB color)
        {
            pFont = font;
            fFontSize = fontSize;
            Color = color;
        }
        CPDF_Font*  pFont;
        FX_FLOAT    fFontSize;
        FX_ARGB		Color;
    } CRF_TEXTDATAATT;
    inline bool isTextDataAttSame(CRF_TEXTDATAATT data1, CRF_TEXTDATAATT data2)
    {
        if (data1.pFont != data2.pFont) {
            return false;
        }
        if (data1.Color != data2.Color) {
            return false;
        }
        if (fabs(data1.fFontSize - data2.fFontSize) > 0.0f) {
            return false;
        }
        return true;
    };
};
#define TYPE_UNKNOW		0
#define TYPE_TEXT		1
#define TYPE_PATH		2
#define TYPE_IMAGE		3
#define TYPE_LINE		4
class CRF_Data 
{
public:
    typedef enum {Unknow, Text, Image, Path, Line, paragraph} RF_DataType;
    CRF_Data()
    {
        m_Type = Unknow;
        m_Width = 0;
        m_PosY = 0;
        m_PosX = 0;
        m_Height = 0;
    }
    RF_DataType	GetType()
    {
        return m_Type;
    }
    virtual		~CRF_Data() {}
    RF_DataType 	m_Type;
    FX_FLOAT	m_PosX;
    FX_FLOAT	m_PosY;
    FX_FLOAT	m_Width;
    FX_FLOAT	m_Height;
};
class CRF_LineData : public CRF_Data
{
public:
    CRF_LineData()
    {
        m_Type = Line;
    }
};
class CRF_CharData : public CRF_Data
{
public:
    CRF_CharData()
    {
        m_Type = Text;
        m_CharCode = -1;
    }
    CRF_CharState*	m_pCharState;
    FX_DWORD		m_CharCode;
};
class CRF_ImageData : public CRF_Data
{
public:
    CRF_ImageData()
    {
        m_Type = Image;
        m_pBitmap = NULL;
    }
    ~CRF_ImageData()
    {
        if(m_pBitmap) {
            delete m_pBitmap;
        }
        m_pBitmap = NULL;
    }
    CFX_AffineMatrix m_Matrix;
    CFX_DIBitmap*	m_pBitmap;
};
class CRF_PathData : public CRF_Data
{
public:
    CRF_PathData()
    {
        m_Type = Path;
        m_bDecoration = FALSE;
    }
    ~CRF_PathData() {};
    FX_BOOL			m_bDecoration;
    CPDF_Path			m_pPathData;
    CFX_AffineMatrix	m_pPath2Device;
    CPDF_GraphState		m_pGraphState;
    FX_ARGB		m_fill_argb;
    FX_ARGB		m_stroke_argb;
    int			m_fill_mode;
};

#endif  // CORE_SRC_REFLOW_REFLOWEDPAGE_H_

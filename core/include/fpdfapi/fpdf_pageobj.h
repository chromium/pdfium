// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_PAGEOBJ_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_PAGEOBJ_H_

#include "../fxge/fx_ge.h"
#include "fpdf_resource.h"

class CPDF_Path;
class CPDF_ClipPathData;
class CPDF_ClipPath;
class CPDF_ColorStateData;
class CPDF_ColorState;
class CPDF_GraphState;
class CPDF_TextStateData;
class CPDF_TextState;
class CPDF_GeneralStateData;
class CPDF_GeneralState;
class CPDF_ContentMarkItem;
class CPDF_ContentMark;
class CPDF_GraphicStates;
class CPDF_PageObject;
class CPDF_TextObject;
class CPDF_PathObject;
class CPDF_ImageObject;
class CPDF_ShadingObject;
class CPDF_FormObject;
class CPDF_InlineImages;
typedef CFX_PathData CPDF_PathData;
class CPDF_Path : public CFX_CountRef<CFX_PathData>
{
public:




    int					GetPointCount()
    {
        return m_pObject->m_PointCount;
    }

    int					GetFlag(int index)
    {
        return m_pObject->m_pPoints[index].m_Flag;
    }

    FX_FLOAT			GetPointX(int index)
    {
        return m_pObject->m_pPoints[index].m_PointX;
    }

    FX_FLOAT			GetPointY(int index)
    {
        return m_pObject->m_pPoints[index].m_PointY;
    }




    FX_PATHPOINT*		GetPoints()
    {
        return m_pObject->m_pPoints;
    }


    CFX_FloatRect		GetBoundingBox() const
    {
        return m_pObject->GetBoundingBox();
    }

    CFX_FloatRect		GetBoundingBox(FX_FLOAT line_width, FX_FLOAT miter_limit) const
    {
        return m_pObject->GetBoundingBox(line_width, miter_limit);
    }

    void				Transform(const CFX_AffineMatrix* pMatrix)
    {
        GetModify()->Transform(pMatrix);
    }

    void				Append(CPDF_Path src, const CFX_AffineMatrix* pMatrix)
    {
        m_pObject->Append(src.m_pObject, pMatrix);
    }

    void				AppendRect(FX_FLOAT left, FX_FLOAT bottom, FX_FLOAT right, FX_FLOAT top)
    {
        m_pObject->AppendRect(left, bottom, right, top);
    }

    FX_BOOL				IsRect() const
    {
        return m_pObject->IsRect();
    }
};
class CPDF_ClipPathData 
{
public:

    CPDF_ClipPathData();

    CPDF_ClipPathData(const CPDF_ClipPathData&);

    ~CPDF_ClipPathData();

    void				SetCount(int path_count, int text_count);
public:

    int					m_PathCount;

    CPDF_Path*			m_pPathList;

    FX_BYTE*			m_pTypeList;

    int					m_TextCount;

    CPDF_TextObject**	m_pTextList;
};
class CPDF_ClipPath : public CFX_CountRef<CPDF_ClipPathData>
{
public:

    FX_DWORD			GetPathCount() const
    {
        return m_pObject->m_PathCount;
    }

    CPDF_Path			GetPath(int i) const
    {
        return m_pObject->m_pPathList[i];
    }

    int					GetClipType(int i) const
    {
        return m_pObject->m_pTypeList[i];
    }

    FX_DWORD			GetTextCount() const
    {
        return m_pObject->m_TextCount;
    }

    CPDF_TextObject*	GetText(int i) const
    {
        return m_pObject->m_pTextList[i];
    }

    CFX_FloatRect		GetClipBox() const;

    void				AppendPath(CPDF_Path path, int type, FX_BOOL bAutoMerge);

    void				DeletePath(int layer_index);

    void				AppendTexts(CPDF_TextObject** pTexts, int count);

    void				Transform(const CFX_AffineMatrix& matrix);
};
class CPDF_ColorStateData 
{
public:

    CPDF_ColorStateData(): m_FillRGB(0), m_StrokeRGB(0) {}

    CPDF_ColorStateData(const CPDF_ColorStateData& src);

    void				Default();

    CPDF_Color			m_FillColor;

    FX_DWORD			m_FillRGB;

    CPDF_Color			m_StrokeColor;

    FX_DWORD			m_StrokeRGB;
};
class CPDF_ColorState : public CFX_CountRef<CPDF_ColorStateData>
{
public:

    CPDF_Color*			GetFillColor() const
    {
        return m_pObject ? &m_pObject->m_FillColor : NULL;
    }

    CPDF_Color*			GetStrokeColor() const
    {
        return m_pObject ? &m_pObject->m_StrokeColor : NULL;
    }

    void				SetFillColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues);

    void				SetStrokeColor(CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues);

    void				SetFillPattern(CPDF_Pattern* pattern, FX_FLOAT* pValue, int nValues);

    void				SetStrokePattern(CPDF_Pattern* pattern, FX_FLOAT* pValue, int nValues);
private:
    void				SetColor(CPDF_Color& color, FX_DWORD& rgb, CPDF_ColorSpace* pCS, FX_FLOAT* pValue, int nValues);
};
typedef CFX_GraphStateData CPDF_GraphStateData;
class CPDF_GraphState : public CFX_CountRef<CFX_GraphStateData>
{
public:
};
class CPDF_TextStateData 
{
public:

    CPDF_TextStateData();

    CPDF_TextStateData(const CPDF_TextStateData& src);

    ~CPDF_TextStateData();

    CPDF_Font*			m_pFont;

    CPDF_Document*		m_pDocument;

    FX_FLOAT			m_FontSize;

    FX_FLOAT			m_CharSpace;

    FX_FLOAT			m_WordSpace;

    FX_FLOAT		m_Matrix[4];

    int					m_TextMode;

    FX_FLOAT		m_CTM[4];
};
class CPDF_TextState : public CFX_CountRef<CPDF_TextStateData>
{
public:

    CPDF_Font*			GetFont() const
    {
        return m_pObject->m_pFont;
    }

    void				SetFont(CPDF_Font* pFont);

    FX_FLOAT			GetFontSize() const
    {
        return m_pObject->m_FontSize;
    }

    FX_FLOAT*			GetMatrix() const
    {
        return m_pObject->m_Matrix;
    }



    FX_FLOAT			GetFontSizeV() const;

    FX_FLOAT			GetFontSizeH() const;

    FX_FLOAT			GetBaselineAngle() const;

    FX_FLOAT			GetShearAngle() const;

};
class CPDF_TransferFunc;
class CPDF_GeneralStateData 
{
public:

    CPDF_GeneralStateData();

    CPDF_GeneralStateData(const CPDF_GeneralStateData& src);
    ~CPDF_GeneralStateData();

    void				SetBlendMode(FX_BSTR blend_mode);

    char				m_BlendMode[16];

    int					m_BlendType;

    CPDF_Object*		m_pSoftMask;

    FX_FLOAT			m_SMaskMatrix[6];

    FX_FLOAT			m_StrokeAlpha;

    FX_FLOAT			m_FillAlpha;

    CPDF_Object*		m_pTR;

    CPDF_TransferFunc*	m_pTransferFunc;

    CFX_Matrix			m_Matrix;

    int					m_RenderIntent;

    FX_BOOL				m_StrokeAdjust;

    FX_BOOL				m_AlphaSource;

    FX_BOOL				m_TextKnockout;

    FX_BOOL				m_StrokeOP;

    FX_BOOL				m_FillOP;

    int					m_OPMode;

    CPDF_Object*		m_pBG;

    CPDF_Object*		m_pUCR;

    CPDF_Object*		m_pHT;

    FX_FLOAT			m_Flatness;

    FX_FLOAT			m_Smoothness;
};
class CPDF_GeneralState : public CFX_CountRef<CPDF_GeneralStateData>
{
public:

    void				SetRenderIntent(const CFX_ByteString& ri);

    int					GetBlendType() const
    {
        return m_pObject ? m_pObject->m_BlendType : FXDIB_BLEND_NORMAL;
    }

    int					GetAlpha(FX_BOOL bStroke) const
    {
        return m_pObject ? FXSYS_round((bStroke ? m_pObject->m_StrokeAlpha : m_pObject->m_FillAlpha) * 255) : 255;
    }
};
class CPDF_ContentMarkItem 
{
public:

    typedef enum {
        None,
        PropertiesDict,
        DirectDict,
        MCID
    } ParamType;

    CPDF_ContentMarkItem();

    CPDF_ContentMarkItem(const CPDF_ContentMarkItem& src);

    ~CPDF_ContentMarkItem();

    inline const CFX_ByteString&	GetName() const
    {
        return m_MarkName;
    }

    inline ParamType	GetParamType() const
    {
        return m_ParamType;
    }

    inline void*		GetParam() const
    {
        return m_pParam;
    }

    inline FX_BOOL		HasMCID() const;

    inline void			SetName(const CFX_ByteString& name)
    {
        m_MarkName = name;
    }

    inline void			SetParam(ParamType type, void* param)
    {
        m_ParamType = type;
        m_pParam = param;
    }
private:

    CFX_ByteString		m_MarkName;

    ParamType			m_ParamType;

    void*				m_pParam;
};
class CPDF_ContentMarkData 
{
public:

    CPDF_ContentMarkData() { }

    CPDF_ContentMarkData(const CPDF_ContentMarkData& src);

    inline int			CountItems() const
    {
        return m_Marks.GetSize();
    }

    inline CPDF_ContentMarkItem&	GetItem(int index) const
    {
        return m_Marks[index];
    }

    int					GetMCID() const;

    void				AddMark(const CFX_ByteString& name, CPDF_Dictionary* pDict, FX_BOOL bDictNeedClone);

    void				DeleteLastMark();
private:

    CFX_ObjectArray<CPDF_ContentMarkItem>	m_Marks;
};
class CPDF_ContentMark : public CFX_CountRef<CPDF_ContentMarkData>
{
public:

    int					GetMCID() const
    {
        return m_pObject ? m_pObject->GetMCID() : -1;
    }

    FX_BOOL				HasMark(FX_BSTR mark) const;

    FX_BOOL				LookupMark(FX_BSTR mark, CPDF_Dictionary*& pDict) const;
};
#define PDFPAGE_TEXT		1
#define PDFPAGE_PATH		2
#define PDFPAGE_IMAGE		3
#define PDFPAGE_SHADING		4
#define PDFPAGE_FORM		5
#define PDFPAGE_INLINES		6
class CPDF_GraphicStates 
{
public:

    void				CopyStates(const CPDF_GraphicStates& src);

    void				DefaultStates();

    CPDF_ClipPath		m_ClipPath;

    CPDF_GraphState		m_GraphState;

    CPDF_ColorState		m_ColorState;

    CPDF_TextState		m_TextState;

    CPDF_GeneralState	m_GeneralState;
};
class CPDF_PageObject : public CPDF_GraphicStates
{
public:
    static CPDF_PageObject* Create(int type);
    virtual ~CPDF_PageObject();

    CPDF_PageObject*	Clone() const;

    void				Copy(const CPDF_PageObject* pSrcObject);

    virtual void		Transform(const CFX_AffineMatrix& matrix) = 0;



    void				RemoveClipPath();

    void				AppendClipPath(CPDF_Path path, int type, FX_BOOL bAutoMerge);

    void				CopyClipPath(CPDF_PageObject* pObj);

    void				TransformClipPath(CFX_AffineMatrix& matrix);

    void				TransformGeneralState(CFX_AffineMatrix& matrix);


    void				SetColorState(CPDF_ColorState state)
    {
        m_ColorState = state;
    }

    FX_RECT				GetBBox(const CFX_AffineMatrix* pMatrix) const;

    int					m_Type;

    FX_FLOAT			m_Left;

    FX_FLOAT			m_Right;

    FX_FLOAT			m_Top;

    FX_FLOAT			m_Bottom;

    CPDF_ContentMark	m_ContentMark;
protected:

    virtual void		CopyData(const CPDF_PageObject* pSrcObject) {}

    void				RecalcBBox();

    CPDF_PageObject() {}

};
struct CPDF_TextObjectItem  {

    FX_DWORD			m_CharCode;

    FX_FLOAT			m_OriginX;

    FX_FLOAT			m_OriginY;
};
class CPDF_TextObject : public CPDF_PageObject
{
public:

    CPDF_TextObject();

    virtual ~CPDF_TextObject();

    int					CountItems() const
    {
        return m_nChars;
    }

    void				GetItemInfo(int index, CPDF_TextObjectItem* pInfo) const;

    int					CountChars() const;

    void				GetCharInfo(int index, FX_DWORD& charcode, FX_FLOAT& kerning) const;
    void				GetCharInfo(int index, CPDF_TextObjectItem* pInfo) const;

    void				GetCharRect(int index, CFX_FloatRect& rect) const;


    FX_FLOAT			GetCharWidth(FX_DWORD charcode) const;
    FX_FLOAT			GetSpaceCharWidth() const;

    FX_FLOAT			GetPosX() const
    {
        return m_PosX;
    }

    FX_FLOAT			GetPosY() const
    {
        return m_PosY;
    }

    void				GetTextMatrix(CFX_AffineMatrix* pMatrix) const;

    CPDF_Font*			GetFont() const
    {
        return m_TextState.GetFont();
    }

    FX_FLOAT			GetFontSize() const
    {
        return m_TextState.GetFontSize();
    }

    void				SetEmpty();

    void				SetText(const CFX_ByteString& text);

    void				SetText(CFX_ByteString* pStrs, FX_FLOAT* pKerning, int nSegs);

    void				SetText(int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pKernings);

    void				SetPosition(FX_FLOAT x, FX_FLOAT y);

    void				SetTextState(CPDF_TextState TextState);
    virtual void		Transform(const CFX_AffineMatrix& matrix);

    void				CalcCharPos(FX_FLOAT* pPosArray) const;



    void				SetData(int nChars, FX_DWORD* pCharCodes, FX_FLOAT* pCharPos, FX_FLOAT x, FX_FLOAT y);

    void				GetData(int& nChars, FX_DWORD*& pCharCodes, FX_FLOAT*& pCharPos)
    {
        nChars = m_nChars;
        pCharCodes = m_pCharCodes;
        pCharPos = m_pCharPos;
    }


    void				RecalcPositionData()
    {
        CalcPositionData(NULL, NULL, 1);
    }
protected:
    virtual void		CopyData(const CPDF_PageObject* pSrcObject);

    FX_FLOAT			m_PosX;

    FX_FLOAT			m_PosY;

    int					m_nChars;

    FX_DWORD*			m_pCharCodes;

    FX_FLOAT*		m_pCharPos;

    void				SetSegments(const CFX_ByteString* pStrs, FX_FLOAT* pKerning, int nSegs);

    void				CalcPositionData(FX_FLOAT* pTextAdvanceX, FX_FLOAT* pTextAdvanceY, FX_FLOAT horz_scale, int level = 0);
    friend class		CPDF_StreamContentParser;
    friend class		CPDF_RenderStatus;
    friend class		CPDF_QuickDrawer;
    friend class		CPDF_TextRenderer;
    friend class		CTextPage;
    friend class		CPDF_ContentGenerator;
};
class CPDF_PathObject : public CPDF_PageObject
{
public:

    CPDF_PathObject()
    {
        m_Type = PDFPAGE_PATH;
    }

    virtual ~CPDF_PathObject() {}
    virtual void		Transform(const CFX_AffineMatrix& maxtrix);

    void				SetGraphState(CPDF_GraphState GraphState);

    CPDF_Path			m_Path;

    int					m_FillType;

    FX_BOOL				m_bStroke;

    CFX_AffineMatrix	m_Matrix;


    void				CalcBoundingBox();
protected:
    virtual void		CopyData(const CPDF_PageObject* pSrcObjet);
};
class CPDF_ImageObject : public CPDF_PageObject
{
public:

    CPDF_ImageObject();

    virtual ~CPDF_ImageObject();
    virtual void		Transform(const CFX_AffineMatrix& matrix);

    CPDF_Image*			m_pImage;

    CFX_AffineMatrix	m_Matrix;

    void				CalcBoundingBox();
private:
    virtual void		CopyData(const CPDF_PageObject* pSrcObjet);
};
class CPDF_ShadingObject : public CPDF_PageObject
{
public:

    CPDF_ShadingObject();

    virtual ~CPDF_ShadingObject();

    CPDF_ShadingPattern*	m_pShading;

    CFX_AffineMatrix	m_Matrix;

    CPDF_Page*			m_pPage;
    virtual void		Transform(const CFX_AffineMatrix& matrix);

    void				CalcBoundingBox();
protected:
    virtual void		CopyData(const CPDF_PageObject* pSrcObjet);
};
class CPDF_FormObject : public CPDF_PageObject
{
public:

    CPDF_FormObject()
    {
        m_Type = PDFPAGE_FORM;
        m_pForm = NULL;
    }

    virtual ~CPDF_FormObject();
    virtual void		Transform(const CFX_AffineMatrix& matrix);

    CPDF_Form*			m_pForm;

    CFX_AffineMatrix	m_FormMatrix;

    void				CalcBoundingBox();
protected:
    virtual void		CopyData(const CPDF_PageObject* pSrcObjet);
};
class CPDF_InlineImages : public CPDF_PageObject
{
public:

    CPDF_InlineImages();

    virtual ~CPDF_InlineImages();

    CPDF_Stream*		m_pStream;

    CFX_DIBitmap*		m_pBitmap;

    CFX_ArrayTemplate<CFX_AffineMatrix>	m_Matrices;

    void				AddMatrix(CFX_AffineMatrix& matrix);
protected:
    virtual void		Transform(const CFX_AffineMatrix& matrix) {}
    virtual void		CopyData(const CPDF_PageObject* pSrcObjet) {}
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_PAGEOBJ_H_

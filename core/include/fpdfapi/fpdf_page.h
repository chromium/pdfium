// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_PAGE_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_PAGE_H_

#include "../fxge/fx_dib.h"
#include "fpdf_parser.h"
#include "fpdf_resource.h"

class CPDF_PageObjects;
class CPDF_Page;
class CPDF_Form;
class CPDF_ParseOptions;
class CPDF_PageObject;
class CPDF_PageRenderCache;
class CPDF_StreamFilter;
class CPDF_AllStates;
class CPDF_ContentParser;
class CPDF_StreamContentParser;
class CPDF_ResourceNaming;
#define PDFTRANS_GROUP			0x0100
#define PDFTRANS_ISOLATED		0x0200
#define PDFTRANS_KNOCKOUT		0x0400
#define PDF_CONTENT_NOT_PARSED	0
#define PDF_CONTENT_PARSING		1
#define PDF_CONTENT_PARSED		2
class CPDF_PageObjects 
{
public:

    CPDF_PageObjects(FX_BOOL bReleaseMembers = TRUE);

    ~CPDF_PageObjects();




    void				ContinueParse(IFX_Pause* pPause);

    int					GetParseState() const
    {
        return m_ParseState;
    }

    FX_BOOL				IsParsed() const
    {
        return m_ParseState == PDF_CONTENT_PARSED;
    }

    int					EstimateParseProgress() const;




    FX_POSITION			GetFirstObjectPosition() const
    {
        return m_ObjectList.GetHeadPosition();
    }

    FX_POSITION			GetLastObjectPosition() const
    {
        return m_ObjectList.GetTailPosition();
    }

    CPDF_PageObject*	GetNextObject(FX_POSITION& pos) const
    {
        return (CPDF_PageObject*)m_ObjectList.GetNext(pos);
    }

    CPDF_PageObject*	GetPrevObject(FX_POSITION& pos) const
    {
        return (CPDF_PageObject*)m_ObjectList.GetPrev(pos);
    }

    CPDF_PageObject*	GetObjectAt(FX_POSITION pos) const
    {
        return (CPDF_PageObject*)m_ObjectList.GetAt(pos);
    }

    FX_DWORD			CountObjects() const
    {
        return m_ObjectList.GetCount();
    }

    int					GetObjectIndex(CPDF_PageObject* pObj) const;

    CPDF_PageObject*	GetObjectByIndex(int index) const;





    FX_POSITION			InsertObject(FX_POSITION posInsertAfter, CPDF_PageObject* pNewObject);

    void				Transform(const CFX_AffineMatrix& matrix);

    FX_BOOL				BackgroundAlphaNeeded() const
    {
        return m_bBackgroundAlphaNeeded;
    }

    CFX_FloatRect		CalcBoundingBox() const;

    CPDF_Dictionary*	m_pFormDict;

    CPDF_Stream*		m_pFormStream;

    CPDF_Document*		m_pDocument;

    CPDF_Dictionary*	m_pPageResources;

    CPDF_Dictionary*	m_pResources;

    CFX_FloatRect		m_BBox;

    int					m_Transparency;

protected:
    friend class		CPDF_ContentParser;
    friend class		CPDF_StreamContentParser;
    friend class		CPDF_AllStates;

    CFX_PtrList			m_ObjectList;

    FX_BOOL				m_bBackgroundAlphaNeeded;

    FX_BOOL				m_bReleaseMembers;
    void				LoadTransInfo();
    void                ClearCacheObjects();

    CPDF_ContentParser*	m_pParser;

    FX_BOOL				m_ParseState;
};
class CPDF_Page : public CPDF_PageObjects, public CFX_PrivateData
{
public:

    CPDF_Page();

    ~CPDF_Page();

    void				Load(CPDF_Document* pDocument, CPDF_Dictionary* pPageDict, FX_BOOL bPageCache = TRUE);

    void				StartParse(CPDF_ParseOptions* pOptions = NULL, FX_BOOL bReParse = FALSE);

    void				ParseContent(CPDF_ParseOptions* pOptions = NULL, FX_BOOL bReParse = FALSE);

    void				GetDisplayMatrix(CFX_AffineMatrix& matrix, int xPos, int yPos,
                                         int xSize, int ySize, int iRotate) const;

    FX_FLOAT			GetPageWidth() const
    {
        return m_PageWidth;
    }

    FX_FLOAT			GetPageHeight() const
    {
        return m_PageHeight;
    }

    CFX_FloatRect		GetPageBBox() const
    {
        return m_BBox;
    }

    const CFX_AffineMatrix&	GetPageMatrix() const
    {
        return m_PageMatrix;
    }

    CPDF_Object*		GetPageAttr(FX_BSTR name) const;



    CPDF_PageRenderCache*	GetRenderCache() const
    {
        return m_pPageRender;
    }

    void				ClearRenderCache();

protected:
    friend class		CPDF_ContentParser;

    FX_FLOAT			m_PageWidth;

    FX_FLOAT			m_PageHeight;

    CFX_AffineMatrix	m_PageMatrix;

    CPDF_PageRenderCache*	m_pPageRender;
};
class CPDF_ParseOptions 
{
public:

    CPDF_ParseOptions();

    FX_BOOL				m_bTextOnly;

    FX_BOOL				m_bMarkedContent;

    FX_BOOL				m_bSeparateForm;

    FX_BOOL				m_bDecodeInlineImage;
};
class CPDF_Form : public CPDF_PageObjects
{
public:

    CPDF_Form(CPDF_Document* pDocument, CPDF_Dictionary* pPageResources, CPDF_Stream* pFormStream, CPDF_Dictionary* pParentResources = NULL);

    ~CPDF_Form();

    void				StartParse(CPDF_AllStates* pGraphicStates,  CFX_AffineMatrix* pParentMatrix,
                                   CPDF_Type3Char* pType3Char, CPDF_ParseOptions* pOptions, int level = 0);

    void				ParseContent(CPDF_AllStates* pGraphicStates, CFX_AffineMatrix* pParentMatrix,
                                     CPDF_Type3Char* pType3Char, CPDF_ParseOptions* pOptions, int level = 0);

    CPDF_Form*			Clone() const;
};
class CPDF_PageContentGenerate 
{
public:
    CPDF_PageContentGenerate(CPDF_Page* pPage);
    ~CPDF_PageContentGenerate();
    FX_BOOL InsertPageObject(CPDF_PageObject* pPageObject);
    void GenerateContent();
    void TransformContent(CFX_Matrix& matrix);
protected:
    void            ProcessImage(CFX_ByteTextBuf& buf, CPDF_ImageObject* pImageObj);
    void            ProcessForm(CFX_ByteTextBuf& buf, FX_LPCBYTE data, FX_DWORD size, CFX_Matrix& matrix);
    CFX_ByteString  RealizeResource(CPDF_Object* pResourceObj, const FX_CHAR* szType);
private:
    CPDF_Page*      m_pPage;
    CPDF_Document*  m_pDocument;
    CFX_PtrArray    m_pageObjects;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_PAGE_H_

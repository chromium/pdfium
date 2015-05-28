// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_REFLOW_REFLOWENGINE_H_
#define CORE_INCLUDE_REFLOW_REFLOWENGINE_H_

#include "../fpdfapi/fpdf_pageobj.h"
#include "../fpdfapi/fpdf_render.h"
#include "../fpdfdoc/fpdf_tagged.h"
#include "../fpdftext/fpdf_text.h"
#include "fpdf_layout.h"

class IPDF_ReflowedPage
{
public:
    static IPDF_ReflowedPage* Create();

    virtual ~IPDF_ReflowedPage() {}
    virtual CFX_PrivateData*	GetPrivateDataCtrl() = 0;


    virtual void		GetDisplayMatrix(CFX_AffineMatrix& matrix, FX_INT32 xPos, FX_INT32 yPos, FX_INT32 xSize, FX_INT32 ySize, FX_INT32 iRotate, const CFX_AffineMatrix* pPageMatrix) = 0;
    virtual FX_FLOAT	GetPageHeight() = 0;
    virtual FX_FLOAT	GetPageWidth() = 0;
    virtual void		FocusGetData(const CFX_AffineMatrix matrix, FX_INT32 x, FX_INT32 y, CFX_ByteString& str) = 0;
    virtual FX_BOOL		FocusGetPosition(const CFX_AffineMatrix matrix, CFX_ByteString str, FX_INT32& x, FX_INT32& y) = 0;
};
typedef struct _RF_ParseStyle {
    _RF_ParseStyle()
    {
        m_LineSpace = 0;
    };
    FX_FLOAT m_LineSpace;
} RF_ParseStyle;
class IPDF_ProgressiveReflowPageParser
{
public:
    static IPDF_ProgressiveReflowPageParser* Create();
    static FX_BOOL	IsTaggedPage(CPDF_PageObjects*pPage);

    virtual ~IPDF_ProgressiveReflowPageParser() {}
    typedef enum { Ready, ToBeContinued, Done, Failed } ParseStatus;

    virtual ParseStatus		GetStatus() = 0;
    virtual void			SetParserStyle(RF_ParseStyle style) = 0;
    virtual void			Start(IPDF_ReflowedPage* pReflowPage, CPDF_Page* pPage, FX_FLOAT TopIndent, FX_FLOAT fWidth, FX_FLOAT fHeight, IFX_Pause* pPause, int flags) = 0;
    virtual void			Continue(IFX_Pause* pPause) = 0;

    virtual int				GetPosition() = 0;


    virtual void			Clear() = 0;
};
class IPDF_ProgressiveReflowPageRender
{
public:
    static IPDF_ProgressiveReflowPageRender* Create();

    virtual ~IPDF_ProgressiveReflowPageRender() {}
    typedef enum { Ready, ToBeContinued, Waiting, Done, Failed } RenderStatus;

    virtual RenderStatus	GetStatus() = 0;


    virtual void		Start(IPDF_ReflowedPage* pReflowPage, CFX_RenderDevice* pDevice, const CFX_AffineMatrix* pMatrix, IFX_Pause* pPause, int DitherBits ) = 0;
    virtual void		Continue(IFX_Pause* pPause) = 0;
    virtual int			GetPosition() = 0;


    virtual void				Clear() = 0;
};
IPDF_ReflowedPage* Create_ReflowPage();
IPDF_ProgressiveReflowPageParser* Create_ReflowPageParser();
IPDF_ProgressiveReflowPageRender* Create_ReflowPageRender();

#endif  // CORE_INCLUDE_REFLOW_REFLOWENGINE_H_

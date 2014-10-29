// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef	_FXFA_FORMFILLER_RENDERCONTEXT_IMP_H
#define _FXFA_FORMFILLER_RENDERCONTEXT_IMP_H
class CXFA_RenderContext : public IXFA_RenderContext, public CFX_Object
{
public:
    CXFA_RenderContext();
    virtual ~CXFA_RenderContext();
    virtual void		Release()
    {
        delete this;
    }
    virtual FX_INT32	StartRender(IXFA_PageView* pPageView, CFX_Graphics* pGS, const CFX_Matrix& matrix, const CXFA_RenderOptions& options);
    virtual FX_INT32	DoRender(IFX_Pause* pPause = NULL);
    virtual	void		StopRender();
protected:
    IXFA_WidgetIterator* m_pWidgetIterator;
    XFA_HWIDGET			 m_pWidget;
    IXFA_PageView*		 m_pPageView;
    CFX_Graphics*		 m_pGS;
    CFX_Matrix			 m_matrix;
    CXFA_RenderOptions	 m_options;
    FX_DWORD			 m_dwStatus;
    CFX_RectF			 m_rtClipRect;
};
#endif

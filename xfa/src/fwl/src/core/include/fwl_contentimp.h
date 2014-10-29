// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_CONTENT_IMP_H
#define _FWL_CONTENT_IMP_H
class CFWL_WidgetImp;
class IFWL_Widget;
class CFWL_ContentImp;
class CFWL_ContentImp : public CFWL_WidgetImp
{
public:
    CFWL_ContentImp();
    CFWL_ContentImp(const CFWL_WidgetImpProperties &properties);
    virtual ~CFWL_ContentImp();
    virtual FWL_ERR				InsertWidget(IFWL_Widget *pChild, FX_INT32 nIndex = -1);
    virtual FWL_ERR				RemoveWidget(IFWL_Widget *pWidget);
    virtual	FWL_ERR				RemoveAllWidgets();
    virtual FWL_ERR		GetMinSize(FX_FLOAT &fWidth, FX_FLOAT &fHeight);
    virtual FWL_ERR		SetMinSize(FX_FLOAT fWidth, FX_FLOAT fHeight);
    virtual	FWL_ERR		GetMaxSize(FX_FLOAT &fWidth, FX_FLOAT &fHeight);
    virtual FWL_ERR		SetMaxSize(FX_FLOAT fWidth, FX_FLOAT fHeight);
protected:
    FX_FLOAT m_fWidthMin;
    FX_FLOAT m_fWidthMax;
    FX_FLOAT m_fHeightMax;
    FX_FLOAT m_fHeightMin;
};
#endif

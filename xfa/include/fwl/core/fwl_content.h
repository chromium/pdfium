// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_CONTENT_H
#define _FWL_CONTENT_H
class IFWL_Widget;
class IFWL_Content;
class IFWL_Content : public IFWL_Widget
{
public:
    static IFWL_Content* Create();
    FWL_ERR		Initialize();
    FWL_ERR		InsertWidget(IFWL_Widget *pChild, FX_INT32 nIndex = -1);
    FWL_ERR		RemoveWidget(IFWL_Widget *pWidget);
    FWL_ERR		RemoveAllWidgets();
    FWL_ERR		GetMinSize(FX_FLOAT &fWidth, FX_FLOAT &fHeight);
    FWL_ERR		SetMinSize(FX_FLOAT fWidth, FX_FLOAT fHeight);
    FWL_ERR		GetMaxSize(FX_FLOAT &fWidth, FX_FLOAT &fHeight);
    FWL_ERR		SetMaxSize(FX_FLOAT fWidth, FX_FLOAT fHeight);
protected:
    IFWL_Content();
    virtual ~IFWL_Content();
};
#endif

// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PANEL_IMP_H
#define _FWL_PANEL_IMP_H
class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class IFWL_Widget;
class IFWL_Content;
class CFWL_PanelImp;
class CFWL_PanelImp : public CFWL_WidgetImp {
 public:
  CFWL_PanelImp(const CFWL_WidgetImpProperties& properties,
                IFWL_Widget* pOuter);
  virtual ~CFWL_PanelImp();
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;
  virtual FWL_ERR GetWidgetRect(CFX_RectF& rect, FX_BOOL bAutoSize = FALSE);
  virtual FWL_ERR Update();
  virtual IFWL_Content* GetContent();
  virtual FWL_ERR SetContent(IFWL_Content* pContent);

 protected:
  IFWL_Content* m_pContent;
};
#endif

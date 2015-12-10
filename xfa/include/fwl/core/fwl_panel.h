// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PANEL_H
#define _FWL_PANEL_H
class IFWL_Widget;
class IFWL_Content;
class IFWL_Panel;
class IFWL_CustomPanel;
#define FWL_CLASS_Panel L"FWL_Panel"
#define FWL_CLASSHASH_Panel 881567292

class IFWL_Panel : public IFWL_Widget {
 public:
  static IFWL_Panel* Create(CFWL_WidgetImpProperties& properties,
                            IFWL_Widget* pOuter);

  IFWL_Content* GetContent();
  FWL_ERR SetContent(IFWL_Content* pContent);

 protected:
  IFWL_Panel();
};

class IFWL_CustomPanel : public IFWL_Widget {
 public:
  static IFWL_CustomPanel* Create(CFWL_WidgetImpProperties& properties,
                                  IFWL_Widget* pOuter);

  IFWL_Content* GetContent();
  FWL_ERR SetContent(IFWL_Content* pContent);
  FWL_ERR SetProxy(IFWL_Proxy* pProxy);

 protected:
  IFWL_CustomPanel();
};
#endif

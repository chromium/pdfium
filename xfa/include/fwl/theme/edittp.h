// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_EDITTP_H
#define _FWL_EDITTP_H
class CFWL_WidgetTP;
class CFWL_EditTP;
class CFWL_EditTP : public CFWL_WidgetTP {
 public:
  CFWL_EditTP();
  virtual ~CFWL_EditTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
};
#endif

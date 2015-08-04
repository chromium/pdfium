// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_PICTUREBOXTP_H
#define _FWL_PICTUREBOXTP_H
class CFWL_WidgetTP;
class CFWL_PictureBoxTP;
class CFWL_PictureBoxTP : public CFWL_WidgetTP {
 public:
  CFWL_PictureBoxTP();
  virtual ~CFWL_PictureBoxTP();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
};
#endif

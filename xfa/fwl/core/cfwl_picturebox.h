// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_PICTUREBOX_H_
#define XFA_FWL_CORE_CFWL_PICTUREBOX_H_

#include "xfa/fwl/core/cfwl_widget.h"
#include "xfa/fwl/core/ifwl_picturebox.h"
#include "xfa/fwl/core/ifwl_widget.h"

class CFWL_PictureBox : public CFWL_Widget, public IFWL_Widget::DataProvider {
 public:
  explicit CFWL_PictureBox(const CFWL_App* pApp);
  ~CFWL_PictureBox() override;

  void Initialize();
};

#endif  // XFA_FWL_CORE_CFWL_PICTUREBOX_H_

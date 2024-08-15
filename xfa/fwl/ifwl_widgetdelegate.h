// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_IFWL_WIDGETDELEGATE_H_
#define XFA_FWL_IFWL_WIDGETDELEGATE_H_

#include "v8/include/cppgc/garbage-collected.h"

class CFGAS_GEGraphics;
class CFX_Matrix;

namespace pdfium {

class CFWL_Event;
class CFWL_Message;

class IFWL_WidgetDelegate : public cppgc::GarbageCollectedMixin {
 public:
  virtual ~IFWL_WidgetDelegate() = default;

  virtual void OnProcessMessage(CFWL_Message* pMessage) = 0;
  virtual void OnProcessEvent(CFWL_Event* pEvent) = 0;
  virtual void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                            const CFX_Matrix& matrix) = 0;
};

}  // namespace pdfium

// TODO(crbug.com/42271761): Remove.
using pdfium::IFWL_WidgetDelegate;

#endif  // XFA_FWL_IFWL_WIDGETDELEGATE_H_

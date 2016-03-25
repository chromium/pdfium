// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_BASEWIDGET_FWL_CARETIMP_H_
#define XFA_FWL_BASEWIDGET_FWL_CARETIMP_H_

#include "xfa/fwl/core/fwl_widgetimp.h"
#include "xfa/fwl/core/ifwl_timer.h"
#include "xfa/fxgraphics/cfx_color.h"

class CFWL_WidgetImpProperties;
class IFWL_Widget;
class CFWL_CaretImpDelegate;

class CFWL_CaretImp : public CFWL_WidgetImp {
 public:
  CFWL_CaretImp(const CFWL_WidgetImpProperties& properties,
                IFWL_Widget* pOuter);
  virtual ~CFWL_CaretImp();

  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual uint32_t GetClassID() const;

  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);

  virtual FWL_ERR ShowCaret(FX_BOOL bFlag = TRUE);
  virtual FWL_ERR GetFrequency(uint32_t& elapse);
  virtual FWL_ERR SetFrequency(uint32_t elapse);
  virtual FWL_ERR SetColor(CFX_Color crFill);

 protected:
  FX_BOOL DrawCaretBK(CFX_Graphics* pGraphics,
                      IFWL_ThemeProvider* pTheme,
                      const CFX_Matrix* pMatrix);
  class CFWL_CaretTimer : public IFWL_Timer {
   public:
    explicit CFWL_CaretTimer(CFWL_CaretImp* pCaret);
    ~CFWL_CaretTimer() override {}
    int32_t Run(FWL_HTIMER hTimer) override;
    CFWL_CaretImp* const m_pCaret;
  };
  CFWL_CaretTimer* m_pTimer;
  FWL_HTIMER m_hTimer;
  uint32_t m_dwElapse;
  CFX_Color m_crFill;
  FX_BOOL m_bSetColor;
  friend class CFWL_CaretImpDelegate;
  friend class CFWL_CaretTimer;
};
class CFWL_CaretImpDelegate : public CFWL_WidgetImpDelegate {
 public:
  CFWL_CaretImpDelegate(CFWL_CaretImp* pOwner);
  int32_t OnProcessMessage(CFWL_Message* pMessage) override;
  FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                       const CFX_Matrix* pMatrix = NULL) override;

 protected:
  CFWL_CaretImp* m_pOwner;
};

#endif  // XFA_FWL_BASEWIDGET_FWL_CARETIMP_H_

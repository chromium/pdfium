// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FWL_CARET_IMP_H
#define _FWL_CARET_IMP_H

#include "xfa/include/fwl/core/fwl_timer.h"

class CFWL_WidgetImp;
class CFWL_WidgetImpProperties;
class CFWL_WidgetImpDelegate;
class IFWL_Widget;
class CFWL_CaretImp;
class CFWL_CaretImpDelegate;
class CFWL_CaretImp : public CFWL_WidgetImp {
 public:
  CFWL_CaretImp(const CFWL_WidgetImpProperties& properties,
                IFWL_Widget* pOuter);
  virtual ~CFWL_CaretImp();

  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const;
  virtual FX_DWORD GetClassID() const;

  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();

  virtual FWL_ERR DrawWidget(CFX_Graphics* pGraphics,
                             const CFX_Matrix* pMatrix = NULL);

  virtual FWL_ERR ShowCaret(FX_BOOL bFlag = TRUE);
  virtual FWL_ERR GetFrequency(FX_DWORD& elapse);
  virtual FWL_ERR SetFrequency(FX_DWORD elapse);
  virtual FWL_ERR SetColor(CFX_Color crFill);

 protected:
  FX_BOOL DrawCaretBK(CFX_Graphics* pGraphics,
                      IFWL_ThemeProvider* pTheme,
                      const CFX_Matrix* pMatrix);
  class CFWL_CaretTimer : public IFWL_Timer {
   public:
    CFWL_CaretTimer(CFWL_CaretImp* m_pCaret);
    ~CFWL_CaretTimer() override {}
    int32_t Run(FWL_HTIMER hTimer) override;
    CFWL_CaretImp* m_pCaret;
  };
  CFWL_CaretTimer* m_pTimer;
  FWL_HTIMER m_hTimer;
  FX_DWORD m_dwElapse;
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
#endif

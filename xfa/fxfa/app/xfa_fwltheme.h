// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FWLTHEME_H_
#define XFA_FXFA_APP_XFA_FWLTHEME_H_

#include <memory>

#include "xfa/fwl/core/ifwl_themeprovider.h"
#include "xfa/fwl/theme/cfwl_barcodetp.h"
#include "xfa/fwl/theme/cfwl_carettp.h"
#include "xfa/fwl/theme/cfwl_checkboxtp.h"
#include "xfa/fwl/theme/cfwl_comboboxtp.h"
#include "xfa/fwl/theme/cfwl_datetimepickertp.h"
#include "xfa/fwl/theme/cfwl_edittp.h"
#include "xfa/fwl/theme/cfwl_listboxtp.h"
#include "xfa/fwl/theme/cfwl_monthcalendartp.h"
#include "xfa/fwl/theme/cfwl_pictureboxtp.h"
#include "xfa/fwl/theme/cfwl_pushbuttontp.h"
#include "xfa/fwl/theme/cfwl_scrollbartp.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"
#include "xfa/fxfa/include/xfa_ffapp.h"

class CXFA_FWLTheme : public IFWL_ThemeProvider {
 public:
  CXFA_FWLTheme(CXFA_FFApp* pApp);
  virtual ~CXFA_FWLTheme();
  virtual FWL_Error Release() {
    delete this;
    return FWL_Error::Succeeded;
  }
  virtual IFWL_Widget* Retain() { return NULL; }
  virtual FWL_Error GetClassName(CFX_WideString& wsClass) const {
    return FWL_Error::Succeeded;
  }
  virtual uint32_t GetHashCode() const { return 0; }
  virtual FWL_Error Initialize();
  virtual FWL_Error Finalize();
  virtual bool IsValidWidget(IFWL_Widget* pWidget);
  virtual uint32_t GetThemeID(IFWL_Widget* pWidget);
  virtual uint32_t SetThemeID(IFWL_Widget* pWidget,
                              uint32_t dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual FX_BOOL DrawText(CFWL_ThemeText* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart,
                            CFWL_WidgetCapacity dwCapacity);
  virtual FX_BOOL IsCustomizedLayout(IFWL_Widget* pWidget);
  virtual FWL_Error GetPartRect(CFWL_ThemePart* pThemePart);
  virtual FX_BOOL IsInPart(CFWL_ThemePart* pThemePart,
                           FX_FLOAT fx,
                           FX_FLOAT fy);

  virtual FX_BOOL CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect);
  virtual FWL_Error GetThemeMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) {
    return FWL_Error::Succeeded;
  }
  virtual FWL_Error SetThemeMatrix(IFWL_Widget* pWidget,
                                   const CFX_Matrix& matrix) {
    return FWL_Error::Succeeded;
  }
  virtual FWL_Error GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF& rtPart) {
    return FWL_Error::Succeeded;
  }

 protected:
  CFWL_WidgetTP* GetTheme(IFWL_Widget* pWidget);
  CFWL_CheckBoxTP* m_pCheckBoxTP;
  CFWL_ListBoxTP* m_pListBoxTP;
  CFWL_PictureBoxTP* m_pPictureBoxTP;
  CFWL_ScrollBarTP* m_pSrollBarTP;
  CFWL_EditTP* m_pEditTP;
  CFWL_ComboBoxTP* m_pComboBoxTP;
  CFWL_MonthCalendarTP* m_pMonthCalendarTP;
  CFWL_DateTimePickerTP* m_pDateTimePickerTP;
  CFWL_PushButtonTP* m_pPushButtonTP;
  CFWL_CaretTP* m_pCaretTP;
  CFWL_BarcodeTP* m_pBarcodeTP;
  std::unique_ptr<CFDE_TextOut> m_pTextOut;
  FX_FLOAT m_fCapacity;
  uint32_t m_dwCapacity;
  IFX_Font* m_pCalendarFont;
  CFX_WideString m_wsResource;
  CXFA_FFApp* m_pApp;
  CFX_RectF m_Rect;
  CFX_SizeF m_SizeAboveBelow;
};
class CXFA_FWLCheckBoxTP : public CFWL_CheckBoxTP {
 public:
  CXFA_FWLCheckBoxTP();
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);

 protected:
  void DrawCheckSign(IFWL_Widget* pWidget,
                     CFX_Graphics* pGraphics,
                     const CFX_RectF* pRtBox,
                     int32_t iState,
                     CFX_Matrix* pMatrix);
};
class CXFA_FWLEditTP : public CFWL_EditTP {
 public:
  CXFA_FWLEditTP();
  virtual ~CXFA_FWLEditTP();

 public:
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
};

#endif  // XFA_FXFA_APP_XFA_FWLTHEME_H_

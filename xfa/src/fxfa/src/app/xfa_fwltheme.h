// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _XFA_FWL_THEME_IMP_H
#define _XFA_FWL_THEME_IMP_H
class CXFA_FWLTheme : public IFWL_ThemeProvider {
 public:
  CXFA_FWLTheme(CXFA_FFApp* pApp);
  virtual ~CXFA_FWLTheme();
  virtual FWL_ERR Release() {
    delete this;
    return FWL_ERR_Succeeded;
  }
  virtual IFWL_Target* Retain() { return NULL; }
  virtual FWL_ERR GetClassName(CFX_WideString& wsClass) const {
    return FWL_ERR_Succeeded;
  }
  virtual FX_DWORD GetHashCode() const { return 0; }
  virtual FWL_ERR Initialize();
  virtual FWL_ERR Finalize();
  virtual FX_BOOL IsValidWidget(IFWL_Widget* pWidget);
  virtual FX_DWORD GetThemeID(IFWL_Widget* pWidget);
  virtual FX_DWORD SetThemeID(IFWL_Widget* pWidget,
                              FX_DWORD dwThemeID,
                              FX_BOOL bChildren = TRUE);
  virtual FX_BOOL DrawBackground(CFWL_ThemeBackground* pParams);
  virtual FX_BOOL DrawText(CFWL_ThemeText* pParams);
  virtual void* GetCapacity(CFWL_ThemePart* pThemePart, FX_DWORD dwCapacity);
  virtual FX_BOOL IsCustomizedLayout(IFWL_Widget* pWidget);
  virtual FWL_ERR GetPartRect(CFWL_ThemePart* pThemePart);
  virtual FX_BOOL IsInPart(CFWL_ThemePart* pThemePart,
                           FX_FLOAT fx,
                           FX_FLOAT fy);

  virtual FX_BOOL CalcTextRect(CFWL_ThemeText* pParams, CFX_RectF& rect);
  virtual FWL_ERR GetThemeMatrix(IFWL_Widget* pWidget, CFX_Matrix& matrix) {
    return FWL_ERR_Succeeded;
  }
  virtual FWL_ERR SetThemeMatrix(IFWL_Widget* pWidget,
                                 const CFX_Matrix& matrix) {
    return FWL_ERR_Succeeded;
  }
  virtual FWL_ERR GetPartRect(CFWL_ThemePart* pThemePart, CFX_RectF& rtPart) {
    return FWL_ERR_Succeeded;
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
  IFDE_TextOut* m_pTextOut;
  FX_FLOAT m_fCapacity;
  FX_DWORD m_dwCapacity;
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
#endif

// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_WIDGET_H_
#define FPDFSDK_INCLUDE_CPDFSDK_WIDGET_H_

#include <set>

#include "core/fpdfdoc/include/cpdf_aaction.h"
#include "core/fpdfdoc/include/cpdf_action.h"
#include "core/fpdfdoc/include/cpdf_annot.h"
#include "core/fxcrt/include/fx_coordinates.h"
#include "core/fxcrt/include/fx_string.h"
#include "fpdfsdk/include/cpdfsdk_baannot.h"
#include "fpdfsdk/include/pdfsdk_fieldaction.h"
#include "fpdfsdk/pdfwindow/PWL_Wnd.h"

class CFX_RenderDevice;
class CPDF_Annot;
class CPDF_Dictionary;
class CPDF_FormControl;
class CPDF_FormField;
class CPDF_RenderOptions;
class CPDF_Stream;
class CPDFSDK_InterForm;
class CPDFSDK_PageView;

#ifdef PDF_ENABLE_XFA
class CXFA_FFWidget;
class CXFA_FFWidgetHandler;
#endif  // PDF_ENABLE_XFA

class CPDFSDK_Widget : public CPDFSDK_BAAnnot {
 public:
#ifdef PDF_ENABLE_XFA
  CXFA_FFWidget* GetMixXFAWidget() const;
  CXFA_FFWidget* GetGroupMixXFAWidget();
  CXFA_FFWidgetHandler* GetXFAWidgetHandler() const;

  FX_BOOL HasXFAAAction(PDFSDK_XFAAActionType eXFAAAT);
  FX_BOOL OnXFAAAction(PDFSDK_XFAAActionType eXFAAAT,
                       PDFSDK_FieldAction& data,
                       CPDFSDK_PageView* pPageView);

  void Synchronize(FX_BOOL bSynchronizeElse);
  void SynchronizeXFAValue();
  void SynchronizeXFAItems();

  static void SynchronizeXFAValue(CXFA_FFDocView* pXFADocView,
                                  CXFA_FFWidget* hWidget,
                                  CPDF_FormField* pFormField,
                                  CPDF_FormControl* pFormControl);
  static void SynchronizeXFAItems(CXFA_FFDocView* pXFADocView,
                                  CXFA_FFWidget* hWidget,
                                  CPDF_FormField* pFormField,
                                  CPDF_FormControl* pFormControl);
#endif  // PDF_ENABLE_XFA

  CPDFSDK_Widget(CPDF_Annot* pAnnot,
                 CPDFSDK_PageView* pPageView,
                 CPDFSDK_InterForm* pInterForm);
  ~CPDFSDK_Widget() override;

  bool IsSignatureWidget() const override;
  CPDF_Action GetAAction(CPDF_AAction::AActionType eAAT) override;
  FX_BOOL IsAppearanceValid() override;

  int GetLayoutOrder() const override;

  int GetFieldType() const;
  int GetFieldFlags() const;
  int GetRotate() const;

  FX_BOOL GetFillColor(FX_COLORREF& color) const;
  FX_BOOL GetBorderColor(FX_COLORREF& color) const;
  FX_BOOL GetTextColor(FX_COLORREF& color) const;
  FX_FLOAT GetFontSize() const;

  int GetSelectedIndex(int nIndex) const;
#ifndef PDF_ENABLE_XFA
  CFX_WideString GetValue() const;
#else
  CFX_WideString GetValue(FX_BOOL bDisplay = TRUE) const;
#endif  // PDF_ENABLE_XFA
  CFX_WideString GetDefaultValue() const;
  CFX_WideString GetOptionLabel(int nIndex) const;
  int CountOptions() const;
  FX_BOOL IsOptionSelected(int nIndex) const;
  int GetTopVisibleIndex() const;
  bool IsChecked() const;
  int GetAlignment() const;
  int GetMaxLen() const;
#ifdef PDF_ENABLE_XFA
  CFX_WideString GetName() const;
#endif  // PDF_ENABLE_XFA
  CFX_WideString GetAlternateName() const;

  void SetCheck(bool bChecked, bool bNotify);
  void SetValue(const CFX_WideString& sValue, FX_BOOL bNotify);
  void SetDefaultValue(const CFX_WideString& sValue);
  void SetOptionSelection(int index, FX_BOOL bSelected, FX_BOOL bNotify);
  void ClearSelection(FX_BOOL bNotify);
  void SetTopVisibleIndex(int index);

#ifdef PDF_ENABLE_XFA
  void ResetAppearance(FX_BOOL bValueChanged);
#endif  // PDF_ENABLE_XFA
  void ResetAppearance(const CFX_WideString* sValue, FX_BOOL bValueChanged);
  void ResetFieldAppearance(FX_BOOL bValueChanged);
  void UpdateField();
  CFX_WideString OnFormat(FX_BOOL& bFormatted);

  FX_BOOL OnAAction(CPDF_AAction::AActionType type,
                    PDFSDK_FieldAction& data,
                    CPDFSDK_PageView* pPageView);

  CPDFSDK_InterForm* GetInterForm() const { return m_pInterForm; }
  CPDF_FormField* GetFormField() const;
  CPDF_FormControl* GetFormControl() const;
  static CPDF_FormControl* GetFormControl(CPDF_InterForm* pInterForm,
                                          const CPDF_Dictionary* pAnnotDict);

  void DrawShadow(CFX_RenderDevice* pDevice, CPDFSDK_PageView* pPageView);

  void SetAppModified();
  void ClearAppModified();
  FX_BOOL IsAppModified() const;

  int32_t GetAppearanceAge() const;
  int32_t GetValueAge() const;

  FX_BOOL IsWidgetAppearanceValid(CPDF_Annot::AppearanceMode mode);
  void DrawAppearance(CFX_RenderDevice* pDevice,
                      const CFX_Matrix* pUser2Device,
                      CPDF_Annot::AppearanceMode mode,
                      const CPDF_RenderOptions* pOptions) override;

  FX_BOOL HitTest(FX_FLOAT pageX, FX_FLOAT pageY);

 private:
  void ResetAppearance_PushButton();
  void ResetAppearance_CheckBox();
  void ResetAppearance_RadioButton();
  void ResetAppearance_ComboBox(const CFX_WideString* sValue);
  void ResetAppearance_ListBox();
  void ResetAppearance_TextField(const CFX_WideString* sValue);

  CFX_FloatRect GetClientRect() const;
  CFX_FloatRect GetRotatedRect() const;

  CFX_ByteString GetBackgroundAppStream() const;
  CFX_ByteString GetBorderAppStream() const;
  CFX_Matrix GetMatrix() const;

  CPWL_Color GetTextPWLColor() const;
  CPWL_Color GetBorderPWLColor() const;
  CPWL_Color GetFillPWLColor() const;

  void AddImageToAppearance(const CFX_ByteString& sAPType, CPDF_Stream* pImage);
  void RemoveAppearance(const CFX_ByteString& sAPType);

  CPDFSDK_InterForm* const m_pInterForm;
  FX_BOOL m_bAppModified;
  int32_t m_nAppAge;
  int32_t m_nValueAge;

#ifdef PDF_ENABLE_XFA
  mutable CXFA_FFWidget* m_hMixXFAWidget;
  mutable CXFA_FFWidgetHandler* m_pWidgetHandler;
#endif  // PDF_ENABLE_XFA
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_WIDGET_H_

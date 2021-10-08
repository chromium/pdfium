// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_CPDFSDK_WIDGET_H_
#define FPDFSDK_CPDFSDK_WIDGET_H_

#include "core/fpdfdoc/cpdf_aaction.h"
#include "core/fpdfdoc/cpdf_action.h"
#include "core/fpdfdoc/cpdf_annot.h"
#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/cfx_color.h"
#include "fpdfsdk/cpdfsdk_baannot.h"
#include "third_party/abseil-cpp/absl/types/optional.h"

class CFX_RenderDevice;
class CPDF_Annot;
class CPDF_FormControl;
class CPDF_FormField;
class CPDF_RenderOptions;
class CPDFSDK_FormFillEnvironment;
class CPDFSDK_InteractiveForm;
class CPDFSDK_PageView;
struct CFFL_FieldAction;

#ifdef PDF_ENABLE_XFA
class CXFA_FFWidget;
class CXFA_FFWidgetHandler;

enum PDFSDK_XFAAActionType {
  PDFSDK_XFA_Click = 0,
  PDFSDK_XFA_Full,
  PDFSDK_XFA_PreOpen,
  PDFSDK_XFA_PostOpen
};
#endif  // PDF_ENABLE_XFA

class CPDFSDK_Widget final : public CPDFSDK_BAAnnot {
 public:
  enum ValueChanged : bool { kValueUnchanged = false, kValueChanged = true };

  CPDFSDK_Widget(CPDF_Annot* pAnnot,
                 CPDFSDK_PageView* pPageView,
                 CPDFSDK_InteractiveForm* pInteractiveForm);
  ~CPDFSDK_Widget() override;

  // CPDFSDK_BAAnnot:
  CPDF_Action GetAAction(CPDF_AAction::AActionType eAAT) override;
  bool IsAppearanceValid() override;

  // CPDFSDK_Annot:
  int GetLayoutOrder() const override;

  bool IsSignatureWidget() const;
  void SetRect(const CFX_FloatRect& rect);
  FormFieldType GetFieldType() const;
  int GetFieldFlags() const;
  int GetRotate() const;

  absl::optional<FX_COLORREF> GetFillColor() const;
  absl::optional<FX_COLORREF> GetBorderColor() const;
  absl::optional<FX_COLORREF> GetTextColor() const;
  float GetFontSize() const;

  int GetSelectedIndex(int nIndex) const;
  WideString GetValue() const;
  WideString GetExportValue() const;
  WideString GetOptionLabel(int nIndex) const;
  int CountOptions() const;
  bool IsOptionSelected(int nIndex) const;
  int GetTopVisibleIndex() const;
  bool IsChecked() const;
  int GetAlignment() const;
  int GetMaxLen() const;

  void SetCheck(bool bChecked);
  void SetValue(const WideString& sValue);
  void SetOptionSelection(int index);
  void ClearSelection();
  void SetTopVisibleIndex(int index);

#ifdef PDF_ENABLE_XFA
  CXFA_FFWidget* GetMixXFAWidget() const;
  bool HasXFAAAction(PDFSDK_XFAAActionType eXFAAAT) const;
  bool OnXFAAAction(PDFSDK_XFAAActionType eXFAAAT,
                    CFFL_FieldAction* data,
                    const CPDFSDK_PageView* pPageView);
  void Synchronize(bool bSynchronizeElse);
  // TODO(thestig): Figure out if the parameter should be used or removed.
  void ResetXFAAppearance(ValueChanged bValueChanged);
#endif  // PDF_ENABLE_XFA

  void ResetAppearance(absl::optional<WideString> sValue,
                       ValueChanged bValueChanged);
  void ResetFieldAppearance();
  void UpdateField();
  absl::optional<WideString> OnFormat();

  bool OnAAction(CPDF_AAction::AActionType type,
                 CFFL_FieldAction* data,
                 const CPDFSDK_PageView* pPageView);

  CPDFSDK_InteractiveForm* GetInteractiveForm() const {
    return m_pInteractiveForm.Get();
  }
  CPDF_FormField* GetFormField() const;
  CPDF_FormControl* GetFormControl() const;

  void DrawShadow(CFX_RenderDevice* pDevice, CPDFSDK_PageView* pPageView);

  void SetAppModified();
  void ClearAppModified();
  bool IsAppModified() const;

  uint32_t GetAppearanceAge() const { return m_nAppearanceAge; }
  uint32_t GetValueAge() const { return m_nValueAge; }

  bool IsWidgetAppearanceValid(CPDF_Annot::AppearanceMode mode);
  void DrawAppearance(CFX_RenderDevice* pDevice,
                      const CFX_Matrix& mtUser2Device,
                      CPDF_Annot::AppearanceMode mode,
                      const CPDF_RenderOptions* pOptions) override;

  CFX_Matrix GetMatrix() const;
  CFX_FloatRect GetClientRect() const;
  CFX_FloatRect GetRotatedRect() const;
  CFX_Color GetTextPWLColor() const;
  CFX_Color GetBorderPWLColor() const;
  CFX_Color GetFillPWLColor() const;

 private:
#ifdef PDF_ENABLE_XFA
  CXFA_FFWidgetHandler* GetXFAWidgetHandler() const;
  CXFA_FFWidget* GetGroupMixXFAWidget() const;
  WideString GetName() const;
  bool HandleXFAAAction(CPDF_AAction::AActionType type,
                        CFFL_FieldAction* data,
                        CPDFSDK_FormFillEnvironment* pFormFillEnv);
#endif  // PDF_ENABLE_XFA

  UnownedPtr<CPDFSDK_InteractiveForm> const m_pInteractiveForm;
  bool m_bAppModified = false;
  uint32_t m_nAppearanceAge = 0;
  uint32_t m_nValueAge = 0;
};

inline CPDFSDK_Widget* ToCPDFSDKWidget(CPDFSDK_Annot* pAnnot) {
  return pAnnot && pAnnot->GetAnnotSubtype() == CPDF_Annot::Subtype::WIDGET
             ? static_cast<CPDFSDK_Widget*>(pAnnot)
             : nullptr;
}

#endif  // FPDFSDK_CPDFSDK_WIDGET_H_

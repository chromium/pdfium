// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_INCLUDE_CPDFSDK_INTERFORM_H_
#define FPDFSDK_INCLUDE_CPDFSDK_INTERFORM_H_

#include <map>
#include <vector>

#include "core/fpdfdoc/include/cpdf_action.h"
#include "core/fpdfdoc/include/ipdf_formnotify.h"
#include "core/fxcrt/include/fx_basic.h"
#include "core/fxge/include/fx_dib.h"

class CPDF_Dictionary;
class CPDF_FormControl;
class CPDF_FormField;
class CPDF_InterForm;
class CPDF_Object;
class CPDFSDK_Document;
class CPDFSDK_Widget;

#ifdef PDF_ENABLE_XFA
class CPDFSDK_XFAWidget;
class CXFA_FFWidget;
#endif  // PDF_ENABLE_XFA

class CPDFSDK_InterForm : public IPDF_FormNotify {
 public:
  explicit CPDFSDK_InterForm(CPDFSDK_Document* pDocument);
  ~CPDFSDK_InterForm() override;

  CPDF_InterForm* GetInterForm() const { return m_pInterForm.get(); }
  CPDFSDK_Document* GetDocument() const { return m_pDocument; }

  FX_BOOL HighlightWidgets();

  CPDFSDK_Widget* GetSibling(CPDFSDK_Widget* pWidget, FX_BOOL bNext) const;
  CPDFSDK_Widget* GetWidget(CPDF_FormControl* pControl,
                            bool createIfNeeded) const;
  void GetWidgets(const CFX_WideString& sFieldName,
                  std::vector<CPDFSDK_Widget*>* widgets) const;
  void GetWidgets(CPDF_FormField* pField,
                  std::vector<CPDFSDK_Widget*>* widgets) const;

  void AddMap(CPDF_FormControl* pControl, CPDFSDK_Widget* pWidget);
  void RemoveMap(CPDF_FormControl* pControl);

  void EnableCalculate(FX_BOOL bEnabled);
  FX_BOOL IsCalculateEnabled() const;

#ifdef PDF_ENABLE_XFA
  void AddXFAMap(CXFA_FFWidget* hWidget, CPDFSDK_XFAWidget* pWidget);
  void RemoveXFAMap(CXFA_FFWidget* hWidget);
  CPDFSDK_XFAWidget* GetXFAWidget(CXFA_FFWidget* hWidget);
  void XfaEnableCalculate(FX_BOOL bEnabled);
  FX_BOOL IsXfaCalculateEnabled() const;
  FX_BOOL IsXfaValidationsEnabled();
  void XfaSetValidationsEnabled(FX_BOOL bEnabled);
  void SynchronizeField(CPDF_FormField* pFormField, FX_BOOL bSynchronizeElse);
#endif  // PDF_ENABLE_XFA

  FX_BOOL OnKeyStrokeCommit(CPDF_FormField* pFormField,
                            const CFX_WideString& csValue);
  FX_BOOL OnValidate(CPDF_FormField* pFormField, const CFX_WideString& csValue);
  void OnCalculate(CPDF_FormField* pFormField = nullptr);
  CFX_WideString OnFormat(CPDF_FormField* pFormField, FX_BOOL& bFormatted);

  void ResetFieldAppearance(CPDF_FormField* pFormField,
                            const CFX_WideString* sValue,
                            FX_BOOL bValueChanged);
  void UpdateField(CPDF_FormField* pFormField);

  FX_BOOL DoAction_Hide(const CPDF_Action& action);
  FX_BOOL DoAction_SubmitForm(const CPDF_Action& action);
  FX_BOOL DoAction_ResetForm(const CPDF_Action& action);
  FX_BOOL DoAction_ImportData(const CPDF_Action& action);

  std::vector<CPDF_FormField*> GetFieldFromObjects(
      const std::vector<CPDF_Object*>& objects) const;
  FX_BOOL IsValidField(CPDF_Dictionary* pFieldDict);
  FX_BOOL SubmitFields(const CFX_WideString& csDestination,
                       const std::vector<CPDF_FormField*>& fields,
                       bool bIncludeOrExclude,
                       bool bUrlEncoded);
  FX_BOOL SubmitForm(const CFX_WideString& sDestination, FX_BOOL bUrlEncoded);
  FX_BOOL ExportFormToFDFTextBuf(CFX_ByteTextBuf& textBuf);
  FX_BOOL ExportFieldsToFDFTextBuf(const std::vector<CPDF_FormField*>& fields,
                                   bool bIncludeOrExclude,
                                   CFX_ByteTextBuf& textBuf);
  CFX_WideString GetTemporaryFileName(const CFX_WideString& sFileExt);

  FX_BOOL IsNeedHighLight(int nFieldType);
  void RemoveAllHighLight();
  void SetHighlightAlpha(uint8_t alpha) { m_iHighlightAlpha = alpha; }
  uint8_t GetHighlightAlpha() { return m_iHighlightAlpha; }
  void SetHighlightColor(FX_COLORREF clr, int nFieldType);
  FX_COLORREF GetHighlightColor(int nFieldType);

 private:
  // IPDF_FormNotify:
  int BeforeValueChange(CPDF_FormField* pField,
                        const CFX_WideString& csValue) override;
  void AfterValueChange(CPDF_FormField* pField) override;
  int BeforeSelectionChange(CPDF_FormField* pField,
                            const CFX_WideString& csValue) override;
  void AfterSelectionChange(CPDF_FormField* pField) override;
  void AfterCheckedStatusChange(CPDF_FormField* pField) override;
  int BeforeFormReset(CPDF_InterForm* pForm) override;
  void AfterFormReset(CPDF_InterForm* pForm) override;
  int BeforeFormImportData(CPDF_InterForm* pForm) override;
  void AfterFormImportData(CPDF_InterForm* pForm) override;

  FX_BOOL FDFToURLEncodedData(CFX_WideString csFDFFile,
                              CFX_WideString csTxtFile);
  FX_BOOL FDFToURLEncodedData(uint8_t*& pBuf, FX_STRSIZE& nBufSize);
  int GetPageIndexByAnnotDict(CPDF_Document* pDocument,
                              CPDF_Dictionary* pAnnotDict) const;

  using CPDFSDK_WidgetMap = std::map<CPDF_FormControl*, CPDFSDK_Widget*>;

  CPDFSDK_Document* m_pDocument;
  std::unique_ptr<CPDF_InterForm> m_pInterForm;
  CPDFSDK_WidgetMap m_Map;
#ifdef PDF_ENABLE_XFA
  std::map<CXFA_FFWidget*, CPDFSDK_XFAWidget*> m_XFAMap;
  FX_BOOL m_bXfaCalculate;
  FX_BOOL m_bXfaValidationsEnabled;
  static const int kNumFieldTypes = 7;
#else   // PDF_ENABLE_XFA
  static const int kNumFieldTypes = 6;
#endif  // PDF_ENABLE_XFA
  FX_BOOL m_bCalculate;
  FX_BOOL m_bBusy;

  FX_COLORREF m_aHighlightColor[kNumFieldTypes];
  uint8_t m_iHighlightAlpha;
  FX_BOOL m_bNeedHightlight[kNumFieldTypes];
};

#endif  // FPDFSDK_INCLUDE_CPDFSDK_INTERFORM_H_

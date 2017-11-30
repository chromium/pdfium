// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_WIDGETDATA_H_
#define XFA_FXFA_PARSER_CXFA_WIDGETDATA_H_

#include <utility>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "fxbarcode/BC_Library.h"
#include "xfa/fxfa/parser/cxfa_binddata.h"
#include "xfa/fxfa/parser/cxfa_borderdata.h"
#include "xfa/fxfa/parser/cxfa_calculatedata.h"
#include "xfa/fxfa/parser/cxfa_captiondata.h"
#include "xfa/fxfa/parser/cxfa_datadata.h"
#include "xfa/fxfa/parser/cxfa_fontdata.h"
#include "xfa/fxfa/parser/cxfa_margindata.h"
#include "xfa/fxfa/parser/cxfa_paradata.h"
#include "xfa/fxfa/parser/cxfa_validatedata.h"

enum XFA_CHECKSTATE {
  XFA_CHECKSTATE_On = 0,
  XFA_CHECKSTATE_Off = 1,
  XFA_CHECKSTATE_Neutral = 2,
};

enum XFA_VALUEPICTURE {
  XFA_VALUEPICTURE_Raw = 0,
  XFA_VALUEPICTURE_Display,
  XFA_VALUEPICTURE_Edit,
  XFA_VALUEPICTURE_DataBind,
};

class CXFA_Node;
class IFX_Locale;

class CXFA_WidgetData : public CXFA_DataData {
 public:
  explicit CXFA_WidgetData(CXFA_Node* pNode);

  CXFA_Node* GetUIChild();
  XFA_Element GetUIType();
  CFX_RectF GetUIMargin();

  WideString GetRawValue() const;
  int32_t GetRotate() const;

  bool IsOpenAccess() const;
  bool IsListBox();
  bool IsAllowNeutral();
  bool IsRadioButton();
  bool IsChoiceListAllowTextEntry();
  bool IsMultiLine();

  CXFA_BorderData GetBorderData(bool bModified);
  CXFA_CaptionData GetCaptionData();
  CXFA_FontData GetFontData(bool bModified);
  CXFA_MarginData GetMarginData();
  CXFA_ParaData GetParaData();
  CXFA_ValueData GetDefaultValueData();
  CXFA_ValueData GetFormValueData();
  CXFA_CalculateData GetCalculateData();
  CXFA_ValidateData GetValidateData(bool bModified);
  CXFA_BorderData GetUIBorderData();

  std::vector<CXFA_Node*> GetEventByActivity(XFA_AttributeEnum iActivity,
                                             bool bIsFormReady);

  pdfium::Optional<float> TryWidth();
  pdfium::Optional<float> TryHeight();
  pdfium::Optional<float> TryMinWidth();
  pdfium::Optional<float> TryMinHeight();
  pdfium::Optional<float> TryMaxWidth();
  pdfium::Optional<float> TryMaxHeight();

  XFA_AttributeEnum GetButtonHighlight();
  bool HasButtonRollover() const;
  bool HasButtonDown() const;

  bool IsCheckButtonRound();
  XFA_AttributeEnum GetCheckButtonMark();
  float GetCheckButtonSize();

  XFA_CHECKSTATE GetCheckState();
  void SetCheckState(XFA_CHECKSTATE eCheckState, bool bNotify);

  CXFA_Node* GetSelectedMember();
  CXFA_Node* SetSelectedMember(const WideStringView& wsName, bool bNotify);
  void SetSelectedMemberByValue(const WideStringView& wsValue,
                                bool bNotify,
                                bool bScriptModify,
                                bool bSyncData);

  CXFA_Node* GetExclGroupFirstMember();
  CXFA_Node* GetExclGroupNextMember(CXFA_Node* pNode);

  int32_t CountChoiceListItems(bool bSaveValue);
  pdfium::Optional<WideString> GetChoiceListItem(int32_t nIndex,
                                                 bool bSaveValue);
  bool IsChoiceListMultiSelect();
  bool IsChoiceListCommitOnSelect();
  std::vector<WideString> GetChoiceListItems(bool bSaveValue);

  int32_t CountSelectedItems();
  int32_t GetSelectedItem(int32_t nIndex);
  std::vector<int32_t> GetSelectedItems();
  std::vector<WideString> GetSelectedItemsValue();
  void SetSelectedItems(const std::vector<int32_t>& iSelArray,
                        bool bNotify,
                        bool bScriptModify,
                        bool bSyncData);
  void InsertItem(const WideString& wsLabel,
                  const WideString& wsValue,
                  bool bNotify);
  bool DeleteItem(int32_t nIndex, bool bNotify, bool bScriptModify);
  void ClearAllSelections();

  bool GetItemState(int32_t nIndex);
  void SetItemState(int32_t nIndex,
                    bool bSelected,
                    bool bNotify,
                    bool bScriptModify,
                    bool bSyncData);

  WideString GetItemValue(const WideStringView& wsLabel);

  bool IsHorizontalScrollPolicyOff();
  bool IsVerticalScrollPolicyOff();
  pdfium::Optional<int32_t> GetNumberOfCells();

  bool SetValue(XFA_VALUEPICTURE eValueType, const WideString& wsValue);
  WideString GetValue(XFA_VALUEPICTURE eValueType);

  WideString GetPictureContent(XFA_VALUEPICTURE ePicture);
  IFX_Locale* GetLocale();

  WideString GetNormalizeDataValue(const WideString& wsValue);
  WideString GetFormatDataValue(const WideString& wsValue);
  WideString NormalizeNumStr(const WideString& wsValue);

  WideString GetBarcodeType();
  pdfium::Optional<BC_CHAR_ENCODING> GetBarcodeAttribute_CharEncoding();
  pdfium::Optional<bool> GetBarcodeAttribute_Checksum();
  pdfium::Optional<int32_t> GetBarcodeAttribute_DataLength();
  pdfium::Optional<char> GetBarcodeAttribute_StartChar();
  pdfium::Optional<char> GetBarcodeAttribute_EndChar();
  pdfium::Optional<int32_t> GetBarcodeAttribute_ECLevel();
  pdfium::Optional<int32_t> GetBarcodeAttribute_ModuleWidth();
  pdfium::Optional<int32_t> GetBarcodeAttribute_ModuleHeight();
  pdfium::Optional<bool> GetBarcodeAttribute_PrintChecksum();
  pdfium::Optional<BC_TEXT_LOC> GetBarcodeAttribute_TextLocation();
  pdfium::Optional<bool> GetBarcodeAttribute_Truncate();
  pdfium::Optional<int8_t> GetBarcodeAttribute_WideNarrowRatio();

  WideString GetPasswordChar();
  std::pair<XFA_Element, int32_t> GetMaxChars();
  int32_t GetFracDigits();
  int32_t GetLeadDigits();

  WideString NumericLimit(const WideString& wsValue,
                          int32_t iLead,
                          int32_t iTread) const;

  bool IsPreNull() const { return m_bPreNull; }
  void SetPreNull(bool val) { m_bPreNull = val; }
  bool IsNull() const { return m_bIsNull; }
  void SetIsNull(bool val) { m_bIsNull = val; }

 private:
  CXFA_BindData GetBindData();
  void SyncValue(const WideString& wsValue, bool bNotify);
  void InsertListTextItem(CXFA_Node* pItems,
                          const WideString& wsText,
                          int32_t nIndex);
  WideString FormatNumStr(const WideString& wsValue, IFX_Locale* pLocale);
  CXFA_Node* GetExclGroupNode();
  void GetItemLabel(const WideStringView& wsValue, WideString& wsLabel);
  std::vector<CXFA_Node*> GetEventList();

  bool m_bIsNull;
  bool m_bPreNull;
  CXFA_Node* m_pUiChildNode;
  XFA_Element m_eUIType;
};

#endif  // XFA_FXFA_PARSER_CXFA_WIDGETDATA_H_

// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_WIDGETACC_H_
#define XFA_FXFA_CXFA_WIDGETACC_H_

#include <memory>
#include <utility>
#include <vector>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"
#include "fxbarcode/BC_Library.h"
#include "xfa/fxfa/fxfa_basic.h"

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

class CFGAS_GEFont;
class CXFA_Bind;
class CXFA_Border;
class CXFA_Calculate;
class CXFA_Caption;
class CXFA_Event;
class CXFA_EventParam;
class CXFA_FFApp;
class CXFA_FFDoc;
class CXFA_FFDocView;
class CXFA_FFWidget;
class CXFA_Font;
class CXFA_Margin;
class CXFA_Node;
class CXFA_Script;
class CXFA_Para;
class CXFA_TextLayout;
class CXFA_Value;
class CXFA_Validate;
class CXFA_WidgetLayoutData;
class IXFA_AppProvider;
class IFX_Locale;

class CXFA_WidgetAcc {
 public:
  CXFA_WidgetAcc(CXFA_FFDocView* pDocView, CXFA_Node* pNode);
  ~CXFA_WidgetAcc();

  void ResetData();

  XFA_Element GetElementType() const;

  CXFA_WidgetAcc* GetExclGroup();
  CXFA_FFDoc* GetDoc();

  int32_t ProcessEvent(XFA_AttributeEnum iActivity,
                       CXFA_EventParam* pEventParam);
  int32_t ProcessEvent(CXFA_Event* event, CXFA_EventParam* pEventParam);
  int32_t ProcessCalculate();
  int32_t ProcessValidate(int32_t iFlags);
  int32_t ExecuteScript(CXFA_Script* script, CXFA_EventParam* pEventParam);
  std::pair<int32_t, bool> ExecuteBoolScript(CXFA_Script* script,
                                             CXFA_EventParam* pEventParam);

  CXFA_FFWidget* GetNextWidget(CXFA_FFWidget* pWidget);
  void StartWidgetLayout(float& fCalcWidth, float& fCalcHeight);
  bool FindSplitPos(int32_t iBlockIndex, float& fCalcHeight);

  bool LoadCaption();
  CXFA_TextLayout* GetCaptionTextLayout();

  void LoadText();
  CXFA_TextLayout* GetTextLayout();

  bool LoadImageImage();
  bool LoadImageEditImage();
  void GetImageDpi(int32_t& iImageXDpi, int32_t& iImageYDpi);
  void GetImageEditDpi(int32_t& iImageXDpi, int32_t& iImageYDpi);

  RetainPtr<CFX_DIBitmap> GetImageImage();
  RetainPtr<CFX_DIBitmap> GetImageEditImage();
  void SetImageEdit(const WideString& wsContentType,
                    const WideString& wsHref,
                    const WideString& wsData);
  void SetImageImage(const RetainPtr<CFX_DIBitmap>& newImage);
  void SetImageEditImage(const RetainPtr<CFX_DIBitmap>& newImage);
  void UpdateUIDisplay(CXFA_FFDocView* docView, CXFA_FFWidget* pExcept);

  CXFA_Node* GetDatasets();
  RetainPtr<CFGAS_GEFont> GetFDEFont();
  float GetFontSize();
  FX_ARGB GetTextColor();
  float GetLineHeight();

  CXFA_Node* GetNode() const { return m_pNode; }

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

  CXFA_Border* GetBorder(bool bModified);
  CXFA_Caption* GetCaption();
  CXFA_Font* GetFont(bool bModified);
  CXFA_Margin* GetMargin();
  CXFA_Para* GetPara();
  CXFA_Value* GetDefaultValue();
  CXFA_Value* GetFormValue();
  CXFA_Calculate* GetCalculate();
  CXFA_Validate* GetValidate(bool bModified);
  CXFA_Border* GetUIBorder();

  std::vector<CXFA_Event*> GetEventByActivity(XFA_AttributeEnum iActivity,
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
  IXFA_AppProvider* GetAppProvider();
  void ProcessScriptTestValidate(CXFA_Validate* validate,
                                 int32_t iRet,
                                 bool pRetValue,
                                 bool bVersionFlag);
  int32_t ProcessFormatTestValidate(CXFA_Validate* validate, bool bVersionFlag);
  int32_t ProcessNullTestValidate(CXFA_Validate* validate,
                                  int32_t iFlags,
                                  bool bVersionFlag);
  WideString GetValidateCaptionName(bool bVersionFlag);
  WideString GetValidateMessage(bool bError, bool bVersionFlag);
  void CalcCaptionSize(CFX_SizeF& szCap);
  bool CalculateFieldAutoSize(CFX_SizeF& size);
  bool CalculateWidgetAutoSize(CFX_SizeF& size);
  bool CalculateTextEditAutoSize(CFX_SizeF& size);
  bool CalculateCheckButtonAutoSize(CFX_SizeF& size);
  bool CalculatePushButtonAutoSize(CFX_SizeF& size);
  CFX_SizeF CalculateImageSize(float img_width,
                               float img_height,
                               float dpi_x,
                               float dpi_y);
  bool CalculateImageEditAutoSize(CFX_SizeF& size);
  bool CalculateImageAutoSize(CFX_SizeF& size);
  bool CalculateTextAutoSize(CFX_SizeF& size);
  float CalculateWidgetAutoHeight(float fHeightCalc);
  float CalculateWidgetAutoWidth(float fWidthCalc);
  float GetWidthWithoutMargin(float fWidthCalc);
  float GetHeightWithoutMargin(float fHeightCalc);
  void CalculateTextContentSize(CFX_SizeF& size);
  void CalculateAccWidthAndHeight(XFA_Element eUIType,
                                  float& fWidth,
                                  float& fCalcHeight);
  void InitLayoutData();
  void StartTextLayout(float& fCalcWidth, float& fCalcHeight);

  CXFA_Bind* GetBind();
  void InsertListTextItem(CXFA_Node* pItems,
                          const WideString& wsText,
                          int32_t nIndex);
  WideString FormatNumStr(const WideString& wsValue, IFX_Locale* pLocale);
  CXFA_Node* GetExclGroupNode();
  void GetItemLabel(const WideStringView& wsValue, WideString& wsLabel);

  CXFA_FFDocView* m_pDocView;
  std::unique_ptr<CXFA_WidgetLayoutData> m_pLayoutData;
  uint32_t m_nRecursionDepth;
  bool m_bIsNull;
  bool m_bPreNull;
  CXFA_Node* m_pUiChildNode;
  XFA_Element m_eUIType;
  CXFA_Node* m_pNode;
};

#endif  // XFA_FXFA_CXFA_WIDGETACC_H_

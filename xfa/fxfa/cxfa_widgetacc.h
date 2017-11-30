// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_WIDGETACC_H_
#define XFA_FXFA_CXFA_WIDGETACC_H_

#include <memory>
#include <utility>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/dib/cfx_dibitmap.h"
#include "core/fxge/fx_dib.h"
#include "xfa/fxfa/parser/cxfa_boxdata.h"
#include "xfa/fxfa/parser/cxfa_eventdata.h"
#include "xfa/fxfa/parser/cxfa_imagedata.h"
#include "xfa/fxfa/parser/cxfa_margindata.h"
#include "xfa/fxfa/parser/cxfa_scriptdata.h"
#include "xfa/fxfa/parser/cxfa_valuedata.h"
#include "xfa/fxfa/parser/cxfa_widgetdata.h"

class CFGAS_GEFont;
class CXFA_EventParam;
class CXFA_FFApp;
class CXFA_FFDoc;
class CXFA_FFDocView;
class CXFA_FFWidget;
class CXFA_Node;
class CXFA_TextLayout;
class CXFA_WidgetLayoutData;
class IXFA_AppProvider;

class CXFA_WidgetAcc : public CXFA_WidgetData {
 public:
  CXFA_WidgetAcc(CXFA_FFDocView* pDocView, CXFA_Node* pNode);
  ~CXFA_WidgetAcc() override;

  void ResetData();

  CXFA_WidgetAcc* GetExclGroup();
  CXFA_FFDoc* GetDoc();

  bool ProcessValueChanged();
  int32_t ProcessEvent(XFA_AttributeEnum iActivity,
                       CXFA_EventParam* pEventParam);
  int32_t ProcessEvent(const CXFA_EventData& eventData,
                       CXFA_EventParam* pEventParam);
  int32_t ProcessCalculate();
  int32_t ProcessValidate(int32_t iFlags);
  int32_t ExecuteScript(const CXFA_ScriptData& scriptData,
                        CXFA_EventParam* pEventParam);
  std::pair<int32_t, bool> ExecuteBoolScript(CXFA_ScriptData scriptData,
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
  void UpdateUIDisplay(CXFA_FFWidget* pExcept = nullptr);

  CXFA_Node* GetDatasets();
  RetainPtr<CFGAS_GEFont> GetFDEFont();
  float GetFontSize();
  FX_ARGB GetTextColor();
  float GetLineHeight();

 private:
  IXFA_AppProvider* GetAppProvider();
  void ProcessScriptTestValidate(CXFA_ValidateData validateData,
                                 int32_t iRet,
                                 bool pRetValue,
                                 bool bVersionFlag);
  int32_t ProcessFormatTestValidate(CXFA_ValidateData validateData,
                                    bool bVersionFlag);
  int32_t ProcessNullTestValidate(CXFA_ValidateData validateData,
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

  CXFA_FFDocView* m_pDocView;
  std::unique_ptr<CXFA_WidgetLayoutData> m_pLayoutData;
  uint32_t m_nRecursionDepth;
};

#endif  // XFA_FXFA_CXFA_WIDGETACC_H_

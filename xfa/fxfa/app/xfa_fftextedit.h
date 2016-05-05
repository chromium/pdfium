// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_APP_XFA_FFTEXTEDIT_H_
#define XFA_FXFA_APP_XFA_FFTEXTEDIT_H_

#include <vector>

#include "xfa/fxfa/app/xfa_fffield.h"

class CXFA_FFTextEdit : public CXFA_FFField {
 public:
  CXFA_FFTextEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  ~CXFA_FFTextEdit() override;

  // CXFA_FFField:
  FX_BOOL LoadWidget() override;
  void UpdateWidgetProperty() override;
  FX_BOOL OnLButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy) override;
  FX_BOOL OnRButtonDown(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy) override;
  FX_BOOL OnRButtonUp(uint32_t dwFlags, FX_FLOAT fx, FX_FLOAT fy) override;
  FX_BOOL OnSetFocus(CXFA_FFWidget* pOldWidget) override;
  FX_BOOL OnKillFocus(CXFA_FFWidget* pNewWidget) override;
  FX_BOOL CanUndo() override;
  FX_BOOL CanRedo() override;
  FX_BOOL Undo() override;
  FX_BOOL Redo() override;
  FX_BOOL CanCopy() override;
  FX_BOOL CanCut() override;
  FX_BOOL CanPaste() override;
  FX_BOOL CanSelectAll() override;
  FX_BOOL Copy(CFX_WideString& wsCopy) override;
  FX_BOOL Cut(CFX_WideString& wsCut) override;
  FX_BOOL Paste(const CFX_WideString& wsPaste) override;
  FX_BOOL SelectAll() override;
  FX_BOOL Delete() override;
  FX_BOOL DeSelect() override;
  FX_BOOL GetSuggestWords(CFX_PointF pointf,
                          std::vector<CFX_ByteString>& sSuggest) override;
  FX_BOOL ReplaceSpellCheckWord(CFX_PointF pointf,
                                const CFX_ByteStringC& bsReplace) override;

  // IFWL_WidgetDelegate:
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = NULL) override;

  void OnTextChanged(IFWL_Widget* pWidget,
                     const CFX_WideString& wsChanged,
                     const CFX_WideString& wsPrevText);
  void OnTextFull(IFWL_Widget* pWidget);
  FX_BOOL CheckWord(const CFX_ByteStringC& sWord);
  FX_BOOL GetSuggestWords(const CFX_ByteStringC& sWord,
                          std::vector<CFX_ByteString>& sSuggest);

 protected:
  uint32_t GetAlignment();
  FX_BOOL CommitData() override;
  FX_BOOL UpdateFWLData() override;
  FX_BOOL IsDataChanged() override;
  void ValidateNumberField(const CFX_WideString& wsText);

  IFWL_WidgetDelegate* m_pOldDelegate;
};

class CXFA_FFNumericEdit : public CXFA_FFTextEdit {
 public:
  CXFA_FFNumericEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFNumericEdit();
  virtual FX_BOOL LoadWidget();
  virtual void UpdateWidgetProperty();
  virtual void OnProcessEvent(CFWL_Event* pEvent);

 public:
  FX_BOOL OnValidate(IFWL_Widget* pWidget, CFX_WideString& wsText);
};
class CXFA_FFPasswordEdit : public CXFA_FFTextEdit {
 public:
  CXFA_FFPasswordEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFPasswordEdit();
  virtual FX_BOOL LoadWidget();
  virtual void UpdateWidgetProperty();

 protected:
};
enum XFA_DATETIMETYPE {
  XFA_DATETIMETYPE_Date = 0,
  XFA_DATETIMETYPE_Time,
  XFA_DATETIMETYPE_DateAndTime
};
class CXFA_FFDateTimeEdit : public CXFA_FFTextEdit {
 public:
  CXFA_FFDateTimeEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFDateTimeEdit();

  virtual FX_BOOL GetBBox(CFX_RectF& rtBox,
                          uint32_t dwStatus,
                          FX_BOOL bDrawFocus = FALSE);
  virtual FX_BOOL LoadWidget();
  virtual void UpdateWidgetProperty();

  virtual FX_BOOL CanUndo();
  virtual FX_BOOL CanRedo();
  virtual FX_BOOL Undo();
  virtual FX_BOOL Redo();

  virtual FX_BOOL CanCopy();
  virtual FX_BOOL CanCut();
  virtual FX_BOOL CanPaste();
  virtual FX_BOOL CanSelectAll();
  virtual FX_BOOL Copy(CFX_WideString& wsCopy);
  virtual FX_BOOL Cut(CFX_WideString& wsCut);
  virtual FX_BOOL Paste(const CFX_WideString& wsPaste);
  virtual FX_BOOL SelectAll();
  virtual FX_BOOL Delete();
  virtual FX_BOOL DeSelect();

 protected:
  uint32_t GetAlignment();

  virtual FX_BOOL PtInActiveRect(FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL CommitData();
  virtual FX_BOOL UpdateFWLData();
  virtual FX_BOOL IsDataChanged();

 public:
  void OnSelectChanged(IFWL_Widget* pWidget,
                       int32_t iYear,
                       int32_t iMonth,
                       int32_t iDay);
  virtual void OnProcessEvent(CFWL_Event* pEvent);
};

#endif  // XFA_FXFA_APP_XFA_FFTEXTEDIT_H_

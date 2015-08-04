// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_TEXTEDIT_IMP_H
#define _FXFA_FORMFILLER_TEXTEDIT_IMP_H
class CXFA_FFTextEdit : public CXFA_FFField {
 public:
  CXFA_FFTextEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFTextEdit();
  virtual FX_BOOL LoadWidget();
  virtual void UpdateWidgetProperty();
  virtual FX_BOOL OnLButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDown(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnRButtonUp(FX_DWORD dwFlags, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnSetFocus(CXFA_FFWidget* pOldWidget);
  virtual FX_BOOL OnKillFocus(CXFA_FFWidget* pNewWidget);
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
  FX_BOOL GetSuggestWords(CFX_PointF pointf, CFX_ByteStringArray& sSuggest);
  FX_BOOL ReplaceSpellCheckWord(CFX_PointF pointf,
                                const CFX_ByteStringC& bsReplace);

 protected:
  FX_DWORD GetAlignment();
  virtual FX_BOOL CommitData();
  virtual FX_BOOL UpdateFWLData();
  virtual FX_BOOL IsDataChanged();
  void ValidateNumberField(const CFX_WideString& wsText);
  IFWL_WidgetDelegate* m_pOldDelegate;

 public:
  void OnTextChanged(IFWL_Widget* pWidget,
                     const CFX_WideString& wsChanged,
                     const CFX_WideString& wsPrevText);
  void OnTextFull(IFWL_Widget* pWidget);
  void OnAddDoRecord(IFWL_Widget* pWidget);
  FX_BOOL CheckWord(const CFX_ByteStringC& sWord);
  FX_BOOL GetSuggestWords(const CFX_ByteStringC& sWord,
                          CFX_ByteStringArray& sSuggest);
  virtual int32_t OnProcessMessage(CFWL_Message* pMessage);
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);
  virtual FWL_ERR OnDrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = NULL);
};
class CXFA_FFNumericEdit : public CXFA_FFTextEdit {
 public:
  CXFA_FFNumericEdit(CXFA_FFPageView* pPageView, CXFA_WidgetAcc* pDataAcc);
  virtual ~CXFA_FFNumericEdit();
  virtual FX_BOOL LoadWidget();
  virtual void UpdateWidgetProperty();
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);

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
                          FX_DWORD dwStatus,
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
  FX_DWORD GetAlignment();

  virtual FX_BOOL PtInActiveRect(FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL CommitData();
  virtual FX_BOOL UpdateFWLData();
  virtual FX_BOOL IsDataChanged();

 public:
  void OnSelectChanged(IFWL_Widget* pWidget,
                       int32_t iYear,
                       int32_t iMonth,
                       int32_t iDay);
  virtual FWL_ERR OnProcessEvent(CFWL_Event* pEvent);
};
#endif

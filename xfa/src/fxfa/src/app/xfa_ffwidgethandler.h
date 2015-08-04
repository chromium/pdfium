// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FXFA_FORMFILLER_ANNOTHANDLER_IMP_H
#define _FXFA_FORMFILLER_ANNOTHANDLER_IMP_H
class CXFA_FFDocView;
class CXFA_FFWidgetHandler : public IXFA_WidgetHandler {
 public:
  CXFA_FFWidgetHandler(CXFA_FFDocView* pDocView);
  ~CXFA_FFWidgetHandler();
  virtual IXFA_Widget* CreateWidget(IXFA_Widget* hParent,
                                    XFA_WIDGETTYPE eType,
                                    IXFA_Widget* hBefore = NULL);
  virtual IXFA_PageView* GetPageView(IXFA_Widget* hWidget);
  virtual void GetRect(IXFA_Widget* hWidget, CFX_RectF& rt);
  virtual FX_DWORD GetStatus(IXFA_Widget* hWidget);
  virtual FX_BOOL GetBBox(IXFA_Widget* hWidget,
                          CFX_RectF& rtBox,
                          FX_DWORD dwStatus,
                          FX_BOOL bDrawFocus = FALSE);
  virtual CXFA_WidgetAcc* GetDataAcc(IXFA_Widget* hWidget);
  virtual void GetName(IXFA_Widget* hWidget,
                       CFX_WideString& wsName,
                       int32_t iNameType = 0);
  virtual FX_BOOL GetToolTip(IXFA_Widget* hWidget, CFX_WideString& wsToolTip);
  virtual void SetPrivateData(IXFA_Widget* hWidget,
                              void* module_id,
                              void* pData,
                              PD_CALLBACK_FREEDATA callback);
  virtual void* GetPrivateData(IXFA_Widget* hWidget, void* module_id);
  virtual FX_BOOL OnMouseEnter(IXFA_Widget* hWidget);
  virtual FX_BOOL OnMouseExit(IXFA_Widget* hWidget);
  virtual FX_BOOL OnLButtonDown(IXFA_Widget* hWidget,
                                FX_DWORD dwFlags,
                                FX_FLOAT fx,
                                FX_FLOAT fy);
  virtual FX_BOOL OnLButtonUp(IXFA_Widget* hWidget,
                              FX_DWORD dwFlags,
                              FX_FLOAT fx,
                              FX_FLOAT fy);
  virtual FX_BOOL OnLButtonDblClk(IXFA_Widget* hWidget,
                                  FX_DWORD dwFlags,
                                  FX_FLOAT fx,
                                  FX_FLOAT fy);
  virtual FX_BOOL OnMouseMove(IXFA_Widget* hWidget,
                              FX_DWORD dwFlags,
                              FX_FLOAT fx,
                              FX_FLOAT fy);
  virtual FX_BOOL OnMouseWheel(IXFA_Widget* hWidget,
                               FX_DWORD dwFlags,
                               int16_t zDelta,
                               FX_FLOAT fx,
                               FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDown(IXFA_Widget* hWidget,
                                FX_DWORD dwFlags,
                                FX_FLOAT fx,
                                FX_FLOAT fy);
  virtual FX_BOOL OnRButtonUp(IXFA_Widget* hWidget,
                              FX_DWORD dwFlags,
                              FX_FLOAT fx,
                              FX_FLOAT fy);
  virtual FX_BOOL OnRButtonDblClk(IXFA_Widget* hWidget,
                                  FX_DWORD dwFlags,
                                  FX_FLOAT fx,
                                  FX_FLOAT fy);

  virtual FX_BOOL OnKeyDown(IXFA_Widget* hWidget,
                            FX_DWORD dwKeyCode,
                            FX_DWORD dwFlags);
  virtual FX_BOOL OnKeyUp(IXFA_Widget* hWidget,
                          FX_DWORD dwKeyCode,
                          FX_DWORD dwFlags);
  virtual FX_BOOL OnChar(IXFA_Widget* hWidget,
                         FX_DWORD dwChar,
                         FX_DWORD dwFlags);
  virtual FX_DWORD OnHitTest(IXFA_Widget* hWidget, FX_FLOAT fx, FX_FLOAT fy);
  virtual FX_BOOL OnSetCursor(IXFA_Widget* hWidget, FX_FLOAT fx, FX_FLOAT fy);
  virtual void RenderWidget(IXFA_Widget* hWidget,
                            CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix = NULL,
                            FX_BOOL bHighlight = FALSE);
  virtual FX_BOOL HasEvent(CXFA_WidgetAcc* pWidgetAcc,
                           XFA_EVENTTYPE eEventType);
  virtual int32_t ProcessEvent(CXFA_WidgetAcc* pWidgetAcc,
                               CXFA_EventParam* pParam);

 protected:
  CXFA_Node* CreateWidgetFormItem(XFA_WIDGETTYPE eType,
                                  CXFA_Node* pParent,
                                  CXFA_Node* pBefore) const;

  CXFA_Node* CreatePushButton(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateCheckButton(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateExclGroup(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateRadioButton(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateDatetimeEdit(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateDecimalField(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateNumericField(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateSignature(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateTextEdit(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateDropdownList(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateListBox(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateImageField(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreatePasswordEdit(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateField(XFA_ELEMENT eElement,
                         CXFA_Node* pParent,
                         CXFA_Node* pBefore) const;
  CXFA_Node* CreateArc(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateRectangle(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateImage(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateLine(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateText(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateDraw(XFA_ELEMENT eElement,
                        CXFA_Node* pParent,
                        CXFA_Node* pBefore) const;

  CXFA_Node* CreateSubform(CXFA_Node* pParent, CXFA_Node* pBefore) const;
  CXFA_Node* CreateFormItem(XFA_ELEMENT eElement,
                            CXFA_Node* pParent,
                            CXFA_Node* pBefore) const;
  CXFA_Node* CreateCopyNode(XFA_ELEMENT eElement,
                            CXFA_Node* pParent,
                            CXFA_Node* pBefore = NULL) const;
  CXFA_Node* CreateTemplateNode(XFA_ELEMENT eElement,
                                CXFA_Node* pParent,
                                CXFA_Node* pBefore) const;
  CXFA_Node* CreateFontNode(CXFA_Node* pParent) const;
  CXFA_Node* CreateMarginNode(CXFA_Node* pParent,
                              FX_DWORD dwFlags,
                              FX_FLOAT fInsets[4]) const;
  CXFA_Node* CreateValueNode(XFA_ELEMENT eValue, CXFA_Node* pParent) const;
  IXFA_ObjFactory* GetObjFactory() const;
  CXFA_Document* GetXFADoc() const;

  CXFA_FFDocView* m_pDocView;
};
class CXFA_FFMenuHandler : public IXFA_MenuHandler {
 public:
  CXFA_FFMenuHandler();
  ~CXFA_FFMenuHandler();
  virtual FX_BOOL CanCopy(IXFA_Widget* hWidget);
  virtual FX_BOOL CanCut(IXFA_Widget* hWidget);
  virtual FX_BOOL CanPaste(IXFA_Widget* hWidget);
  virtual FX_BOOL CanSelectAll(IXFA_Widget* hWidget);
  virtual FX_BOOL CanDelete(IXFA_Widget* hWidget);
  virtual FX_BOOL CanDeSelect(IXFA_Widget* hWidget);
  virtual FX_BOOL Copy(IXFA_Widget* hWidget, CFX_WideString& wsText);
  virtual FX_BOOL Cut(IXFA_Widget* hWidget, CFX_WideString& wsText);
  virtual FX_BOOL Paste(IXFA_Widget* hWidget, const CFX_WideString& wsText);
  virtual FX_BOOL SelectAll(IXFA_Widget* hWidget);
  virtual FX_BOOL Delete(IXFA_Widget* hWidget);
  virtual FX_BOOL DeSelect(IXFA_Widget* hWidget);
  virtual FX_BOOL CanUndo(IXFA_Widget* hWidget);
  virtual FX_BOOL CanRedo(IXFA_Widget* hWidget);
  virtual FX_BOOL Undo(IXFA_Widget* hWidget);
  virtual FX_BOOL Redo(IXFA_Widget* hWidget);
  virtual FX_BOOL GetSuggestWords(IXFA_Widget* hWidget,
                                  CFX_PointF pointf,
                                  CFX_ByteStringArray& sSuggest);
  virtual FX_BOOL ReplaceSpellCheckWord(IXFA_Widget* hWidget,
                                        CFX_PointF pointf,
                                        const CFX_ByteStringC& bsReplace);
};
#endif

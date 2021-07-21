// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_COMBOBOX_H_
#define XFA_FWL_CFWL_COMBOBOX_H_

#include "xfa/fgas/graphics/cfgas_gegraphics.h"
#include "xfa/fwl/cfwl_comboedit.h"
#include "xfa/fwl/cfwl_combolist.h"
#include "xfa/fwl/cfwl_listbox.h"
#include "xfa/fwl/cfwl_widget.h"

#define FWL_STYLEEXT_CMB_DropDown (1L << 0)
#define FWL_STYLEEXT_CMB_Sort (1L << 1)
#define FWL_STYLEEXT_CMB_OwnerDraw (1L << 3)
#define FWL_STYLEEXT_CMB_EditHNear 0
#define FWL_STYLEEXT_CMB_EditHCenter (1L << 4)
#define FWL_STYLEEXT_CMB_EditVNear 0
#define FWL_STYLEEXT_CMB_EditVCenter (1L << 6)
#define FWL_STYLEEXT_CMB_EditVFar (2L << 6)
#define FWL_STYLEEXT_CMB_EditJustified (1L << 8)
#define FWL_STYLEEXT_CMB_EditHAlignMask (3L << 4)
#define FWL_STYLEEXT_CMB_EditVAlignMask (3L << 6)
#define FWL_STYLEEXT_CMB_ListItemLeftAlign 0
#define FWL_STYLEEXT_CMB_ListItemCenterAlign (1L << 10)
#define FWL_STYLEEXT_CMB_ListItemAlignMask (3L << 10)
#define FWL_STYLEEXT_CMB_ReadOnly (1L << 13)

class CFWL_ComboBox final : public CFWL_Widget {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CFWL_ComboBox() override;

  // CFWL_Widget
  void Trace(cppgc::Visitor* visitor) const override;
  FWL_Type GetClassID() const override;
  void ModifyStyleExts(uint32_t dwStyleExtsAdded,
                       uint32_t dwStyleExtsRemoved) override;
  void SetStates(uint32_t dwStates) override;
  void RemoveStates(uint32_t dwStates) override;
  void Update() override;
  FWL_WidgetHit HitTest(const CFX_PointF& point) override;
  void DrawWidget(CFGAS_GEGraphics* pGraphics,
                  const CFX_Matrix& matrix) override;
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFGAS_GEGraphics* pGraphics,
                    const CFX_Matrix& matrix) override;

  WideString GetTextByIndex(int32_t iIndex) const;
  int32_t GetCurSel() const { return m_iCurSel; }
  void SetCurSel(int32_t iSel);

  void AddString(const WideString& wsText);
  void RemoveAt(int32_t iIndex);
  void RemoveAll();

  void SetEditText(const WideString& wsText);
  WideString GetEditText() const;

  bool EditCanUndo() const { return m_pEdit->CanUndo(); }
  bool EditCanRedo() const { return m_pEdit->CanRedo(); }
  bool EditUndo() { return m_pEdit->Undo(); }
  bool EditRedo() { return m_pEdit->Redo(); }
  bool EditCanCopy() const { return m_pEdit->HasSelection(); }
  bool EditCanCut() const {
    if (m_pEdit->GetStyleExts() & FWL_STYLEEXT_EDT_ReadOnly)
      return false;
    return EditCanCopy();
  }
  bool EditCanSelectAll() const { return m_pEdit->GetTextLength() > 0; }
  Optional<WideString> EditCopy() const { return m_pEdit->Copy(); }
  Optional<WideString> EditCut() { return m_pEdit->Cut(); }
  bool EditPaste(const WideString& wsPaste) { return m_pEdit->Paste(wsPaste); }
  void EditSelectAll() { m_pEdit->SelectAll(); }
  void EditDelete() { m_pEdit->ClearText(); }
  void EditDeSelect() { m_pEdit->ClearSelection(); }

  CFX_RectF GetBBox() const;
  void EditModifyStyleExts(uint32_t dwStyleExtsAdded,
                           uint32_t dwStyleExtsRemoved);
  void ShowDropDownList();
  void HideDropDownList();

  CFWL_ComboEdit* GetComboEdit() const { return m_pEdit; }

  void ProcessSelChanged(bool bLButtonUp);
  int32_t GetCurrentSelection() const { return m_iCurSel; }

 private:
  explicit CFWL_ComboBox(CFWL_App* pApp);

  bool IsDropDownStyle() const {
    return !!(GetStyleExts() & FWL_STYLEEXT_CMB_DropDown);
  }
  void MatchEditText();
  void SyncEditText(int32_t iListItem);
  void Layout();
  void ResetEditAlignment();
  void ResetListItemAlignment();
  void GetPopupPos(float fMinHeight,
                   float fMaxHeight,
                   const CFX_RectF& rtAnchor,
                   CFX_RectF* pPopupRect);
  void OnLButtonUp(CFWL_MessageMouse* pMsg);
  bool IsDropListVisible() const { return m_pListBox->IsVisible(); }
  void OnLButtonDown(CFWL_MessageMouse* pMsg);
  void OnFocusGained();
  void OnFocusLost();
  void OnKey(CFWL_MessageKey* pMsg);
  void RepaintInflatedListBoxRect();

  CFX_RectF m_ClientRect;
  CFX_RectF m_ContentRect;
  CFX_RectF m_BtnRect;
  cppgc::Member<CFWL_ComboEdit> const m_pEdit;
  cppgc::Member<CFWL_ComboList> const m_pListBox;
  int32_t m_iCurSel = -1;
  int32_t m_iBtnState = CFWL_PartState_Normal;
};

#endif  // XFA_FWL_CFWL_COMBOBOX_H_

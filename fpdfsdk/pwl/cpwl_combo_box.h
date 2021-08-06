// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_COMBO_BOX_H_
#define FPDFSDK_PWL_CPWL_COMBO_BOX_H_

#include <memory>

#include "core/fxcrt/unowned_ptr.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"

class CFFL_FormField;
class CPWL_Edit;
class CPWL_CBButton;
class CPWL_CBListBox;
class IPWL_FillerNotify;

class CPWL_ComboBox final : public CPWL_Wnd {
 public:
  CPWL_ComboBox(
      const CreateParams& cp,
      std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData);
  ~CPWL_ComboBox() override;

  CPWL_Edit* GetEdit() const { return m_pEdit.Get(); }

  // CPWL_Wnd:
  void OnDestroy() override;
  bool OnKeyDown(FWL_VKEYCODE nChar, FWL_EventFlagMask nFlag) override;
  bool OnChar(uint16_t nChar, FWL_EventFlagMask nFlag) override;
  void NotifyLButtonDown(CPWL_Wnd* child, const CFX_PointF& pos) override;
  void NotifyLButtonUp(CPWL_Wnd* child, const CFX_PointF& pos) override;
  void CreateChildWnd(const CreateParams& cp) override;
  bool RePosChildWnd() override;
  CFX_FloatRect GetFocusRect() const override;
  void SetFocus() override;
  void KillFocus() override;
  WideString GetText() override;
  WideString GetSelectedText() override;
  void ReplaceSelection(const WideString& text) override;
  bool SelectAllText() override;
  bool CanUndo() override;
  bool CanRedo() override;
  bool Undo() override;
  bool Redo() override;

  void SetFillerNotify(IPWL_FillerNotify* pNotify);

  void SetText(const WideString& text);
  void AddString(const WideString& str);
  int32_t GetSelect() const;
  void SetSelect(int32_t nItemIndex);

  void SetEditSelection(int32_t nStartChar, int32_t nEndChar);
  void ClearSelection();
  void SelectAll();
  bool IsPopup() const;
  void SetSelectText();
  void AttachFFLData(CFFL_FormField* pData) { m_pFormFiller = pData; }

 private:
  void CreateEdit(const CreateParams& cp);
  void CreateButton(const CreateParams& cp);
  void CreateListBox(const CreateParams& cp);

  // Returns |true| iff this instance is still allocated.
  bool SetPopup(bool bPopup);

  UnownedPtr<CPWL_Edit> m_pEdit;
  UnownedPtr<CPWL_CBButton> m_pButton;
  UnownedPtr<CPWL_CBListBox> m_pList;
  CFX_FloatRect m_rcOldWindow;
  bool m_bPopup = false;
  bool m_bBottom = true;
  int32_t m_nSelectItem = -1;
  UnownedPtr<IPWL_FillerNotify> m_pFillerNotify;
  UnownedPtr<CFFL_FormField> m_pFormFiller;
};

#endif  // FPDFSDK_PWL_CPWL_COMBO_BOX_H_

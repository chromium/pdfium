// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_EDIT_H_
#define FPDFSDK_PWL_CPWL_EDIT_H_

#include <memory>
#include <utility>

#include "core/fpdfdoc/cpvt_wordrange.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "fpdfsdk/pwl/cpwl_wnd.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"

class CPDF_Font;
class CPWL_Caret;
class CPWL_EditImpl;
class IPWL_FillerNotify;

enum PWL_EDIT_ALIGNFORMAT_H { PEAH_LEFT = 0, PEAH_MIDDLE, PEAH_RIGHT };

enum PWL_EDIT_ALIGNFORMAT_V { PEAV_TOP = 0, PEAV_CENTER, PEAV_BOTTOM };

class CPWL_Edit final : public CPWL_Wnd {
 public:
  CPWL_Edit(const CreateParams& cp,
            std::unique_ptr<IPWL_FillerNotify::PerWindowData> pAttachedData);
  ~CPWL_Edit() override;

  // CPWL_Wnd:
  bool RepositionChildWnd() override;
  CFX_FloatRect GetClientRect() const override;
  void DrawThisAppearance(CFX_RenderDevice* pDevice,
                          const CFX_Matrix& mtUser2Device) override;
  bool OnMouseWheel(Mask<FWL_EVENTFLAG> nFlag,
                    const CFX_PointF& point,
                    const CFX_Vector& delta) override;
  bool OnKeyDown(FWL_VKEYCODE nKeyCode, Mask<FWL_EVENTFLAG> nFlag) override;
  bool OnChar(uint16_t nChar, Mask<FWL_EVENTFLAG> nFlag) override;
  CFX_FloatRect GetFocusRect() const override;
  void OnSetFocus() override;
  void OnKillFocus() override;
  void OnCreated() override;
  void OnDestroy() override;
  bool OnLButtonDown(Mask<FWL_EVENTFLAG> nFlag,
                     const CFX_PointF& point) override;
  bool OnLButtonUp(Mask<FWL_EVENTFLAG> nFlag, const CFX_PointF& point) override;
  bool OnLButtonDblClk(Mask<FWL_EVENTFLAG> nFlag,
                       const CFX_PointF& point) override;
  bool OnRButtonUp(Mask<FWL_EVENTFLAG> nFlag, const CFX_PointF& point) override;
  bool OnMouseMove(Mask<FWL_EVENTFLAG> nFlag, const CFX_PointF& point) override;
  void SetScrollInfo(const PWL_SCROLL_INFO& info) override;
  void SetScrollPosition(float pos) override;
  void ScrollWindowVertically(float pos) override;
  void CreateChildWnd(const CreateParams& cp) override;
  void SetFontSize(float fFontSize) override;
  float GetFontSize() const override;
  void SetCursor() override;
  WideString GetText() override;
  WideString GetSelectedText() override;
  void ReplaceAndKeepSelection(const WideString& text) override;
  void ReplaceSelection(const WideString& text) override;
  bool SelectAllText() override;
  bool CanUndo() override;
  bool CanRedo() override;
  bool Undo() override;
  bool Redo() override;

  void SetSelection(int32_t nStartChar, int32_t nEndChar);
  std::pair<int32_t, int32_t> GetSelection() const;
  void ClearSelection();

  CFX_PointF GetScrollPos() const;
  void SetScrollPos(const CFX_PointF& point);

  void SetCharSet(FX_Charset nCharSet) { char_set_ = nCharSet; }
  FX_Charset GetCharSet() const { return char_set_; }

  void SetReadyToInput();
  void SetAlignFormatVerticalCenter();
  void SetCharArray(int32_t nCharArray);
  void SetLimitChar(int32_t nLimitChar);
  bool CanSelectAll() const;
  void SetText(const WideString& csText);
  bool IsTextFull() const;
  void SetMaxUndoItemsForTest(size_t items);

  static float GetCharArrayAutoFontSize(const CPDF_Font* font,
                                        const CFX_FloatRect& rcPlate,
                                        int32_t nCharArray);

  bool SetCaret(bool bVisible,
                const CFX_PointF& ptHead,
                const CFX_PointF& ptFoot);

 private:
  bool OnKeyDownInternal(FWL_VKEYCODE nKeyCode, Mask<FWL_EVENTFLAG> nFlag);
  bool OnCharInternal(uint16_t nChar, Mask<FWL_EVENTFLAG> nFlag);

  bool IsWndHorV() const;
  void Delete();
  void GetCaretInfo(CFX_PointF* ptHead, CFX_PointF* ptFoot) const;
  void SetEditCaret(bool bVisible);

  void CreateEditCaret(const CreateParams& cp);

  CPVT_WordRange GetSelectWordRange() const;
  bool IsVScrollBarVisible() const;
  void SetParamByFlag();

  bool mouse_down_ = false;
  bool focus_ = false;
  FX_Charset char_set_ = FX_Charset::kDefault;
  CFX_FloatRect old_window_rect_;
  std::unique_ptr<CPWL_EditImpl> const edit_impl_;
  UnownedPtr<CPWL_Caret> caret_;
};

#endif  // FPDFSDK_PWL_CPWL_EDIT_H_

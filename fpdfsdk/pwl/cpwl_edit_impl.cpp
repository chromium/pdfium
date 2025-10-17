// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fpdfsdk/pwl/cpwl_edit_impl.h"

#include <algorithm>
#include <memory>
#include <utility>

#include "constants/ascii.h"
#include "core/fpdfapi/font/cpdf_font.h"
#include "core/fpdfapi/render/cpdf_renderoptions.h"
#include "core/fpdfapi/render/cpdf_textrenderer.h"
#include "core/fpdfdoc/cpvt_word.h"
#include "core/fpdfdoc/ipvt_fontmap.h"
#include "core/fxcrt/autorestorer.h"
#include "core/fxcrt/check.h"
#include "core/fxcrt/check_op.h"
#include "core/fxcrt/fx_codepage.h"
#include "core/fxge/cfx_fillrenderoptions.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "core/fxge/cfx_path.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/pwl/cpwl_edit.h"
#include "fpdfsdk/pwl/cpwl_scroll_bar.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"

namespace {

void DrawTextString(CFX_RenderDevice* pDevice,
                    const CFX_PointF& pt,
                    CPDF_Font* font,
                    float fFontSize,
                    const CFX_Matrix& mtUser2Device,
                    const ByteString& str,
                    FX_ARGB crTextFill) {
  if (!font) {
    return;
  }

  CFX_PointF pos = mtUser2Device.Transform(pt);
  CPDF_RenderOptions ro;
  CHECK(ro.GetOptions().bClearType);
  ro.SetColorMode(CPDF_RenderOptions::kNormal);
  CPDF_TextRenderer::DrawTextString(pDevice, pos.x, pos.y, font, fFontSize,
                                    mtUser2Device, str, crTextFill, ro);
}

}  // namespace

CPWL_EditImpl::Iterator::Iterator(CPWL_EditImpl* pEdit,
                                  CPVT_VariableText::Iterator* pVTIterator)
    : edit_(pEdit), vt_iterator_(pVTIterator) {}

CPWL_EditImpl::Iterator::~Iterator() = default;

bool CPWL_EditImpl::Iterator::NextWord() {
  return vt_iterator_->NextWord();
}

bool CPWL_EditImpl::Iterator::GetWord(CPVT_Word& word) const {
  CHECK(edit_);

  if (vt_iterator_->GetWord(word)) {
    word.ptWord = edit_->VTToEdit(word.ptWord);
    return true;
  }
  return false;
}

bool CPWL_EditImpl::Iterator::GetLine(CPVT_Line& line) const {
  CHECK(edit_);

  if (vt_iterator_->GetLine(line)) {
    line.ptLine = edit_->VTToEdit(line.ptLine);
    return true;
  }
  return false;
}

void CPWL_EditImpl::Iterator::SetAt(int32_t nWordIndex) {
  vt_iterator_->SetAt(nWordIndex);
}

void CPWL_EditImpl::Iterator::SetAt(const CPVT_WordPlace& place) {
  vt_iterator_->SetAt(place);
}

const CPVT_WordPlace& CPWL_EditImpl::Iterator::GetAt() const {
  return vt_iterator_->GetWordPlace();
}

class CPWL_EditImpl::Provider final : public CPVT_VariableText::Provider {
 public:
  explicit Provider(IPVT_FontMap* font_map);
  ~Provider() override;

  // CPVT_VariableText::Provider:
  int GetCharWidth(int32_t nFontIndex, uint16_t word) override;
  int32_t GetWordFontIndex(uint16_t word,
                           FX_Charset charset,
                           int32_t nFontIndex) override;
};

CPWL_EditImpl::Provider::Provider(IPVT_FontMap* font_map)
    : CPVT_VariableText::Provider(font_map) {}

CPWL_EditImpl::Provider::~Provider() = default;

int CPWL_EditImpl::Provider::GetCharWidth(int32_t nFontIndex, uint16_t word) {
  RetainPtr<CPDF_Font> pPDFFont = GetFontMap()->GetPDFFont(nFontIndex);
  if (!pPDFFont) {
    return 0;
  }

  uint32_t charcode = pPDFFont->IsUnicodeCompatible()
                          ? pPDFFont->CharCodeFromUnicode(word)
                          : GetFontMap()->CharCodeFromUnicode(nFontIndex, word);
  if (charcode == CPDF_Font::kInvalidCharCode) {
    return 0;
  }

  return pPDFFont->GetCharWidthF(charcode);
}

int32_t CPWL_EditImpl::Provider::GetWordFontIndex(uint16_t word,
                                                  FX_Charset charset,
                                                  int32_t nFontIndex) {
  return GetFontMap()->GetWordFontIndex(word, charset, nFontIndex);
}

CPWL_EditImpl::RefreshState::RefreshState() = default;

CPWL_EditImpl::RefreshState::~RefreshState() = default;

void CPWL_EditImpl::RefreshState::BeginRefresh() {
  old_line_rects_ = std::move(new_line_rects_);
  new_line_rects_.clear();
  refresh_rects_.clear();
}

void CPWL_EditImpl::RefreshState::Push(const CPVT_WordRange& linerange,
                                       const CFX_FloatRect& rect) {
  new_line_rects_.emplace_back(linerange, rect);
}

void CPWL_EditImpl::RefreshState::NoAnalyse() {
  for (const auto& lineRect : old_line_rects_) {
    Add(lineRect.line_rect_);
  }

  for (const auto& lineRect : new_line_rects_) {
    Add(lineRect.line_rect_);
  }
}

std::vector<CFX_FloatRect>* CPWL_EditImpl::RefreshState::GetRefreshRects() {
  return &refresh_rects_;
}

void CPWL_EditImpl::RefreshState::EndRefresh() {
  refresh_rects_.clear();
}

void CPWL_EditImpl::RefreshState::Add(const CFX_FloatRect& new_rect) {
  // Check for overlapped area.
  for (const auto& rect : refresh_rects_) {
    if (rect.Contains(new_rect)) {
      return;
    }
  }
  refresh_rects_.push_back(new_rect);
}

bool CPWL_EditImpl::UndoItemIface::IsSentinel() {
  return false;
}

CPWL_EditImpl::UndoStack::UndoStack() = default;

CPWL_EditImpl::UndoStack::~UndoStack() = default;

bool CPWL_EditImpl::UndoStack::CanUndo() const {
  return cur_undo_pos_ > 0;
}

void CPWL_EditImpl::UndoStack::Undo() {
  CHECK(!working_);
  working_ = true;
  bool first_undo = true;
  while (CanUndo()) {
    --cur_undo_pos_;
    std::unique_ptr<UndoItemIface>& item = undo_item_stack_[cur_undo_pos_];
    item->Undo();

    if (first_undo) {
      first_undo = false;
      if (!item->IsSentinel()) {
        break;
      }
    } else {
      if (item->IsSentinel()) {
        break;
      }
    }
  }
  CHECK(working_);
  working_ = false;
}

bool CPWL_EditImpl::UndoStack::CanRedo() const {
  return cur_undo_pos_ < undo_item_stack_.size();
}

void CPWL_EditImpl::UndoStack::Redo() {
  CHECK(!working_);
  working_ = true;

  bool first_undo = true;
  while (CanRedo()) {
    std::unique_ptr<UndoItemIface>& item = undo_item_stack_[cur_undo_pos_];
    ++cur_undo_pos_;
    item->Redo();
    if (first_undo) {
      first_undo = false;
      if (!item->IsSentinel()) {
        break;
      }
    } else {
      if (item->IsSentinel()) {
        break;
      }
    }
  }
  CHECK(working_);
  working_ = false;
}

void CPWL_EditImpl::UndoStack::SetMaxUndoItemsForTest(size_t items) {
  CHECK_GE(items, kMinEditUndoMaxItems);
  max_undo_items_ = items;
}

void CPWL_EditImpl::UndoStack::AddItem(std::unique_ptr<UndoItemIface> pItem) {
  CHECK(!working_);
  CHECK(pItem);
  if (CanRedo()) {
    RemoveTails();
  }

  if (undo_item_stack_.size() >= max_undo_items_) {
    RemoveHeads();
  }

  undo_item_stack_.push_back(std::move(pItem));
  cur_undo_pos_ = undo_item_stack_.size();
}

void CPWL_EditImpl::UndoStack::RemoveHeads() {
  CHECK(!undo_item_stack_.empty());
  if (!undo_item_stack_.front()->IsSentinel()) {
    undo_item_stack_.pop_front();
    return;
  }
  // Pop everything from the initial sentinel, until the next sentinel item. Or
  // keep popping until the queue is empty.
  undo_item_stack_.pop_front();
  while (!undo_item_stack_.empty()) {
    bool is_sentinel = undo_item_stack_.front()->IsSentinel();
    undo_item_stack_.pop_front();
    if (is_sentinel) {
      break;
    }
  }
}

void CPWL_EditImpl::UndoStack::RemoveTails() {
  // Note: this covers the sentinel items in the queue automatically, since it
  // always pops all redo items.
  while (CanRedo()) {
    undo_item_stack_.pop_back();
  }
}

class CPWL_EditImpl::UndoInsertWord final
    : public CPWL_EditImpl::UndoItemIface {
 public:
  UndoInsertWord(CPWL_EditImpl* pEdit,
                 const CPVT_WordPlace& wpOldPlace,
                 const CPVT_WordPlace& wpNewPlace,
                 uint16_t word,
                 FX_Charset charset);
  ~UndoInsertWord() override;

  // UndoItemIface:
  void Redo() override;
  void Undo() override;

 private:
  UnownedPtr<CPWL_EditImpl> edit_;

  CPVT_WordPlace wp_old_;
  CPVT_WordPlace wp_new_;
  uint16_t word_;
  FX_Charset charset_;
};

CPWL_EditImpl::UndoInsertWord::UndoInsertWord(CPWL_EditImpl* pEdit,
                                              const CPVT_WordPlace& wpOldPlace,
                                              const CPVT_WordPlace& wpNewPlace,
                                              uint16_t word,
                                              FX_Charset charset)
    : edit_(pEdit),
      wp_old_(wpOldPlace),
      wp_new_(wpNewPlace),
      word_(word),
      charset_(charset) {
  CHECK(edit_);
}

CPWL_EditImpl::UndoInsertWord::~UndoInsertWord() = default;

void CPWL_EditImpl::UndoInsertWord::Redo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_old_);
  edit_->InsertWord(word_, charset_, false);
}

void CPWL_EditImpl::UndoInsertWord::Undo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_new_);
  edit_->Backspace(false);
}

class CPWL_EditImpl::UndoInsertReturn final
    : public CPWL_EditImpl::UndoItemIface {
 public:
  UndoInsertReturn(CPWL_EditImpl* pEdit,
                   const CPVT_WordPlace& wpOldPlace,
                   const CPVT_WordPlace& wpNewPlace);
  ~UndoInsertReturn() override;

  // UndoItemIface:
  void Redo() override;
  void Undo() override;

 private:
  UnownedPtr<CPWL_EditImpl> edit_;

  CPVT_WordPlace wp_old_;
  CPVT_WordPlace wp_new_;
};

CPWL_EditImpl::UndoInsertReturn::UndoInsertReturn(
    CPWL_EditImpl* pEdit,
    const CPVT_WordPlace& wpOldPlace,
    const CPVT_WordPlace& wpNewPlace)
    : edit_(pEdit), wp_old_(wpOldPlace), wp_new_(wpNewPlace) {
  CHECK(edit_);
}

CPWL_EditImpl::UndoInsertReturn::~UndoInsertReturn() = default;

void CPWL_EditImpl::UndoInsertReturn::Redo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_old_);
  edit_->InsertReturn(false);
}

void CPWL_EditImpl::UndoInsertReturn::Undo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_new_);
  edit_->Backspace(false);
}

class CPWL_EditImpl::UndoReplaceSelection final
    : public CPWL_EditImpl::UndoItemIface {
 public:
  UndoReplaceSelection() = default;
  ~UndoReplaceSelection() override = default;

  // UndoItemIface:
  void Redo() override {}
  void Undo() override {}
  bool IsSentinel() override { return true; }

 private:
  UnownedPtr<CPWL_EditImpl> edit_;
};

class CPWL_EditImpl::UndoBackspace final : public CPWL_EditImpl::UndoItemIface {
 public:
  UndoBackspace(CPWL_EditImpl* pEdit,
                const CPVT_WordPlace& wpOldPlace,
                const CPVT_WordPlace& wpNewPlace,
                uint16_t word,
                FX_Charset charset);
  ~UndoBackspace() override;

  // UndoItemIface:
  void Redo() override;
  void Undo() override;

 private:
  UnownedPtr<CPWL_EditImpl> edit_;

  CPVT_WordPlace wp_old_;
  CPVT_WordPlace wp_new_;
  uint16_t word_;
  FX_Charset charset_;
};

CPWL_EditImpl::UndoBackspace::UndoBackspace(CPWL_EditImpl* pEdit,
                                            const CPVT_WordPlace& wpOldPlace,
                                            const CPVT_WordPlace& wpNewPlace,
                                            uint16_t word,
                                            FX_Charset charset)
    : edit_(pEdit),
      wp_old_(wpOldPlace),
      wp_new_(wpNewPlace),
      word_(word),
      charset_(charset) {
  CHECK(edit_);
}

CPWL_EditImpl::UndoBackspace::~UndoBackspace() = default;

void CPWL_EditImpl::UndoBackspace::Redo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_old_);
  edit_->Backspace(false);
}

void CPWL_EditImpl::UndoBackspace::Undo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_new_);
  if (wp_new_.nSecIndex != wp_old_.nSecIndex) {
    edit_->InsertReturn(false);
  } else {
    edit_->InsertWord(word_, charset_, false);
  }
}

class CPWL_EditImpl::UndoDelete final : public CPWL_EditImpl::UndoItemIface {
 public:
  UndoDelete(CPWL_EditImpl* pEdit,
             const CPVT_WordPlace& wpOldPlace,
             const CPVT_WordPlace& wpNewPlace,
             uint16_t word,
             FX_Charset charset,
             bool bSecEnd);
  ~UndoDelete() override;

  // UndoItemIface:
  void Redo() override;
  void Undo() override;

 private:
  UnownedPtr<CPWL_EditImpl> edit_;

  CPVT_WordPlace wp_old_;
  CPVT_WordPlace wp_new_;
  uint16_t word_;
  FX_Charset charset_;
  bool sec_end_;
};

CPWL_EditImpl::UndoDelete::UndoDelete(CPWL_EditImpl* pEdit,
                                      const CPVT_WordPlace& wpOldPlace,
                                      const CPVT_WordPlace& wpNewPlace,
                                      uint16_t word,
                                      FX_Charset charset,
                                      bool bSecEnd)
    : edit_(pEdit),
      wp_old_(wpOldPlace),
      wp_new_(wpNewPlace),
      word_(word),
      charset_(charset),
      sec_end_(bSecEnd) {
  CHECK(edit_);
}

CPWL_EditImpl::UndoDelete::~UndoDelete() = default;

void CPWL_EditImpl::UndoDelete::Redo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_old_);
  edit_->Delete(false);
}

void CPWL_EditImpl::UndoDelete::Undo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_new_);
  if (sec_end_) {
    edit_->InsertReturn(false);
  } else {
    edit_->InsertWord(word_, charset_, false);
  }
}

class CPWL_EditImpl::UndoClear final : public CPWL_EditImpl::UndoItemIface {
 public:
  UndoClear(CPWL_EditImpl* pEdit,
            const CPVT_WordRange& wrSel,
            const WideString& swText);
  ~UndoClear() override;

  // UndoItemIface:
  void Redo() override;
  void Undo() override;

 private:
  UnownedPtr<CPWL_EditImpl> edit_;

  CPVT_WordRange wr_sel_;
  WideString sw_text_;
};

CPWL_EditImpl::UndoClear::UndoClear(CPWL_EditImpl* pEdit,
                                    const CPVT_WordRange& wrSel,
                                    const WideString& swText)
    : edit_(pEdit), wr_sel_(wrSel), sw_text_(swText) {
  CHECK(edit_);
}

CPWL_EditImpl::UndoClear::~UndoClear() = default;

void CPWL_EditImpl::UndoClear::Redo() {
  edit_->SelectNone();
  edit_->SetSelection(wr_sel_.BeginPos, wr_sel_.EndPos);
  edit_->Clear(false);
}

void CPWL_EditImpl::UndoClear::Undo() {
  edit_->SelectNone();
  edit_->SetCaret(wr_sel_.BeginPos);
  edit_->InsertText(sw_text_, FX_Charset::kDefault, false);
  edit_->SetSelection(wr_sel_.BeginPos, wr_sel_.EndPos);
}

class CPWL_EditImpl::UndoInsertText final
    : public CPWL_EditImpl::UndoItemIface {
 public:
  UndoInsertText(CPWL_EditImpl* pEdit,
                 const CPVT_WordPlace& wpOldPlace,
                 const CPVT_WordPlace& wpNewPlace,
                 const WideString& swText,
                 FX_Charset charset);
  ~UndoInsertText() override;

  // UndoItemIface:
  void Redo() override;
  void Undo() override;

 private:
  UnownedPtr<CPWL_EditImpl> edit_;

  CPVT_WordPlace wp_old_;
  CPVT_WordPlace wp_new_;
  WideString sw_text_;
  FX_Charset charset_;
};

CPWL_EditImpl::UndoInsertText::UndoInsertText(CPWL_EditImpl* pEdit,
                                              const CPVT_WordPlace& wpOldPlace,
                                              const CPVT_WordPlace& wpNewPlace,
                                              const WideString& swText,
                                              FX_Charset charset)
    : edit_(pEdit),
      wp_old_(wpOldPlace),
      wp_new_(wpNewPlace),
      sw_text_(swText),
      charset_(charset) {
  CHECK(edit_);
}

CPWL_EditImpl::UndoInsertText::~UndoInsertText() = default;

void CPWL_EditImpl::UndoInsertText::Redo() {
  edit_->SelectNone();
  edit_->SetCaret(wp_old_);
  edit_->InsertText(sw_text_, charset_, false);
}

void CPWL_EditImpl::UndoInsertText::Undo() {
  edit_->SelectNone();
  edit_->SetSelection(wp_old_, wp_new_);
  edit_->Clear(false);
}

void CPWL_EditImpl::DrawEdit(CFX_RenderDevice* pDevice,
                             const CFX_Matrix& mtUser2Device,
                             FX_COLORREF crTextFill,
                             const CFX_FloatRect& rcClip,
                             const CFX_PointF& ptOffset,
                             const CPVT_WordRange* pRange,
                             IPWL_FillerNotify* pFillerNotify,
                             IPWL_FillerNotify::PerWindowData* pSystemData) {
  const bool bContinuous = GetCharArray() == 0;
  uint16_t SubWord = GetPasswordChar();
  float fFontSize = GetFontSize();
  CPVT_WordRange wrSelect = GetSelectWordRange();
  FX_COLORREF crCurFill = crTextFill;
  FX_COLORREF crOldFill = crCurFill;
  bool bSelect = false;
  const FX_COLORREF crWhite = ArgbEncode(255, 255, 255, 255);
  const FX_COLORREF crSelBK = ArgbEncode(255, 0, 51, 113);

  int32_t nFontIndex = -1;
  CFX_PointF ptBT;
  CFX_RenderDevice::StateRestorer restorer(pDevice);
  if (!rcClip.IsEmpty()) {
    pDevice->SetClip_Rect(mtUser2Device.TransformRect(rcClip).ToFxRect());
  }

  Iterator* pIterator = GetIterator();
  IPVT_FontMap* font_map = GetFontMap();
  if (!font_map) {
    return;
  }

  if (pRange) {
    pIterator->SetAt(pRange->BeginPos);
  } else {
    pIterator->SetAt(0);
  }

  ByteString sTextBuf;
  CPVT_WordPlace oldplace;
  while (pIterator->NextWord()) {
    CPVT_WordPlace place = pIterator->GetAt();
    if (pRange && place > pRange->EndPos) {
      break;
    }

    if (!wrSelect.IsEmpty()) {
      bSelect = place > wrSelect.BeginPos && place <= wrSelect.EndPos;
      crCurFill = bSelect ? crWhite : crTextFill;
    }
    if (pFillerNotify->IsSelectionImplemented()) {
      crCurFill = crTextFill;
      crOldFill = crCurFill;
    }
    CPVT_Word word;
    if (pIterator->GetWord(word)) {
      if (bSelect) {
        CPVT_Line line;
        pIterator->GetLine(line);
        if (pFillerNotify->IsSelectionImplemented()) {
          CFX_FloatRect rc(word.ptWord.x, line.ptLine.y + line.fLineDescent,
                           word.ptWord.x + word.fWidth,
                           line.ptLine.y + line.fLineAscent);
          rc.Intersect(rcClip);
          pFillerNotify->OutputSelectedRect(pSystemData, rc);
        } else {
          CFX_Path pathSelBK;
          pathSelBK.AppendRect(word.ptWord.x, line.ptLine.y + line.fLineDescent,
                               word.ptWord.x + word.fWidth,
                               line.ptLine.y + line.fLineAscent);

          pDevice->DrawPath(pathSelBK, &mtUser2Device, nullptr, crSelBK, 0,
                            CFX_FillRenderOptions::WindingOptions());
        }
      }
      if (bContinuous) {
        if (place.LineCmp(oldplace) != 0 || word.nFontIndex != nFontIndex ||
            crOldFill != crCurFill) {
          if (!sTextBuf.IsEmpty()) {
            DrawTextString(pDevice,
                           CFX_PointF(ptBT.x + ptOffset.x, ptBT.y + ptOffset.y),
                           font_map->GetPDFFont(nFontIndex).Get(), fFontSize,
                           mtUser2Device, sTextBuf, crOldFill);
            sTextBuf.clear();
          }
          nFontIndex = word.nFontIndex;
          ptBT = word.ptWord;
          crOldFill = crCurFill;
        }
        sTextBuf += GetPDFWordString(word.nFontIndex, word.Word, SubWord);
      } else {
        DrawTextString(
            pDevice,
            CFX_PointF(word.ptWord.x + ptOffset.x, word.ptWord.y + ptOffset.y),
            font_map->GetPDFFont(word.nFontIndex).Get(), fFontSize,
            mtUser2Device,
            GetPDFWordString(word.nFontIndex, word.Word, SubWord), crCurFill);
      }
      oldplace = place;
    }
  }
  if (!sTextBuf.IsEmpty()) {
    DrawTextString(pDevice,
                   CFX_PointF(ptBT.x + ptOffset.x, ptBT.y + ptOffset.y),
                   font_map->GetPDFFont(nFontIndex).Get(), fFontSize,
                   mtUser2Device, sTextBuf, crOldFill);
  }
}

CPWL_EditImpl::CPWL_EditImpl()
    : vt_(std::make_unique<CPVT_VariableText>(nullptr)) {}

CPWL_EditImpl::~CPWL_EditImpl() = default;

void CPWL_EditImpl::Initialize() {
  vt_->Initialize();
  SetCaret(vt_->GetBeginWordPlace());
  SetCaretOrigin();
}

void CPWL_EditImpl::SetFontMap(IPVT_FontMap* font_map) {
  vt_provider_ = std::make_unique<Provider>(font_map);
  vt_->SetProvider(vt_provider_.get());
}

void CPWL_EditImpl::SetNotify(CPWL_Edit* pNotify) {
  notify_ = pNotify;
}

CPWL_EditImpl::Iterator* CPWL_EditImpl::GetIterator() {
  if (!iterator_) {
    iterator_ = std::make_unique<Iterator>(this, vt_->GetIterator());
  }
  return iterator_.get();
}

IPVT_FontMap* CPWL_EditImpl::GetFontMap() {
  return vt_provider_ ? vt_provider_->GetFontMap() : nullptr;
}

void CPWL_EditImpl::SetPlateRect(const CFX_FloatRect& rect) {
  vt_->SetPlateRect(rect);
  scroll_pos_point_ = CFX_PointF(rect.left, rect.top);
}

void CPWL_EditImpl::SetAlignmentH(int32_t nFormat) {
  vt_->SetAlignment(nFormat);
}

void CPWL_EditImpl::SetAlignmentV(int32_t nFormat) {
  alignment_ = nFormat;
}

void CPWL_EditImpl::SetPasswordChar(uint16_t wSubWord) {
  vt_->SetPasswordChar(wSubWord);
}

void CPWL_EditImpl::SetLimitChar(int32_t nLimitChar) {
  vt_->SetLimitChar(nLimitChar);
}

void CPWL_EditImpl::SetCharArray(int32_t nCharArray) {
  vt_->SetCharArray(nCharArray);
}

void CPWL_EditImpl::SetMultiLine(bool bMultiLine) {
  vt_->SetMultiLine(bMultiLine);
}

void CPWL_EditImpl::SetAutoReturn(bool bAuto) {
  vt_->SetAutoReturn(bAuto);
}

void CPWL_EditImpl::SetAutoFontSize(bool bAuto) {
  vt_->SetAutoFontSize(bAuto);
}

void CPWL_EditImpl::SetFontSize(float fFontSize) {
  vt_->SetFontSize(fFontSize);
}

void CPWL_EditImpl::SetAutoScroll(bool bAuto) {
  enable_scroll_ = bAuto;
}

void CPWL_EditImpl::SetTextOverflow(bool bAllowed) {
  enable_overflow_ = bAllowed;
}

void CPWL_EditImpl::SetSelection(int32_t nStartChar, int32_t nEndChar) {
  if (vt_->IsValid()) {
    if (nStartChar == 0 && nEndChar < 0) {
      SelectAll();
    } else if (nStartChar < 0) {
      SelectNone();
    } else {
      if (nStartChar < nEndChar) {
        SetSelection(vt_->WordIndexToWordPlace(nStartChar),
                     vt_->WordIndexToWordPlace(nEndChar));
      } else {
        SetSelection(vt_->WordIndexToWordPlace(nEndChar),
                     vt_->WordIndexToWordPlace(nStartChar));
      }
    }
  }
}

void CPWL_EditImpl::SetSelection(const CPVT_WordPlace& begin,
                                 const CPVT_WordPlace& end) {
  if (!vt_->IsValid()) {
    return;
  }

  SelectNone();
  sel_state_.Set(begin, end);
  SetCaret(sel_state_.EndPos);
  ScrollToCaret();
  if (!sel_state_.IsEmpty()) {
    Refresh();
  }
  SetCaretInfo();
}

std::pair<int32_t, int32_t> CPWL_EditImpl::GetSelection() const {
  if (!vt_->IsValid()) {
    return std::make_pair(-1, -1);
  }

  if (sel_state_.IsEmpty()) {
    return std::make_pair(vt_->WordPlaceToWordIndex(wp_caret_),
                          vt_->WordPlaceToWordIndex(wp_caret_));
  }
  if (sel_state_.BeginPos < sel_state_.EndPos) {
    return std::make_pair(vt_->WordPlaceToWordIndex(sel_state_.BeginPos),
                          vt_->WordPlaceToWordIndex(sel_state_.EndPos));
  }
  return std::make_pair(vt_->WordPlaceToWordIndex(sel_state_.EndPos),
                        vt_->WordPlaceToWordIndex(sel_state_.BeginPos));
}

int32_t CPWL_EditImpl::GetCaret() const {
  if (vt_->IsValid()) {
    return vt_->WordPlaceToWordIndex(wp_caret_);
  }

  return -1;
}

CPVT_WordPlace CPWL_EditImpl::GetCaretWordPlace() const {
  return wp_caret_;
}

WideString CPWL_EditImpl::GetText() const {
  WideString swRet;
  if (!vt_->IsValid()) {
    return swRet;
  }

  CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
  pIterator->SetAt(0);

  CPVT_Word wordinfo;
  CPVT_WordPlace oldplace = pIterator->GetWordPlace();
  while (pIterator->NextWord()) {
    CPVT_WordPlace place = pIterator->GetWordPlace();
    if (pIterator->GetWord(wordinfo)) {
      swRet += wordinfo.Word;
    }
    if (oldplace.nSecIndex != place.nSecIndex) {
      swRet += L"\r\n";
    }
    oldplace = place;
  }
  return swRet;
}

WideString CPWL_EditImpl::GetRangeText(const CPVT_WordRange& range) const {
  WideString swRet;
  if (!vt_->IsValid()) {
    return swRet;
  }

  CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
  CPVT_WordRange wrTemp = range;
  vt_->UpdateWordPlace(wrTemp.BeginPos);
  vt_->UpdateWordPlace(wrTemp.EndPos);
  pIterator->SetAt(wrTemp.BeginPos);

  CPVT_Word wordinfo;
  CPVT_WordPlace oldplace = wrTemp.BeginPos;
  while (pIterator->NextWord()) {
    CPVT_WordPlace place = pIterator->GetWordPlace();
    if (place > wrTemp.EndPos) {
      break;
    }
    if (pIterator->GetWord(wordinfo)) {
      swRet += wordinfo.Word;
    }
    if (oldplace.nSecIndex != place.nSecIndex) {
      swRet += L"\r\n";
    }
    oldplace = place;
  }
  return swRet;
}

WideString CPWL_EditImpl::GetSelectedText() const {
  return GetRangeText(sel_state_.ConvertToWordRange());
}

int32_t CPWL_EditImpl::GetTotalLines() const {
  int32_t nLines = 1;

  CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
  pIterator->SetAt(0);
  while (pIterator->NextLine()) {
    ++nLines;
  }

  return nLines;
}

CPVT_WordRange CPWL_EditImpl::GetSelectWordRange() const {
  return sel_state_.ConvertToWordRange();
}

void CPWL_EditImpl::SetText(const WideString& sText) {
  Clear();
  DoInsertText(CPVT_WordPlace(0, 0, -1), sText, FX_Charset::kDefault);
}

bool CPWL_EditImpl::InsertWord(uint16_t word, FX_Charset charset) {
  return InsertWord(word, charset, true);
}

void CPWL_EditImpl::InsertReturn() {
  InsertReturn(true);
}

void CPWL_EditImpl::Backspace() {
  Backspace(true);
}

bool CPWL_EditImpl::Delete() {
  return Delete(true);
}

bool CPWL_EditImpl::ClearSelection() {
  return Clear(true);
}

void CPWL_EditImpl::InsertText(const WideString& sText, FX_Charset charset) {
  InsertText(sText, charset, true);
}

float CPWL_EditImpl::GetFontSize() const {
  return vt_->GetFontSize();
}

uint16_t CPWL_EditImpl::GetPasswordChar() const {
  return vt_->GetPasswordChar();
}

int32_t CPWL_EditImpl::GetCharArray() const {
  return vt_->GetCharArray();
}

CFX_FloatRect CPWL_EditImpl::GetContentRect() const {
  return VTToEdit(vt_->GetContentRect());
}

CPVT_WordRange CPWL_EditImpl::GetWholeWordRange() const {
  if (vt_->IsValid()) {
    return CPVT_WordRange(vt_->GetBeginWordPlace(), vt_->GetEndWordPlace());
  }

  return CPVT_WordRange();
}

CPVT_WordRange CPWL_EditImpl::GetVisibleWordRange() const {
  if (enable_overflow_) {
    return GetWholeWordRange();
  }

  if (vt_->IsValid()) {
    CFX_FloatRect rcPlate = vt_->GetPlateRect();

    CPVT_WordPlace place1 =
        vt_->SearchWordPlace(EditToVT(CFX_PointF(rcPlate.left, rcPlate.top)));
    CPVT_WordPlace place2 = vt_->SearchWordPlace(
        EditToVT(CFX_PointF(rcPlate.right, rcPlate.bottom)));

    return CPVT_WordRange(place1, place2);
  }

  return CPVT_WordRange();
}

CPVT_WordPlace CPWL_EditImpl::SearchWordPlace(const CFX_PointF& point) const {
  if (vt_->IsValid()) {
    return vt_->SearchWordPlace(EditToVT(point));
  }

  return CPVT_WordPlace();
}

void CPWL_EditImpl::Paint() {
  if (vt_->IsValid()) {
    RearrangeAll();
    ScrollToCaret();
    Refresh();
    SetCaretOrigin();
    SetCaretInfo();
  }
}

void CPWL_EditImpl::RearrangeAll() {
  if (vt_->IsValid()) {
    vt_->UpdateWordPlace(wp_caret_);
    vt_->RearrangeAll();
    vt_->UpdateWordPlace(wp_caret_);
    SetScrollInfo();
    SetContentChanged();
  }
}

void CPWL_EditImpl::RearrangePart(const CPVT_WordRange& range) {
  if (vt_->IsValid()) {
    vt_->UpdateWordPlace(wp_caret_);
    vt_->RearrangePart(range);
    vt_->UpdateWordPlace(wp_caret_);
    SetScrollInfo();
    SetContentChanged();
  }
}

void CPWL_EditImpl::SetContentChanged() {
  if (notify_) {
    CFX_FloatRect rcContent = vt_->GetContentRect();
    if (rcContent.Width() != old_content_rect_.Width() ||
        rcContent.Height() != old_content_rect_.Height()) {
      old_content_rect_ = rcContent;
    }
  }
}

void CPWL_EditImpl::SelectAll() {
  if (!vt_->IsValid()) {
    return;
  }
  sel_state_ = SelectState(GetWholeWordRange());
  SetCaret(sel_state_.EndPos);
  ScrollToCaret();
  Refresh();
  SetCaretInfo();
}

void CPWL_EditImpl::SelectNone() {
  if (!vt_->IsValid() || sel_state_.IsEmpty()) {
    return;
  }

  sel_state_.Reset();
  Refresh();
}

bool CPWL_EditImpl::IsSelected() const {
  return !sel_state_.IsEmpty();
}

CFX_PointF CPWL_EditImpl::VTToEdit(const CFX_PointF& point) const {
  CFX_FloatRect rcContent = vt_->GetContentRect();
  CFX_FloatRect rcPlate = vt_->GetPlateRect();

  float fPadding = 0.0f;

  switch (alignment_) {
    case 0:
      fPadding = 0.0f;
      break;
    case 1:
      fPadding = (rcPlate.Height() - rcContent.Height()) * 0.5f;
      break;
    case 2:
      fPadding = rcPlate.Height() - rcContent.Height();
      break;
  }

  return CFX_PointF(point.x - (scroll_pos_point_.x - rcPlate.left),
                    point.y - (scroll_pos_point_.y + fPadding - rcPlate.top));
}

CFX_PointF CPWL_EditImpl::EditToVT(const CFX_PointF& point) const {
  CFX_FloatRect rcContent = vt_->GetContentRect();
  CFX_FloatRect rcPlate = vt_->GetPlateRect();

  float fPadding = 0.0f;

  switch (alignment_) {
    case 0:
      fPadding = 0.0f;
      break;
    case 1:
      fPadding = (rcPlate.Height() - rcContent.Height()) * 0.5f;
      break;
    case 2:
      fPadding = rcPlate.Height() - rcContent.Height();
      break;
  }

  return CFX_PointF(point.x + (scroll_pos_point_.x - rcPlate.left),
                    point.y + (scroll_pos_point_.y + fPadding - rcPlate.top));
}

CFX_FloatRect CPWL_EditImpl::VTToEdit(const CFX_FloatRect& rect) const {
  CFX_PointF ptLeftBottom = VTToEdit(CFX_PointF(rect.left, rect.bottom));
  CFX_PointF ptRightTop = VTToEdit(CFX_PointF(rect.right, rect.top));

  return CFX_FloatRect(ptLeftBottom.x, ptLeftBottom.y, ptRightTop.x,
                       ptRightTop.y);
}

void CPWL_EditImpl::SetScrollInfo() {
  if (!notify_) {
    return;
  }

  CFX_FloatRect rcPlate = vt_->GetPlateRect();
  CFX_FloatRect rcContent = vt_->GetContentRect();
  if (notify_flag_) {
    return;
  }

  AutoRestorer<bool> restorer(&notify_flag_);
  notify_flag_ = true;

  PWL_SCROLL_INFO Info;
  Info.fPlateWidth = rcPlate.top - rcPlate.bottom;
  Info.fContentMin = rcContent.bottom;
  Info.fContentMax = rcContent.top;
  Info.fSmallStep = rcPlate.Height() / 3;
  Info.fBigStep = rcPlate.Height();
  notify_->SetScrollInfo(Info);
}

void CPWL_EditImpl::SetScrollPosX(float fx) {
  if (!enable_scroll_) {
    return;
  }

  if (vt_->IsValid()) {
    if (!FXSYS_IsFloatEqual(scroll_pos_point_.x, fx)) {
      scroll_pos_point_.x = fx;
      Refresh();
    }
  }
}

void CPWL_EditImpl::SetScrollPosY(float fy) {
  if (!enable_scroll_) {
    return;
  }

  if (vt_->IsValid()) {
    if (!FXSYS_IsFloatEqual(scroll_pos_point_.y, fy)) {
      scroll_pos_point_.y = fy;
      Refresh();

      if (notify_) {
        if (!notify_flag_) {
          AutoRestorer<bool> restorer(&notify_flag_);
          notify_flag_ = true;
          notify_->SetScrollPosition(fy);
        }
      }
    }
  }
}

void CPWL_EditImpl::SetScrollPos(const CFX_PointF& point) {
  SetScrollPosX(point.x);
  SetScrollPosY(point.y);
  SetScrollLimit();
  SetCaretInfo();
}

CFX_PointF CPWL_EditImpl::GetScrollPos() const {
  return scroll_pos_point_;
}

void CPWL_EditImpl::SetScrollLimit() {
  if (vt_->IsValid()) {
    CFX_FloatRect rcContent = vt_->GetContentRect();
    CFX_FloatRect rcPlate = vt_->GetPlateRect();

    if (rcPlate.Width() > rcContent.Width()) {
      SetScrollPosX(rcPlate.left);
    } else {
      if (FXSYS_IsFloatSmaller(scroll_pos_point_.x, rcContent.left)) {
        SetScrollPosX(rcContent.left);
      } else if (FXSYS_IsFloatBigger(scroll_pos_point_.x,
                                     rcContent.right - rcPlate.Width())) {
        SetScrollPosX(rcContent.right - rcPlate.Width());
      }
    }

    if (rcPlate.Height() > rcContent.Height()) {
      SetScrollPosY(rcPlate.top);
    } else {
      if (FXSYS_IsFloatSmaller(scroll_pos_point_.y,
                               rcContent.bottom + rcPlate.Height())) {
        SetScrollPosY(rcContent.bottom + rcPlate.Height());
      } else if (FXSYS_IsFloatBigger(scroll_pos_point_.y, rcContent.top)) {
        SetScrollPosY(rcContent.top);
      }
    }
  }
}

void CPWL_EditImpl::ScrollToCaret() {
  SetScrollLimit();

  if (!vt_->IsValid()) {
    return;
  }

  CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
  pIterator->SetAt(wp_caret_);

  CFX_PointF ptHead;
  CFX_PointF ptFoot;
  CPVT_Word word;
  CPVT_Line line;
  if (pIterator->GetWord(word)) {
    ptHead.x = word.ptWord.x + word.fWidth;
    ptHead.y = word.ptWord.y + word.fAscent;
    ptFoot.x = word.ptWord.x + word.fWidth;
    ptFoot.y = word.ptWord.y + word.fDescent;
  } else if (pIterator->GetLine(line)) {
    ptHead.x = line.ptLine.x;
    ptHead.y = line.ptLine.y + line.fLineAscent;
    ptFoot.x = line.ptLine.x;
    ptFoot.y = line.ptLine.y + line.fLineDescent;
  }

  CFX_PointF ptHeadEdit = VTToEdit(ptHead);
  CFX_PointF ptFootEdit = VTToEdit(ptFoot);
  CFX_FloatRect rcPlate = vt_->GetPlateRect();
  if (!FXSYS_IsFloatEqual(rcPlate.left, rcPlate.right)) {
    if (FXSYS_IsFloatSmaller(ptHeadEdit.x, rcPlate.left) ||
        FXSYS_IsFloatEqual(ptHeadEdit.x, rcPlate.left)) {
      SetScrollPosX(ptHead.x);
    } else if (FXSYS_IsFloatBigger(ptHeadEdit.x, rcPlate.right)) {
      SetScrollPosX(ptHead.x - rcPlate.Width());
    }
  }

  if (!FXSYS_IsFloatEqual(rcPlate.top, rcPlate.bottom)) {
    if (FXSYS_IsFloatSmaller(ptFootEdit.y, rcPlate.bottom) ||
        FXSYS_IsFloatEqual(ptFootEdit.y, rcPlate.bottom)) {
      if (FXSYS_IsFloatSmaller(ptHeadEdit.y, rcPlate.top)) {
        SetScrollPosY(ptFoot.y + rcPlate.Height());
      }
    } else if (FXSYS_IsFloatBigger(ptHeadEdit.y, rcPlate.top)) {
      if (FXSYS_IsFloatBigger(ptFootEdit.y, rcPlate.bottom)) {
        SetScrollPosY(ptHead.y);
      }
    }
  }
}

void CPWL_EditImpl::Refresh() {
  if (enable_refresh_ && vt_->IsValid()) {
    refresh_.BeginRefresh();
    RefreshPushLineRects(GetVisibleWordRange());

    refresh_.NoAnalyse();
    pt_refresh_scroll_pos_ = scroll_pos_point_;

    if (notify_) {
      if (!notify_flag_) {
        AutoRestorer<bool> restorer(&notify_flag_);
        notify_flag_ = true;
        std::vector<CFX_FloatRect>* pRects = refresh_.GetRefreshRects();
        for (auto& rect : *pRects) {
          if (!notify_->InvalidateRect(&rect)) {
            notify_ = nullptr;  // Gone, dangling even.
            break;
          }
        }
      }
    }

    refresh_.EndRefresh();
  }
}

void CPWL_EditImpl::RefreshPushLineRects(const CPVT_WordRange& wr) {
  if (!vt_->IsValid()) {
    return;
  }

  CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
  CPVT_WordPlace wpBegin = wr.BeginPos;
  vt_->UpdateWordPlace(wpBegin);
  CPVT_WordPlace wpEnd = wr.EndPos;
  vt_->UpdateWordPlace(wpEnd);
  pIterator->SetAt(wpBegin);

  CPVT_Line lineinfo;
  do {
    if (!pIterator->GetLine(lineinfo)) {
      break;
    }
    if (lineinfo.lineplace.LineCmp(wpEnd) > 0) {
      break;
    }

    CFX_FloatRect rcLine(lineinfo.ptLine.x,
                         lineinfo.ptLine.y + lineinfo.fLineDescent,
                         lineinfo.ptLine.x + lineinfo.fLineWidth,
                         lineinfo.ptLine.y + lineinfo.fLineAscent);

    refresh_.Push(CPVT_WordRange(lineinfo.lineplace, lineinfo.lineEnd),
                  VTToEdit(rcLine));
  } while (pIterator->NextLine());
}

void CPWL_EditImpl::RefreshWordRange(const CPVT_WordRange& wr) {
  CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
  CPVT_WordRange wrTemp = wr;

  vt_->UpdateWordPlace(wrTemp.BeginPos);
  vt_->UpdateWordPlace(wrTemp.EndPos);
  pIterator->SetAt(wrTemp.BeginPos);

  CPVT_Word wordinfo;
  CPVT_Line lineinfo;
  CPVT_WordPlace place;

  while (pIterator->NextWord()) {
    place = pIterator->GetWordPlace();
    if (place > wrTemp.EndPos) {
      break;
    }

    pIterator->GetWord(wordinfo);
    pIterator->GetLine(lineinfo);
    if (place.LineCmp(wrTemp.BeginPos) == 0 ||
        place.LineCmp(wrTemp.EndPos) == 0) {
      CFX_FloatRect rcWord(wordinfo.ptWord.x,
                           lineinfo.ptLine.y + lineinfo.fLineDescent,
                           wordinfo.ptWord.x + wordinfo.fWidth,
                           lineinfo.ptLine.y + lineinfo.fLineAscent);

      if (notify_) {
        if (!notify_flag_) {
          AutoRestorer<bool> restorer(&notify_flag_);
          notify_flag_ = true;
          CFX_FloatRect rcRefresh = VTToEdit(rcWord);
          if (!notify_->InvalidateRect(&rcRefresh)) {
            notify_ = nullptr;  // Gone, dangling even.
          }
        }
      }
    } else {
      CFX_FloatRect rcLine(lineinfo.ptLine.x,
                           lineinfo.ptLine.y + lineinfo.fLineDescent,
                           lineinfo.ptLine.x + lineinfo.fLineWidth,
                           lineinfo.ptLine.y + lineinfo.fLineAscent);

      if (notify_) {
        if (!notify_flag_) {
          AutoRestorer<bool> restorer(&notify_flag_);
          notify_flag_ = true;
          CFX_FloatRect rcRefresh = VTToEdit(rcLine);
          if (!notify_->InvalidateRect(&rcRefresh)) {
            notify_ = nullptr;  // Gone, dangling even.
          }
        }
      }

      pIterator->NextLine();
    }
  }
}

void CPWL_EditImpl::SetCaret(const CPVT_WordPlace& place) {
  wp_old_caret_ = wp_caret_;
  wp_caret_ = place;
}

void CPWL_EditImpl::SetCaretInfo() {
  if (notify_) {
    if (!notify_flag_) {
      CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
      pIterator->SetAt(wp_caret_);

      CFX_PointF ptHead;
      CFX_PointF ptFoot;
      CPVT_Word word;
      CPVT_Line line;
      if (pIterator->GetWord(word)) {
        ptHead.x = word.ptWord.x + word.fWidth;
        ptHead.y = word.ptWord.y + word.fAscent;
        ptFoot.x = word.ptWord.x + word.fWidth;
        ptFoot.y = word.ptWord.y + word.fDescent;
      } else if (pIterator->GetLine(line)) {
        ptHead.x = line.ptLine.x;
        ptHead.y = line.ptLine.y + line.fLineAscent;
        ptFoot.x = line.ptLine.x;
        ptFoot.y = line.ptLine.y + line.fLineDescent;
      }

      AutoRestorer<bool> restorer(&notify_flag_);
      notify_flag_ = true;
      notify_->SetCaret(sel_state_.IsEmpty(), VTToEdit(ptHead),
                        VTToEdit(ptFoot));
    }
  }
}

void CPWL_EditImpl::OnMouseDown(const CFX_PointF& point,
                                bool bShift,
                                bool bCtrl) {
  if (!vt_->IsValid()) {
    return;
  }

  SelectNone();
  SetCaret(vt_->SearchWordPlace(EditToVT(point)));
  sel_state_.Set(wp_caret_, wp_caret_);
  ScrollToCaret();
  SetCaretOrigin();
  SetCaretInfo();
}

void CPWL_EditImpl::OnMouseMove(const CFX_PointF& point,
                                bool bShift,
                                bool bCtrl) {
  if (!vt_->IsValid()) {
    return;
  }

  SetCaret(vt_->SearchWordPlace(EditToVT(point)));
  if (wp_caret_ == wp_old_caret_) {
    return;
  }

  sel_state_.SetEndPos(wp_caret_);
  ScrollToCaret();
  Refresh();
  SetCaretOrigin();
  SetCaretInfo();
}

void CPWL_EditImpl::OnVK_UP(bool bShift) {
  if (!vt_->IsValid()) {
    return;
  }

  SetCaret(vt_->GetUpWordPlace(wp_caret_, caret_point_));
  if (bShift) {
    if (sel_state_.IsEmpty()) {
      sel_state_.Set(wp_old_caret_, wp_caret_);
    } else {
      sel_state_.SetEndPos(wp_caret_);
    }

    if (wp_old_caret_ != wp_caret_) {
      ScrollToCaret();
      Refresh();
      SetCaretInfo();
    }
  } else {
    SelectNone();
    ScrollToCaret();
    SetCaretInfo();
  }
}

void CPWL_EditImpl::OnVK_DOWN(bool bShift) {
  if (!vt_->IsValid()) {
    return;
  }

  SetCaret(vt_->GetDownWordPlace(wp_caret_, caret_point_));
  if (bShift) {
    if (sel_state_.IsEmpty()) {
      sel_state_.Set(wp_old_caret_, wp_caret_);
    } else {
      sel_state_.SetEndPos(wp_caret_);
    }

    if (wp_old_caret_ != wp_caret_) {
      ScrollToCaret();
      Refresh();
      SetCaretInfo();
    }
  } else {
    SelectNone();
    ScrollToCaret();
    SetCaretInfo();
  }
}

void CPWL_EditImpl::OnVK_LEFT(bool bShift) {
  if (!vt_->IsValid()) {
    return;
  }

  if (bShift) {
    if (wp_caret_ == vt_->GetLineBeginPlace(wp_caret_) &&
        wp_caret_ != vt_->GetSectionBeginPlace(wp_caret_)) {
      SetCaret(vt_->GetPrevWordPlace(wp_caret_));
    }
    SetCaret(vt_->GetPrevWordPlace(wp_caret_));
    if (sel_state_.IsEmpty()) {
      sel_state_.Set(wp_old_caret_, wp_caret_);
    } else {
      sel_state_.SetEndPos(wp_caret_);
    }

    if (wp_old_caret_ != wp_caret_) {
      ScrollToCaret();
      Refresh();
      SetCaretInfo();
    }
  } else {
    if (!sel_state_.IsEmpty()) {
      if (sel_state_.BeginPos < sel_state_.EndPos) {
        SetCaret(sel_state_.BeginPos);
      } else {
        SetCaret(sel_state_.EndPos);
      }

      SelectNone();
      ScrollToCaret();
      SetCaretInfo();
    } else {
      if (wp_caret_ == vt_->GetLineBeginPlace(wp_caret_) &&
          wp_caret_ != vt_->GetSectionBeginPlace(wp_caret_)) {
        SetCaret(vt_->GetPrevWordPlace(wp_caret_));
      }
      SetCaret(vt_->GetPrevWordPlace(wp_caret_));
      ScrollToCaret();
      SetCaretOrigin();
      SetCaretInfo();
    }
  }
}

void CPWL_EditImpl::OnVK_RIGHT(bool bShift) {
  if (!vt_->IsValid()) {
    return;
  }

  if (bShift) {
    SetCaret(vt_->GetNextWordPlace(wp_caret_));
    if (wp_caret_ == vt_->GetLineEndPlace(wp_caret_) &&
        wp_caret_ != vt_->GetSectionEndPlace(wp_caret_)) {
      SetCaret(vt_->GetNextWordPlace(wp_caret_));
    }

    if (sel_state_.IsEmpty()) {
      sel_state_.Set(wp_old_caret_, wp_caret_);
    } else {
      sel_state_.SetEndPos(wp_caret_);
    }

    if (wp_old_caret_ != wp_caret_) {
      ScrollToCaret();
      Refresh();
      SetCaretInfo();
    }
  } else {
    if (!sel_state_.IsEmpty()) {
      if (sel_state_.BeginPos > sel_state_.EndPos) {
        SetCaret(sel_state_.BeginPos);
      } else {
        SetCaret(sel_state_.EndPos);
      }

      SelectNone();
      ScrollToCaret();
      SetCaretInfo();
    } else {
      SetCaret(vt_->GetNextWordPlace(wp_caret_));
      if (wp_caret_ == vt_->GetLineEndPlace(wp_caret_) &&
          wp_caret_ != vt_->GetSectionEndPlace(wp_caret_)) {
        SetCaret(vt_->GetNextWordPlace(wp_caret_));
      }
      ScrollToCaret();
      SetCaretOrigin();
      SetCaretInfo();
    }
  }
}

void CPWL_EditImpl::OnVK_HOME(bool bShift, bool bCtrl) {
  if (!vt_->IsValid()) {
    return;
  }

  if (bShift) {
    if (bCtrl) {
      SetCaret(vt_->GetBeginWordPlace());
    } else {
      SetCaret(vt_->GetLineBeginPlace(wp_caret_));
    }

    if (sel_state_.IsEmpty()) {
      sel_state_.Set(wp_old_caret_, wp_caret_);
    } else {
      sel_state_.SetEndPos(wp_caret_);
    }

    ScrollToCaret();
    Refresh();
    SetCaretInfo();
  } else {
    if (!sel_state_.IsEmpty()) {
      SetCaret(std::min(sel_state_.BeginPos, sel_state_.EndPos));
      SelectNone();
      ScrollToCaret();
      SetCaretInfo();
    } else {
      if (bCtrl) {
        SetCaret(vt_->GetBeginWordPlace());
      } else {
        SetCaret(vt_->GetLineBeginPlace(wp_caret_));
      }

      ScrollToCaret();
      SetCaretOrigin();
      SetCaretInfo();
    }
  }
}

void CPWL_EditImpl::OnVK_END(bool bShift, bool bCtrl) {
  if (!vt_->IsValid()) {
    return;
  }

  if (bShift) {
    if (bCtrl) {
      SetCaret(vt_->GetEndWordPlace());
    } else {
      SetCaret(vt_->GetLineEndPlace(wp_caret_));
    }

    if (sel_state_.IsEmpty()) {
      sel_state_.Set(wp_old_caret_, wp_caret_);
    } else {
      sel_state_.SetEndPos(wp_caret_);
    }

    ScrollToCaret();
    Refresh();
    SetCaretInfo();
  } else {
    if (!sel_state_.IsEmpty()) {
      SetCaret(std::max(sel_state_.BeginPos, sel_state_.EndPos));
      SelectNone();
      ScrollToCaret();
      SetCaretInfo();
    } else {
      if (bCtrl) {
        SetCaret(vt_->GetEndWordPlace());
      } else {
        SetCaret(vt_->GetLineEndPlace(wp_caret_));
      }

      ScrollToCaret();
      SetCaretOrigin();
      SetCaretInfo();
    }
  }
}

bool CPWL_EditImpl::InsertWord(uint16_t word,
                               FX_Charset charset,
                               bool bAddUndo) {
  if (IsTextOverflow() || !vt_->IsValid()) {
    return false;
  }

  vt_->UpdateWordPlace(wp_caret_);
  SetCaret(
      vt_->InsertWord(wp_caret_, word, GetCharSetFromUnicode(word, charset)));
  sel_state_.Set(wp_caret_, wp_caret_);
  if (wp_caret_ == wp_old_caret_) {
    return false;
  }

  if (bAddUndo && enable_undo_) {
    AddEditUndoItem(std::make_unique<UndoInsertWord>(this, wp_old_caret_,
                                                     wp_caret_, word, charset));
  }
  PaintInsertText(wp_old_caret_, wp_caret_);
  return true;
}

void CPWL_EditImpl::InsertReturn(bool bAddUndo) {
  if (IsTextOverflow() || !vt_->IsValid()) {
    return;
  }

  vt_->UpdateWordPlace(wp_caret_);
  SetCaret(vt_->InsertSection(wp_caret_));
  sel_state_.Set(wp_caret_, wp_caret_);
  if (wp_caret_ == wp_old_caret_) {
    return;
  }

  if (bAddUndo && enable_undo_) {
    AddEditUndoItem(
        std::make_unique<UndoInsertReturn>(this, wp_old_caret_, wp_caret_));
  }
  RearrangePart(CPVT_WordRange(wp_old_caret_, wp_caret_));
  ScrollToCaret();
  Refresh();
  SetCaretOrigin();
  SetCaretInfo();
}

void CPWL_EditImpl::Backspace(bool bAddUndo) {
  if (!vt_->IsValid() || wp_caret_ == vt_->GetBeginWordPlace()) {
    return;
  }

  CPVT_Word word;
  if (bAddUndo) {
    CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
    pIterator->SetAt(wp_caret_);
    pIterator->GetWord(word);
  }
  vt_->UpdateWordPlace(wp_caret_);
  SetCaret(vt_->BackSpaceWord(wp_caret_));
  sel_state_.Set(wp_caret_, wp_caret_);
  if (wp_caret_ == wp_old_caret_) {
    return;
  }

  if (bAddUndo && enable_undo_) {
    AddEditUndoItem(std::make_unique<UndoBackspace>(
        this, wp_old_caret_, wp_caret_, word.Word, word.nCharset));
  }
  RearrangePart(CPVT_WordRange(wp_caret_, wp_old_caret_));
  ScrollToCaret();
  Refresh();
  SetCaretOrigin();
  SetCaretInfo();
}

bool CPWL_EditImpl::Delete(bool bAddUndo) {
  if (!vt_->IsValid() || wp_caret_ == vt_->GetEndWordPlace()) {
    return false;
  }

  CPVT_Word word;
  if (bAddUndo) {
    CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
    pIterator->SetAt(vt_->GetNextWordPlace(wp_caret_));
    pIterator->GetWord(word);
  }
  vt_->UpdateWordPlace(wp_caret_);
  bool bSecEnd = (wp_caret_ == vt_->GetSectionEndPlace(wp_caret_));
  SetCaret(vt_->DeleteWord(wp_caret_));
  sel_state_.Set(wp_caret_, wp_caret_);
  if (bAddUndo && enable_undo_) {
    if (bSecEnd) {
      AddEditUndoItem(std::make_unique<UndoDelete>(
          this, wp_old_caret_, wp_caret_, word.Word, word.nCharset, bSecEnd));
    } else {
      AddEditUndoItem(std::make_unique<UndoDelete>(
          this, wp_old_caret_, wp_caret_, word.Word, word.nCharset, bSecEnd));
    }
  }
  RearrangePart(CPVT_WordRange(wp_old_caret_, wp_caret_));
  ScrollToCaret();
  Refresh();
  SetCaretOrigin();
  SetCaretInfo();
  return true;
}

bool CPWL_EditImpl::Clear() {
  if (vt_->IsValid()) {
    vt_->DeleteWords(GetWholeWordRange());
    SetCaret(vt_->GetBeginWordPlace());

    return true;
  }

  return false;
}

bool CPWL_EditImpl::Clear(bool bAddUndo) {
  if (!vt_->IsValid() || sel_state_.IsEmpty()) {
    return false;
  }

  CPVT_WordRange range = sel_state_.ConvertToWordRange();
  if (bAddUndo && enable_undo_) {
    AddEditUndoItem(
        std::make_unique<UndoClear>(this, range, GetSelectedText()));
  }
  SelectNone();
  SetCaret(vt_->DeleteWords(range));
  sel_state_.Set(wp_caret_, wp_caret_);
  RearrangePart(range);
  ScrollToCaret();
  Refresh();
  SetCaretOrigin();
  SetCaretInfo();
  return true;
}

void CPWL_EditImpl::InsertText(const WideString& sText,
                               FX_Charset charset,
                               bool bAddUndo) {
  if (IsTextOverflow()) {
    return;
  }

  vt_->UpdateWordPlace(wp_caret_);
  SetCaret(DoInsertText(wp_caret_, sText, charset));
  sel_state_.Set(wp_caret_, wp_caret_);
  if (wp_caret_ == wp_old_caret_) {
    return;
  }

  if (bAddUndo && enable_undo_) {
    AddEditUndoItem(std::make_unique<UndoInsertText>(
        this, wp_old_caret_, wp_caret_, sText, charset));
  }
  PaintInsertText(wp_old_caret_, wp_caret_);
}

void CPWL_EditImpl::PaintInsertText(const CPVT_WordPlace& wpOld,
                                    const CPVT_WordPlace& wpNew) {
  if (vt_->IsValid()) {
    RearrangePart(CPVT_WordRange(wpOld, wpNew));
    ScrollToCaret();
    Refresh();
    SetCaretOrigin();
    SetCaretInfo();
  }
}

void CPWL_EditImpl::ReplaceAndKeepSelection(const WideString& text) {
  AddEditUndoItem(std::make_unique<UndoReplaceSelection>());
  ClearSelection();

  // Insert the text and then select it
  CPVT_WordPlace caret_before_insert = wp_caret_;
  InsertText(text, FX_Charset::kDefault);
  CPVT_WordPlace caret_after_insert = wp_caret_;
  sel_state_.Set(caret_before_insert, caret_after_insert);

  AddEditUndoItem(std::make_unique<UndoReplaceSelection>());
}

void CPWL_EditImpl::ReplaceSelection(const WideString& text) {
  // UndoReplaceSelection acts as a sentinel object in the undo queue. Since
  // ClearSelection and InsertText only optionally add undo items there are a
  // variable number of items in the queue for this action. These sentinel
  // objects mark the start and the end of the series of related undo items.
  AddEditUndoItem(std::make_unique<UndoReplaceSelection>());
  ClearSelection();
  InsertText(text, FX_Charset::kDefault);
  AddEditUndoItem(std::make_unique<UndoReplaceSelection>());
}

void CPWL_EditImpl::TypeChar(uint16_t word, FX_Charset charset) {
  bool was_selected = IsSelected();

  // Backspace is special because it always needs only one undo item.
  // ClearSelection() deletes the selected text so Backspace() isn't needed in
  // that case.
  if (word == pdfium::ascii::kBackspace) {
    if (was_selected) {
      ClearSelection();
    } else {
      Backspace();
    }
    return;
  }

  // Don't add the UndoReplaceSelection sentinel items if there's no selection
  // so that in the normal "typing one character" case only one undo item goes
  // into the queue.
  if (was_selected) {
    AddEditUndoItem(std::make_unique<UndoReplaceSelection>());
    ClearSelection();
  }

  if (word == pdfium::ascii::kReturn) {
    InsertReturn();
  } else {
    // Not a special character, just typing letters
    InsertWord(word, charset);
  }

  if (was_selected) {
    AddEditUndoItem(std::make_unique<UndoReplaceSelection>());
  }
}

bool CPWL_EditImpl::Redo() {
  if (enable_undo_) {
    if (undo_.CanRedo()) {
      undo_.Redo();
      return true;
    }
  }

  return false;
}

bool CPWL_EditImpl::Undo() {
  if (enable_undo_) {
    if (undo_.CanUndo()) {
      undo_.Undo();
      return true;
    }
  }

  return false;
}

void CPWL_EditImpl::SetCaretOrigin() {
  if (!vt_->IsValid()) {
    return;
  }

  CPVT_VariableText::Iterator* pIterator = vt_->GetIterator();
  pIterator->SetAt(wp_caret_);
  CPVT_Word word;
  CPVT_Line line;
  if (pIterator->GetWord(word)) {
    caret_point_.x = word.ptWord.x + word.fWidth;
    caret_point_.y = word.ptWord.y;
  } else if (pIterator->GetLine(line)) {
    caret_point_.x = line.ptLine.x;
    caret_point_.y = line.ptLine.y;
  }
}

CPVT_WordPlace CPWL_EditImpl::WordIndexToWordPlace(int32_t index) const {
  if (vt_->IsValid()) {
    return vt_->WordIndexToWordPlace(index);
  }

  return CPVT_WordPlace();
}

bool CPWL_EditImpl::IsTextFull() const {
  int32_t nTotalWords = vt_->GetTotalWords();
  int32_t nLimitChar = vt_->GetLimitChar();
  int32_t nCharArray = vt_->GetCharArray();

  return IsTextOverflow() || (nLimitChar > 0 && nTotalWords >= nLimitChar) ||
         (nCharArray > 0 && nTotalWords >= nCharArray);
}

bool CPWL_EditImpl::IsTextOverflow() const {
  if (!enable_scroll_ && !enable_overflow_) {
    CFX_FloatRect rcPlate = vt_->GetPlateRect();
    CFX_FloatRect rcContent = vt_->GetContentRect();

    if (vt_->IsMultiLine() && GetTotalLines() > 1 &&
        FXSYS_IsFloatBigger(rcContent.Height(), rcPlate.Height())) {
      return true;
    }

    if (FXSYS_IsFloatBigger(rcContent.Width(), rcPlate.Width())) {
      return true;
    }
  }

  return false;
}

bool CPWL_EditImpl::CanUndo() const {
  if (enable_undo_) {
    return undo_.CanUndo();
  }

  return false;
}

bool CPWL_EditImpl::CanRedo() const {
  if (enable_undo_) {
    return undo_.CanRedo();
  }

  return false;
}

void CPWL_EditImpl::SetMaxUndoItemsForTest(size_t items) {
  undo_.SetMaxUndoItemsForTest(items);
}

void CPWL_EditImpl::EnableRefresh(bool bRefresh) {
  enable_refresh_ = bRefresh;
}

void CPWL_EditImpl::EnableUndo(bool bUndo) {
  enable_undo_ = bUndo;
}

CPVT_WordPlace CPWL_EditImpl::DoInsertText(const CPVT_WordPlace& place,
                                           const WideString& sText,
                                           FX_Charset charset) {
  if (!vt_->IsValid()) {
    return place;
  }

  CPVT_WordPlace wp = place;
  for (size_t i = 0; i < sText.GetLength(); ++i) {
    uint16_t word = sText[i];
    switch (word) {
      case '\r':
        wp = vt_->InsertSection(wp);
        if (i + 1 < sText.GetLength() && sText[i + 1] == '\n') {
          i++;
        }
        break;
      case '\n':
        wp = vt_->InsertSection(wp);
        break;
      case '\t':
        word = ' ';
        [[fallthrough]];
      default:
        wp = vt_->InsertWord(wp, word, GetCharSetFromUnicode(word, charset));
        break;
    }
  }
  return wp;
}

FX_Charset CPWL_EditImpl::GetCharSetFromUnicode(uint16_t word,
                                                FX_Charset nOldCharset) {
  if (IPVT_FontMap* font_map = GetFontMap()) {
    return font_map->CharSetFromUnicode(word, nOldCharset);
  }
  return nOldCharset;
}

void CPWL_EditImpl::AddEditUndoItem(
    std::unique_ptr<UndoItemIface> pEditUndoItem) {
  undo_.AddItem(std::move(pEditUndoItem));
}

ByteString CPWL_EditImpl::GetPDFWordString(int32_t nFontIndex,
                                           uint16_t Word,
                                           uint16_t SubWord) {
  IPVT_FontMap* font_map = GetFontMap();
  RetainPtr<CPDF_Font> pPDFFont = font_map->GetPDFFont(nFontIndex);
  if (!pPDFFont) {
    return ByteString();
  }

  ByteString sWord;
  if (SubWord > 0) {
    Word = SubWord;
  } else {
    uint32_t dwCharCode = pPDFFont->IsUnicodeCompatible()
                              ? pPDFFont->CharCodeFromUnicode(Word)
                              : font_map->CharCodeFromUnicode(nFontIndex, Word);
    if (dwCharCode > 0) {
      pPDFFont->AppendChar(&sWord, dwCharCode);
      return sWord;
    }
  }
  pPDFFont->AppendChar(&sWord, Word);
  return sWord;
}

CPWL_EditImpl::SelectState::SelectState() = default;

CPWL_EditImpl::SelectState::SelectState(const CPVT_WordRange& range) {
  Set(range.BeginPos, range.EndPos);
}

CPVT_WordRange CPWL_EditImpl::SelectState::ConvertToWordRange() const {
  return CPVT_WordRange(BeginPos, EndPos);
}

void CPWL_EditImpl::SelectState::Reset() {
  BeginPos.Reset();
  EndPos.Reset();
}

void CPWL_EditImpl::SelectState::Set(const CPVT_WordPlace& begin,
                                     const CPVT_WordPlace& end) {
  BeginPos = begin;
  EndPos = end;
}

void CPWL_EditImpl::SelectState::SetEndPos(const CPVT_WordPlace& end) {
  EndPos = end;
}

bool CPWL_EditImpl::SelectState::IsEmpty() const {
  return BeginPos == EndPos;
}

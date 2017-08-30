// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fde/cfde_texteditengine.h"

#include <algorithm>
#include <limits>

#include "xfa/fde/cfde_textout.h"

namespace {

constexpr size_t kMaxEditOperations = 128;
constexpr size_t kGapSize = 128;
constexpr size_t kPageWidthMax = 0xffff;

class InsertOperation : public CFDE_TextEditEngine::Operation {
 public:
  InsertOperation(CFDE_TextEditEngine* engine,
                  size_t start_idx,
                  const CFX_WideString& added_text)
      : engine_(engine), start_idx_(start_idx), added_text_(added_text) {}

  ~InsertOperation() override {}

  void Redo() const override {
    engine_->Insert(start_idx_, added_text_,
                    CFDE_TextEditEngine::RecordOperation::kSkipRecord);
  }

  void Undo() const override {
    engine_->Delete(start_idx_, added_text_.GetLength(),
                    CFDE_TextEditEngine::RecordOperation::kSkipRecord);
  }

 private:
  CFX_UnownedPtr<CFDE_TextEditEngine> engine_;
  size_t start_idx_;
  CFX_WideString added_text_;
};

class DeleteOperation : public CFDE_TextEditEngine::Operation {
 public:
  DeleteOperation(CFDE_TextEditEngine* engine,
                  size_t start_idx,
                  const CFX_WideString& removed_text)
      : engine_(engine), start_idx_(start_idx), removed_text_(removed_text) {}

  ~DeleteOperation() override {}

  void Redo() const override {
    engine_->Delete(start_idx_, removed_text_.GetLength(),
                    CFDE_TextEditEngine::RecordOperation::kSkipRecord);
  }

  void Undo() const override {
    engine_->Insert(start_idx_, removed_text_,
                    CFDE_TextEditEngine::RecordOperation::kSkipRecord);
  }

 private:
  CFX_UnownedPtr<CFDE_TextEditEngine> engine_;
  size_t start_idx_;
  CFX_WideString removed_text_;
};

class ReplaceOperation : public CFDE_TextEditEngine::Operation {
 public:
  ReplaceOperation(CFDE_TextEditEngine* engine,
                   size_t start_idx,
                   const CFX_WideString& removed_text,
                   const CFX_WideString& added_text)
      : insert_op_(engine, start_idx, added_text),
        delete_op_(engine, start_idx, removed_text) {}

  ~ReplaceOperation() override {}

  void Redo() const override {
    delete_op_.Redo();
    insert_op_.Redo();
  }

  void Undo() const override {
    insert_op_.Undo();
    delete_op_.Undo();
  }

 private:
  InsertOperation insert_op_;
  DeleteOperation delete_op_;
};

}  // namespace

CFDE_TextEditEngine::CFDE_TextEditEngine()
    : font_color_(0xff000000),
      font_size_(10.0f),
      line_spacing_(10.0f),
      text_length_(0),
      gap_position_(0),
      gap_size_(kGapSize),
      available_width_(kPageWidthMax),
      character_limit_(std::numeric_limits<size_t>::max()),
      visible_line_count_(1),
      next_operation_index_to_undo_(kMaxEditOperations - 1),
      next_operation_index_to_insert_(0),
      max_edit_operations_(kMaxEditOperations),
      character_alignment_(CFX_TxtLineAlignment_Left),
      has_character_limit_(false),
      is_comb_text_(false),
      is_dirty_(false),
      validation_enabled_(false),
      is_multiline_(false),
      is_linewrap_enabled_(false),
      limit_horizontal_area_(false),
      limit_vertical_area_(false),
      password_mode_(false),
      password_alias_(L'*'),
      has_selection_(false),
      selection_({0, 0}) {
  content_.resize(gap_size_);
  operation_buffer_.resize(max_edit_operations_);

  text_break_.SetFontSize(font_size_);
  text_break_.SetLineBreakTolerance(2.0f);
  text_break_.SetTabWidth(36);
}

CFDE_TextEditEngine::~CFDE_TextEditEngine() {}

void CFDE_TextEditEngine::Clear() {
  text_length_ = 0;
  gap_position_ = 0;
  gap_size_ = kGapSize;

  content_.clear();
  content_.resize(gap_size_);

  ClearSelection();
  ClearOperationRecords();
}

void CFDE_TextEditEngine::SetMaxEditOperationsForTesting(size_t max) {
  max_edit_operations_ = max;
  operation_buffer_.resize(max);

  ClearOperationRecords();
}

void CFDE_TextEditEngine::AdjustGap(size_t idx, size_t length) {
  static const size_t char_size = sizeof(CFX_WideString::CharType);

  // Move the gap, if necessary.
  if (idx < gap_position_) {
    memmove(content_.data() + idx + gap_size_, content_.data() + idx,
            (gap_position_ - idx) * char_size);
    gap_position_ = idx;
  } else if (idx > gap_position_) {
    memmove(content_.data() + gap_position_,
            content_.data() + gap_position_ + gap_size_,
            (idx - gap_position_) * char_size);
    gap_position_ = idx;
  }

  // If the gap is too small, make it bigger.
  if (length >= gap_size_) {
    size_t new_gap_size = length + kGapSize;
    content_.resize(text_length_ + new_gap_size);

    memmove(content_.data() + gap_position_ + new_gap_size,
            content_.data() + gap_position_ + gap_size_,
            (text_length_ - gap_position_) * char_size);

    gap_size_ = new_gap_size;
  }
}

size_t CFDE_TextEditEngine::CountCharsExceedingSize(const CFX_WideString& text,
                                                    size_t num_to_check) {
  if (!limit_horizontal_area_ && !limit_vertical_area_)
    return 0;

  auto text_out = pdfium::MakeUnique<CFDE_TextOut>();
  text_out->SetLineSpace(line_spacing_);
  text_out->SetFont(font_);
  text_out->SetFontSize(font_size_);

  FDE_TextStyle style;
  style.single_line_ = !is_multiline_;

  CFX_RectF text_rect;
  if (is_linewrap_enabled_) {
    style.line_wrap_ = true;
    text_rect.width = available_width_;
  } else {
    text_rect.width = kPageWidthMax;
  }
  text_out->SetStyles(style);

  size_t length = text.GetLength();
  CFX_WideStringC temp(text.c_str(), length);

  float vertical_height = line_spacing_ * visible_line_count_;
  size_t chars_exceeding_size = 0;
  // TODO(dsinclair): Can this get changed to a binary search?
  for (size_t i = 0; i < num_to_check; i++) {
    // This does a lot of string copying ....
    // TODO(dsinclair): make CalcLogicSize take a WideStringC instead.
    text_out->CalcLogicSize(CFX_WideString(temp), text_rect);

    if (limit_horizontal_area_ && text_rect.width <= available_width_)
      break;
    if (limit_vertical_area_ && text_rect.height <= vertical_height)
      break;

    --length;
    temp = temp.Mid(0, length);
    ++chars_exceeding_size;
  }

  return chars_exceeding_size;
}

void CFDE_TextEditEngine::Insert(size_t idx,
                                 const CFX_WideString& text,
                                 RecordOperation add_operation) {
  if (idx > text_length_)
    idx = text_length_;

  size_t length = text.GetLength();
  if (length == 0)
    return;

  // If we're going to be too big we insert what we can and notify the
  // delegate we've filled the text after the insert is done.
  bool exceeded_limit = false;
  if (has_character_limit_ && text_length_ + length > character_limit_) {
    exceeded_limit = true;
    length = character_limit_ - text_length_;
  }

  AdjustGap(idx, length);

  if (validation_enabled_ || limit_horizontal_area_ || limit_vertical_area_) {
    CFX_WideString str;
    if (gap_position_ > 0)
      str += CFX_WideStringC(content_.data(), gap_position_);

    str += text;

    if (text_length_ - gap_position_ > 0) {
      str += CFX_WideStringC(content_.data() + gap_position_ + gap_size_,
                             text_length_ - gap_position_);
    }

    if (validation_enabled_ && delegate_ && !delegate_->OnValidate(str)) {
      // TODO(dsinclair): Notify delegate of validation failure?
      return;
    }

    // Check if we've limited the horizontal/vertical area, and if so determine
    // how many of our characters would be outside the area.
    size_t chars_exceeding = CountCharsExceedingSize(str, length);
    if (chars_exceeding > 0) {
      // If none of the characters will fit, notify and exit.
      if (chars_exceeding == length) {
        if (delegate_)
          delegate_->NotifyTextFull();
        return;
      }

      // Some, but not all, chars will fit, insert them and then notify
      // we're full.
      exceeded_limit = true;
      length -= chars_exceeding;
    }
  }

  if (add_operation == RecordOperation::kInsertRecord) {
    AddOperationRecord(
        pdfium::MakeUnique<InsertOperation>(this, gap_position_, text));
  }

  CFX_WideString previous_text;
  if (delegate_)
    previous_text = GetText();

  // Copy the new text into the gap.
  static const size_t char_size = sizeof(CFX_WideString::CharType);
  memcpy(content_.data() + gap_position_, text.c_str(), length * char_size);
  gap_position_ += length;
  gap_size_ -= length;
  text_length_ += length;

  is_dirty_ = true;

  // Inserting text resets the selection.
  ClearSelection();

  if (delegate_) {
    if (exceeded_limit)
      delegate_->NotifyTextFull();

    delegate_->OnTextChanged(previous_text);
  }
}

void CFDE_TextEditEngine::AddOperationRecord(std::unique_ptr<Operation> op) {
  size_t last_insert_position = next_operation_index_to_insert_ == 0
                                    ? max_edit_operations_ - 1
                                    : next_operation_index_to_insert_ - 1;

  // If our undo record is not the last thing we inserted then we need to
  // remove all the undo records between our insert position and the undo marker
  // and make that our new insert position.
  if (next_operation_index_to_undo_ != last_insert_position) {
    if (next_operation_index_to_undo_ > last_insert_position) {
      // Our Undo position is ahead of us, which means we need to clear out the
      // head of the queue.
      while (last_insert_position != 0) {
        operation_buffer_[last_insert_position].reset();
        --last_insert_position;
      }
      operation_buffer_[0].reset();

      // Moving this will let us then clear out the end, setting the undo
      // position to before the insert position.
      last_insert_position = max_edit_operations_ - 1;
    }

    // Clear out the vector from undo position to our set insert position.
    while (next_operation_index_to_undo_ != last_insert_position) {
      operation_buffer_[last_insert_position].reset();
      --last_insert_position;
    }
  }

  // We're now pointing at the next thing we want to Undo, so insert at the
  // next position in the queue.
  ++last_insert_position;
  if (last_insert_position >= max_edit_operations_)
    last_insert_position = 0;

  operation_buffer_[last_insert_position] = std::move(op);
  next_operation_index_to_insert_ =
      (last_insert_position + 1) % max_edit_operations_;
  next_operation_index_to_undo_ = last_insert_position;
}

void CFDE_TextEditEngine::ClearOperationRecords() {
  for (auto& record : operation_buffer_)
    record.reset();

  next_operation_index_to_undo_ = max_edit_operations_ - 1;
  next_operation_index_to_insert_ = 0;
}

void CFDE_TextEditEngine::LimitHorizontalScroll(bool val) {
  ClearOperationRecords();
  limit_horizontal_area_ = val;
}
void CFDE_TextEditEngine::LimitVerticalScroll(bool val) {
  ClearOperationRecords();
  limit_vertical_area_ = val;
}

bool CFDE_TextEditEngine::CanUndo() const {
  return operation_buffer_[next_operation_index_to_undo_] != nullptr &&
         next_operation_index_to_undo_ != next_operation_index_to_insert_;
}

bool CFDE_TextEditEngine::CanRedo() const {
  size_t idx = (next_operation_index_to_undo_ + 1) % max_edit_operations_;
  return idx != next_operation_index_to_insert_ &&
         operation_buffer_[idx] != nullptr;
}

bool CFDE_TextEditEngine::Redo() {
  if (!CanRedo())
    return false;

  next_operation_index_to_undo_ =
      (next_operation_index_to_undo_ + 1) % max_edit_operations_;
  operation_buffer_[next_operation_index_to_undo_]->Redo();
  return true;
}

bool CFDE_TextEditEngine::Undo() {
  if (!CanUndo())
    return false;

  operation_buffer_[next_operation_index_to_undo_]->Undo();
  next_operation_index_to_undo_ = next_operation_index_to_undo_ == 0
                                      ? max_edit_operations_ - 1
                                      : next_operation_index_to_undo_ - 1;
  return true;
}

void CFDE_TextEditEngine::Layout() {
  if (!is_dirty_)
    return;

  is_dirty_ = false;
  RebuildPieces();
}

CFX_RectF CFDE_TextEditEngine::GetContentsBoundingBox() {
  // Layout if necessary.
  Layout();
  return contents_bounding_box_;
}

void CFDE_TextEditEngine::SetAvailableWidth(size_t width) {
  if (width == available_width_)
    return;

  ClearOperationRecords();

  available_width_ = width;
  if (is_linewrap_enabled_)
    text_break_.SetLineWidth(width);
  if (is_comb_text_)
    SetCombTextWidth();

  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetHasCharacterLimit(bool limit) {
  if (has_character_limit_ == limit)
    return;

  has_character_limit_ = limit;
  if (is_comb_text_)
    SetCombTextWidth();

  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetCharacterLimit(size_t limit) {
  if (character_limit_ == limit)
    return;

  ClearOperationRecords();

  character_limit_ = limit;
  if (is_comb_text_)
    SetCombTextWidth();

  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetFont(CFX_RetainPtr<CFGAS_GEFont> font) {
  if (font_ == font)
    return;

  font_ = font;
  text_break_.SetFont(font_);
  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetFontSize(float size) {
  if (font_size_ == size)
    return;

  font_size_ = size;
  text_break_.SetFontSize(font_size_);
  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetTabWidth(float width) {
  int32_t old_tab_width = text_break_.GetTabWidth();
  text_break_.SetTabWidth(width);
  if (old_tab_width == text_break_.GetTabWidth())
    return;

  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetAlignment(uint32_t alignment) {
  if (alignment == character_alignment_)
    return;

  character_alignment_ = alignment;
  text_break_.SetAlignment(alignment);
  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetVisibleLineCount(size_t count) {
  if (visible_line_count_ == count)
    return;

  visible_line_count_ = std::max(static_cast<size_t>(1), count);
  is_dirty_ = true;
}

void CFDE_TextEditEngine::EnableMultiLine(bool val) {
  if (is_multiline_ == val)
    return;

  is_multiline_ = true;

  uint32_t style = text_break_.GetLayoutStyles();
  if (is_multiline_)
    style &= ~FX_LAYOUTSTYLE_SingleLine;
  else
    style |= FX_LAYOUTSTYLE_SingleLine;
  text_break_.SetLayoutStyles(style);
  is_dirty_ = true;
}

void CFDE_TextEditEngine::EnableLineWrap(bool val) {
  if (is_linewrap_enabled_ == val)
    return;

  is_linewrap_enabled_ = val;
  text_break_.SetLineWidth(is_linewrap_enabled_ ? available_width_
                                                : kPageWidthMax);
  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetCombText(bool enable) {
  if (is_comb_text_ == enable)
    return;

  is_comb_text_ = enable;

  uint32_t style = text_break_.GetLayoutStyles();
  if (enable) {
    style |= FX_LAYOUTSTYLE_CombText;
    SetCombTextWidth();
  } else {
    style &= ~FX_LAYOUTSTYLE_CombText;
  }
  text_break_.SetLayoutStyles(style);
  is_dirty_ = true;
}

void CFDE_TextEditEngine::SetCombTextWidth() {
  size_t width = available_width_;
  if (has_character_limit_)
    width /= character_limit_;

  text_break_.SetCombWidth(width);
}

void CFDE_TextEditEngine::SelectAll() {
  if (text_length_ == 0)
    return;

  has_selection_ = true;
  selection_.start_idx = 0;
  selection_.end_idx = text_length_ - 1;
}

void CFDE_TextEditEngine::ClearSelection() {
  has_selection_ = false;
  selection_.start_idx = 0;
  selection_.end_idx = 0;
}

void CFDE_TextEditEngine::SetSelection(size_t start_idx, size_t end_idx) {
  // If the points are the same, then we pretend the selection doesn't exist
  // anymore.
  if (start_idx == end_idx) {
    ClearSelection();
    return;
  }

  if (start_idx > text_length_)
    return;
  if (end_idx > text_length_)
    end_idx = text_length_ - 1;

  has_selection_ = true;
  selection_.start_idx = start_idx;
  selection_.end_idx = end_idx;
}

CFX_WideString CFDE_TextEditEngine::GetSelectedText() const {
  if (!has_selection_)
    return L"";

  CFX_WideString text;
  if (selection_.start_idx < gap_position_) {
    if (selection_.end_idx < gap_position_) {
      text += CFX_WideStringC(content_.data() + selection_.start_idx,
                              selection_.end_idx - selection_.start_idx + 1);
      return text;
    }

    text += CFX_WideStringC(content_.data() + selection_.start_idx,
                            gap_position_ - selection_.start_idx);
    text += CFX_WideStringC(
        content_.data() + gap_position_ + gap_size_,
        selection_.end_idx - (gap_position_ - selection_.start_idx) + 1);
    return text;
  }

  text += CFX_WideStringC(content_.data() + gap_size_ + selection_.start_idx,
                          selection_.end_idx - selection_.start_idx + 1);
  return text;
}

CFX_WideString CFDE_TextEditEngine::DeleteSelectedText(
    RecordOperation add_operation) {
  if (!has_selection_)
    return L"";

  return Delete(selection_.start_idx,
                selection_.end_idx - selection_.start_idx + 1, add_operation);
}

CFX_WideString CFDE_TextEditEngine::Delete(size_t start_idx,
                                           size_t length,
                                           RecordOperation add_operation) {
  if (start_idx >= text_length_)
    return L"";

  length = std::min(length, text_length_ - start_idx);
  AdjustGap(start_idx + length, 0);

  CFX_WideString ret;
  ret += CFX_WideStringC(content_.data() + start_idx, length);

  if (add_operation == RecordOperation::kInsertRecord) {
    AddOperationRecord(
        pdfium::MakeUnique<DeleteOperation>(this, start_idx, ret));
  }

  CFX_WideString previous_text = GetText();

  gap_position_ = start_idx;
  gap_size_ += length;

  text_length_ -= length;
  ClearSelection();

  if (delegate_)
    delegate_->OnTextChanged(previous_text);

  return ret;
}

void CFDE_TextEditEngine::ReplaceSelectedText(const CFX_WideString& rep) {
  size_t start_idx = selection_.start_idx;

  CFX_WideString txt = DeleteSelectedText(RecordOperation::kSkipRecord);
  Insert(gap_position_, rep, RecordOperation::kSkipRecord);

  AddOperationRecord(
      pdfium::MakeUnique<ReplaceOperation>(this, start_idx, txt, rep));
}

CFX_WideString CFDE_TextEditEngine::GetText() const {
  CFX_WideString str;
  if (gap_position_ > 0)
    str += CFX_WideStringC(content_.data(), gap_position_);
  if (text_length_ - gap_position_ > 0) {
    str += CFX_WideStringC(content_.data() + gap_position_ + gap_size_,
                           text_length_ - gap_position_);
  }
  return str;
}

size_t CFDE_TextEditEngine::GetLength() const {
  return text_length_;
}

wchar_t CFDE_TextEditEngine::GetChar(size_t idx) const {
  if (idx >= text_length_)
    return L'\0';
  if (password_mode_)
    return password_alias_;

  return idx < gap_position_
             ? content_[idx]
             : content_[gap_position_ + gap_size_ + (idx - gap_position_)];
}

size_t CFDE_TextEditEngine::GetWidthOfChar(size_t idx) {
  // Recalculate the widths if necessary.
  Layout();
  return idx < char_widths_.size() ? char_widths_[idx] : 0;
}

size_t CFDE_TextEditEngine::GetIndexForPoint(const CFX_PointF& point) {
  // Recalculate the widths if necessary.
  Layout();

  auto start_it = text_piece_info_.begin();
  for (; start_it < text_piece_info_.end(); ++start_it) {
    if (start_it->rtPiece.top <= point.y &&
        point.y < start_it->rtPiece.bottom())
      break;
  }
  // We didn't find the point before getting to the end of the text, return
  // end of text.
  if (start_it == text_piece_info_.end())
    return text_length_;

  auto end_it = start_it;
  for (; end_it < text_piece_info_.end(); ++end_it) {
    // We've moved past where the point should be and didn't find anything.
    // Return the start of the current piece as the location.
    if (end_it->rtPiece.bottom() <= point.y || point.y < end_it->rtPiece.top)
      break;
  }
  // Make sure the end iterator is pointing to our text pieces.
  if (end_it == text_piece_info_.end())
    --end_it;

  size_t start_it_idx = start_it->nStart;
  for (; start_it <= end_it; ++start_it) {
    if (!start_it->rtPiece.Contains(point))
      continue;

    std::vector<CFX_RectF> rects = GetCharRects(*start_it);
    for (size_t i = 0; i < rects.size(); ++i) {
      if (!rects[i].Contains(point))
        continue;
      size_t pos = start_it->nStart + i;
      if (pos >= text_length_)
        return text_length_;

      wchar_t wch = GetChar(pos);
      if (wch == L'\n' || wch == L'\r') {
        if (wch == L'\n' && pos > 0 && GetChar(pos - 1) == L'\r')
          --pos;
        return pos;
      }

      // TODO(dsinclair): Old code had a before flag set based on bidi?
      return pos;
    }
  }

  if (start_it == text_piece_info_.end())
    return start_it_idx;
  if (start_it == end_it)
    return start_it->nStart;

  // We didn't find the point before going over all of the pieces, we want to
  // return the start of the piece after the point.
  return end_it->nStart;
}

std::vector<CFX_RectF> CFDE_TextEditEngine::GetCharRects(
    const FDE_TEXTEDITPIECE& piece) {
  if (piece.nCount < 1)
    return {};

  FX_TXTRUN tr;
  tr.pEdtEngine = this;
  tr.pIdentity = &piece;
  tr.iLength = piece.nCount;
  tr.pFont = font_;
  tr.fFontSize = font_size_;
  tr.dwStyles = text_break_.GetLayoutStyles();
  tr.dwCharStyles = piece.dwCharStyles;
  tr.pRect = &piece.rtPiece;
  return text_break_.GetCharRects(&tr, false);
}

std::vector<FXTEXT_CHARPOS> CFDE_TextEditEngine::GetDisplayPos(
    const FDE_TEXTEDITPIECE& piece) {
  if (piece.nCount < 1)
    return std::vector<FXTEXT_CHARPOS>();

  FX_TXTRUN tr;
  tr.pEdtEngine = this;
  tr.pIdentity = &piece;
  tr.iLength = piece.nCount;
  tr.pFont = font_;
  tr.fFontSize = font_size_;
  tr.dwStyles = text_break_.GetLayoutStyles();
  tr.dwCharStyles = piece.dwCharStyles;
  tr.pRect = &piece.rtPiece;

  std::vector<FXTEXT_CHARPOS> data(text_break_.GetDisplayPos(&tr, nullptr));
  text_break_.GetDisplayPos(&tr, data.data());
  return data;
}

void CFDE_TextEditEngine::RebuildPieces() {
  text_break_.EndBreak(CFX_BreakType::Paragraph);
  text_break_.ClearBreakPieces();

  char_widths_.clear();
  text_piece_info_.clear();

  // Must have a font set in order to break the text.
  if (text_length_ == 0 || !font_)
    return;

  bool initialized_bounding_box = false;
  contents_bounding_box_ = CFX_RectF();

  auto iter = pdfium::MakeUnique<CFDE_TextEditEngine::Iterator>(this);
  while (!iter->IsEOF(true)) {
    iter->Next(false);

    CFX_BreakType break_status = text_break_.AppendChar(
        password_mode_ ? password_alias_ : iter->GetChar());
    if (iter->IsEOF(true) && CFX_BreakTypeNoneOrPiece(break_status))
      break_status = text_break_.EndBreak(CFX_BreakType::Paragraph);

    if (CFX_BreakTypeNoneOrPiece(break_status))
      continue;

    size_t current_piece_start = 0;
    float current_line_start = 0;
    int32_t piece_count = text_break_.CountBreakPieces();
    for (int32_t i = 0; i < piece_count; ++i) {
      const CFX_BreakPiece* piece = text_break_.GetBreakPieceUnstable(i);

      FDE_TEXTEDITPIECE txtEdtPiece;
      memset(&txtEdtPiece, 0, sizeof(FDE_TEXTEDITPIECE));

      txtEdtPiece.nBidiLevel = piece->m_iBidiLevel;
      txtEdtPiece.nCount = piece->GetLength();
      txtEdtPiece.nStart = current_piece_start;
      txtEdtPiece.dwCharStyles = piece->m_dwCharStyles;
      if (FX_IsOdd(piece->m_iBidiLevel))
        txtEdtPiece.dwCharStyles |= FX_TXTCHARSTYLE_OddBidiLevel;

      txtEdtPiece.rtPiece.left = piece->m_iStartPos / 20000.0f;
      txtEdtPiece.rtPiece.top = current_line_start;
      txtEdtPiece.rtPiece.width = piece->m_iWidth / 20000.0f;
      txtEdtPiece.rtPiece.height = line_spacing_;
      text_piece_info_.push_back(txtEdtPiece);

      if (initialized_bounding_box) {
        contents_bounding_box_.Union(txtEdtPiece.rtPiece);
      } else {
        contents_bounding_box_ = txtEdtPiece.rtPiece;
        initialized_bounding_box = true;
      }

      current_piece_start += txtEdtPiece.nCount;
      for (int32_t k = 0; k < txtEdtPiece.nCount; ++k)
        char_widths_.push_back(piece->GetChar(k)->m_iCharWidth);
    }

    current_line_start += line_spacing_;
    text_break_.ClearBreakPieces();
  }

  float delta = 0.0;
  bool bounds_smaller = contents_bounding_box_.width < available_width_;
  if (IsAlignedRight() && bounds_smaller) {
    delta = available_width_ - contents_bounding_box_.width;
  } else if (IsAlignedCenter() && bounds_smaller) {
    // TODO(dsinclair): Old code used CombText here and set the space to
    // something unrelated to the available width .... Figure out if this is
    // needed and what it should do.
    // if (is_comb_text_) {
    // } else {
    delta = (available_width_ - contents_bounding_box_.width) / 2.0f;
    // }
  }

  if (delta != 0.0) {
    float offset = delta - contents_bounding_box_.left;
    for (auto& info : text_piece_info_)
      info.rtPiece.Offset(offset, 0.0f);
    contents_bounding_box_.Offset(offset, 0.0f);
  }

  // Shrink the last piece down to the font_size.
  contents_bounding_box_.height -= line_spacing_ - font_size_;
  text_piece_info_.back().rtPiece.height = font_size_;
}

std::pair<int32_t, CFX_RectF> CFDE_TextEditEngine::GetCharacterInfo(
    int32_t start_idx) {
  ASSERT(start_idx >= 0);
  ASSERT(static_cast<size_t>(start_idx) <= text_length_);

  // Make sure the current available data is fresh.
  Layout();

  auto it = text_piece_info_.begin();
  for (; it != text_piece_info_.end(); ++it) {
    if (it->nStart <= start_idx && start_idx < it->nStart + it->nCount)
      break;
  }
  if (it == text_piece_info_.end()) {
    NOTREACHED();
    return {0, CFX_RectF()};
  }

  return {it->nBidiLevel, GetCharRects(*it)[start_idx - it->nStart]};
}

std::vector<CFX_RectF> CFDE_TextEditEngine::GetCharacterRectsInRange(
    int32_t start_idx,
    int32_t count) {
  // Make sure the current available data is fresh.
  Layout();

  auto it = text_piece_info_.begin();
  for (; it != text_piece_info_.end(); ++it) {
    if (it->nStart <= start_idx && start_idx < it->nStart + it->nCount)
      break;
  }
  if (it == text_piece_info_.end())
    return {};

  int32_t end_idx = start_idx + count - 1;
  std::vector<CFX_RectF> rects;
  while (it != text_piece_info_.end()) {
    // If we end inside the current piece, extract what we need and we're done.
    if (it->nStart <= end_idx && end_idx < it->nStart + it->nCount) {
      std::vector<CFX_RectF> arr = GetCharRects(*it);
      CFX_RectF piece = arr[0];
      piece.Union(arr[end_idx - it->nStart]);
      rects.push_back(piece);
      break;
    }
    rects.push_back(it->rtPiece);
    ++it;
  }

  return rects;
}

CFDE_TextEditEngine::Iterator::Iterator(CFDE_TextEditEngine* engine)
    : engine_(engine), current_position_(-1) {}

CFDE_TextEditEngine::Iterator::~Iterator() {}

bool CFDE_TextEditEngine::Iterator::Next(bool bPrev) {
  if (bPrev && current_position_ == -1)
    return false;
  if (!bPrev && current_position_ > -1 &&
      static_cast<size_t>(current_position_) == engine_->GetLength())
    return false;

  if (bPrev)
    --current_position_;
  else
    ++current_position_;

  return true;
}

wchar_t CFDE_TextEditEngine::Iterator::GetChar() const {
  return engine_->GetChar(current_position_);
}

void CFDE_TextEditEngine::Iterator::SetAt(int32_t nIndex) {
  NOTREACHED();
}

int32_t CFDE_TextEditEngine::Iterator::GetAt() const {
  return current_position_;
}

bool CFDE_TextEditEngine::Iterator::IsEOF(bool bTail) const {
  return bTail ? current_position_ > -1 &&
                     static_cast<size_t>(current_position_) ==
                         engine_->GetLength()
               : current_position_ == -1;
}

std::unique_ptr<IFX_CharIter> CFDE_TextEditEngine::Iterator::Clone() const {
  NOTREACHED();
  return pdfium::MakeUnique<CFDE_TextEditEngine::Iterator>(engine_.Get());
}

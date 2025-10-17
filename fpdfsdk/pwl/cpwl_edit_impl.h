// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_EDIT_IMPL_H_
#define FPDFSDK_PWL_CPWL_EDIT_IMPL_H_

#include <deque>
#include <memory>
#include <utility>
#include <vector>

#include "core/fpdfdoc/cpvt_variabletext.h"
#include "core/fpdfdoc/cpvt_wordrange.h"
#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/fx_codepage_forward.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxge/dib/fx_dib.h"
#include "fpdfsdk/pwl/ipwl_fillernotify.h"

class CFX_RenderDevice;
class CPWL_Edit;

class CPWL_EditImpl {
 public:
  class Iterator {
   public:
    Iterator(CPWL_EditImpl* pEdit, CPVT_VariableText::Iterator* pVTIterator);
    ~Iterator();

    bool NextWord();
    bool GetWord(CPVT_Word& word) const;
    bool GetLine(CPVT_Line& line) const;
    void SetAt(int32_t nWordIndex);
    void SetAt(const CPVT_WordPlace& place);
    const CPVT_WordPlace& GetAt() const;

   private:
    UnownedPtr<CPWL_EditImpl> edit_;
    UnownedPtr<CPVT_VariableText::Iterator> vt_iterator_;
  };

  CPWL_EditImpl();
  ~CPWL_EditImpl();

  void DrawEdit(CFX_RenderDevice* pDevice,
                const CFX_Matrix& mtUser2Device,
                FX_COLORREF crTextFill,
                const CFX_FloatRect& rcClip,
                const CFX_PointF& ptOffset,
                const CPVT_WordRange* pRange,
                IPWL_FillerNotify* pFillerNotify,
                IPWL_FillerNotify::PerWindowData* pSystemData);

  void SetFontMap(IPVT_FontMap* font_map);
  void SetNotify(CPWL_Edit* pNotify);

  // Returns an iterator for the contents. Should not be released.
  Iterator* GetIterator();
  IPVT_FontMap* GetFontMap();
  void Initialize();

  // Set the bounding box of the text area.
  void SetPlateRect(const CFX_FloatRect& rect);
  void SetScrollPos(const CFX_PointF& point);

  // Set the horizontal text alignment. (nFormat [0:left, 1:middle, 2:right])
  void SetAlignmentH(int32_t nFormat);

  // Set the vertical text alignment. (nFormat [0:left, 1:middle, 2:right])
  void SetAlignmentV(int32_t nFormat);

  // Set the substitution character for hidden text.
  void SetPasswordChar(uint16_t wSubWord);

  // Set the maximum number of words in the text.
  void SetLimitChar(int32_t nLimitChar);
  void SetCharArray(int32_t nCharArray);
  void SetMultiLine(bool bMultiLine);
  void SetAutoReturn(bool bAuto);
  void SetAutoFontSize(bool bAuto);
  void SetAutoScroll(bool bAuto);
  void SetFontSize(float fFontSize);
  void SetTextOverflow(bool bAllowed);
  void OnMouseDown(const CFX_PointF& point, bool bShift, bool bCtrl);
  void OnMouseMove(const CFX_PointF& point, bool bShift, bool bCtrl);
  void OnVK_UP(bool bShift);
  void OnVK_DOWN(bool bShift);
  void OnVK_LEFT(bool bShift);
  void OnVK_RIGHT(bool bShift);
  void OnVK_HOME(bool bShift, bool bCtrl);
  void OnVK_END(bool bShift, bool bCtrl);
  void SetText(const WideString& sText);
  bool InsertWord(uint16_t word, FX_Charset charset);
  void InsertReturn();
  void Backspace();
  bool Delete();
  bool ClearSelection();
  void InsertText(const WideString& sText, FX_Charset charset);
  void ReplaceAndKeepSelection(const WideString& text);
  void ReplaceSelection(const WideString& text);
  void TypeChar(uint16_t word, FX_Charset charset);
  bool Redo();
  bool Undo();
  void SetMaxUndoItemsForTest(size_t items);
  CPVT_WordPlace WordIndexToWordPlace(int32_t index) const;
  CPVT_WordPlace SearchWordPlace(const CFX_PointF& point) const;
  int32_t GetCaret() const;
  CPVT_WordPlace GetCaretWordPlace() const;
  WideString GetSelectedText() const;
  WideString GetText() const;
  float GetFontSize() const;
  uint16_t GetPasswordChar() const;
  CFX_PointF GetScrollPos() const;
  int32_t GetCharArray() const;
  CFX_FloatRect GetContentRect() const;
  WideString GetRangeText(const CPVT_WordRange& range) const;
  void SetSelection(int32_t nStartChar, int32_t nEndChar);
  std::pair<int32_t, int32_t> GetSelection() const;
  void SelectAll();
  void SelectNone();
  bool IsSelected() const;
  void Paint();
  void EnableRefresh(bool bRefresh);
  void RefreshWordRange(const CPVT_WordRange& wr);
  CPVT_WordRange GetWholeWordRange() const;
  CPVT_WordRange GetSelectWordRange() const;
  void EnableUndo(bool bUndo);
  bool IsTextFull() const;
  bool CanUndo() const;
  bool CanRedo() const;
  CPVT_WordRange GetVisibleWordRange() const;

  ByteString GetPDFWordString(int32_t nFontIndex,
                              uint16_t Word,
                              uint16_t SubWord);

 private:
  class RefreshState {
   public:
    RefreshState();
    ~RefreshState();

    void BeginRefresh();
    void Push(const CPVT_WordRange& linerange, const CFX_FloatRect& rect);
    void NoAnalyse();
    std::vector<CFX_FloatRect>* GetRefreshRects();
    void EndRefresh();

   private:
    struct LineRect {
      LineRect(const CPVT_WordRange& wrLine, const CFX_FloatRect& rcLine)
          : line_word_range_(wrLine), line_rect_(rcLine) {}

      CPVT_WordRange line_word_range_;
      CFX_FloatRect line_rect_;
    };

    void Add(const CFX_FloatRect& new_rect);

    std::vector<LineRect> new_line_rects_;
    std::vector<LineRect> old_line_rects_;
    std::vector<CFX_FloatRect> refresh_rects_;
  };

  class SelectState {
   public:
    SelectState();
    explicit SelectState(const CPVT_WordRange& range);

    void Reset();
    void Set(const CPVT_WordPlace& begin, const CPVT_WordPlace& end);
    void SetEndPos(const CPVT_WordPlace& end);

    CPVT_WordRange ConvertToWordRange() const;
    bool IsEmpty() const;

    CPVT_WordPlace BeginPos;
    CPVT_WordPlace EndPos;
  };

  class UndoItemIface {
   public:
    virtual ~UndoItemIface() = default;

    virtual void Undo() = 0;
    virtual void Redo() = 0;

    // Return true if this is a sentinel undo item, the undo stack will continue
    // performing undo/redo operations until the next sentinel undo item.
    //
    // Returns false by default. This is used by UndoReplaceSelection.
    virtual bool IsSentinel();
  };

  class UndoStack {
   public:
    UndoStack();
    ~UndoStack();

    void AddItem(std::unique_ptr<UndoItemIface> pItem);
    void Undo();
    void Redo();
    bool CanUndo() const;
    bool CanRedo() const;
    // items must be at least `kMinEditUndoMaxItems` since
    // CPWL_EditImpl::ReplaceSelection() inserts that many items into the queue
    // and they need to all fit.
    void SetMaxUndoItemsForTest(size_t items);

   private:
    void RemoveHeads();
    void RemoveTails();

    static constexpr int kEditUndoMaxItems = 10000;
    static constexpr int kMinEditUndoMaxItems = 4;
    static_assert(
        kEditUndoMaxItems >= kMinEditUndoMaxItems,
        "CPWL_EditImpl::ReplaceSelection() inserts a group of several "
        "undo items, which must fit in the queue.");

    std::deque<std::unique_ptr<UndoItemIface>> undo_item_stack_;
    size_t cur_undo_pos_ = 0;
    bool working_ = false;
    size_t max_undo_items_ = kEditUndoMaxItems;
  };

  class Provider;
  class UndoBackspace;
  class UndoClear;
  class UndoDelete;
  class UndoInsertReturn;
  class UndoInsertText;
  class UndoInsertWord;
  class UndoReplaceSelection;

  bool IsTextOverflow() const;
  bool Clear();
  CPVT_WordPlace DoInsertText(const CPVT_WordPlace& place,
                              const WideString& sText,
                              FX_Charset charset);
  FX_Charset GetCharSetFromUnicode(uint16_t word, FX_Charset nOldCharset);
  int32_t GetTotalLines() const;
  void SetSelection(const CPVT_WordPlace& begin, const CPVT_WordPlace& end);
  bool Delete(bool bAddUndo);
  bool Clear(bool bAddUndo);
  void InsertText(const WideString& sText, FX_Charset charset, bool bAddUndo);
  bool InsertWord(uint16_t word, FX_Charset charset, bool bAddUndo);
  void InsertReturn(bool bAddUndo);
  void Backspace(bool bAddUndo);
  void SetCaret(const CPVT_WordPlace& place);

  CFX_PointF VTToEdit(const CFX_PointF& point) const;

  void RearrangeAll();
  void RearrangePart(const CPVT_WordRange& range);
  void ScrollToCaret();
  void SetScrollInfo();
  void SetScrollPosX(float fx);
  void SetScrollPosY(float fy);
  void SetScrollLimit();
  void SetContentChanged();

  void PaintInsertText(const CPVT_WordPlace& wpOld,
                       const CPVT_WordPlace& wpNew);

  CFX_PointF EditToVT(const CFX_PointF& point) const;
  CFX_FloatRect VTToEdit(const CFX_FloatRect& rect) const;

  void Refresh();
  void RefreshPushLineRects(const CPVT_WordRange& wr);

  void SetCaretInfo();
  void SetCaretOrigin();

  void AddEditUndoItem(std::unique_ptr<UndoItemIface> pEditUndoItem);

  bool enable_scroll_ = false;
  bool notify_flag_ = false;
  bool enable_overflow_ = false;
  bool enable_refresh_ = true;
  bool enable_undo_ = true;
  int32_t alignment_ = 0;
  std::unique_ptr<Provider> vt_provider_;
  std::unique_ptr<CPVT_VariableText> vt_;  // Must outlive |vt_provider_|.
  UnownedPtr<CPWL_Edit> notify_;
  CPVT_WordPlace wp_caret_;
  CPVT_WordPlace wp_old_caret_;
  SelectState sel_state_;
  CFX_PointF scroll_pos_point_;
  CFX_PointF pt_refresh_scroll_pos_;
  std::unique_ptr<Iterator> iterator_;
  RefreshState refresh_;
  CFX_PointF caret_point_;
  UndoStack undo_;
  CFX_FloatRect old_content_rect_;
};

#endif  // FPDFSDK_PWL_CPWL_EDIT_IMPL_H_

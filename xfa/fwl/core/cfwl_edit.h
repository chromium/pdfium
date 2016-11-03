// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_CFWL_EDIT_H_
#define XFA_FWL_CORE_CFWL_EDIT_H_

#include <vector>

#include "xfa/fwl/core/cfwl_widget.h"
#include "xfa/fwl/core/ifwl_edit.h"

class IFDE_TxtEdtDoRecord;

class CFWL_Edit : public CFWL_Widget {
 public:
  CFWL_Edit(const IFWL_App*);
  ~CFWL_Edit() override;

  void Initialize();

  FWL_Error SetText(const CFX_WideString& wsText);
  int32_t GetTextLength() const;
  FWL_Error GetText(CFX_WideString& wsText,
                    int32_t nStart = 0,
                    int32_t nCount = -1) const;
  FWL_Error ClearText();
  int32_t GetCaretPos() const;
  int32_t SetCaretPos(int32_t nIndex, bool bBefore = true);
  int32_t AddSelRange(int32_t nStart, int32_t nCount = -1);
  int32_t CountSelRanges();
  int32_t GetSelRange(int32_t nIndex, int32_t& nStart);
  FWL_Error ClearSelections();
  int32_t GetLimit();
  FWL_Error SetLimit(int32_t nLimit);
  FWL_Error SetAliasChar(FX_WCHAR wAlias);
  FWL_Error SetFormatString(const CFX_WideString& wsFormat);
  FWL_Error Insert(int32_t nStart, const FX_WCHAR* lpText, int32_t nLen);
  FWL_Error DeleteSelections();
  FWL_Error DeleteRange(int32_t nStart, int32_t nCount = -1);
  FWL_Error Replace(int32_t nStart,
                    int32_t nLen,
                    const CFX_WideStringC& wsReplace);
  FWL_Error DoClipboard(int32_t iCmd);
  bool Redo(const IFDE_TxtEdtDoRecord* pRecord);
  bool Undo(const IFDE_TxtEdtDoRecord* pRecord);
  FWL_Error SetTabWidth(FX_FLOAT fTabWidth, bool bEquidistant);
  FWL_Error SetNumberRange(int32_t iMin, int32_t iMax);
  FWL_Error SetBackColor(uint32_t dwColor);
  FWL_Error SetFont(const CFX_WideString& wsFont, FX_FLOAT fSize);
  bool CanUndo();
  bool CanRedo();
  bool Undo();
  bool Redo();
  bool Copy(CFX_WideString& wsCopy);
  bool Cut(CFX_WideString& wsCut);
  bool Paste(const CFX_WideString& wsPaste);
  bool Delete();
  void SetScrollOffset(FX_FLOAT fScrollOffset);
  bool GetSuggestWords(CFX_PointF pointf,
                       std::vector<CFX_ByteString>& sSuggest);
  bool ReplaceSpellCheckWord(CFX_PointF pointf,
                             const CFX_ByteStringC& bsReplace);
};

#endif  // XFA_FWL_CORE_CFWL_EDIT_H_

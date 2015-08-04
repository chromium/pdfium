// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _IFDE_TXTEDTENGINE_H
#define _IFDE_TXTEDTENGINE_H
class IFDE_TxtEdtBuf;
class IFDE_TxtEdtPage;
class IFDE_TxtEdtEngine;
class IFDE_TxtEdtEventSink;
class IFDE_TxtEdtParag;
#define FDE_TXTEDT_FIND_FLAGS_Prev (0L << 0)
#define FDE_TXTEDT_FIND_FLAGS_Next (1L << 0)
#define FDE_TXTEDT_FIND_FLAGS_WholeWord (1L << 1)
#define FDE_TXTEDT_FIND_FLAGS_NoCase (1L << 2)
typedef struct _FDE_HTXTEDTFIND { void* pData; } * FDE_HTXTEDTFIND;
#define FDE_TEXTEDITMODE_MultiLines (1L << 0)
#define FDE_TEXTEDITMODE_AutoLineWrap (1L << 1)
#define FDE_TEXTEDITMODE_ReadOnly (1L << 2)
#define FDE_TEXTEDITMODE_LimitArea_Vert (1L << 3)
#define FDE_TEXTEDITMODE_LimitArea_Horz (1L << 4)
#define FDE_TEXTEDITMODE_NoRedoUndo (1L << 5)
#define FDE_TEXTEDITMODE_FIELD_TAB (1L << 6)
#define FDE_TEXTEDITMODE_FIELD_AUTO (1L << 7)
#define FDE_TEXTEDITMODE_Validate (1L << 8)
#define FDE_TEXTEDITMODE_Password (1L << 9)
#define FDE_TEXTEDITALIGN_Left (0L << 0)
#define FDE_TEXTEDITALIGN_Center (1L << 0)
#define FDE_TEXTEDITALIGN_Right (2L << 0)
#define FDE_TEXTEDITALIGN_Normal (1L << 3)
#define FDE_TEXTEDITALIGN_Justified (1L << 4)
#define FDE_TEXTEDITALIGN_Distributed (1L << 5)
#define FDE_TEXTEDITLAYOUT_DocVertical (1L << 0)
#define FDE_TEXTEDITLAYOUT_CharVertial (1L << 1)
#define FDE_TEXTEDITLAYOUT_LineReserve (1L << 2)
#define FDE_TEXTEDITLAYOUT_RTL (1L << 3)
#define FDE_TEXTEDITLAYOUT_CombText (1L << 4)
#define FDE_TEXTEDITLAYOUT_ExpandTab (1L << 5)
#define FDE_TEXTEDITLAYOUT_ArabicContext (1L << 6)
#define FDE_TEXTEDITLAYOUT_ArabicShapes (1L << 7)
#define FDE_TEXTEDITLAYOUT_LastLineHeight (1L << 8)
enum FDE_TXTEDTMOVECARET {
  MC_MoveNone = 0,
  MC_Left,
  MC_Right,
  MC_Up,
  MC_Down,
  MC_WordBackward,
  MC_WordForward,
  MC_LineStart,
  MC_LineEnd,
  MC_ParagStart,
  MC_ParagEnd,
  MC_PageUp,
  MC_PageDown,
  MC_Home,
  MC_End,
};
enum FDE_TXTEDT_MODIFY_RET {
  FDE_TXTEDT_MODIFY_RET_F_Tab = -6,
  FDE_TXTEDT_MODIFY_RET_F_Locked = -5,
  FDE_TXTEDT_MODIFY_RET_F_Invalidate = -4,
  FDE_TXTEDT_MODIFY_RET_F_Boundary = -3,
  FDE_TXTEDT_MODIFY_RET_F_Full = -2,
  FDE_TXTEDT_MODIFY_RET_F_Normal = -1,
  FDE_TXTEDT_MODIFY_RET_S_Normal = 0,
  FDE_TXTEDT_MODIFY_RET_S_Full = 1,
  FDE_TXTEDT_MODIFY_RET_S_Part = 2,
  FDE_TXTEDT_MODIFY_RET_S_Empty = 3,
  FDE_TXTEDT_MODIFY_RET_T_Tab = 4,
};
enum FDE_TXTEDIT_LINEEND {
  FDE_TXTEDIT_LINEEND_Auto,
  FDE_TXTEDIT_LINEEND_CRLF,
  FDE_TXTEDIT_LINEEND_CR,
  FDE_TXTEDIT_LINEEND_LF,
};
struct _FDE_TXTEDTPARAMS {
  _FDE_TXTEDTPARAMS()
      : fPlateWidth(0),
        fPlateHeight(0),
        nLineCount(0),
        dwLayoutStyles(0),
        dwAlignment(0),
        dwMode(0),
        pFont(NULL),
        fFontSize(10.0f),
        dwFontColor(0xff000000),
        fLineSpace(10.0f),
        fTabWidth(36),
        bTabEquidistant(FALSE),
        wDefChar(0xFEFF),
        wLineBreakChar('\n'),
        nCharRotation(0),
        nLineEnd(0),
        nHorzScale(100),
        fCharSpace(0),
        pEventSink(NULL) {}
  FX_FLOAT fPlateWidth;
  FX_FLOAT fPlateHeight;
  int32_t nLineCount;
  FX_DWORD dwLayoutStyles;
  FX_DWORD dwAlignment;
  FX_DWORD dwMode;
  IFX_Font* pFont;
  FX_FLOAT fFontSize;
  FX_ARGB dwFontColor;
  FX_FLOAT fLineSpace;
  FX_FLOAT fTabWidth;
  FX_BOOL bTabEquidistant;
  FX_WCHAR wDefChar;
  FX_WCHAR wLineBreakChar;
  int32_t nCharRotation;
  int32_t nLineEnd;
  int32_t nHorzScale;
  FX_FLOAT fCharSpace;
  IFDE_TxtEdtEventSink* pEventSink;
};
typedef _FDE_TXTEDTPARAMS FDE_TXTEDTPARAMS;
typedef _FDE_TXTEDTPARAMS* FDE_LPTXTEDTPARAMS;
enum FDE_TXTEDT_TEXTCHANGE_TYPE {
  FDE_TXTEDT_TEXTCHANGE_TYPE_Insert = 0,
  FDE_TXTEDT_TEXTCHANGE_TYPE_Delete,
  FDE_TXTEDT_TEXTCHANGE_TYPE_Replace,
};
struct _FDE_TXTEDT_TEXTCHANGE_INFO {
  int32_t nChangeType;
  CFX_WideString wsInsert;
  CFX_WideString wsDelete;
  CFX_WideString wsPrevText;
};
typedef _FDE_TXTEDT_TEXTCHANGE_INFO FDE_TXTEDT_TEXTCHANGE_INFO;
typedef _FDE_TXTEDT_TEXTCHANGE_INFO* FDE_LPTXTEDT_TEXTCHANGE_INFO;
class IFDE_TxtEdtEventSink {
 public:
  virtual ~IFDE_TxtEdtEventSink() {}
  virtual void On_CaretChanged(IFDE_TxtEdtEngine* pEdit,
                               int32_t nPage,
                               FX_BOOL bVisible = TRUE) = 0;
  virtual void On_TextChanged(IFDE_TxtEdtEngine* pEdit,
                              FDE_TXTEDT_TEXTCHANGE_INFO& ChangeInfo) = 0;
  virtual void On_PageCountChanged(IFDE_TxtEdtEngine* pEdit) = 0;
  virtual void On_SelChanged(IFDE_TxtEdtEngine* pEdit) = 0;
  virtual FX_BOOL On_PageLoad(IFDE_TxtEdtEngine* pEdit,
                              int32_t nPageIndex,
                              int32_t nPurpose) = 0;
  virtual FX_BOOL On_PageUnload(IFDE_TxtEdtEngine* pEdit,
                                int32_t nPageIndex,
                                int32_t nPurpose) = 0;
  virtual FX_BOOL On_PageChange(IFDE_TxtEdtEngine* pEdit,
                                int32_t nPageIndex) = 0;
  virtual void On_AddDoRecord(IFDE_TxtEdtEngine* pEdit,
                              const CFX_ByteStringC& bsDoRecord) = 0;
  virtual FX_BOOL On_ValidateField(IFDE_TxtEdtEngine* pEdit,
                                   int32_t nBlockIndex,
                                   int32_t nFieldIndex,
                                   const CFX_WideString& wsFieldText,
                                   int32_t nCharIndex) = 0;
  virtual FX_BOOL On_ValidateBlock(IFDE_TxtEdtEngine* pEdit,
                                   int32_t nBlockIndex) = 0;
  virtual FX_BOOL On_GetBlockFormatText(IFDE_TxtEdtEngine* pEdit,
                                        int32_t nBlockIndex,
                                        CFX_WideString& wsBlockText) = 0;
  virtual FX_BOOL On_Validate(IFDE_TxtEdtEngine* pEdit,
                              CFX_WideString& wsText) = 0;
};
class IFX_CharIter {
 public:
  virtual ~IFX_CharIter() {}
  virtual void Release() = 0;
  virtual FX_BOOL Next(FX_BOOL bPrev = FALSE) = 0;
  virtual FX_WCHAR GetChar() = 0;
  virtual void SetAt(int32_t nIndex) = 0;
  virtual int32_t GetAt() const = 0;
  virtual FX_BOOL IsEOF(FX_BOOL bTail = TRUE) const = 0;
  virtual IFX_CharIter* Clone() = 0;
};
class IFDE_TxtEdtEngine {
 public:
  static IFDE_TxtEdtEngine* Create();

  virtual ~IFDE_TxtEdtEngine() {}
  virtual void Release() = 0;
  virtual void SetEditParams(const FDE_TXTEDTPARAMS& params) = 0;
  virtual const FDE_TXTEDTPARAMS* GetEditParams() const = 0;

  virtual int32_t CountPages() const = 0;
  virtual IFDE_TxtEdtPage* GetPage(int32_t nIndex) = 0;
  virtual FX_BOOL SetBufChunkSize(int32_t nChunkSize) = 0;
  virtual void SetTextByStream(IFX_Stream* pStream) = 0;
  virtual void SetText(const CFX_WideString& wsText) = 0;
  virtual int32_t GetTextLength() const = 0;
  virtual void GetText(CFX_WideString& wsText,
                       int32_t nStart,
                       int32_t nCount = -1) = 0;
  virtual void ClearText() = 0;

  virtual int32_t GetCaretRect(CFX_RectF& rtCaret) const = 0;
  virtual int32_t GetCaretPos() const = 0;
  virtual int32_t SetCaretPos(int32_t nIndex, FX_BOOL bBefore = TRUE) = 0;
  virtual int32_t MoveCaretPos(FDE_TXTEDTMOVECARET eMoveCaret,
                               FX_BOOL bShift = FALSE,
                               FX_BOOL bCtrl = FALSE) = 0;

  virtual void Lock() = 0;
  virtual void Unlock() = 0;
  virtual FX_BOOL IsLocked() const = 0;

  virtual int32_t Insert(int32_t nStart,
                         const FX_WCHAR* lpText,
                         int32_t nLength) = 0;
  virtual int32_t Delete(int32_t nStart, FX_BOOL bBackspace = FALSE) = 0;
  virtual int32_t DeleteRange(int32_t nStart, int32_t nCount = -1) = 0;
  virtual int32_t Replace(int32_t nStart,
                          int32_t nLength,
                          const CFX_WideString& wsReplace) = 0;
  virtual void SetLimit(int32_t nLimit) = 0;
  virtual void SetAliasChar(FX_WCHAR wAlias) = 0;
  virtual void SetFormatBlock(int32_t nIndex,
                              const CFX_WideString& wsBlockFormat) = 0;
  virtual int32_t CountEditBlocks() const = 0;
  virtual void GetEditBlockText(int32_t nIndex,
                                CFX_WideString& wsBlockText) const = 0;
  virtual int32_t CountEditFields(int32_t nBlockIndex) const = 0;
  virtual void GetEditFieldText(int32_t nBlockIndex,
                                int32_t nFieldIndex,
                                CFX_WideString& wsFieldText) const = 0;
  virtual void StartEdit() = 0;
  virtual void EndEdit() = 0;
  virtual void AddSelRange(int32_t nStart, int32_t nCount = -1) = 0;
  virtual int32_t CountSelRanges() = 0;
  virtual int32_t GetSelRange(int32_t nIndex, int32_t& nStart) = 0;
  virtual void ClearSelection() = 0;

  virtual FX_BOOL Redo(const CFX_ByteStringC& bsRedo) = 0;
  virtual FX_BOOL Undo(const CFX_ByteStringC& bsUndo) = 0;

  virtual int32_t StartLayout() = 0;
  virtual int32_t DoLayout(IFX_Pause* pPause) = 0;
  virtual void EndLayout() = 0;

  virtual FX_BOOL Optimize(IFX_Pause* pPause = NULL) = 0;
  virtual int32_t CountParags() const = 0;
  virtual IFDE_TxtEdtParag* GetParag(int32_t nParagIndex) const = 0;
  virtual IFX_CharIter* CreateCharIter() = 0;
};
class IFDE_TxtEdtParag {
 public:
  virtual ~IFDE_TxtEdtParag() {}
  virtual int32_t GetTextLength() const = 0;
  virtual int32_t GetStartIndex() const = 0;
  virtual int32_t CountLines() const = 0;
  virtual void GetLineRange(int32_t nLineIndex,
                            int32_t& nStart,
                            int32_t& nCount) const = 0;
};
#endif

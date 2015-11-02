// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _FDE_TEXTOUT_IMP
#define _FDE_TEXTOUT_IMP
struct FDE_TTOPIECE {
 public:
  int32_t iStartChar;
  int32_t iChars;
  FX_DWORD dwCharStyles;
  CFX_RectF rtPiece;
};
typedef FDE_TTOPIECE* FDE_LPTTOPIECE;
typedef CFX_MassArrayTemplate<FDE_TTOPIECE> CFDE_TTOPieceArray;
class CFDE_TTOLine : public CFX_Target {
 public:
  CFDE_TTOLine();
  CFDE_TTOLine(const CFDE_TTOLine& ttoLine);
  ~CFDE_TTOLine();
  int32_t AddPiece(int32_t index, const FDE_TTOPIECE& ttoPiece);
  int32_t GetSize() const;
  FDE_LPTTOPIECE GetPtrAt(int32_t index);
  void RemoveLast(int32_t iCount);
  void RemoveAll(FX_BOOL bLeaveMemory);
  FX_BOOL m_bNewReload;
  CFDE_TTOPieceArray m_pieces;

 protected:
  int32_t m_iPieceCount;
};
typedef CFX_ObjectMassArrayTemplate<CFDE_TTOLine> CFDE_TTOLineArray;
class CFDE_TextOut : public IFDE_TextOut, public CFX_Target {
 public:
  CFDE_TextOut();
  ~CFDE_TextOut();
  virtual void Release() { delete this; }
  virtual void SetFont(IFX_Font* pFont);
  virtual void SetFontSize(FX_FLOAT fFontSize);
  virtual void SetTextColor(FX_ARGB color);
  virtual void SetStyles(FX_DWORD dwStyles);
  virtual void SetTabWidth(FX_FLOAT fTabWidth);
  virtual void SetEllipsisString(const CFX_WideString& wsEllipsis);
  virtual void SetParagraphBreakChar(FX_WCHAR wch);
  virtual void SetAlignment(int32_t iAlignment);
  virtual void SetLineSpace(FX_FLOAT fLineSpace);
  virtual void SetDIBitmap(CFX_DIBitmap* pDIB);
  virtual void SetRenderDevice(CFX_RenderDevice* pDevice);
  virtual void SetClipRect(const CFX_Rect& rtClip);
  virtual void SetClipRect(const CFX_RectF& rtClip);
  virtual void SetMatrix(const CFX_Matrix& matrix);
  virtual void SetLineBreakTolerance(FX_FLOAT fTolerance);
  virtual void CalcSize(const FX_WCHAR* pwsStr,
                        int32_t iLength,
                        CFX_Size& size);
  virtual void CalcSize(const FX_WCHAR* pwsStr,
                        int32_t iLength,
                        CFX_SizeF& size);
  virtual void CalcSize(const FX_WCHAR* pwsStr,
                        int32_t iLength,
                        CFX_Rect& rect);
  virtual void CalcSize(const FX_WCHAR* pwsStr,
                        int32_t iLength,
                        CFX_RectF& rect);

  virtual void DrawText(const FX_WCHAR* pwsStr,
                        int32_t iLength,
                        int32_t x,
                        int32_t y);
  virtual void DrawText(const FX_WCHAR* pwsStr,
                        int32_t iLength,
                        FX_FLOAT x,
                        FX_FLOAT y);
  virtual void DrawText(const FX_WCHAR* pwsStr,
                        int32_t iLength,
                        const CFX_Rect& rect);
  virtual void DrawText(const FX_WCHAR* pwsStr,
                        int32_t iLength,
                        const CFX_RectF& rect);

  virtual void SetLogicClipRect(const CFX_RectF& rtClip);
  virtual void CalcLogicSize(const FX_WCHAR* pwsStr,
                             int32_t iLength,
                             CFX_SizeF& size);
  virtual void CalcLogicSize(const FX_WCHAR* pwsStr,
                             int32_t iLength,
                             CFX_RectF& rect);
  virtual void DrawLogicText(const FX_WCHAR* pwsStr,
                             int32_t iLength,
                             FX_FLOAT x,
                             FX_FLOAT y);
  virtual void DrawLogicText(const FX_WCHAR* pwsStr,
                             int32_t iLength,
                             const CFX_RectF& rect);
  virtual int32_t GetTotalLines();

 protected:
  void CalcTextSize(const FX_WCHAR* pwsStr, int32_t iLength, CFX_RectF& rect);
  FX_BOOL RetrieveLineWidth(FX_DWORD dwBreakStatus,
                            FX_FLOAT& fStartPos,
                            FX_FLOAT& fWidth,
                            FX_FLOAT& fHeight);
  void SetLineWidth(CFX_RectF& rect);
  void DrawText(const FX_WCHAR* pwsStr,
                int32_t iLength,
                const CFX_RectF& rect,
                const CFX_RectF& rtClip);
  void LoadText(const FX_WCHAR* pwsStr, int32_t iLength, const CFX_RectF& rect);
  void LoadEllipsis();
  void ExpandBuffer(int32_t iSize, int32_t iType);
  void RetrieveEllPieces(int32_t*& pCharWidths);

  void Reload(const CFX_RectF& rect);
  void ReloadLinePiece(CFDE_TTOLine* pLine, const CFX_RectF& rect);
  FX_BOOL RetriecePieces(FX_DWORD dwBreakStatus,
                         int32_t& iStartChar,
                         int32_t& iPieceWidths,
                         FX_BOOL bReload,
                         const CFX_RectF& rect);
  void AppendPiece(const FDE_TTOPIECE& ttoPiece,
                   FX_BOOL bNeedReload,
                   FX_BOOL bEnd);
  void ReplaceWidthEllipsis();
  void DoAlignment(const CFX_RectF& rect);
  void OnDraw(const CFX_RectF& rtClip);
  int32_t GetDisplayPos(FDE_LPTTOPIECE pPiece);
  int32_t GetCharRects(FDE_LPTTOPIECE pPiece);

  void ToTextRun(const FDE_LPTTOPIECE pPiece, FX_TXTRUN& tr);
  void DrawLine(const FDE_LPTTOPIECE pPiece, IFDE_Pen*& pPen);

  IFX_TxtBreak* m_pTxtBreak;
  IFX_Font* m_pFont;
  FX_FLOAT m_fFontSize;
  FX_FLOAT m_fLineSpace;
  FX_FLOAT m_fLinePos;
  FX_FLOAT m_fTolerance;
  int32_t m_iAlignment;
  int32_t m_iTxtBkAlignment;
  int32_t* m_pCharWidths;
  int32_t m_iChars;
  int32_t* m_pEllCharWidths;
  int32_t m_iEllChars;
  FX_WCHAR m_wParagraphBkChar;
  FX_ARGB m_TxtColor;
  FX_DWORD m_dwStyles;
  FX_DWORD m_dwTxtBkStyles;
  CFX_WideString m_wsEllipsis;
  FX_BOOL m_bElliChanged;
  int32_t m_iEllipsisWidth;
  CFX_WideString m_wsText;
  CFX_RectF m_rtClip;
  CFX_RectF m_rtLogicClip;
  CFX_Matrix m_Matrix;
  CFDE_TTOLineArray m_ttoLines;
  int32_t m_iCurLine;
  int32_t m_iCurPiece;
  int32_t m_iTotalLines;
  FXTEXT_CHARPOS* m_pCharPos;
  int32_t m_iCharPosSize;
  IFDE_RenderDevice* m_pRenderDevice;
  CFX_Int32Array m_hotKeys;
  CFX_RectFArray m_rectArray;
};
#endif

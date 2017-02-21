// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_XFA_FFWIDGET_H_
#define XFA_FXFA_XFA_FFWIDGET_H_

#include <vector>

#include "core/fxcodec/fx_codec_def.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/parser/cxfa_contentlayoutitem.h"

class CXFA_FFPageView;
class CXFA_FFDocView;
class CXFA_FFDoc;
class CXFA_FFApp;
enum class FWL_WidgetHit;

inline FX_FLOAT XFA_UnitPx2Pt(FX_FLOAT fPx, FX_FLOAT fDpi) {
  return fPx * 72.0f / fDpi;
}

#define XFA_FLOAT_PERCISION 0.001f

enum XFA_WIDGETITEM {
  XFA_WIDGETITEM_Parent,
  XFA_WIDGETITEM_FirstChild,
  XFA_WIDGETITEM_NextSibling,
  XFA_WIDGETITEM_PrevSibling,
};

class CXFA_CalcData {
 public:
  CXFA_CalcData();
  ~CXFA_CalcData();

  std::vector<CXFA_WidgetAcc*> m_Globals;
  int32_t m_iRefCount;
};

class CXFA_FFWidget : public CXFA_ContentLayoutItem {
 public:
  explicit CXFA_FFWidget(CXFA_WidgetAcc* pDataAcc);
  ~CXFA_FFWidget() override;

  virtual CFX_RectF GetBBox(uint32_t dwStatus, bool bDrawFocus = false);
  virtual void RenderWidget(CFX_Graphics* pGS,
                            CFX_Matrix* pMatrix,
                            uint32_t dwStatus);
  virtual bool IsLoaded();
  virtual bool LoadWidget();
  virtual void UnloadWidget();
  virtual bool PerformLayout();
  virtual bool UpdateFWLData();
  virtual void UpdateWidgetProperty();
  virtual bool OnMouseEnter();
  virtual bool OnMouseExit();
  virtual bool OnLButtonDown(uint32_t dwFlags, const CFX_PointF& point);
  virtual bool OnLButtonUp(uint32_t dwFlags, const CFX_PointF& point);
  virtual bool OnLButtonDblClk(uint32_t dwFlags, const CFX_PointF& point);
  virtual bool OnMouseMove(uint32_t dwFlags, const CFX_PointF& point);
  virtual bool OnMouseWheel(uint32_t dwFlags,
                            int16_t zDelta,
                            const CFX_PointF& point);
  virtual bool OnRButtonDown(uint32_t dwFlags, const CFX_PointF& point);
  virtual bool OnRButtonUp(uint32_t dwFlags, const CFX_PointF& point);
  virtual bool OnRButtonDblClk(uint32_t dwFlags, const CFX_PointF& point);

  virtual bool OnSetFocus(CXFA_FFWidget* pOldWidget);
  virtual bool OnKillFocus(CXFA_FFWidget* pNewWidget);
  virtual bool OnKeyDown(uint32_t dwKeyCode, uint32_t dwFlags);
  virtual bool OnKeyUp(uint32_t dwKeyCode, uint32_t dwFlags);
  virtual bool OnChar(uint32_t dwChar, uint32_t dwFlags);
  virtual FWL_WidgetHit OnHitTest(const CFX_PointF& point);
  virtual bool OnSetCursor(const CFX_PointF& point);
  virtual bool CanUndo();
  virtual bool CanRedo();
  virtual bool Undo();
  virtual bool Redo();
  virtual bool CanCopy();
  virtual bool CanCut();
  virtual bool CanPaste();
  virtual bool CanSelectAll();
  virtual bool CanDelete();
  virtual bool CanDeSelect();
  virtual bool Copy(CFX_WideString& wsCopy);
  virtual bool Cut(CFX_WideString& wsCut);
  virtual bool Paste(const CFX_WideString& wsPaste);
  virtual void SelectAll();
  virtual void Delete();
  virtual void DeSelect();
  virtual bool GetSuggestWords(CFX_PointF pointf,
                               std::vector<CFX_ByteString>& sSuggest);
  virtual bool ReplaceSpellCheckWord(CFX_PointF pointf,
                                     const CFX_ByteStringC& bsReplace);

  CXFA_FFPageView* GetPageView() const { return m_pPageView; }
  void SetPageView(CXFA_FFPageView* pPageView) { m_pPageView = pPageView; }
  CFX_RectF GetWidgetRect();
  CFX_RectF RecacheWidgetRect();
  uint32_t GetStatus();
  void ModifyStatus(uint32_t dwAdded, uint32_t dwRemoved);

  CXFA_WidgetAcc* GetDataAcc();
  bool GetToolTip(CFX_WideString& wsToolTip);

  CXFA_FFDocView* GetDocView();
  void SetDocView(CXFA_FFDocView* pDocView);
  CXFA_FFDoc* GetDoc();
  CXFA_FFApp* GetApp();
  IXFA_AppProvider* GetAppProvider();
  void InvalidateWidget(const CFX_RectF* pRect = nullptr);
  void AddInvalidateRect(const CFX_RectF* pRect = nullptr);
  bool GetCaptionText(CFX_WideString& wsCap);
  bool IsFocused();
  CFX_PointF Rotate2Normal(const CFX_PointF& point);
  CFX_Matrix GetRotateMatrix();
  bool IsLayoutRectEmpty();
  CXFA_FFWidget* GetParent();
  bool IsAncestorOf(CXFA_FFWidget* pWidget);
  const CFWL_App* GetFWLApp();

 protected:
  virtual bool PtInActiveRect(const CFX_PointF& point);

  void DrawBorder(CFX_Graphics* pGS,
                  CXFA_Box box,
                  const CFX_RectF& rtBorder,
                  CFX_Matrix* pMatrix,
                  uint32_t dwFlags = 0);

  CFX_RectF GetRectWithoutRotate();
  bool IsMatchVisibleStatus(uint32_t dwStatus);
  void EventKillFocus();
  bool IsButtonDown();
  void SetButtonDown(bool bSet);

  CXFA_FFDocView* m_pDocView;
  CXFA_FFPageView* m_pPageView;
  CXFA_WidgetAcc* m_pDataAcc;
  CFX_RectF m_rtWidget;
};

int32_t XFA_StrokeTypeSetLineDash(CFX_Graphics* pGraphics,
                                  int32_t iStrokeType,
                                  int32_t iCapType);
CFX_GraphStateData::LineCap XFA_LineCapToFXGE(int32_t iLineCap);
void XFA_DrawImage(CFX_Graphics* pGS,
                   const CFX_RectF& rtImage,
                   CFX_Matrix* pMatrix,
                   CFX_DIBitmap* pDIBitmap,
                   int32_t iAspect,
                   int32_t iImageXDpi,
                   int32_t iImageYDpi,
                   int32_t iHorzAlign = XFA_ATTRIBUTEENUM_Left,
                   int32_t iVertAlign = XFA_ATTRIBUTEENUM_Top);

CFX_DIBitmap* XFA_LoadImageData(CXFA_FFDoc* pDoc,
                                CXFA_Image* pImage,
                                bool& bNameImage,
                                int32_t& iImageXDpi,
                                int32_t& iImageYDpi);

CFX_DIBitmap* XFA_LoadImageFromBuffer(
    const CFX_RetainPtr<IFX_SeekableReadStream>& pImageFileRead,
    FXCODEC_IMAGE_TYPE type,
    int32_t& iImageXDpi,
    int32_t& iImageYDpi);

FXCODEC_IMAGE_TYPE XFA_GetImageType(const CFX_WideString& wsType);
FX_CHAR* XFA_Base64Encode(const uint8_t* buf, int32_t buf_len);
void XFA_RectWidthoutMargin(CFX_RectF& rt,
                            const CXFA_Margin& mg,
                            bool bUI = false);
CXFA_FFWidget* XFA_GetWidgetFromLayoutItem(CXFA_LayoutItem* pLayoutItem);
bool XFA_IsCreateWidget(XFA_Element iType);
#define XFA_DRAWBOX_ForceRound 1
#define XFA_DRAWBOX_Lowered3D 2
void XFA_DrawBox(CXFA_Box box,
                 CFX_Graphics* pGS,
                 const CFX_RectF& rtWidget,
                 CFX_Matrix* pMatrix,
                 uint32_t dwFlags = 0);

#endif  // XFA_FXFA_XFA_FFWIDGET_H_

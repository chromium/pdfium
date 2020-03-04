// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFWIDGET_H_
#define XFA_FXFA_CXFA_FFWIDGET_H_

#include <vector>

#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffpageview.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"

class CFX_DIBitmap;
class CXFA_Box;
class CXFA_FFApp;
class CXFA_FFDoc;
class CXFA_FFDocView;
class CXFA_FFField;
class CXFA_FFPageView;
class CXFA_FFWidgetHandler;
class CXFA_Graphics;
class CXFA_Image;
class CXFA_Margin;
enum class FWL_WidgetHit;

inline float XFA_UnitPx2Pt(float fPx, float fDpi) {
  return fPx * 72.0f / fDpi;
}

constexpr float kXFAWidgetPrecision = 0.001f;

void XFA_DrawImage(CXFA_Graphics* pGS,
                   const CFX_RectF& rtImage,
                   const CFX_Matrix& matrix,
                   const RetainPtr<CFX_DIBitmap>& pDIBitmap,
                   XFA_AttributeValue iAspect,
                   const CFX_Size& dpi,
                   XFA_AttributeValue iHorzAlign = XFA_AttributeValue::Left,
                   XFA_AttributeValue iVertAlign = XFA_AttributeValue::Top);

RetainPtr<CFX_DIBitmap> XFA_LoadImageFromBuffer(
    const RetainPtr<IFX_SeekableReadStream>& pImageFileRead,
    FXCODEC_IMAGE_TYPE type,
    int32_t& iImageXDpi,
    int32_t& iImageYDpi);

void XFA_RectWithoutMargin(CFX_RectF* rt, const CXFA_Margin* margin);
CXFA_FFWidget* XFA_GetWidgetFromLayoutItem(CXFA_LayoutItem* pLayoutItem);

class CXFA_CalcData {
 public:
  CXFA_CalcData();
  ~CXFA_CalcData();

  std::vector<CXFA_Node*> m_Globals;
};

class CXFA_FFWidget : public Observable, public CFWL_Widget::AdapterIface {
 public:
  enum FocusOption { kDoNotDrawFocus = 0, kDrawFocus };
  enum HighlightOption { kNoHighlight = 0, kHighlight };

  explicit CXFA_FFWidget(CXFA_Node* pNode);
  ~CXFA_FFWidget() override;

  // CFWL_Widget::AdapterIface:
  CFX_Matrix GetRotateMatrix() override;
  void DisplayCaret(bool bVisible, const CFX_RectF* pRtAnchor) override;
  void GetBorderColorAndThickness(FX_ARGB* cr, float* fWidth) override;

  virtual CXFA_FFField* AsField();

  virtual CFX_RectF GetBBox(FocusOption focus);
  virtual void RenderWidget(CXFA_Graphics* pGS,
                            const CFX_Matrix& matrix,
                            HighlightOption highlight);
  virtual bool IsLoaded();
  virtual bool LoadWidget();
  virtual bool PerformLayout();
  virtual bool UpdateFWLData();
  virtual void UpdateWidgetProperty();
  // |command| must be LeftButtonDown or RightButtonDown.
  virtual bool AcceptsFocusOnButtonDown(uint32_t dwFlags,
                                        const CFX_PointF& point,
                                        FWL_MouseCommand command);

  // Caution: Returning false from an On* method may mean |this| is destroyed.
  virtual bool OnMouseEnter() WARN_UNUSED_RESULT;
  virtual bool OnMouseExit() WARN_UNUSED_RESULT;
  virtual bool OnLButtonDown(uint32_t dwFlags,
                             const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnLButtonUp(uint32_t dwFlags,
                           const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnLButtonDblClk(uint32_t dwFlags,
                               const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnMouseMove(uint32_t dwFlags,
                           const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnMouseWheel(uint32_t dwFlags,
                            int16_t zDelta,
                            const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnRButtonDown(uint32_t dwFlags,
                             const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnRButtonUp(uint32_t dwFlags,
                           const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnRButtonDblClk(uint32_t dwFlags,
                               const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnSetFocus(CXFA_FFWidget* pOldWidget) WARN_UNUSED_RESULT;
  virtual bool OnKillFocus(CXFA_FFWidget* pNewWidget) WARN_UNUSED_RESULT;
  virtual bool OnKeyDown(uint32_t dwKeyCode,
                         uint32_t dwFlags) WARN_UNUSED_RESULT;
  virtual bool OnKeyUp(uint32_t dwKeyCode, uint32_t dwFlags) WARN_UNUSED_RESULT;
  virtual bool OnChar(uint32_t dwChar, uint32_t dwFlags) WARN_UNUSED_RESULT;

  virtual FWL_WidgetHit HitTest(const CFX_PointF& point);
  virtual bool CanUndo();
  virtual bool CanRedo();
  virtual bool CanCopy();
  virtual bool CanCut();
  virtual bool CanPaste();
  virtual bool CanSelectAll();
  virtual bool CanDelete();
  virtual bool CanDeSelect();
  virtual bool Undo();
  virtual bool Redo();
  virtual Optional<WideString> Copy();
  virtual Optional<WideString> Cut();
  virtual bool Paste(const WideString& wsPaste);
  virtual void SelectAll();
  virtual void Delete();
  virtual void DeSelect();
  virtual WideString GetText();
  virtual FormFieldType GetFormFieldType();

  CXFA_Node* GetNode() const { return m_pNode.Get(); }
  CXFA_ContentLayoutItem* GetLayoutItem() const { return m_pLayoutItem.Get(); }
  void SetLayoutItem(CXFA_ContentLayoutItem* pItem) { m_pLayoutItem = pItem; }
  CXFA_FFPageView* GetPageView() const { return m_pPageView.Get(); }
  void SetPageView(CXFA_FFPageView* pPageView) { m_pPageView.Reset(pPageView); }
  CXFA_FFDocView* GetDocView() const { return m_pDocView.Get(); }
  void SetDocView(CXFA_FFDocView* pDocView) { m_pDocView = pDocView; }

  CXFA_FFWidget* GetNextFFWidget() const;
  const CFX_RectF& GetWidgetRect() const;
  const CFX_RectF& RecacheWidgetRect() const;
  void ModifyStatus(uint32_t dwAdded, uint32_t dwRemoved);

  CXFA_FFDoc* GetDoc();
  CXFA_FFApp* GetApp();
  IXFA_AppProvider* GetAppProvider();
  void InvalidateRect();
  bool IsFocused() const {
    return GetLayoutItem()->TestStatusBits(XFA_WidgetStatus_Focused);
  }
  CFX_PointF Rotate2Normal(const CFX_PointF& point);
  bool IsLayoutRectEmpty();
  CXFA_LayoutItem* GetParent();
  bool IsAncestorOf(CXFA_FFWidget* pWidget);
  const CFWL_App* GetFWLApp();

  bool HasEventUnderHandler(XFA_EVENTTYPE eEventType,
                            CXFA_FFWidgetHandler* pHandler);
  bool ProcessEventUnderHandler(CXFA_EventParam* params,
                                CXFA_FFWidgetHandler* pHandler);

 protected:
  virtual bool PtInActiveRect(const CFX_PointF& point);

  void DrawBorder(CXFA_Graphics* pGS,
                  CXFA_Box* box,
                  const CFX_RectF& rtBorder,
                  const CFX_Matrix& matrix);
  void DrawBorderWithFlag(CXFA_Graphics* pGS,
                          CXFA_Box* box,
                          const CFX_RectF& rtBorder,
                          const CFX_Matrix& matrix,
                          bool forceRound);

  CFX_RectF GetRectWithoutRotate();
  bool HasVisibleStatus() const;
  void EventKillFocus();
  bool IsButtonDown();
  void SetButtonDown(bool bSet);

  UnownedPtr<CXFA_ContentLayoutItem> m_pLayoutItem;
  UnownedPtr<CXFA_FFDocView> m_pDocView;
  ObservedPtr<CXFA_FFPageView> m_pPageView;
  UnownedPtr<CXFA_Node> const m_pNode;
  mutable CFX_RectF m_rtWidget;
};

inline CXFA_FFField* ToField(CXFA_FFWidget* widget) {
  return widget ? widget->AsField() : nullptr;
}

#endif  // XFA_FXFA_CXFA_FFWIDGET_H_

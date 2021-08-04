// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FFWIDGET_H_
#define XFA_FXFA_CXFA_FFWIDGET_H_

#include <stdint.h>

#include "core/fpdfdoc/cpdf_formfield.h"
#include "core/fxcodec/fx_codec_def.h"
#include "core/fxcrt/retain_ptr.h"
#include "core/fxge/cfx_graphstatedata.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/prefinalizer.h"
#include "v8/include/cppgc/visitor.h"
#include "xfa/fwl/cfwl_app.h"
#include "xfa/fwl/cfwl_messagemouse.h"
#include "xfa/fwl/cfwl_widget.h"
#include "xfa/fwl/fwl_widgetdef.h"
#include "xfa/fxfa/cxfa_eventparam.h"
#include "xfa/fxfa/cxfa_ffapp.h"
#include "xfa/fxfa/fxfa.h"
#include "xfa/fxfa/layout/cxfa_contentlayoutitem.h"

class CFGAS_GEGraphics;
class CFX_DIBitmap;
class CXFA_Box;
class CXFA_FFApp;
class CXFA_FFDoc;
class CXFA_FFDocView;
class CXFA_FFField;
class CXFA_FFPageView;
class CXFA_FFWidgetHandler;
class CXFA_Margin;
class IFX_SeekableReadStream;
enum class FWL_WidgetHit;

inline float XFA_UnitPx2Pt(float fPx, float fDpi) {
  return fPx * 72.0f / fDpi;
}

constexpr float kXFAWidgetPrecision = 0.001f;

void XFA_DrawImage(CFGAS_GEGraphics* pGS,
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

class CXFA_FFWidget : public cppgc::GarbageCollected<CXFA_FFWidget>,
                      public CFWL_Widget::AdapterIface {
  CPPGC_USING_PRE_FINALIZER(CXFA_FFWidget, PreFinalize);

 public:
  enum FocusOption { kDoNotDrawFocus = 0, kDrawFocus };
  enum HighlightOption { kNoHighlight = 0, kHighlight };

  class IteratorIface {
   public:
    virtual ~IteratorIface() = default;

    virtual CXFA_FFWidget* MoveToFirst() = 0;
    virtual CXFA_FFWidget* MoveToLast() = 0;
    virtual CXFA_FFWidget* MoveToNext() = 0;
    virtual CXFA_FFWidget* MoveToPrevious() = 0;
    virtual CXFA_FFWidget* GetCurrentWidget() = 0;
    virtual bool SetCurrentWidget(CXFA_FFWidget* hWidget) = 0;
  };

  static CXFA_FFWidget* FromLayoutItem(CXFA_LayoutItem* pLayoutItem);

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FFWidget() override;

  virtual void PreFinalize();
  void Trace(cppgc::Visitor* visitor) const override;

  // CFWL_Widget::AdapterIface:
  CFX_Matrix GetRotateMatrix() override;
  void DisplayCaret(bool bVisible, const CFX_RectF* pRtAnchor) override;
  void GetBorderColorAndThickness(FX_ARGB* cr, float* fWidth) override;

  virtual CXFA_FFField* AsField();
  virtual CFX_RectF GetBBox(FocusOption focus);
  virtual void RenderWidget(CFGAS_GEGraphics* pGS,
                            const CFX_Matrix& matrix,
                            HighlightOption highlight);
  virtual bool IsLoaded();
  virtual bool LoadWidget();
  virtual bool PerformLayout();
  virtual bool UpdateFWLData();
  virtual void UpdateWidgetProperty();
  // |command| must be LeftButtonDown or RightButtonDown.
  virtual bool AcceptsFocusOnButtonDown(
      uint32_t dwFlags,
      const CFX_PointF& point,
      CFWL_MessageMouse::MouseCommand command);

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
                            const CFX_PointF& point,
                            const CFX_Vector& delta) WARN_UNUSED_RESULT;
  virtual bool OnRButtonDown(uint32_t dwFlags,
                             const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnRButtonUp(uint32_t dwFlags,
                           const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnRButtonDblClk(uint32_t dwFlags,
                               const CFX_PointF& point) WARN_UNUSED_RESULT;
  virtual bool OnSetFocus(CXFA_FFWidget* pOldWidget) WARN_UNUSED_RESULT;
  virtual bool OnKillFocus(CXFA_FFWidget* pNewWidget) WARN_UNUSED_RESULT;
  virtual bool OnKeyDown(XFA_FWL_VKEYCODE dwKeyCode,
                         uint32_t dwFlags) WARN_UNUSED_RESULT;
  virtual bool OnKeyUp(XFA_FWL_VKEYCODE dwKeyCode,
                       uint32_t dwFlags) WARN_UNUSED_RESULT;
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
  CXFA_FFPageView* GetPageView() const { return m_pPageView; }
  void SetPageView(CXFA_FFPageView* pPageView) { m_pPageView = pPageView; }
  CXFA_FFDocView* GetDocView() const { return m_pDocView; }
  void SetDocView(CXFA_FFDocView* pDocView) { m_pDocView = pDocView; }

  CXFA_FFWidget* GetNextFFWidget() const;
  const CFX_RectF& GetWidgetRect() const;
  const CFX_RectF& RecacheWidgetRect() const;
  void ModifyStatus(XFA_WidgetStatusMask dwAdded,
                    XFA_WidgetStatusMask dwRemoved);

  CXFA_FFDoc* GetDoc();
  CXFA_FFApp* GetApp();
  CXFA_FFApp::CallbackIface* GetAppProvider();
  CFWL_App* GetFWLApp() const;
  void InvalidateRect();
  bool IsFocused() const {
    return GetLayoutItem()->TestStatusBits(XFA_WidgetStatus_Focused);
  }
  CFX_PointF Rotate2Normal(const CFX_PointF& point);
  bool IsLayoutRectEmpty();
  CXFA_LayoutItem* GetParent();
  bool IsAncestorOf(CXFA_FFWidget* pWidget);
  bool HasEventUnderHandler(XFA_EVENTTYPE eEventType,
                            CXFA_FFWidgetHandler* pHandler);
  bool ProcessEventUnderHandler(CXFA_EventParam* params,
                                CXFA_FFWidgetHandler* pHandler);

 protected:
  explicit CXFA_FFWidget(CXFA_Node* pNode);
  virtual bool PtInActiveRect(const CFX_PointF& point);

  void DrawBorder(CFGAS_GEGraphics* pGS,
                  CXFA_Box* box,
                  const CFX_RectF& rtBorder,
                  const CFX_Matrix& matrix);
  void DrawBorderWithFlag(CFGAS_GEGraphics* pGS,
                          CXFA_Box* box,
                          const CFX_RectF& rtBorder,
                          const CFX_Matrix& matrix,
                          bool forceRound);

  CFX_RectF GetRectWithoutRotate();
  bool HasVisibleStatus() const;
  void EventKillFocus();
  bool IsButtonDown();
  void SetButtonDown(bool bSet);

  cppgc::Member<CXFA_ContentLayoutItem> m_pLayoutItem;
  cppgc::Member<CXFA_FFDocView> m_pDocView;
  cppgc::Member<CXFA_FFPageView> m_pPageView;
  cppgc::Member<CXFA_Node> const m_pNode;
  mutable CFX_RectF m_WidgetRect;
};

inline CXFA_FFField* ToField(CXFA_FFWidget* widget) {
  return widget ? widget->AsField() : nullptr;
}

#endif  // XFA_FXFA_CXFA_FFWIDGET_H_

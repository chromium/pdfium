// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FPDFSDK_PWL_CPWL_WND_H_
#define FPDFSDK_PWL_CPWL_WND_H_

#include <memory>
#include <vector>

#include "core/fxcrt/cfx_timer.h"
#include "core/fxcrt/observed_ptr.h"
#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "core/fxge/cfx_color.h"
#include "core/fxge/cfx_renderdevice.h"
#include "fpdfsdk/pwl/ipwl_systemhandler.h"

class CPWL_Edit;
class CPWL_MsgControl;
class CPWL_ScrollBar;
class IPVT_FontMap;
struct PWL_SCROLL_INFO;

// window styles
#define PWS_BORDER 0x40000000L
#define PWS_BACKGROUND 0x20000000L
#define PWS_VSCROLL 0x08000000L
#define PWS_VISIBLE 0x04000000L
#define PWS_READONLY 0x01000000L
#define PWS_AUTOFONTSIZE 0x00800000L
#define PWS_AUTOTRANSPARENT 0x00400000L
#define PWS_NOREFRESHCLIP 0x00200000L

// edit and label styles
#define PES_MULTILINE 0x0001L
#define PES_PASSWORD 0x0002L
#define PES_LEFT 0x0004L
#define PES_RIGHT 0x0008L
#define PES_MIDDLE 0x0010L
#define PES_TOP 0x0020L
#define PES_CENTER 0x0080L
#define PES_CHARARRAY 0x0100L
#define PES_AUTOSCROLL 0x0200L
#define PES_AUTORETURN 0x0400L
#define PES_UNDO 0x0800L
#define PES_RICH 0x1000L
#define PES_TEXTOVERFLOW 0x4000L

// listbox styles
#define PLBS_MULTIPLESEL 0x0001L
#define PLBS_HOVERSEL 0x0008L

// combobox styles
#define PCBS_ALLOWCUSTOMTEXT 0x0001L

struct CPWL_Dash {
  CPWL_Dash() : nDash(0), nGap(0), nPhase(0) {}
  CPWL_Dash(int32_t dash, int32_t gap, int32_t phase)
      : nDash(dash), nGap(gap), nPhase(phase) {}

  void Reset() {
    nDash = 0;
    nGap = 0;
    nPhase = 0;
  }

  int32_t nDash;
  int32_t nGap;
  int32_t nPhase;
};

class CPWL_Wnd : public Observable {
 public:
  static const CFX_Color kDefaultBlackColor;
  static const CFX_Color kDefaultWhiteColor;

  class ProviderIface : public Observable {
   public:
    virtual ~ProviderIface() = default;

    // get a matrix which map user space to CWnd client space
    virtual CFX_Matrix GetWindowMatrix(
        const IPWL_SystemHandler::PerWindowData* pAttached) = 0;
  };

  class FocusHandlerIface {
   public:
    virtual ~FocusHandlerIface() = default;
    virtual void OnSetFocus(CPWL_Edit* pEdit) = 0;
  };

  // Caller-provided options for window creation.
  class CreateParams {
   public:
    CreateParams();
    CreateParams(const CreateParams& other);
    ~CreateParams();

    // Required:
    CFX_FloatRect rcRectWnd;
    ObservedPtr<CFX_Timer::HandlerIface> pTimerHandler;
    UnownedPtr<IPWL_SystemHandler> pSystemHandler;
    UnownedPtr<IPVT_FontMap> pFontMap;
    ObservedPtr<ProviderIface> pProvider;

    // Optional:
    UnownedPtr<FocusHandlerIface> pFocusHandler;
    uint32_t dwFlags = 0;
    CFX_Color sBackgroundColor;
    BorderStyle nBorderStyle = BorderStyle::kSolid;
    int32_t dwBorderWidth = 1;
    CFX_Color sBorderColor;
    CFX_Color sTextColor;
    int32_t nTransparency = 255;
    float fFontSize;
    CPWL_Dash sDash;

    // Ignore, used internally only:
    CPWL_MsgControl* pMsgControl = nullptr;
    IPWL_SystemHandler::CursorStyle eCursorType =
        IPWL_SystemHandler::CursorStyle::kArrow;
  };

  static bool IsSHIFTKeyDown(uint32_t nFlag);
  static bool IsCTRLKeyDown(uint32_t nFlag);
  static bool IsALTKeyDown(uint32_t nFlag);
  static bool IsMETAKeyDown(uint32_t nFlag);

  // Selects between IsCTRLKeyDown() and IsMETAKeyDown() depending on platform.
  static bool IsPlatformShortcutKey(uint32_t nFlag);

  CPWL_Wnd(const CreateParams& cp,
           std::unique_ptr<IPWL_SystemHandler::PerWindowData> pAttachedData);
  virtual ~CPWL_Wnd();

  // Returns |true| iff this instance is still allocated.
  virtual bool InvalidateRect(const CFX_FloatRect* pRect);

  virtual bool OnKeyDown(uint16_t nChar, uint32_t nFlag);
  virtual bool OnChar(uint16_t nChar, uint32_t nFlag);
  virtual bool OnLButtonDblClk(uint32_t nFlag, const CFX_PointF& point);
  virtual bool OnLButtonDown(uint32_t nFlag, const CFX_PointF& point);
  virtual bool OnLButtonUp(uint32_t nFlag, const CFX_PointF& point);
  virtual bool OnRButtonDown(uint32_t nFlag, const CFX_PointF& point);
  virtual bool OnRButtonUp(uint32_t nFlag, const CFX_PointF& point);
  virtual bool OnMouseMove(uint32_t nFlag, const CFX_PointF& point);
  virtual bool OnMouseWheel(uint32_t nFlag,
                            const CFX_PointF& point,
                            const CFX_Vector& delta);
  virtual void SetScrollInfo(const PWL_SCROLL_INFO& info);
  virtual void SetScrollPosition(float pos);
  virtual void ScrollWindowVertically(float pos);
  virtual void NotifyLButtonDown(CPWL_Wnd* child, const CFX_PointF& pos);
  virtual void NotifyLButtonUp(CPWL_Wnd* child, const CFX_PointF& pos);
  virtual void NotifyMouseMove(CPWL_Wnd* child, const CFX_PointF& pos);
  virtual void SetFocus();
  virtual void KillFocus();
  virtual void SetCursor();

  // Returns |true| iff this instance is still allocated.
  virtual bool SetVisible(bool bVisible);
  virtual void SetFontSize(float fFontSize);
  virtual float GetFontSize() const;

  virtual WideString GetText();
  virtual WideString GetSelectedText();
  virtual void ReplaceSelection(const WideString& text);
  virtual bool SelectAllText();

  virtual bool CanUndo();
  virtual bool CanRedo();
  virtual bool Undo();
  virtual bool Redo();

  virtual CFX_FloatRect GetFocusRect() const;
  virtual CFX_FloatRect GetClientRect() const;

  void AddChild(std::unique_ptr<CPWL_Wnd> pWnd);
  void RemoveChild(CPWL_Wnd* pWnd);
  void Realize();
  void Destroy();
  bool Move(const CFX_FloatRect& rcNew, bool bReset, bool bRefresh);

  void InvalidateFocusHandler(FocusHandlerIface* handler);
  void InvalidateProvider(ProviderIface* provider);
  void SetCapture();
  void ReleaseCapture();
  void DrawAppearance(CFX_RenderDevice* pDevice,
                      const CFX_Matrix& mtUser2Device);

  CFX_Color GetBackgroundColor() const;
  void SetBackgroundColor(const CFX_Color& color);
  CFX_Color GetBorderColor() const;
  CFX_Color GetTextColor() const;
  void SetTextColor(const CFX_Color& color);
  CFX_Color GetBorderLeftTopColor(BorderStyle nBorderStyle) const;
  CFX_Color GetBorderRightBottomColor(BorderStyle nBorderStyle) const;
  BorderStyle GetBorderStyle() const;
  const CPWL_Dash& GetBorderDash() const;

  int32_t GetBorderWidth() const;
  int32_t GetInnerBorderWidth() const;
  CFX_FloatRect GetWindowRect() const;
  CFX_PointF GetCenterPoint() const;

  bool IsVisible() const { return m_bVisible; }
  bool HasFlag(uint32_t dwFlags) const;
  void AddFlag(uint32_t dwFlags);
  void RemoveFlag(uint32_t dwFlags);

  void SetClipRect(const CFX_FloatRect& rect);
  const CFX_FloatRect& GetClipRect() const;

  CPWL_Wnd* GetParentWindow() const { return m_pParent.Get(); }
  const IPWL_SystemHandler::PerWindowData* GetAttachedData() const {
    return m_pAttachedData.get();
  }
  std::unique_ptr<IPWL_SystemHandler::PerWindowData> CloneAttachedData() const;

  bool WndHitTest(const CFX_PointF& point) const;
  bool ClientHitTest(const CFX_PointF& point) const;
  bool IsCaptureMouse() const;

  const CPWL_Wnd* GetFocused() const;
  bool IsFocused() const;
  bool IsReadOnly() const;
  CPWL_ScrollBar* GetVScrollBar() const;

  IPVT_FontMap* GetFontMap() const { return m_CreationParams.pFontMap.Get(); }
  ProviderIface* GetProvider() const {
    return m_CreationParams.pProvider.Get();
  }
  FocusHandlerIface* GetFocusHandler() const {
    return m_CreationParams.pFocusHandler.Get();
  }

  int32_t GetTransparency();
  void SetTransparency(int32_t nTransparency);

  CFX_Matrix GetWindowMatrix() const;

  virtual void OnSetFocus();
  virtual void OnKillFocus();

 protected:
  virtual void CreateChildWnd(const CreateParams& cp);

  // Returns |true| iff this instance is still allocated.
  virtual bool RePosChildWnd();

  virtual void DrawThisAppearance(CFX_RenderDevice* pDevice,
                                  const CFX_Matrix& mtUser2Device);

  virtual void OnCreated();
  virtual void OnDestroy();

  bool IsNotifying() const { return m_bNotifying; }
  bool IsValid() const { return m_bCreated; }
  CreateParams* GetCreationParams() { return &m_CreationParams; }
  CFX_Timer::HandlerIface* GetTimerHandler() const {
    return m_CreationParams.pTimerHandler.Get();
  }
  IPWL_SystemHandler* GetSystemHandler() const {
    return m_CreationParams.pSystemHandler.Get();
  }

  // Returns |true| iff this instance is still allocated.
  bool InvalidateRectMove(const CFX_FloatRect& rcOld,
                          const CFX_FloatRect& rcNew);

  bool IsWndCaptureMouse(const CPWL_Wnd* pWnd) const;
  bool IsWndCaptureKeyboard(const CPWL_Wnd* pWnd) const;

 private:
  void DrawChildAppearance(CFX_RenderDevice* pDevice,
                           const CFX_Matrix& mtUser2Device);

  CFX_FloatRect PWLtoWnd(const CFX_FloatRect& rect) const;

  void CreateScrollBar(const CreateParams& cp);
  void CreateVScrollBar(const CreateParams& cp);

  void AdjustStyle();
  void CreateMsgControl();
  void DestroyMsgControl();

  CPWL_MsgControl* GetMsgControl() const;

  CreateParams m_CreationParams;
  std::unique_ptr<IPWL_SystemHandler::PerWindowData> m_pAttachedData;
  UnownedPtr<CPWL_Wnd> m_pParent;
  std::vector<std::unique_ptr<CPWL_Wnd>> m_Children;
  UnownedPtr<CPWL_ScrollBar> m_pVScrollBar;
  CFX_FloatRect m_rcWindow;
  CFX_FloatRect m_rcClip;
  bool m_bCreated = false;
  bool m_bVisible = false;
  bool m_bNotifying = false;
};

#endif  // FPDFSDK_PWL_CPWL_WND_H_

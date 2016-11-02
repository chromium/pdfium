// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CORE_IFWL_WIDGET_H_
#define XFA_FWL_CORE_IFWL_WIDGET_H_

#include <memory>

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_system.h"
#include "xfa/fwl/core/cfwl_event.h"
#include "xfa/fwl/core/cfwl_themepart.h"
#include "xfa/fwl/core/fwl_widgethit.h"
#include "xfa/fwl/core/ifwl_widgetdelegate.h"
#include "xfa/fwl/theme/cfwl_widgettp.h"

// FWL contains two parallel inheritance hierarchies, which reference each
// other via pointers as follows:
//
//                  m_pAssociate
//                  <----------
//      CFWL_Widget ----------> IFWL_Widget
//           |       m_pIface        |
//           A                       A
//           |                       |
//      CFWL_...                IFWL_...
//
// TODO(tsepez): Collapse these into a single hierarchy.
//

enum class FWL_Type {
  Unknown = 0,

  Barcode,
  Caret,
  CheckBox,
  ComboBox,
  DateTimePicker,
  Edit,
  Form,
  FormProxy,
  ListBox,
  MonthCalendar,
  PictureBox,
  PushButton,
  ScrollBar,
  SpinButton,
  ToolTip
};

class CFWL_AppImp;
class CFWL_MsgKey;
class CFWL_Widget;
class CFWL_WidgetImpProperties;
class CFWL_WidgetMgr;
class IFWL_App;
class IFWL_DataProvider;
class IFWL_ThemeProvider;
class IFWL_Widget;
enum class FWL_Type;

class IFWL_Widget : public IFWL_WidgetDelegate {
 public:
  ~IFWL_Widget() override;

  virtual FWL_Type GetClassID() const = 0;
  virtual bool IsInstance(const CFX_WideStringC& wsClass) const;

  virtual FWL_Error GetWidgetRect(CFX_RectF& rect, bool bAutoSize = false);
  virtual FWL_Error GetGlobalRect(CFX_RectF& rect);
  virtual FWL_Error SetWidgetRect(const CFX_RectF& rect);
  virtual FWL_Error GetClientRect(CFX_RectF& rect);

  virtual IFWL_Widget* GetParent();
  virtual FWL_Error SetParent(IFWL_Widget* pParent);

  virtual IFWL_Widget* GetOwner();
  virtual FWL_Error SetOwner(IFWL_Widget* pOwner);

  virtual IFWL_Widget* GetOuter();

  virtual uint32_t GetStyles();
  virtual FWL_Error ModifyStyles(uint32_t dwStylesAdded,
                                 uint32_t dwStylesRemoved);
  virtual uint32_t GetStylesEx();
  virtual FWL_Error ModifyStylesEx(uint32_t dwStylesExAdded,
                                   uint32_t dwStylesExRemoved);

  virtual uint32_t GetStates();
  virtual void SetStates(uint32_t dwStates, bool bSet = true);

  virtual FWL_Error Update();
  virtual FWL_Error LockUpdate();
  virtual FWL_Error UnlockUpdate();

  virtual FWL_WidgetHit HitTest(FX_FLOAT fx, FX_FLOAT fy);

  virtual FWL_Error TransformTo(IFWL_Widget* pWidget,
                                FX_FLOAT& fx,
                                FX_FLOAT& fy);
  virtual FWL_Error TransformTo(IFWL_Widget* pWidget, CFX_RectF& rt);

  virtual FWL_Error GetMatrix(CFX_Matrix& matrix, bool bGlobal = false);
  virtual FWL_Error SetMatrix(const CFX_Matrix& matrix);

  virtual FWL_Error DrawWidget(CFX_Graphics* pGraphics,
                               const CFX_Matrix* pMatrix = nullptr);

  virtual IFWL_ThemeProvider* GetThemeProvider();
  virtual FWL_Error SetThemeProvider(IFWL_ThemeProvider* pThemeProvider);

  void SetDelegate(IFWL_WidgetDelegate* delegate) { m_pDelegate = delegate; }
  IFWL_WidgetDelegate* GetDelegate() {
    return m_pDelegate ? m_pDelegate : this;
  }
  const IFWL_WidgetDelegate* GetDelegate() const {
    return m_pDelegate ? m_pDelegate : this;
  }

  // IFWL_WidgetDelegate.
  void OnProcessMessage(CFWL_Message* pMessage) override;
  void OnProcessEvent(CFWL_Event* pEvent) override;
  void OnDrawWidget(CFX_Graphics* pGraphics,
                    const CFX_Matrix* pMatrix = nullptr) override;

  const IFWL_App* GetOwnerApp() const;

  CFX_SizeF GetOffsetFromParent(IFWL_Widget* pParent);

  uint32_t GetEventKey() const;
  void SetEventKey(uint32_t key);

  void* GetLayoutItem() const;
  void SetLayoutItem(void* pItem);

  void SetAssociateWidget(CFWL_Widget* pAssociate);

  void SetFocus(bool bFocus);
  void Repaint(const CFX_RectF* pRect = nullptr);

 protected:
  friend class CFWL_WidgetImpDelegate;

  IFWL_Widget(const IFWL_App* app,
              const CFWL_WidgetImpProperties& properties,
              IFWL_Widget* pOuter);

  bool IsEnabled() const;
  bool IsVisible() const;
  bool IsActive() const;
  bool IsOverLapper() const;
  bool IsPopup() const;
  bool IsChild() const;
  bool IsLocked() const;
  bool IsOffscreen() const;
  bool HasBorder() const;
  bool HasEdge() const;
  void GetEdgeRect(CFX_RectF& rtEdge);
  FX_FLOAT GetBorderSize(bool bCX = true);
  FX_FLOAT GetEdgeWidth();
  void GetRelativeRect(CFX_RectF& rect);
  void* GetThemeCapacity(CFWL_WidgetCapacity dwCapacity);
  IFWL_ThemeProvider* GetAvailableTheme();
  IFWL_Widget* GetRootOuter();
  CFX_SizeF CalcTextSize(const CFX_WideString& wsText,
                         IFWL_ThemeProvider* pTheme,
                         bool bMultiLine = false,
                         int32_t iLineWidth = -1);
  void CalcTextRect(const CFX_WideString& wsText,
                    IFWL_ThemeProvider* pTheme,
                    uint32_t dwTTOStyles,
                    int32_t iTTOAlign,
                    CFX_RectF& rect);
  void SetGrab(bool bSet);
  bool GetPopupPos(FX_FLOAT fMinHeight,
                   FX_FLOAT fMaxHeight,
                   const CFX_RectF& rtAnchor,
                   CFX_RectF& rtPopup);
  bool GetPopupPosMenu(FX_FLOAT fMinHeight,
                       FX_FLOAT fMaxHeight,
                       const CFX_RectF& rtAnchor,
                       CFX_RectF& rtPopup);
  bool GetPopupPosComboBox(FX_FLOAT fMinHeight,
                           FX_FLOAT fMaxHeight,
                           const CFX_RectF& rtAnchor,
                           CFX_RectF& rtPopup);
  bool GetPopupPosGeneral(FX_FLOAT fMinHeight,
                          FX_FLOAT fMaxHeight,
                          const CFX_RectF& rtAnchor,
                          CFX_RectF& rtPopup);
  bool GetScreenSize(FX_FLOAT& fx, FX_FLOAT& fy);
  void RegisterEventTarget(IFWL_Widget* pEventSource = nullptr,
                           uint32_t dwFilter = FWL_EVENT_ALL_MASK);
  void UnregisterEventTarget();
  void DispatchKeyEvent(CFWL_MsgKey* pNote);
  void DispatchEvent(CFWL_Event* pEvent);
  void DrawBackground(CFX_Graphics* pGraphics,
                      CFWL_Part iPartBk,
                      IFWL_ThemeProvider* pTheme,
                      const CFX_Matrix* pMatrix = nullptr);
  void DrawBorder(CFX_Graphics* pGraphics,
                  CFWL_Part iPartBorder,
                  IFWL_ThemeProvider* pTheme,
                  const CFX_Matrix* pMatrix = nullptr);
  void DrawEdge(CFX_Graphics* pGraphics,
                CFWL_Part iPartEdge,
                IFWL_ThemeProvider* pTheme,
                const CFX_Matrix* pMatrix = nullptr);
  void NotifyDriver();

  bool IsParent(IFWL_Widget* pParent);

  const IFWL_App* const m_pOwnerApp;
  CFWL_WidgetMgr* const m_pWidgetMgr;
  std::unique_ptr<CFWL_WidgetImpProperties> m_pProperties;
  IFWL_Widget* m_pOuter;
  void* m_pLayoutItem;
  CFWL_Widget* m_pAssociate;
  int32_t m_iLock;
  uint32_t m_nEventKey;

 private:
  IFWL_WidgetDelegate* m_pDelegate;  // Not owned.
};

#endif  // XFA_FWL_CORE_IFWL_WIDGET_H_

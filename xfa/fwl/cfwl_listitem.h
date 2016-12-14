// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FWL_CFWL_LISTITEM_H_
#define XFA_FWL_CFWL_LISTITEM_H_

#include "core/fxcrt/fx_coordinates.h"
#include "core/fxcrt/fx_string.h"

class CFX_DIBitmap;

class CFWL_ListItem {
 public:
  explicit CFWL_ListItem(const CFX_WideString& text);
  ~CFWL_ListItem();

  CFX_RectF GetRect() const { return m_rtItem; }
  void SetRect(const CFX_RectF& rect) { m_rtItem = rect; }

  uint32_t GetStyles() const { return m_dwStates; }
  void SetStyles(uint32_t dwStyle) { m_dwStates = dwStyle; }

  CFX_RectF GetCheckRect() const { return m_rtCheckBox; }
  void SetCheckRect(const CFX_RectF& rtCheck) { m_rtCheckBox = rtCheck; }

  uint32_t GetStates() const { return GetStyles() | GetCheckState(); }

  uint32_t GetCheckState() const { return m_dwCheckState; }
  void SetCheckState(uint32_t dwCheckState) { m_dwCheckState = dwCheckState; }

  CFX_DIBitmap* GetIcon() const { return m_pDIB; }

  CFX_WideString GetText() const { return m_wsText; }

 private:
  CFX_RectF m_rtItem;
  uint32_t m_dwStates;
  CFX_WideString m_wsText;
  CFX_DIBitmap* m_pDIB;
  uint32_t m_dwCheckState;
  CFX_RectF m_rtCheckBox;
};

#endif  // XFA_FWL_CFWL_LISTITEM_H_

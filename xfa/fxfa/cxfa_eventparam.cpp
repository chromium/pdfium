// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "xfa/fxfa/cxfa_eventparam.h"

#include "xfa/fxfa/fxfa.h"

CXFA_EventParam::CXFA_EventParam()
    : m_pTarget(nullptr),
      m_eType(XFA_EVENT_Unknown),
      m_bCancelAction(false),
      m_iCommitKey(0),
      m_bKeyDown(false),
      m_bModifier(false),
      m_bReenter(false),
      m_iSelEnd(0),
      m_iSelStart(0),
      m_bShift(false),
      m_bIsFormReady(false) {}

CXFA_EventParam::~CXFA_EventParam() = default;

CXFA_EventParam::CXFA_EventParam(const CXFA_EventParam& other) = default;

void CXFA_EventParam::Reset() {
  m_wsChange.clear();
  m_bCancelAction = false;
  m_iCommitKey = 0;
  m_wsFullText.clear();
  m_bKeyDown = false;
  m_bModifier = false;
  m_wsNewContentType.clear();
  m_wsPrevContentType.clear();
  m_wsPrevText.clear();
  m_bReenter = false;
  m_iSelEnd = 0;
  m_iSelStart = 0;
  m_bShift = false;
  m_wsSoapFaultCode.clear();
  m_wsSoapFaultString.clear();
  m_bIsFormReady = false;
}

WideString CXFA_EventParam::GetNewText() const {
  return m_wsPrevText.Left(m_iSelStart) + m_wsChange +
         m_wsPrevText.Right(m_wsPrevText.GetLength() - m_iSelEnd);
}

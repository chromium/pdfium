// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_
#define XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_

#include <memory>
#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "fxjs/gc/heap.h"
#include "third_party/base/optional.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "xfa/fgas/crt/locale_mgr_iface.h"
#include "xfa/fxfa/parser/gced_locale_iface.h"

class CXFA_Node;
class CXFA_NodeLocale;
class CXFA_XMLLocale;

class CXFA_LocaleMgr : public cppgc::GarbageCollected<CXFA_LocaleMgr>,
                       public LocaleMgrIface {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_LocaleMgr() override;

  void Trace(cppgc::Visitor* visitor) const;

  GCedLocaleIface* GetDefLocale() override;
  GCedLocaleIface* GetLocaleByName(const WideString& wsLocaleName) override;

  void SetDefLocale(GCedLocaleIface* pLocale);
  Optional<WideString> GetConfigLocaleName(CXFA_Node* pConfig) const;

 private:
  CXFA_LocaleMgr(cppgc::Heap* pHeap,
                 CXFA_Node* pLocaleSet,
                 WideString wsDeflcid);

  // May allocate a new object on the cppgc heap.
  CXFA_XMLLocale* GetLocale(uint16_t lcid);

  UnownedPtr<cppgc::Heap> m_pHeap;
  std::vector<cppgc::Member<CXFA_NodeLocale>> m_LocaleArray;
  std::vector<cppgc::Member<CXFA_XMLLocale>> m_XMLLocaleArray;
  cppgc::Member<GCedLocaleIface> m_pDefLocale;

  // Note: three possiblities
  // 1. we might never have tried to determine |m_wsConfigLocale|.
  // 2. we might have tried but gotten nothing and want to continue
  //    to return nothing without ever trying again.
  // 3. we might have tried and gotten something.
  // So |m_bConfigLocaleCached| indicates whether we've already tried,
  // and |m_wsConfigLocale| is the possibly nothing we got if we tried.
  mutable Optional<WideString> m_wsConfigLocale;
  mutable bool m_bConfigLocaleCached = false;

  uint16_t m_dwDeflcid;
};

#endif  // XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_

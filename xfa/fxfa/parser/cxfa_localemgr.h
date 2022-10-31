// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_
#define XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_

#include <vector>

#include "core/fxcrt/unowned_ptr.h"
#include "core/fxcrt/widestring.h"
#include "fxjs/gc/heap.h"
#include "third_party/abseil-cpp/absl/types/optional.h"
#include "v8/include/cppgc/garbage-collected.h"
#include "v8/include/cppgc/member.h"
#include "xfa/fgas/crt/locale_mgr_iface.h"
#include "xfa/fxfa/parser/gced_locale_iface.h"

class CXFA_Node;
class CXFA_NodeLocale;
class CXFA_XMLLocale;

class CXFA_LocaleMgr final : public cppgc::GarbageCollected<CXFA_LocaleMgr>,
                             public LocaleMgrIface {
 public:
  enum class LangID : uint16_t {
    k_zh_HK = 0x0c04,
    k_zh_CN = 0x0804,
    k_zh_TW = 0x0404,
    k_nl_NL = 0x0413,
    k_en_GB = 0x0809,
    k_en_US = 0x0409,
    k_fr_FR = 0x040c,
    k_de_DE = 0x0407,
    k_it_IT = 0x0410,
    k_ja_JP = 0x0411,
    k_ko_KR = 0x0412,
    k_pt_BR = 0x0416,
    k_ru_RU = 0x0419,
    k_es_LA = 0x080a,
    k_es_ES = 0x0c0a,
  };

  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_LocaleMgr() override;

  void Trace(cppgc::Visitor* visitor) const;

  GCedLocaleIface* GetDefLocale() override;
  GCedLocaleIface* GetLocaleByName(const WideString& wsLocaleName) override;

  void SetDefLocale(GCedLocaleIface* pLocale);
  absl::optional<WideString> GetConfigLocaleName(CXFA_Node* pConfig) const;

 private:
  CXFA_LocaleMgr(cppgc::Heap* pHeap,
                 CXFA_Node* pLocaleSet,
                 WideString wsDeflcid);

  // May allocate a new object on the cppgc heap.
  CXFA_XMLLocale* GetLocale(LangID lcid);

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
  mutable absl::optional<WideString> m_wsConfigLocale;
  mutable bool m_bConfigLocaleCached = false;

  LangID m_eDeflcid;
};

#endif  // XFA_FXFA_PARSER_CXFA_LOCALEMGR_H_

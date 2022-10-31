// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_CXFA_FONTMGR_H_
#define XFA_FXFA_CXFA_FONTMGR_H_

#include <map>
#include <utility>

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/retain_ptr.h"
#include "fxjs/gc/heap.h"
#include "v8/include/cppgc/garbage-collected.h"

class CFGAS_GEFont;
class CXFA_FFDoc;

class CXFA_FontMgr final : public cppgc::GarbageCollected<CXFA_FontMgr> {
 public:
  CONSTRUCT_VIA_MAKE_GARBAGE_COLLECTED;
  ~CXFA_FontMgr();

  void Trace(cppgc::Visitor* visitor) const;
  RetainPtr<CFGAS_GEFont> GetFont(CXFA_FFDoc* hDoc,
                                  const WideString& wsFontFamily,
                                  uint32_t dwFontStyles);

 private:
  CXFA_FontMgr();

  std::map<std::pair<WideString, uint32_t>, RetainPtr<CFGAS_GEFont>> m_FontMap;
};

#endif  //  XFA_FXFA_CXFA_FONTMGR_H_

// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_FXFA_PARSER_GCED_LOCALE_IFACE_H_
#define XFA_FXFA_PARSER_GCED_LOCALE_IFACE_H_

#include "v8/include/cppgc/garbage-collected.h"
#include "xfa/fgas/crt/locale_iface.h"

class GCedLocaleIface : public cppgc::GarbageCollected<GCedLocaleIface>,
                        public LocaleIface {
 public:
  virtual void Trace(cppgc::Visitor* visitor) const = 0;
};

#endif  // XFA_FXFA_PARSER_GCED_LOCALE_IFACE_H_

// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef XFA_FGAS_FONT_CFGAS_GEMODULE_H_
#define XFA_FGAS_FONT_CFGAS_GEMODULE_H_

#include <memory>

class CFGAS_FontMgr;

class CFGAS_GEModule {
 public:
  static void Create();
  static void Destroy();
  static CFGAS_GEModule* Get();

  CFGAS_FontMgr* GetFontMgr() { return font_mgr_.get(); }

 private:
  CFGAS_GEModule();
  ~CFGAS_GEModule();

  std::unique_ptr<CFGAS_FontMgr> font_mgr_;
};

#endif  // XFA_FGAS_FONT_CFGAS_GEMODULE_H_

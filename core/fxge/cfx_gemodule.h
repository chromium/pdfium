// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FXGE_CFX_GEMODULE_H_
#define CORE_FXGE_CFX_GEMODULE_H_

#include <memory>

class CFX_FontCache;
class CFX_FontMgr;

class CFX_GEModule {
 public:
  class PlatformIface {
   public:
    static std::unique_ptr<PlatformIface> Create();
    virtual ~PlatformIface() {}

    virtual void Init() = 0;
  };

  static CFX_GEModule* Get();
  static void Destroy();

  void Init(const char** pUserFontPaths);
  CFX_FontCache* GetFontCache();
  CFX_FontMgr* GetFontMgr() const { return m_pFontMgr.get(); }
  PlatformIface* GetPlatform() const { return m_pPlatform.get(); }
  const char** GetUserFontPaths() const { return m_pUserFontPaths; }

 private:
  CFX_GEModule();
  ~CFX_GEModule();

  std::unique_ptr<CFX_FontCache> m_pFontCache;
  std::unique_ptr<CFX_FontMgr> m_pFontMgr;
  std::unique_ptr<PlatformIface> m_pPlatform;
  const char** m_pUserFontPaths = nullptr;
};

#endif  // CORE_FXGE_CFX_GEMODULE_H_

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

  static void Create(const char** pUserFontPaths);
  static void Destroy();
  static CFX_GEModule* Get();

  CFX_FontCache* GetFontCache() const { return m_pFontCache.get(); }
  CFX_FontMgr* GetFontMgr() const { return m_pFontMgr.get(); }
  PlatformIface* GetPlatform() const { return m_pPlatform.get(); }
  const char** GetUserFontPaths() const { return m_pUserFontPaths; }

 private:
  explicit CFX_GEModule(const char** pUserFontPaths);
  ~CFX_GEModule();

  std::unique_ptr<PlatformIface> const m_pPlatform;
  std::unique_ptr<CFX_FontMgr> const m_pFontMgr;
  std::unique_ptr<CFX_FontCache> const m_pFontCache;
  const char** const m_pUserFontPaths;
};

#endif  // CORE_FXGE_CFX_GEMODULE_H_

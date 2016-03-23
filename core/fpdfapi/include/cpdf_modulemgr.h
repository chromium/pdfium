// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_INCLUDE_CPDF_MODULEMGR_H_
#define CORE_FPDFAPI_INCLUDE_CPDF_MODULEMGR_H_

#include <memory>

#include "core/fpdfapi/ipdf_pagemodule.h"
#include "core/fxcrt/include/fx_basic.h"

class CCodec_ModuleMgr;
class ICodec_FaxModule;
class ICodec_FlateModule;
class ICodec_IccModule;
class ICodec_Jbig2Module;
class ICodec_JpegModule;
class ICodec_JpxModule;

class IPDF_PageModule;
class IPDF_RenderModule;

class CPDF_ModuleMgr {
 public:
  static CPDF_ModuleMgr* Get();
  static void Create();
  static void Destroy();
  static const int kFileBufSize = 512;

  void SetCodecModule(CCodec_ModuleMgr* pModule) { m_pCodecModule = pModule; }
  CCodec_ModuleMgr* GetCodecModule() { return m_pCodecModule; }

  void InitPageModule();
  void InitRenderModule();

  IPDF_RenderModule* GetRenderModule() const { return m_pRenderModule.get(); }
  IPDF_PageModule* GetPageModule() const { return m_pPageModule.get(); }

  void LoadEmbeddedGB1CMaps();
  void LoadEmbeddedCNS1CMaps();
  void LoadEmbeddedJapan1CMaps();
  void LoadEmbeddedKorea1CMaps();

  ICodec_FaxModule* GetFaxModule();
  ICodec_JpegModule* GetJpegModule();
  ICodec_JpxModule* GetJpxModule();
  ICodec_Jbig2Module* GetJbig2Module();
  ICodec_IccModule* GetIccModule();
  ICodec_FlateModule* GetFlateModule();

  void SetPrivateData(void* module_id,
                      void* pData,
                      PD_CALLBACK_FREEDATA callback);

  void* GetPrivateData(void* module_id);

 private:
  CPDF_ModuleMgr();
  ~CPDF_ModuleMgr();

  CCodec_ModuleMgr* m_pCodecModule;
  std::unique_ptr<IPDF_RenderModule> m_pRenderModule;
  std::unique_ptr<IPDF_PageModule> m_pPageModule;
  CFX_PrivateData m_privateData;
};

#endif  // CORE_FPDFAPI_INCLUDE_CPDF_MODULEMGR_H_

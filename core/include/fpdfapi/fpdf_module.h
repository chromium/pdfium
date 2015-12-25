// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_INCLUDE_FPDFAPI_FPDF_MODULE_H_
#define CORE_INCLUDE_FPDFAPI_FPDF_MODULE_H_

#include <memory>

#include "core/include/fxcrt/fx_coordinates.h"
#include "core/include/fxcrt/fx_system.h"

class CCodec_ModuleMgr;
class CFX_BitmapDevice;
class CFX_DIBSource;
class CFX_Matrix;
class CPDF_ColorSpace;
class CPDF_Dictionary;
class CPDF_DocPageData;
class CPDF_DocRenderData;
class CPDF_Document;
class CPDF_FontGlobals;
class CPDF_Image;
class CPDF_Page;
class CPDF_PageObjects;
class CPDF_PageRenderCache;
class CPDF_RenderConfig;
class CPDF_RenderOptions;
class CPDF_SecurityHandler;
class CPDF_Stream;
class ICodec_FaxModule;
class ICodec_FlateModule;
class ICodec_IccModule;
class ICodec_Jbig2Module;
class ICodec_JpegModule;
class ICodec_JpxModule;
class IPDF_FontMapper;
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
  FX_BOOL (*m_pDownloadCallback)(const FX_CHAR* module_name);
  CFX_PrivateData m_privateData;
};

class IPDF_PageModule {
 public:
  virtual ~IPDF_PageModule() {}

  virtual CPDF_DocPageData* CreateDocData(CPDF_Document* pDoc) = 0;
  virtual void ReleaseDoc(CPDF_Document*) = 0;
  virtual void ClearDoc(CPDF_Document*) = 0;
  virtual CPDF_FontGlobals* GetFontGlobals() = 0;
  virtual void ClearStockFont(CPDF_Document* pDoc) = 0;
  virtual void NotifyCJKAvailable() = 0;
  virtual CPDF_ColorSpace* GetStockCS(int family) = 0;
};

class IPDF_RenderModule {
 public:
  virtual ~IPDF_RenderModule() {}

  virtual CPDF_DocRenderData* CreateDocData(CPDF_Document* pDoc) = 0;
  virtual void DestroyDocData(CPDF_DocRenderData*) = 0;
  virtual void ClearDocData(CPDF_DocRenderData*) = 0;
  virtual CPDF_DocRenderData* GetRenderData() = 0;
  virtual CPDF_PageRenderCache* CreatePageCache(CPDF_Page* pPage) = 0;
  virtual void DestroyPageCache(CPDF_PageRenderCache*) = 0;
  virtual CPDF_RenderConfig* GetConfig() = 0;
};

#endif  // CORE_INCLUDE_FPDFAPI_FPDF_MODULE_H_

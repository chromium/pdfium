// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "core/include/fxcodec/fx_codec.h"
#include "core/include/fpdfapi/fpdf_module.h"

namespace {

CPDF_ModuleMgr* g_FPDFAPI_pDefaultMgr = nullptr;

}  // namespace

// static
CPDF_ModuleMgr* CPDF_ModuleMgr::Get() {
  return g_FPDFAPI_pDefaultMgr;
}

// static
void CPDF_ModuleMgr::Create() {
  ASSERT(!g_FPDFAPI_pDefaultMgr);
  g_FPDFAPI_pDefaultMgr = new CPDF_ModuleMgr;
}

// static
void CPDF_ModuleMgr::Destroy() {
  delete g_FPDFAPI_pDefaultMgr;
  g_FPDFAPI_pDefaultMgr = nullptr;
}

CPDF_ModuleMgr::CPDF_ModuleMgr() : m_pCodecModule(nullptr) {}

CPDF_ModuleMgr::~CPDF_ModuleMgr() {}

void CPDF_ModuleMgr::SetPrivateData(void* module_id,
                                    void* pData,
                                    PD_CALLBACK_FREEDATA callback) {
  m_privateData.SetPrivateData(module_id, pData, callback);
}
void* CPDF_ModuleMgr::GetPrivateData(void* module_id) {
  return m_privateData.GetPrivateData(module_id);
}
ICodec_FaxModule* CPDF_ModuleMgr::GetFaxModule() {
  return m_pCodecModule ? m_pCodecModule->GetFaxModule() : NULL;
}
ICodec_JpegModule* CPDF_ModuleMgr::GetJpegModule() {
  return m_pCodecModule ? m_pCodecModule->GetJpegModule() : NULL;
}
ICodec_JpxModule* CPDF_ModuleMgr::GetJpxModule() {
  return m_pCodecModule ? m_pCodecModule->GetJpxModule() : NULL;
}
ICodec_Jbig2Module* CPDF_ModuleMgr::GetJbig2Module() {
  return m_pCodecModule ? m_pCodecModule->GetJbig2Module() : NULL;
}
ICodec_IccModule* CPDF_ModuleMgr::GetIccModule() {
  return m_pCodecModule ? m_pCodecModule->GetIccModule() : NULL;
}
ICodec_FlateModule* CPDF_ModuleMgr::GetFlateModule() {
  return m_pCodecModule ? m_pCodecModule->GetFlateModule() : NULL;
}

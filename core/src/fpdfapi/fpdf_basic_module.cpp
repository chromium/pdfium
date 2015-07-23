// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcodec/fx_codec.h"
#include "../../include/fpdfapi/fpdf_module.h"

namespace {

CPDF_ModuleMgr* g_FPDFAPI_pDefaultMgr = nullptr;

const char kAddinNameCJK[] = "Eastern Asian Language Support";

}  // namespace

// static
CPDF_ModuleMgr* CPDF_ModuleMgr::Get()
{
    return g_FPDFAPI_pDefaultMgr;
}

// static
void CPDF_ModuleMgr::Create()
{
    ASSERT(!g_FPDFAPI_pDefaultMgr);
    g_FPDFAPI_pDefaultMgr = new CPDF_ModuleMgr;
}

// static
void CPDF_ModuleMgr::Destroy()
{
    delete g_FPDFAPI_pDefaultMgr;
    g_FPDFAPI_pDefaultMgr = nullptr;
}

CPDF_ModuleMgr::CPDF_ModuleMgr()
    : m_pCodecModule(nullptr)
{
}

CPDF_ModuleMgr::~CPDF_ModuleMgr()
{
}

void CPDF_ModuleMgr::SetDownloadCallback(FX_BOOL (*callback)(const FX_CHAR* module_name))
{
    m_pDownloadCallback = callback;
}
FX_BOOL CPDF_ModuleMgr::DownloadModule(const FX_CHAR* module_name)
{
    if (m_pDownloadCallback == NULL) {
        return FALSE;
    }
    return m_pDownloadCallback(module_name);
}
void CPDF_ModuleMgr::NotifyModuleAvailable(const FX_CHAR* module_name)
{
    if (FXSYS_strcmp(module_name, kAddinNameCJK) == 0) {
        m_pPageModule->NotifyCJKAvailable();
    }
}
void CPDF_ModuleMgr::RegisterSecurityHandler(const FX_CHAR* filter, CPDF_SecurityHandler * (*CreateHandler)(void* param), void* param)
{
    if (CreateHandler == NULL) {
        m_SecurityHandlerMap.RemoveKey(filter);
    } else {
        m_SecurityHandlerMap.SetAt(filter, (void*)CreateHandler);
    }
    if (param) {
        m_SecurityHandlerMap.SetAt(FX_BSTRC("_param_") + filter, param);
    }
}
void CPDF_ModuleMgr::SetPrivateData(void* module_id, void* pData, PD_CALLBACK_FREEDATA callback)
{
    m_privateData.SetPrivateData(module_id, pData, callback);
}
void* CPDF_ModuleMgr::GetPrivateData(void* module_id)
{
    return m_privateData.GetPrivateData(module_id);
}
CPDF_SecurityHandler* CPDF_ModuleMgr::CreateSecurityHandler(const FX_CHAR* filter)
{
    CPDF_SecurityHandler* (*CreateHandler)(void*) = NULL;
    if (!m_SecurityHandlerMap.Lookup(filter, (void*&)CreateHandler)) {
        return NULL;
    }
    if (CreateHandler == NULL) {
        return NULL;
    }
    void* param = NULL;
    m_SecurityHandlerMap.Lookup(FX_BSTRC("_param_") + filter, param);
    return CreateHandler(param);
}
ICodec_FaxModule* CPDF_ModuleMgr::GetFaxModule()
{
    return m_pCodecModule ? m_pCodecModule->GetFaxModule() : NULL;
}
ICodec_JpegModule* CPDF_ModuleMgr::GetJpegModule()
{
    return m_pCodecModule ? m_pCodecModule->GetJpegModule() : NULL;
}
ICodec_JpxModule* CPDF_ModuleMgr::GetJpxModule()
{
    return m_pCodecModule ? m_pCodecModule->GetJpxModule() : NULL;
}
ICodec_Jbig2Module* CPDF_ModuleMgr::GetJbig2Module()
{
    return m_pCodecModule ? m_pCodecModule->GetJbig2Module() : NULL;
}
ICodec_IccModule* CPDF_ModuleMgr::GetIccModule()
{
    return m_pCodecModule ? m_pCodecModule->GetIccModule() : NULL;
}
ICodec_FlateModule* CPDF_ModuleMgr::GetFlateModule()
{
    return m_pCodecModule ? m_pCodecModule->GetFlateModule() : NULL;
}

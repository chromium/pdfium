// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../include/fxcodec/fx_codec.h"
#include "../../include/fpdfapi/fpdf_module.h"
static CPDF_ModuleMgr*	g_FPDFAPI_pDefaultMgr = NULL;
CPDF_ModuleMgr* CPDF_ModuleMgr::Get()
{
    return g_FPDFAPI_pDefaultMgr;
}
void CPDF_ModuleMgr::Create()
{
    g_FPDFAPI_pDefaultMgr = FX_NEW CPDF_ModuleMgr;
    g_FPDFAPI_pDefaultMgr->Initialize();
}
void CPDF_ModuleMgr::Destroy()
{
    if (g_FPDFAPI_pDefaultMgr) {
        delete g_FPDFAPI_pDefaultMgr;
    }
    g_FPDFAPI_pDefaultMgr = NULL;
}
CPDF_ModuleMgr::CPDF_ModuleMgr()
{
    m_pCodecModule = NULL;
    m_pPageModule = NULL;
    m_pRenderModule = NULL;
    m_FileBufSize = 512;
}
void CPDF_ModuleMgr::Initialize()
{
    InitModules();
    m_FileBufSize = 512;
}
void CPDF_ModuleMgr::InitModules()
{
    m_pCodecModule = NULL;
    m_pPageModule = FX_NEW CPDF_PageModuleDef;
    m_pRenderModule = FX_NEW CPDF_RenderModuleDef;
}
CPDF_ModuleMgr::~CPDF_ModuleMgr()
{
    if (m_pPageModule) {
        delete m_pPageModule;
    }
    if (m_pRenderModule) {
        delete m_pRenderModule;
    }
}
void CPDF_ModuleMgr::SetDownloadCallback(FX_BOOL (*callback)(FX_LPCSTR module_name))
{
    m_pDownloadCallback = callback;
}
FX_BOOL CPDF_ModuleMgr::DownloadModule(FX_LPCSTR module_name)
{
    if (m_pDownloadCallback == NULL) {
        return FALSE;
    }
    return m_pDownloadCallback(module_name);
}
void CPDF_ModuleMgr::NotifyModuleAvailable(FX_LPCSTR module_name)
{
    if (FXSYS_strcmp(module_name, ADDIN_NAME_CJK) == 0) {
        m_pPageModule->NotifyCJKAvailable();
    } else if (FXSYS_strcmp(module_name, ADDIN_NAME_DECODER) == 0) {
        m_pRenderModule->NotifyDecoderAvailable();
    }
}
void CPDF_ModuleMgr::RegisterSecurityHandler(FX_LPCSTR filter, CPDF_SecurityHandler * (*CreateHandler)(void* param), void* param)
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
void CPDF_ModuleMgr::SetPrivateData(FX_LPVOID module_id, FX_LPVOID pData, PD_CALLBACK_FREEDATA callback)
{
    m_privateData.SetPrivateData(module_id, pData, callback);
}
FX_LPVOID CPDF_ModuleMgr::GetPrivateData(FX_LPVOID module_id)
{
    return m_privateData.GetPrivateData(module_id);
}
CPDF_SecurityHandler* CPDF_ModuleMgr::CreateSecurityHandler(FX_LPCSTR filter)
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

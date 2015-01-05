// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fxcodec/fx_codec.h"
#include "codec_int.h"
CCodec_Jbig2Context::CCodec_Jbig2Context()
{
    FXSYS_memset32(this, 0, sizeof(CCodec_Jbig2Context));
}
CCodec_Jbig2Module::~CCodec_Jbig2Module()
{
}
void* CCodec_Jbig2Module::CreateJbig2Context()
{
    return FX_NEW CCodec_Jbig2Context();
}
void CCodec_Jbig2Module::DestroyJbig2Context(void* pJbig2Content)
{
    if(pJbig2Content) {
        CJBig2_Context::DestroyContext(((CCodec_Jbig2Context*)pJbig2Content)->m_pContext);
        delete (CCodec_Jbig2Context*)pJbig2Content;
    }
    pJbig2Content = NULL;
}
FX_BOOL CCodec_Jbig2Module::Decode(FX_DWORD width, FX_DWORD height, FX_LPCBYTE src_buf, FX_DWORD src_size,
                                   FX_LPCBYTE global_data, FX_DWORD global_size, FX_LPBYTE dest_buf, FX_DWORD dest_pitch)
{
    FXSYS_memset32(dest_buf, 0, height * dest_pitch);
    CJBig2_Context* pContext = CJBig2_Context::CreateContext(&m_Module,
                               (FX_LPBYTE)global_data, global_size, (FX_LPBYTE)src_buf, src_size, JBIG2_EMBED_STREAM, &m_SymbolDictCache);
    if (pContext == NULL) {
        return FALSE;
    }
    int ret = pContext->getFirstPage(dest_buf, width, height, dest_pitch, NULL);
    CJBig2_Context::DestroyContext(pContext);
    if (ret != JBIG2_SUCCESS) {
        return FALSE;
    }
    int dword_size = height * dest_pitch / 4;
    FX_DWORD* dword_buf = (FX_DWORD*)dest_buf;
    for (int i = 0; i < dword_size; i ++) {
        dword_buf[i] = ~dword_buf[i];
    }
    return TRUE;
}
FX_BOOL CCodec_Jbig2Module::Decode(IFX_FileRead* file_ptr,
                                   FX_DWORD& width, FX_DWORD& height, FX_DWORD& pitch, FX_LPBYTE& dest_buf)
{
    CJBig2_Context* pContext = NULL;
    CJBig2_Image* dest_image = NULL;
    FX_DWORD src_size = (FX_DWORD)file_ptr->GetSize();
    FX_LPBYTE src_buf = FX_Alloc(FX_BYTE, src_size);
    if (src_buf == NULL) {
        return FALSE;
    }
    int ret = 0;
    if(!file_ptr->ReadBlock(src_buf, 0, src_size)) {
        goto failed;
    }
    pContext = CJBig2_Context::CreateContext(&m_Module, NULL, 0, src_buf, src_size, JBIG2_FILE_STREAM, &m_SymbolDictCache);
    if(pContext == NULL) {
        goto failed;
    }
    ret = pContext->getFirstPage(&dest_image, NULL);
    CJBig2_Context::DestroyContext(pContext);
    if (ret != JBIG2_SUCCESS) {
        goto failed;
    }
    width = (FX_DWORD)dest_image->m_nWidth;
    height = (FX_DWORD)dest_image->m_nHeight;
    pitch = (FX_DWORD)dest_image->m_nStride;
    dest_buf = dest_image->m_pData;
    dest_image->m_bNeedFree = FALSE;
    delete dest_image;
    FX_Free(src_buf);
    return TRUE;
failed:
    if(src_buf) {
        FX_Free(src_buf);
    }
    return FALSE;
}
FXCODEC_STATUS CCodec_Jbig2Module::StartDecode(void* pJbig2Context, FX_DWORD width, FX_DWORD height, FX_LPCBYTE src_buf, FX_DWORD src_size,
        FX_LPCBYTE global_data, FX_DWORD global_size, FX_LPBYTE dest_buf, FX_DWORD dest_pitch, IFX_Pause* pPause)
{
    if(!pJbig2Context) {
        return FXCODEC_STATUS_ERR_PARAMS;
    }
    CCodec_Jbig2Context* m_pJbig2Context = (CCodec_Jbig2Context*)pJbig2Context;
    m_pJbig2Context->m_width = width;
    m_pJbig2Context->m_height = height;
    m_pJbig2Context->m_src_buf = (unsigned char *)src_buf;
    m_pJbig2Context->m_src_size = src_size;
    m_pJbig2Context->m_global_data = global_data;
    m_pJbig2Context->m_global_size = global_size;
    m_pJbig2Context->m_dest_buf = dest_buf;
    m_pJbig2Context->m_dest_pitch = dest_pitch;
    m_pJbig2Context->m_pPause = pPause;
    m_pJbig2Context->m_bFileReader = FALSE;
    FXSYS_memset32(dest_buf, 0, height * dest_pitch);
    m_pJbig2Context->m_pContext = CJBig2_Context::CreateContext(&m_Module,
                                  (FX_LPBYTE)global_data, global_size, (FX_LPBYTE)src_buf, src_size, JBIG2_EMBED_STREAM, &m_SymbolDictCache, pPause);
    if(!m_pJbig2Context->m_pContext) {
        return FXCODEC_STATUS_ERROR;
    }
    int ret = m_pJbig2Context->m_pContext->getFirstPage(dest_buf, width, height, dest_pitch, pPause);
    if(m_pJbig2Context->m_pContext->GetProcessiveStatus() == FXCODEC_STATUS_DECODE_FINISH) {
        CJBig2_Context::DestroyContext(m_pJbig2Context->m_pContext);
        m_pJbig2Context->m_pContext = NULL;
        if (ret != JBIG2_SUCCESS) {
            return FXCODEC_STATUS_ERROR;
        }
        int dword_size = height * dest_pitch / 4;
        FX_DWORD* dword_buf = (FX_DWORD*)dest_buf;
        for (int i = 0; i < dword_size; i ++) {
            dword_buf[i] = ~dword_buf[i];
        }
        return FXCODEC_STATUS_DECODE_FINISH;
    }
    return m_pJbig2Context->m_pContext->GetProcessiveStatus();
}
FXCODEC_STATUS CCodec_Jbig2Module::StartDecode(void* pJbig2Context, IFX_FileRead* file_ptr,
        FX_DWORD& width, FX_DWORD& height, FX_DWORD& pitch, FX_LPBYTE& dest_buf, IFX_Pause* pPause)
{
    if(!pJbig2Context) {
        return FXCODEC_STATUS_ERR_PARAMS;
    }
    CCodec_Jbig2Context* m_pJbig2Context = (CCodec_Jbig2Context*)pJbig2Context;
    m_pJbig2Context->m_bFileReader = TRUE;
    m_pJbig2Context->m_dest_image = NULL;
    m_pJbig2Context->m_src_size = (FX_DWORD)file_ptr->GetSize();
    m_pJbig2Context->m_src_buf = FX_Alloc(FX_BYTE, m_pJbig2Context->m_src_size);
    if (m_pJbig2Context->m_src_buf == NULL) {
        return FXCODEC_STATUS_ERR_MEMORY;
    }
    int ret = 0;
    if(!file_ptr->ReadBlock((void*)m_pJbig2Context->m_src_buf, 0, m_pJbig2Context->m_src_size)) {
        goto failed;
    }
    m_pJbig2Context->m_pContext = CJBig2_Context::CreateContext(&m_Module, NULL, 0, m_pJbig2Context->m_src_buf, m_pJbig2Context->m_src_size, JBIG2_FILE_STREAM, &m_SymbolDictCache, pPause);
    if(m_pJbig2Context->m_pContext == NULL) {
        goto failed;
    }
    ret = m_pJbig2Context->m_pContext->getFirstPage(&m_pJbig2Context->m_dest_image, pPause);
    if(m_pJbig2Context->m_pContext->GetProcessiveStatus() == FXCODEC_STATUS_DECODE_TOBECONTINUE) {
        width = (FX_DWORD)m_pJbig2Context->m_dest_image->m_nWidth;
        height = (FX_DWORD)m_pJbig2Context->m_dest_image->m_nHeight;
        pitch = (FX_DWORD)m_pJbig2Context->m_dest_image->m_nStride;
        dest_buf = m_pJbig2Context->m_dest_image->m_pData;
        m_pJbig2Context->m_dest_image->m_bNeedFree = FALSE;
        return FXCODEC_STATUS_DECODE_TOBECONTINUE;
    }
    CJBig2_Context::DestroyContext(m_pJbig2Context->m_pContext);
    m_pJbig2Context->m_pContext = NULL;
    if (ret != JBIG2_SUCCESS) {
        goto failed;
    }
    width = (FX_DWORD)m_pJbig2Context->m_dest_image->m_nWidth;
    height = (FX_DWORD)m_pJbig2Context->m_dest_image->m_nHeight;
    pitch = (FX_DWORD)m_pJbig2Context->m_dest_image->m_nStride;
    dest_buf = m_pJbig2Context->m_dest_image->m_pData;
    m_pJbig2Context->m_dest_image->m_bNeedFree = FALSE;
    delete m_pJbig2Context->m_dest_image;
    FX_Free(m_pJbig2Context->m_src_buf);
    return FXCODEC_STATUS_DECODE_FINISH;
failed:
    if(m_pJbig2Context->m_src_buf) {
        FX_Free(m_pJbig2Context->m_src_buf);
    }
    m_pJbig2Context->m_src_buf = NULL;
    return FXCODEC_STATUS_ERROR;
}
FXCODEC_STATUS CCodec_Jbig2Module::ContinueDecode(void* pJbig2Context, IFX_Pause* pPause)
{
    CCodec_Jbig2Context* m_pJbig2Context = (CCodec_Jbig2Context*)pJbig2Context;
    int ret = m_pJbig2Context->m_pContext->Continue(pPause);
    if(m_pJbig2Context->m_pContext->GetProcessiveStatus() == FXCODEC_STATUS_DECODE_FINISH) {
        if(m_pJbig2Context->m_bFileReader) {
            CJBig2_Context::DestroyContext(m_pJbig2Context->m_pContext);
            m_pJbig2Context->m_pContext = NULL;
            if (ret != JBIG2_SUCCESS) {
                if(m_pJbig2Context->m_src_buf) {
                    FX_Free(m_pJbig2Context->m_src_buf);
                }
                m_pJbig2Context->m_src_buf = NULL;
                return FXCODEC_STATUS_ERROR;
            }
            delete m_pJbig2Context->m_dest_image;
            FX_Free(m_pJbig2Context->m_src_buf);
            return FXCODEC_STATUS_DECODE_FINISH;
        } else {
            CJBig2_Context::DestroyContext(m_pJbig2Context->m_pContext);
            m_pJbig2Context->m_pContext = NULL;
            if (ret != JBIG2_SUCCESS) {
                return FXCODEC_STATUS_ERROR;
            }
            int dword_size = m_pJbig2Context->m_height * m_pJbig2Context->m_dest_pitch / 4;
            FX_DWORD* dword_buf = (FX_DWORD*)m_pJbig2Context->m_dest_buf;
            for (int i = 0; i < dword_size; i ++) {
                dword_buf[i] = ~dword_buf[i];
            }
            return FXCODEC_STATUS_DECODE_FINISH;
        }
    }
    return m_pJbig2Context->m_pContext->GetProcessiveStatus();
}




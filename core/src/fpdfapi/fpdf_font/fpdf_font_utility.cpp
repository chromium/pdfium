// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../include/fpdfapi/fpdf_module.h"
#include "../../../include/fpdfapi/fpdf_resource.h"
#include "../../../include/fxcodec/fx_codec.h"
#include "font_int.h"
typedef struct {
    FXSYS_FILE*	m_pFile;
    int		m_nFiles;
    int		m_IndexSize;
    int		m_IndexOffset;
} FXFC_PACKAGE;
FX_LPVOID FXFC_LoadPackage(FX_LPCSTR name)
{
    FXSYS_FILE* file = FXSYS_fopen(name, "rb");
    if (file == NULL) {
        return NULL;
    }
    FX_BYTE buf[256];
    FXSYS_fread(buf, 1, 20, file);
    if (*(FX_DWORD*)buf != 0x43465846) {
        FXSYS_fclose(file);
        return NULL;
    }
    FXFC_PACKAGE* pPackage = FX_Alloc(FXFC_PACKAGE, 1);
    pPackage->m_pFile = file;
    pPackage->m_nFiles = *(int*)(buf + 8);
    pPackage->m_IndexSize = *(int*)(buf + 12);
    pPackage->m_IndexOffset = *(int*)(buf + 16);
    return pPackage;
}
void FXFC_ClosePackage(FX_LPVOID p)
{
    FXFC_PACKAGE* pPackage = (FXFC_PACKAGE*)p;
    FXSYS_fclose(pPackage->m_pFile);
    FX_Free(pPackage);
}
FX_BOOL FXFC_LoadFile(FX_LPVOID p, FX_LPCSTR name, FX_LPBYTE& pBuffer, FX_DWORD& size)
{
    FXFC_PACKAGE* pPackage = (FXFC_PACKAGE*)p;
    FXSYS_fseek(pPackage->m_pFile, pPackage->m_IndexOffset, FXSYS_SEEK_SET);
    FX_BYTE buf[128];
    for (int i = 0; i < pPackage->m_nFiles; i ++) {
        FXSYS_fread(buf, pPackage->m_IndexSize, 1, pPackage->m_pFile);
        if (FXSYS_stricmp((FX_LPCSTR)buf, name) == 0) {
            FX_DWORD offset = *(FX_DWORD*)&buf[64];
            size = *(FX_DWORD*)&buf[68];
            pBuffer = FX_Alloc(FX_BYTE, size);
            FXSYS_fseek(pPackage->m_pFile, offset, FXSYS_SEEK_SET);
            FXSYS_fread(pBuffer, size, 1, pPackage->m_pFile);
            if (buf[72]) {
                FX_DWORD orig_size;
                FX_LPBYTE comp_buf = pBuffer;
                CPDF_ModuleMgr::Get()->GetFlateModule()->FlateOrLZWDecode(FALSE, comp_buf, size, FALSE,
                        0, 0, 0, 0, 0, pBuffer, orig_size);
                FX_Free(comp_buf);
                size = orig_size;
            }
            return TRUE;
        }
    }
    return FALSE;
}
FX_BOOL CPDF_FXMP::LoadFile(FX_LPVOID pPackage, FX_LPCSTR fileid)
{
    if (m_pHeader) {
        FX_Free(m_pHeader);
        m_pHeader = NULL;
    }
    m_pTable = NULL;
    FX_DWORD size;
    if (!FXFC_LoadFile(pPackage, fileid, (FX_LPBYTE&)m_pHeader, size)) {
        return FALSE;
    }
    if (FXSYS_memcmp32(m_pHeader, "FXMP", 4)) {
        return FALSE;
    }
    m_pTable = (FX_LPBYTE)m_pHeader + m_pHeader->dwDataOffset;
    return TRUE;
}
FX_LPBYTE CPDF_FXMP::GetRecord(FX_DWORD index)
{
    if (m_pTable == NULL) {
        return NULL;
    }
    if ((int)index < (int)m_pHeader->dwStartIndex || index > m_pHeader->dwEndIndex) {
        return NULL;
    }
    return m_pTable + (index - m_pHeader->dwStartIndex) * m_pHeader->dwRecordSize;
}

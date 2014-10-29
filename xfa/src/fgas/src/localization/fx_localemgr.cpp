// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../fgas_base.h"
#include "fx_localemgr.h"
IFX_LocaleMgr*	FX_LocaleMgr_Create(FX_LPCWSTR pszLocalPath, FX_WORD wDefaultLCID)
{
    void* pPathHandle = FX_OpenFolder(pszLocalPath);
    if (!pPathHandle) {
        return NULL;
    }
    CFX_LocaleMgr* pLocaleMgr = FX_NEW CFX_LocaleMgr(wDefaultLCID);
    CFX_WideString wsFileName;
    FX_BOOL bFolder = FALSE;
    while (FX_GetNextFile(pPathHandle, wsFileName, bFolder)) {
        if (!bFolder) {
            if (wsFileName.GetLength() < 4) {
                continue;
            }
            CFX_WideString wsExt = wsFileName.Right(4);
            wsExt.MakeLower();
            if (wsExt != L".xml") {
                continue;
            }
            CFX_WideString wsFullPath(pszLocalPath);
            wsFullPath += L"\\" + wsFileName;
            IFX_FileRead* pRead = FX_CreateFileRead(wsFullPath);
            if (!pRead) {
                continue;
            }
            CXML_Element* pXmlLocale = CXML_Element::Parse(pRead);
            pRead->Release();
            CFX_ByteString bssp = pXmlLocale->GetNamespace();
            if (bssp == "http://www.foxitsoftware.com/localization") {
                CFX_WideString wsLCID = pXmlLocale->GetAttrValue("", "lcid");
                wchar_t* pEnd = NULL;
                FX_DWORD dwLCID = wcstol(wsLCID, &pEnd, 16);
                if (pLocaleMgr->m_lcid2xml.GetValueAt((FX_LPVOID)(FX_UINTPTR)dwLCID)) {
                    delete pXmlLocale;
                } else {
                    pLocaleMgr->m_lcid2xml.SetAt((FX_LPVOID)(FX_UINTPTR)dwLCID, pXmlLocale);
                }
            } else {
                delete pXmlLocale;
            }
        }
    }
    FX_CloseFolder(pPathHandle);
    return pLocaleMgr;
}
CFX_LocaleMgr::CFX_LocaleMgr(FX_WORD wDefLCID)
    : m_wDefLCID(wDefLCID)
{
}
CFX_LocaleMgr::~CFX_LocaleMgr()
{
    FX_POSITION ps = m_lcid2locale.GetStartPosition();
    while (ps) {
        FX_LPVOID plcid;
        IFX_Locale* pLocale = NULL;
        m_lcid2locale.GetNextAssoc(ps, plcid, (void*&)pLocale);
        pLocale->Release();
    }
    m_lcid2locale.RemoveAll();
    ps = m_lcid2xml.GetStartPosition();
    while (ps) {
        FX_LPVOID plcid;
        CXML_Element* pxml = NULL;
        m_lcid2xml.GetNextAssoc(ps, plcid, (void*&)pxml);
        delete pxml;
    }
    m_lcid2xml.RemoveAll();
}
FX_WORD	CFX_LocaleMgr::GetDefLocaleID()
{
    return m_wDefLCID;
}
IFX_Locale* CFX_LocaleMgr::GetDefLocale()
{
    return GetLocale(m_wDefLCID);
}
IFX_Locale* CFX_LocaleMgr::GetLocale(FX_WORD lcid)
{
    IFX_Locale* pLocale = (IFX_Locale*)m_lcid2locale.GetValueAt((FX_LPVOID)(FX_UINTPTR)lcid);
    if (!pLocale) {
        CXML_Element* pxml = (CXML_Element*)m_lcid2xml.GetValueAt((FX_LPVOID)(FX_UINTPTR)lcid);
        if (pxml) {
            pLocale = IFX_Locale::Create(pxml);
            m_lcid2locale.SetAt((FX_LPVOID)(FX_UINTPTR)lcid, pLocale);
        }
    }
    return pLocale;
}
IFX_Locale* CFX_LocaleMgr::GetLocaleByName(FX_WSTR wsLocaleName)
{
    return NULL;
}

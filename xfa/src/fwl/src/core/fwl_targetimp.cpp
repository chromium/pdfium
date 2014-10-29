// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "../../../foxitlib.h"
#include "include/fwl_targetimp.h"
FX_DWORD IFWL_Target::Release()
{
    FX_DWORD dwRef = ((CFWL_Target*)m_pData)->Release();
    if (!dwRef) {
        m_pData = NULL;
        delete this;
    }
    return dwRef;
}
IFWL_Target* IFWL_Target::Retain()
{
    return ((CFWL_Target*)m_pData)->Retain();
}
FX_DWORD IFWL_Target::GetRefCount() const
{
    return ((CFWL_Target*)m_pData)->GetRefCount();
}
FWL_ERR	IFWL_Target::GetClassName(CFX_WideString &wsClass) const
{
    return ((CFWL_Target*)m_pData)->GetClassName(wsClass);
}
FX_DWORD IFWL_Target::GetClassID() const
{
    return ((CFWL_Target*)m_pData)->GetClassID();
}
FX_BOOL	IFWL_Target::IsInstance(FX_WSTR wsClass) const
{
    return ((CFWL_Target*)m_pData)->IsInstance(wsClass);
}
FWL_ERR	IFWL_Target::Initialize()
{
    return ((CFWL_Target*)m_pData)->Initialize();
}
FWL_ERR	IFWL_Target::Finalize()
{
    return ((CFWL_Target*)m_pData)->Finalize();
}
IFWL_Target::~IFWL_Target()
{
}
CFWL_Target::CFWL_Target()
    : m_dwRefCount(1)
{
}
CFWL_Target::~CFWL_Target()
{
}
FX_DWORD CFWL_Target::Release()
{
    m_dwRefCount--;
    FX_DWORD dwRet = m_dwRefCount;
    if (!m_dwRefCount) {
        delete this;
    }
    return dwRet;
}
IFWL_Target* CFWL_Target::Retain()
{
    m_dwRefCount++;
    return (IFWL_Target*)this;
}
FX_DWORD CFWL_Target::GetRefCount() const
{
    return m_dwRefCount;
}
FWL_ERR CFWL_Target::GetClassName(CFX_WideString &wsClass) const
{
    return FWL_ERR_Succeeded;
}
FX_DWORD CFWL_Target::GetClassID() const
{
    return 0;
}
FX_BOOL	CFWL_Target::IsInstance(FX_WSTR wsClass) const
{
    return FALSE;
}
FWL_ERR CFWL_Target::Initialize()
{
    return FWL_ERR_Succeeded;
}
FWL_ERR CFWL_Target::Finalize()
{
    return FWL_ERR_Succeeded;
}
FX_LPVOID IFWL_TargetData::GetData()
{
    return m_pData;
}
void IFWL_TargetData::SetData(FX_LPVOID pData)
{
    m_pData = pData;
}

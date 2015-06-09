// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "JBig2_Object.h"
#include "JBig2_Module.h"
void *CJBig2_Object::operator new(size_t size, CJBig2_Module *pModule, FX_LPCSTR filename, int line)
{
    CJBig2_Object *p;
    p = (CJBig2_Object *)pModule->JBig2_Malloc((FX_DWORD)size);
    p->m_pModule = pModule;
    return p;
}
void CJBig2_Object::operator delete(void *p, CJBig2_Module *pModule, FX_LPCSTR filename, int line)
{
    pModule->JBig2_Free(p);
}
void *CJBig2_Object::operator new(size_t size, CJBig2_Module *pModule)
{
    CJBig2_Object *p;
    p = (CJBig2_Object *)pModule->JBig2_Malloc((FX_DWORD)size);
    p->m_pModule = pModule;
    return p;
}
void CJBig2_Object::operator delete(void *p)
{
    ((CJBig2_Object *)p)->m_pModule->JBig2_Free(p);
}
void CJBig2_Object::operator delete(void *p, CJBig2_Module *pModule)
{
    pModule->JBig2_Free(p);
}
void *CJBig2_Object::operator new[](size_t size, CJBig2_Module *pModule, size_t unit_size,
                                    FX_LPCSTR filename, int line)
{
    void *p;
    uint8_t *pCur, *pEnd;
    p = (uint8_t *)pModule->JBig2_Malloc((FX_DWORD)size);
    pCur = (uint8_t *)p;
    pEnd = pCur + size;
    for(; pCur < pEnd; pCur += unit_size) {
        ((CJBig2_Object *)pCur)->m_pModule = pModule;
    }
    return p;
}
void CJBig2_Object::operator delete[](void *p, CJBig2_Module *pModule, size_t unit_size,
                                      FX_LPCSTR filename, int line)
{
    pModule->JBig2_Free(p);
}
void *CJBig2_Object::operator new[](size_t size, CJBig2_Module *pModule, size_t unit_size)
{
    void *p;
    uint8_t *pCur, *pEnd;
    p = (uint8_t *)pModule->JBig2_Malloc((FX_DWORD)size);
    pCur = (uint8_t *)p;
    pEnd = pCur + size;
    for(; pCur < pEnd; pCur += unit_size) {
        ((CJBig2_Object *)pCur)->m_pModule = pModule;
    }
    return p;
}
void CJBig2_Object::operator delete[](void* p)
{
    ((CJBig2_Object *)p)->m_pModule->JBig2_Free(p);
}
void CJBig2_Object::operator delete[](void *p, CJBig2_Module *pModule, size_t unit_size)
{
    pModule->JBig2_Free(p);
}

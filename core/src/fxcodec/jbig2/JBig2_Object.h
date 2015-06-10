// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.
 
// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_OBJECT_H_
#define _JBIG2_OBJECT_H_
#include "JBig2_Define.h"
class CJBig2_Module;
#define _JBIG2_NO_EXPECTION_
class CJBig2_Object
{
public:

    void *operator new(size_t size, CJBig2_Module *pModule, const FX_CHAR* filename, int line);

    void operator delete(void *p, CJBig2_Module *pModule, const FX_CHAR* filename, int line);

    void *operator new(size_t size, CJBig2_Module *pModule);

    void operator delete(void *p);

    void operator delete(void *p, CJBig2_Module *pModule);

    void *operator new[](size_t size, CJBig2_Module *pModule, size_t unit_size,
                         const FX_CHAR* filename, int line);

    void operator delete[](void *p, CJBig2_Module *pModule, size_t unit_size,
                           const FX_CHAR* filename, int line);

    void *operator new[](size_t size, CJBig2_Module *pModule, size_t unit_size);

    void operator delete[](void* p);

    void operator delete[](void *p, CJBig2_Module *pModule, size_t unit_size);
public:

    CJBig2_Module *m_pModule;
};
#define JBIG2_NEW new(m_pModule)
#define JBIG2_ALLOC(p, a) p = JBIG2_NEW a; p->m_pModule = m_pModule;
#endif

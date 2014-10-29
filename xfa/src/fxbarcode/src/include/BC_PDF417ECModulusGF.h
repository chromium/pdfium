// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417ECMODULUSGF_H_
#define _BC_PDF417ECMODULUSGF_H_
class CBC_PDF417ECModulusPoly;
class CBC_PDF417Common;
class CBC_PDF417ECModulusGF;
class CBC_PDF417ECModulusGF : public CFX_Object
{
public:
    CBC_PDF417ECModulusGF(FX_INT32 modulus, FX_INT32 generator, FX_INT32 &e);
    virtual ~CBC_PDF417ECModulusGF();
    static void Initialize(FX_INT32 &e);
    static void Finalize();
    CBC_PDF417ECModulusPoly* getZero();
    CBC_PDF417ECModulusPoly* getOne();
    CBC_PDF417ECModulusPoly* buildMonomial(FX_INT32 degree, FX_INT32 coefficient, FX_INT32 &e);
    FX_INT32 add(FX_INT32 a, FX_INT32 b);
    FX_INT32 subtract(FX_INT32 a, FX_INT32 b);
    FX_INT32 exp(FX_INT32 a);
    FX_INT32 log(FX_INT32 a, FX_INT32 &e);
    FX_INT32 inverse(FX_INT32 a, FX_INT32 &e);
    FX_INT32 multiply(FX_INT32 a, FX_INT32 b);
    FX_INT32 getSize();
    static CBC_PDF417ECModulusGF* PDF417_GF;
private:
    CFX_Int32Array m_expTable;
    CFX_Int32Array m_logTable;
    CBC_PDF417ECModulusPoly* m_zero;
    CBC_PDF417ECModulusPoly* m_one;
    FX_INT32 m_modulus;
};
#endif

// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417ECERRORCORRECTION_H_
#define _BC_PDF417ECERRORCORRECTION_H_
class CBC_PDF417ECModulusGF;
class CBC_PDF417ECModulusPoly;
class CBC_PDF417ECErrorCorrection;
class CBC_PDF417ECErrorCorrection : public CFX_Object
{
public:
    CBC_PDF417ECErrorCorrection();
    virtual ~CBC_PDF417ECErrorCorrection();
    static void Initialize(FX_INT32 &e);
    static void Finalize();
    static FX_INT32 decode(CFX_Int32Array &received, FX_INT32 numECCodewords, CFX_Int32Array &erasures, FX_INT32 &e);
private:
    static CBC_PDF417ECModulusGF* m_field;
    static CFX_PtrArray* runEuclideanAlgorithm(CBC_PDF417ECModulusPoly* a, CBC_PDF417ECModulusPoly* b, FX_INT32 R, FX_INT32 &e);
    static CFX_Int32Array* findErrorLocations(CBC_PDF417ECModulusPoly* errorLocator, FX_INT32 &e);
    static CFX_Int32Array* findErrorMagnitudes(CBC_PDF417ECModulusPoly* errorEvaluator, CBC_PDF417ECModulusPoly* errorLocator, CFX_Int32Array &errorLocations, FX_INT32 &e);
};
#endif

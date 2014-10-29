// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_REEDSOLOMONDECODER_H_
#define _BC_REEDSOLOMONDECODER_H_
class CBC_ReedSolomonGF256;
class CBC_ReedSolomonGF256Poly;
class CBC_ReedSolomonDecoder;
class CBC_ReedSolomonDecoder : public CFX_Object
{
private:
    CBC_ReedSolomonGF256 * m_field;
public:
    CBC_ReedSolomonDecoder(CBC_ReedSolomonGF256 * field);
    virtual ~CBC_ReedSolomonDecoder();
    void Decode(CFX_Int32Array* received, FX_INT32 twoS, FX_INT32 &e);
    CFX_PtrArray* RunEuclideanAlgorithm(CBC_ReedSolomonGF256Poly* a, CBC_ReedSolomonGF256Poly* b, FX_INT32 R, FX_INT32 &e);
    CFX_Int32Array* FindErrorLocations(CBC_ReedSolomonGF256Poly* errorLocator, FX_INT32 &e);
    CFX_Int32Array* FindErrorMagnitudes(CBC_ReedSolomonGF256Poly* errorEvaluator, CFX_Int32Array* errorLocations, FX_BOOL dataMatrix, FX_INT32 &e);
};
#endif

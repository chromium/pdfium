// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_REEDSOLOMONDECODER_H_
#define _BC_REEDSOLOMONDECODER_H_
class CBC_ReedSolomonGF256;
class CBC_ReedSolomonGF256Poly;
class CBC_ReedSolomonDecoder {
 private:
  CBC_ReedSolomonGF256* m_field;

 public:
  CBC_ReedSolomonDecoder(CBC_ReedSolomonGF256* field);
  virtual ~CBC_ReedSolomonDecoder();
  void Decode(CFX_Int32Array* received, int32_t twoS, int32_t& e);
  CFX_PtrArray* RunEuclideanAlgorithm(CBC_ReedSolomonGF256Poly* a,
                                      CBC_ReedSolomonGF256Poly* b,
                                      int32_t R,
                                      int32_t& e);
  CFX_Int32Array* FindErrorLocations(CBC_ReedSolomonGF256Poly* errorLocator,
                                     int32_t& e);
  CFX_Int32Array* FindErrorMagnitudes(CBC_ReedSolomonGF256Poly* errorEvaluator,
                                      CFX_Int32Array* errorLocations,
                                      FX_BOOL dataMatrix,
                                      int32_t& e);
};
#endif

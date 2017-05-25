// Copyright 2017 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_FUNCTION_H_
#define CORE_FPDFAPI_PAGE_CPDF_FUNCTION_H_

#include <memory>

class CPDF_ExpIntFunc;
class CPDF_Object;
class CPDF_SampledFunc;
class CPDF_StitchFunc;

class CPDF_Function {
 public:
  enum class Type {
    kTypeInvalid = -1,
    kType0Sampled = 0,
    kType2ExpotentialInterpolation = 2,
    kType3Stitching = 3,
    kType4PostScript = 4,
  };

  static std::unique_ptr<CPDF_Function> Load(CPDF_Object* pFuncObj);
  static Type IntegerToFunctionType(int iType);

  virtual ~CPDF_Function();

  bool Call(float* inputs,
            uint32_t ninputs,
            float* results,
            int* nresults) const;
  uint32_t CountInputs() const { return m_nInputs; }
  uint32_t CountOutputs() const { return m_nOutputs; }
  float GetDomain(int i) const { return m_pDomains[i]; }
  float GetRange(int i) const { return m_pRanges[i]; }
  float Interpolate(float x,
                    float xmin,
                    float xmax,
                    float ymin,
                    float ymax) const;

  const CPDF_SampledFunc* ToSampledFunc() const;
  const CPDF_ExpIntFunc* ToExpIntFunc() const;
  const CPDF_StitchFunc* ToStitchFunc() const;

 protected:
  explicit CPDF_Function(Type type);

  bool Init(CPDF_Object* pObj);
  virtual bool v_Init(CPDF_Object* pObj) = 0;
  virtual bool v_Call(float* inputs, float* results) const = 0;

  uint32_t m_nInputs;
  uint32_t m_nOutputs;
  float* m_pDomains;
  float* m_pRanges;
  const Type m_Type;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_FUNCTION_H_

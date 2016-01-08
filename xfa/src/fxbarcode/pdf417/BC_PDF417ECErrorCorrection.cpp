// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com
// Original code is licensed as follows:
/*
 * Copyright 2012 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include "xfa/src/fxbarcode/barcode.h"
#include "BC_PDF417Common.h"
#include "BC_PDF417ECModulusPoly.h"
#include "BC_PDF417ECModulusGF.h"
#include "BC_PDF417ECErrorCorrection.h"
CBC_PDF417ECModulusGF* CBC_PDF417ECErrorCorrection::m_field = NULL;
void CBC_PDF417ECErrorCorrection::Initialize(int32_t& e) {
  m_field =
      new CBC_PDF417ECModulusGF(CBC_PDF417Common::NUMBER_OF_CODEWORDS, 3, e);
}
void CBC_PDF417ECErrorCorrection::Finalize() {
  delete m_field;
}
CBC_PDF417ECErrorCorrection::CBC_PDF417ECErrorCorrection() {}
CBC_PDF417ECErrorCorrection::~CBC_PDF417ECErrorCorrection() {}
int32_t CBC_PDF417ECErrorCorrection::decode(CFX_Int32Array& received,
                                            int32_t numECCodewords,
                                            CFX_Int32Array& erasures,
                                            int32_t& e) {
  CBC_PDF417ECModulusPoly poly(m_field, received, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, -1);
  CFX_Int32Array S;
  S.SetSize(numECCodewords);
  FX_BOOL error = FALSE;
  for (int32_t l = numECCodewords; l > 0; l--) {
    int32_t eval = poly.evaluateAt(m_field->exp(l));
    S[numECCodewords - l] = eval;
    if (eval != 0) {
      error = TRUE;
    }
  }
  if (!error) {
    return 0;
  }
  CBC_PDF417ECModulusPoly* syndrome =
      new CBC_PDF417ECModulusPoly(m_field, S, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, -1);
  CBC_PDF417ECModulusPoly* buildmonomial =
      m_field->buildMonomial(numECCodewords, 1, e);
  if (e != BCExceptionNO) {
    delete syndrome;
    return -1;
  }
  CFX_PtrArray* sigmaOmega =
      runEuclideanAlgorithm(buildmonomial, syndrome, numECCodewords, e);
  delete buildmonomial;
  delete syndrome;
  BC_EXCEPTION_CHECK_ReturnValue(e, -1);
  CBC_PDF417ECModulusPoly* sigma =
      (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(0);
  CBC_PDF417ECModulusPoly* omega =
      (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(1);
  CFX_Int32Array* errorLocations = findErrorLocations(sigma, e);
  if (e != BCExceptionNO) {
    for (int32_t i = 0; i < sigmaOmega->GetSize(); i++) {
      delete (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(i);
    }
    sigmaOmega->RemoveAll();
    delete sigmaOmega;
    return -1;
  }
  CFX_Int32Array* errorMagnitudes =
      findErrorMagnitudes(omega, sigma, *errorLocations, e);
  if (e != BCExceptionNO) {
    delete errorLocations;
    for (int32_t i = 0; i < sigmaOmega->GetSize(); i++) {
      delete (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(i);
    }
    sigmaOmega->RemoveAll();
    delete sigmaOmega;
    return -1;
  }
  for (int32_t i = 0; i < errorLocations->GetSize(); i++) {
    int32_t log = m_field->log(errorLocations->GetAt(i), e);
    ;
    BC_EXCEPTION_CHECK_ReturnValue(e, -1);
    int32_t position = received.GetSize() - 1 - log;
    if (position < 0) {
      e = BCExceptionChecksumException;
      delete errorLocations;
      delete errorMagnitudes;
      for (int32_t j = 0; j < sigmaOmega->GetSize(); j++) {
        delete (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(j);
      }
      sigmaOmega->RemoveAll();
      delete sigmaOmega;
      return -1;
    }
    received[position] =
        m_field->subtract(received[position], errorMagnitudes->GetAt(i));
  }
  int32_t result = errorLocations->GetSize();
  delete errorLocations;
  delete errorMagnitudes;
  for (int32_t k = 0; k < sigmaOmega->GetSize(); k++) {
    delete (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(k);
  }
  sigmaOmega->RemoveAll();
  delete sigmaOmega;
  return result;
}
CFX_PtrArray* CBC_PDF417ECErrorCorrection::runEuclideanAlgorithm(
    CBC_PDF417ECModulusPoly* a,
    CBC_PDF417ECModulusPoly* b,
    int32_t R,
    int32_t& e) {
  if (a->getDegree() < b->getDegree()) {
    CBC_PDF417ECModulusPoly* temp = a;
    a = b;
    b = temp;
  }
  CBC_PDF417ECModulusPoly* rLast = a;
  CBC_PDF417ECModulusPoly* r = b;
  CBC_PDF417ECModulusPoly* tLast = m_field->getZero();
  CBC_PDF417ECModulusPoly* t = m_field->getOne();
  CBC_PDF417ECModulusPoly* qtemp = NULL;
  CBC_PDF417ECModulusPoly* rtemp = NULL;
  CBC_PDF417ECModulusPoly* ttemp = NULL;
  int32_t i = 0;
  int32_t j = 0;
  int32_t m = 0;
  int32_t n = 0;
  while (r->getDegree() >= R / 2) {
    CBC_PDF417ECModulusPoly* rLastLast = rLast;
    CBC_PDF417ECModulusPoly* tLastLast = tLast;
    rLast = r;
    tLast = t;
    m = i;
    n = j;
    if (rLast->isZero()) {
      e = BCExceptionChecksumException;
      if (qtemp) {
        delete qtemp;
      }
      if (rtemp) {
        delete rtemp;
      }
      if (ttemp) {
        delete ttemp;
      }
      return NULL;
    }
    r = rLastLast;
    CBC_PDF417ECModulusPoly* q = m_field->getZero();
    int32_t denominatorLeadingTerm = rLast->getCoefficient(rLast->getDegree());
    int32_t dltInverse = m_field->inverse(denominatorLeadingTerm, e);
    if (e != BCExceptionNO) {
      if (qtemp) {
        delete qtemp;
      }
      if (rtemp) {
        delete rtemp;
      }
      if (ttemp) {
        delete ttemp;
      }
      return NULL;
    }
    while (r->getDegree() >= rLast->getDegree() && !r->isZero()) {
      int32_t degreeDiff = r->getDegree() - rLast->getDegree();
      int32_t scale =
          m_field->multiply(r->getCoefficient(r->getDegree()), dltInverse);
      CBC_PDF417ECModulusPoly* buildmonomial =
          m_field->buildMonomial(degreeDiff, scale, e);
      if (e != BCExceptionNO) {
        if (qtemp) {
          delete qtemp;
        }
        if (rtemp) {
          delete rtemp;
        }
        if (ttemp) {
          delete ttemp;
        }
        return NULL;
      }
      q = q->add(buildmonomial, e);
      delete buildmonomial;
      if (qtemp) {
        delete qtemp;
      }
      if (e != BCExceptionNO) {
        if (rtemp) {
          delete rtemp;
        }
        if (ttemp) {
          delete ttemp;
        }
        return NULL;
      }
      qtemp = q;
      CBC_PDF417ECModulusPoly* multiply =
          rLast->multiplyByMonomial(degreeDiff, scale, e);
      if (e != BCExceptionNO) {
        if (qtemp) {
          delete qtemp;
        }
        if (rtemp) {
          delete rtemp;
        }
        if (ttemp) {
          delete ttemp;
        }
        return NULL;
      }
      CBC_PDF417ECModulusPoly* temp = r;
      r = temp->subtract(multiply, e);
      delete multiply;
      if (m > 1 && i > m) {
        delete temp;
        temp = NULL;
      }
      if (e != BCExceptionNO) {
        if (qtemp) {
          delete qtemp;
        }
        if (rtemp) {
          delete rtemp;
        }
        if (ttemp) {
          delete ttemp;
        }
        return NULL;
      }
      rtemp = r;
      i = m + 1;
    }
    ttemp = q->multiply(tLast, e);
    if (qtemp) {
      delete qtemp;
      qtemp = NULL;
    }
    if (e != BCExceptionNO) {
      if (rtemp) {
        delete rtemp;
      }
      if (ttemp) {
        delete ttemp;
      }
      return NULL;
    }
    t = ttemp->subtract(tLastLast, e);
    if (n > 1 && j > n) {
      delete tLastLast;
    }
    delete ttemp;
    if (e != BCExceptionNO) {
      if (rtemp) {
        delete rtemp;
      }
      return NULL;
    }
    ttemp = t;
    t = ttemp->negative(e);
    delete ttemp;
    if (e != BCExceptionNO) {
      if (rtemp) {
        delete rtemp;
      }
      return NULL;
    }
    ttemp = t;
    j++;
  }
  int32_t sigmaTildeAtZero = t->getCoefficient(0);
  if (sigmaTildeAtZero == 0) {
    e = BCExceptionChecksumException;
    if (rtemp) {
      delete rtemp;
    }
    if (ttemp) {
      delete ttemp;
    }
    return NULL;
  }
  int32_t inverse = m_field->inverse(sigmaTildeAtZero, e);
  if (e != BCExceptionNO) {
    if (rtemp) {
      delete rtemp;
    }
    if (ttemp) {
      delete ttemp;
    }
    return NULL;
  }
  CBC_PDF417ECModulusPoly* sigma = t->multiply(inverse, e);
  if (ttemp) {
    delete ttemp;
  }
  if (e != BCExceptionNO) {
    if (rtemp) {
      delete rtemp;
    }
    return NULL;
  }
  CBC_PDF417ECModulusPoly* omega = r->multiply(inverse, e);
  if (rtemp) {
    delete rtemp;
  }
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CFX_PtrArray* modulusPoly = new CFX_PtrArray;
  modulusPoly->Add(sigma);
  modulusPoly->Add(omega);
  return modulusPoly;
}
CFX_Int32Array* CBC_PDF417ECErrorCorrection::findErrorLocations(
    CBC_PDF417ECModulusPoly* errorLocator,
    int32_t& e) {
  int32_t numErrors = errorLocator->getDegree();
  CFX_Int32Array* result = new CFX_Int32Array;
  result->SetSize(numErrors);
  int32_t ee = 0;
  for (int32_t i = 1; i < m_field->getSize() && ee < numErrors; i++) {
    if (errorLocator->evaluateAt(i) == 0) {
      result->SetAt(ee, m_field->inverse(i, e));
      if (e != BCExceptionNO) {
        delete result;
        return NULL;
      }
      ee++;
    }
  }
  if (ee != numErrors) {
    e = BCExceptionChecksumException;
    delete result;
    return NULL;
  }
  return result;
}
CFX_Int32Array* CBC_PDF417ECErrorCorrection::findErrorMagnitudes(
    CBC_PDF417ECModulusPoly* errorEvaluator,
    CBC_PDF417ECModulusPoly* errorLocator,
    CFX_Int32Array& errorLocations,
    int32_t& e) {
  int32_t errorLocatorDegree = errorLocator->getDegree();
  CFX_Int32Array formalDerivativeCoefficients;
  formalDerivativeCoefficients.SetSize(errorLocatorDegree);
  for (int32_t l = 1; l <= errorLocatorDegree; l++) {
    formalDerivativeCoefficients[errorLocatorDegree - l] =
        m_field->multiply(l, errorLocator->getCoefficient(l));
  }
  CBC_PDF417ECModulusPoly formalDerivative(m_field,
                                           formalDerivativeCoefficients, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  int32_t s = errorLocations.GetSize();
  CFX_Int32Array* result = new CFX_Int32Array;
  result->SetSize(s);
  for (int32_t i = 0; i < s; i++) {
    int32_t xiInverse = m_field->inverse(errorLocations[i], e);
    if (e != BCExceptionNO) {
      delete result;
      return NULL;
    }
    int32_t numerator =
        m_field->subtract(0, errorEvaluator->evaluateAt(xiInverse));
    int32_t denominator =
        m_field->inverse(formalDerivative.evaluateAt(xiInverse), e);
    if (e != BCExceptionNO) {
      delete result;
      return NULL;
    }
    result->SetAt(i, m_field->multiply(numerator, denominator));
  }
  return result;
}

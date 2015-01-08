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

#include "barcode.h"
#include "include/BC_PDF417Common.h"
#include "include/BC_PDF417ECModulusPoly.h"
#include "include/BC_PDF417ECModulusGF.h"
#include "include/BC_PDF417ECErrorCorrection.h"
CBC_PDF417ECModulusGF* CBC_PDF417ECErrorCorrection::m_field = NULL;
void CBC_PDF417ECErrorCorrection::Initialize(FX_INT32 &e)
{
    m_field = FX_NEW CBC_PDF417ECModulusGF(CBC_PDF417Common::NUMBER_OF_CODEWORDS, 3, e);
}
void CBC_PDF417ECErrorCorrection::Finalize()
{
    delete m_field;
}
CBC_PDF417ECErrorCorrection::CBC_PDF417ECErrorCorrection()
{
}
CBC_PDF417ECErrorCorrection::~CBC_PDF417ECErrorCorrection()
{
}
FX_INT32 CBC_PDF417ECErrorCorrection::decode(CFX_Int32Array &received, FX_INT32 numECCodewords, CFX_Int32Array &erasures, FX_INT32 &e)
{
    CBC_PDF417ECModulusPoly poly(m_field, received, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, -1);
    CFX_Int32Array S;
    S.SetSize(numECCodewords);
    FX_BOOL error = FALSE;
    for (FX_INT32 l = numECCodewords; l > 0; l--) {
        FX_INT32 eval = poly.evaluateAt(m_field->exp(l));
        S[numECCodewords - l] = eval;
        if (eval != 0) {
            error = TRUE;
        }
    }
    if (!error) {
        return 0;
    }
    CBC_PDF417ECModulusPoly* syndrome = FX_NEW CBC_PDF417ECModulusPoly(m_field, S, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, -1);
    CBC_PDF417ECModulusPoly* buildmonomial = m_field->buildMonomial(numECCodewords, 1, e);
    if (e != BCExceptionNO) {
        delete syndrome;
        return -1;
    }
    CFX_PtrArray* sigmaOmega = runEuclideanAlgorithm(buildmonomial, syndrome, numECCodewords, e);
    delete buildmonomial;
    delete syndrome;
    BC_EXCEPTION_CHECK_ReturnValue(e, -1);
    CBC_PDF417ECModulusPoly* sigma = (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(0);
    CBC_PDF417ECModulusPoly* omega = (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(1);
    CFX_Int32Array* errorLocations = findErrorLocations(sigma, e);
    if (e != BCExceptionNO) {
        for (FX_INT32 i = 0; i < sigmaOmega->GetSize(); i++) {
            delete (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(i);
        }
        sigmaOmega->RemoveAll();
        delete sigmaOmega;
        return -1;
    }
    CFX_Int32Array* errorMagnitudes = findErrorMagnitudes(omega, sigma, *errorLocations, e);
    if (e != BCExceptionNO) {
        delete errorLocations;
        for (FX_INT32 i = 0; i < sigmaOmega->GetSize(); i++) {
            delete (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(i);
        }
        sigmaOmega->RemoveAll();
        delete sigmaOmega;
        return -1;
    }
    for (FX_INT32 i = 0; i < errorLocations->GetSize(); i++) {
        FX_INT32 log = m_field->log(errorLocations->GetAt(i), e);;
        BC_EXCEPTION_CHECK_ReturnValue(e, -1);
        FX_INT32 position = received.GetSize() - 1 - log;
        if (position < 0) {
            e = BCExceptionChecksumException;
            delete errorLocations;
            delete errorMagnitudes;
            for (FX_INT32 j = 0; j < sigmaOmega->GetSize(); j++) {
                delete (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(j);
            }
            sigmaOmega->RemoveAll();
            delete sigmaOmega;
            return -1;
        }
        received[position] = m_field->subtract(received[position], errorMagnitudes->GetAt(i));
    }
    FX_INT32 result = errorLocations->GetSize();
    delete errorLocations;
    delete errorMagnitudes;
    for (FX_INT32 k = 0; k < sigmaOmega->GetSize(); k++) {
        delete (CBC_PDF417ECModulusPoly*)sigmaOmega->GetAt(k);
    }
    sigmaOmega->RemoveAll();
    delete sigmaOmega;
    return result;
}
CFX_PtrArray* CBC_PDF417ECErrorCorrection::runEuclideanAlgorithm(CBC_PDF417ECModulusPoly* a, CBC_PDF417ECModulusPoly* b, FX_INT32 R, FX_INT32 &e)
{
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
    FX_INT32 i = 0;
    FX_INT32 j = 0;
    FX_INT32 m = 0;
    FX_INT32 n = 0;
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
        FX_INT32 denominatorLeadingTerm = rLast->getCoefficient(rLast->getDegree());
        FX_INT32 dltInverse = m_field->inverse(denominatorLeadingTerm, e);
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
            FX_INT32 degreeDiff = r->getDegree() - rLast->getDegree();
            FX_INT32 scale = m_field->multiply(r->getCoefficient(r->getDegree()), dltInverse);
            CBC_PDF417ECModulusPoly* buildmonomial = m_field->buildMonomial(degreeDiff, scale, e);
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
            CBC_PDF417ECModulusPoly* multiply  = rLast->multiplyByMonomial(degreeDiff, scale, e);
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
    FX_INT32 aa = t->getCoefficient(1);
    FX_INT32 sigmaTildeAtZero = t->getCoefficient(0);
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
    FX_INT32 inverse = m_field->inverse(sigmaTildeAtZero, e);
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
    CFX_PtrArray* modulusPoly = FX_NEW CFX_PtrArray;
    modulusPoly->Add(sigma);
    modulusPoly->Add(omega);
    return modulusPoly;
}
CFX_Int32Array* CBC_PDF417ECErrorCorrection::findErrorLocations(CBC_PDF417ECModulusPoly* errorLocator, FX_INT32 &e)
{
    FX_INT32 numErrors = errorLocator->getDegree();
    CFX_Int32Array* result = FX_NEW CFX_Int32Array;
    result->SetSize(numErrors);
    FX_INT32 ee = 0;
    for (FX_INT32 i = 1; i < m_field->getSize() && ee < numErrors; i++) {
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
CFX_Int32Array* CBC_PDF417ECErrorCorrection::findErrorMagnitudes(CBC_PDF417ECModulusPoly* errorEvaluator, CBC_PDF417ECModulusPoly* errorLocator, CFX_Int32Array &errorLocations, FX_INT32 &e)
{
    FX_INT32 errorLocatorDegree = errorLocator->getDegree();
    CFX_Int32Array formalDerivativeCoefficients;
    formalDerivativeCoefficients.SetSize(errorLocatorDegree);
    for (FX_INT32 l = 1; l <= errorLocatorDegree; l++) {
        formalDerivativeCoefficients[errorLocatorDegree - l] = m_field->multiply(l, errorLocator->getCoefficient(l));
    }
    CBC_PDF417ECModulusPoly formalDerivative(m_field, formalDerivativeCoefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    FX_INT32 s = errorLocations.GetSize();
    CFX_Int32Array* result = FX_NEW CFX_Int32Array;
    result->SetSize(s);
    for (FX_INT32 i = 0; i < s; i++) {
        FX_INT32 xiInverse = m_field->inverse(errorLocations[i], e);
        if (e != BCExceptionNO) {
            delete result;
            return NULL;
        }
        FX_INT32 numerator = m_field->subtract(0, errorEvaluator->evaluateAt(xiInverse));
        FX_INT32 denominator = m_field->inverse(formalDerivative.evaluateAt(xiInverse), e);
        if (e != BCExceptionNO) {
            delete result;
            return NULL;
        }
        result->SetAt(i, m_field->multiply(numerator, denominator));
    }
    return result;
}

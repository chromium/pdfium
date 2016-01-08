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
#include "BC_PDF417ECModulusGF.h"
#include "BC_PDF417ECModulusPoly.h"
CBC_PDF417ECModulusPoly::CBC_PDF417ECModulusPoly(CBC_PDF417ECModulusGF* field,
                                                 CFX_Int32Array& coefficients,
                                                 int32_t& e) {
  if (coefficients.GetSize() == 0) {
    e = BCExceptionIllegalArgument;
  }
  m_field = field;
  int32_t coefficientsLength = coefficients.GetSize();
  if (coefficientsLength > 1 && coefficients[0] == 0) {
    int32_t firstNonZero = 1;
    while (firstNonZero < coefficientsLength &&
           coefficients[firstNonZero] == 0) {
      firstNonZero++;
    }
    if (firstNonZero == coefficientsLength) {
      m_coefficients = field->getZero()->m_coefficients;
    } else {
      m_coefficients.SetSize(coefficientsLength - firstNonZero);
      int32_t l = 0;
      for (int32_t i = firstNonZero;
           i < firstNonZero + m_coefficients.GetSize(); i++) {
        m_coefficients.SetAt(l, coefficients.GetAt(i));
        l++;
      }
    }
  } else {
    m_coefficients.Copy(coefficients);
  }
}
CBC_PDF417ECModulusPoly::~CBC_PDF417ECModulusPoly() {}
CFX_Int32Array& CBC_PDF417ECModulusPoly::getCoefficients() {
  return m_coefficients;
}
CBC_PDF417ECModulusGF* CBC_PDF417ECModulusPoly::getField() {
  return m_field;
}
int32_t CBC_PDF417ECModulusPoly::getDegree() {
  return m_coefficients.GetSize() - 1;
}
FX_BOOL CBC_PDF417ECModulusPoly::isZero() {
  return m_coefficients[0] == 0;
}
int32_t CBC_PDF417ECModulusPoly::getCoefficient(int32_t degree) {
  return m_coefficients[m_coefficients.GetSize() - 1 - degree];
}
int32_t CBC_PDF417ECModulusPoly::evaluateAt(int32_t a) {
  if (a == 0) {
    return getCoefficient(0);
  }
  int32_t size = m_coefficients.GetSize();
  if (a == 1) {
    int32_t result = 0;
    for (int32_t l = 0; l < m_coefficients.GetSize(); l++) {
      int32_t coefficient = m_coefficients.GetAt(l);
      result = m_field->add(result, coefficient);
    }
    return result;
  }
  int32_t result = m_coefficients[0];
  for (int32_t i = 1; i < size; i++) {
    result = m_field->add(m_field->multiply(a, result), m_coefficients[i]);
  }
  return result;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::add(
    CBC_PDF417ECModulusPoly* other,
    int32_t& e) {
  CBC_PDF417ECModulusPoly* modulusPoly = NULL;
  if (isZero()) {
    modulusPoly = new CBC_PDF417ECModulusPoly(other->getField(),
                                              other->getCoefficients(), e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
  }
  if (other->isZero()) {
    modulusPoly = new CBC_PDF417ECModulusPoly(m_field, m_coefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
  }
  CFX_Int32Array smallerCoefficients;
  smallerCoefficients.Copy(m_coefficients);
  CFX_Int32Array largerCoefficients;
  largerCoefficients.Copy(other->m_coefficients);
  if (smallerCoefficients.GetSize() > largerCoefficients.GetSize()) {
    CFX_Int32Array temp;
    temp.Copy(smallerCoefficients);
    smallerCoefficients.Copy(largerCoefficients);
    largerCoefficients.Copy(temp);
  }
  CFX_Int32Array sumDiff;
  sumDiff.SetSize(largerCoefficients.GetSize());
  int32_t lengthDiff =
      largerCoefficients.GetSize() - smallerCoefficients.GetSize();
  for (int32_t l = 0; l < lengthDiff; l++) {
    sumDiff.SetAt(l, largerCoefficients.GetAt(l));
  }
  for (int32_t i = lengthDiff; i < largerCoefficients.GetSize(); i++) {
    sumDiff[i] = m_field->add(smallerCoefficients[i - lengthDiff],
                              largerCoefficients[i]);
  }
  modulusPoly = new CBC_PDF417ECModulusPoly(m_field, sumDiff, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::subtract(
    CBC_PDF417ECModulusPoly* other,
    int32_t& e) {
  CBC_PDF417ECModulusPoly* modulusPoly = NULL;
  if (other->isZero()) {
    modulusPoly = new CBC_PDF417ECModulusPoly(m_field, m_coefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
  }
  CBC_PDF417ECModulusPoly* poly = other->negative(e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  modulusPoly = add(poly, e);
  delete poly;
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::multiply(
    CBC_PDF417ECModulusPoly* other,
    int32_t& e) {
  CBC_PDF417ECModulusPoly* modulusPoly = NULL;
  if (isZero() || other->isZero()) {
    modulusPoly =
        new CBC_PDF417ECModulusPoly(m_field->getZero()->getField(),
                                    m_field->getZero()->getCoefficients(), e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
  }
  CFX_Int32Array aCoefficients;
  aCoefficients.Copy(m_coefficients);
  int32_t aLength = aCoefficients.GetSize();
  CFX_Int32Array bCoefficients;
  bCoefficients.Copy(other->m_coefficients);
  int32_t bLength = bCoefficients.GetSize();
  CFX_Int32Array product;
  product.SetSize(aLength + bLength - 1);
  for (int32_t i = 0; i < aLength; i++) {
    int32_t aCoeff = aCoefficients[i];
    for (int32_t j = 0; j < bLength; j++) {
      product[i + j] = m_field->add(
          product[i + j], m_field->multiply(aCoeff, bCoefficients[j]));
    }
  }
  modulusPoly = new CBC_PDF417ECModulusPoly(m_field, product, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::negative(int32_t& e) {
  int32_t size = m_coefficients.GetSize();
  CFX_Int32Array negativeCoefficients;
  negativeCoefficients.SetSize(size);
  for (int32_t i = 0; i < size; i++) {
    negativeCoefficients[i] = m_field->subtract(0, m_coefficients[i]);
  }
  CBC_PDF417ECModulusPoly* modulusPoly =
      new CBC_PDF417ECModulusPoly(m_field, negativeCoefficients, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::multiply(int32_t scalar,
                                                           int32_t& e) {
  CBC_PDF417ECModulusPoly* modulusPoly = NULL;
  if (scalar == 0) {
    modulusPoly =
        new CBC_PDF417ECModulusPoly(m_field->getZero()->getField(),
                                    m_field->getZero()->getCoefficients(), e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
  }
  if (scalar == 1) {
    modulusPoly = new CBC_PDF417ECModulusPoly(m_field, m_coefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
  }
  int32_t size = m_coefficients.GetSize();
  CFX_Int32Array product;
  product.SetSize(size);
  for (int32_t i = 0; i < size; i++) {
    product[i] = m_field->multiply(m_coefficients[i], scalar);
  }
  modulusPoly = new CBC_PDF417ECModulusPoly(m_field, product, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return modulusPoly;
}
CBC_PDF417ECModulusPoly* CBC_PDF417ECModulusPoly::multiplyByMonomial(
    int32_t degree,
    int32_t coefficient,
    int32_t& e) {
  if (degree < 0) {
    e = BCExceptionIllegalArgument;
    return NULL;
  }
  CBC_PDF417ECModulusPoly* modulusPoly = NULL;
  if (coefficient == 0) {
    modulusPoly = new CBC_PDF417ECModulusPoly(
        m_field->getZero()->m_field, m_field->getZero()->m_coefficients, e);
    BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
    return modulusPoly;
  }
  int32_t size = m_coefficients.GetSize();
  CFX_Int32Array product;
  product.SetSize(size + degree);
  for (int32_t i = 0; i < size; i++) {
    product[i] = m_field->multiply(m_coefficients[i], coefficient);
  }
  modulusPoly = new CBC_PDF417ECModulusPoly(m_field, product, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  return modulusPoly;
}
CFX_PtrArray* CBC_PDF417ECModulusPoly::divide(CBC_PDF417ECModulusPoly* other,
                                              int32_t& e) {
  if (other->isZero()) {
    e = BCExceptionDivideByZero;
    return NULL;
  }
  CBC_PDF417ECModulusPoly* quotient = new CBC_PDF417ECModulusPoly(
      m_field->getZero()->m_field, m_field->getZero()->m_coefficients, e);
  BC_EXCEPTION_CHECK_ReturnValue(e, NULL);
  CBC_PDF417ECModulusPoly* remainder =
      new CBC_PDF417ECModulusPoly(m_field, m_coefficients, e);
  if (e != BCExceptionNO) {
    delete quotient;
    return NULL;
  }
  int32_t denominatorLeadingTerm = other->getCoefficient(other->getDegree());
  int32_t inverseDenominatorLeadingTerm =
      m_field->inverse(denominatorLeadingTerm, e);
  if (e != BCExceptionNO) {
    delete quotient;
    delete remainder;
    return NULL;
  }
  while (remainder->getDegree() >= other->getDegree() && !remainder->isZero()) {
    int32_t degreeDifference = remainder->getDegree() - other->getDegree();
    int32_t scale =
        m_field->multiply(remainder->getCoefficient(remainder->getDegree()),
                          inverseDenominatorLeadingTerm);
    CBC_PDF417ECModulusPoly* term =
        other->multiplyByMonomial(degreeDifference, scale, e);
    if (e != BCExceptionNO) {
      delete quotient;
      delete remainder;
      return NULL;
    }
    CBC_PDF417ECModulusPoly* iterationQuotient =
        m_field->buildMonomial(degreeDifference, scale, e);
    if (e != BCExceptionNO) {
      delete quotient;
      delete remainder;
      delete term;
      return NULL;
    }
    CBC_PDF417ECModulusPoly* temp = quotient;
    quotient = temp->add(iterationQuotient, e);
    delete iterationQuotient;
    delete temp;
    if (e != BCExceptionNO) {
      delete remainder;
      return NULL;
    }
    temp = remainder;
    remainder = temp->subtract(term, e);
    delete term;
    delete temp;
    if (e != BCExceptionNO) {
      delete quotient;
      return NULL;
    }
  }
  CFX_PtrArray* modulusPoly = new CFX_PtrArray;
  modulusPoly->Add(quotient);
  modulusPoly->Add(remainder);
  return modulusPoly;
}
CFX_ByteString CBC_PDF417ECModulusPoly::toString() {
  CFX_ByteString result;
  for (int32_t degree = getDegree(); degree >= 0; degree--) {
    int32_t coefficient = getCoefficient(degree);
    if (coefficient != 0) {
      if (coefficient < 0) {
        result += " - ";
        coefficient = -coefficient;
      } else {
        if (result.GetLength() > 0) {
          result += " + ";
        }
      }
      if (degree == 0 || coefficient != 1) {
        result += coefficient;
      }
      if (degree != 0) {
        if (degree == 1) {
          result += 'x';
        } else {
          result += "x^";
          result += degree;
        }
      }
    }
  }
  return result;
}

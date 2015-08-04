// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _BC_PDF417ECMODULUSGF_H_
#define _BC_PDF417ECMODULUSGF_H_
class CBC_PDF417ECModulusPoly;
class CBC_PDF417Common;
class CBC_PDF417ECModulusGF {
 public:
  CBC_PDF417ECModulusGF(int32_t modulus, int32_t generator, int32_t& e);
  virtual ~CBC_PDF417ECModulusGF();
  static void Initialize(int32_t& e);
  static void Finalize();
  CBC_PDF417ECModulusPoly* getZero();
  CBC_PDF417ECModulusPoly* getOne();
  CBC_PDF417ECModulusPoly* buildMonomial(int32_t degree,
                                         int32_t coefficient,
                                         int32_t& e);
  int32_t add(int32_t a, int32_t b);
  int32_t subtract(int32_t a, int32_t b);
  int32_t exp(int32_t a);
  int32_t log(int32_t a, int32_t& e);
  int32_t inverse(int32_t a, int32_t& e);
  int32_t multiply(int32_t a, int32_t b);
  int32_t getSize();
  static CBC_PDF417ECModulusGF* PDF417_GF;

 private:
  CFX_Int32Array m_expTable;
  CFX_Int32Array m_logTable;
  CBC_PDF417ECModulusPoly* m_zero;
  CBC_PDF417ECModulusPoly* m_one;
  int32_t m_modulus;
};
#endif

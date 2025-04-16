// Copyright 2018 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FPDFAPI_PAGE_CPDF_TRANSPARENCY_H_
#define CORE_FPDFAPI_PAGE_CPDF_TRANSPARENCY_H_

class CPDF_Transparency {
 public:
  CPDF_Transparency();

  CPDF_Transparency(const CPDF_Transparency& other);
  CPDF_Transparency& operator=(const CPDF_Transparency& other);

  bool IsGroup() const { return group_; }
  bool IsIsolated() const { return isolated_; }

  void SetGroup() { group_ = true; }
  void SetIsolated() { isolated_ = true; }

 private:
  bool group_ = false;
  bool isolated_ = false;
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_TRANSPARENCY_H_

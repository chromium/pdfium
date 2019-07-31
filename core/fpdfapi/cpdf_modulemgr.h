// Copyright 2016 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_CPDF_MODULEMGR_H_
#define CORE_FPDFAPI_CPDF_MODULEMGR_H_

#include <memory>
#include <utility>

namespace fpdfapi {

class UnsupportedInfoAdapter {
 public:
  explicit UnsupportedInfoAdapter(void* info) : m_info(info) {}

  void* info() const { return m_info; }

 private:
  void* const m_info;
};

}  // namespace fpdfapi

class CPDF_ModuleMgr {
 public:
  static void Create();
  static void Destroy();
  static CPDF_ModuleMgr* Get();

  void SetUnsupportInfoAdapter(
      std::unique_ptr<fpdfapi::UnsupportedInfoAdapter> pAdapter) {
    m_pUnsupportInfoAdapter = std::move(pAdapter);
  }
  fpdfapi::UnsupportedInfoAdapter* GetUnsupportInfoAdapter() const {
    return m_pUnsupportInfoAdapter.get();
  }

 private:
  CPDF_ModuleMgr();
  ~CPDF_ModuleMgr();

  void InitPageModule();

  std::unique_ptr<fpdfapi::UnsupportedInfoAdapter> m_pUnsupportInfoAdapter;
};

#endif  // CORE_FPDFAPI_CPDF_MODULEMGR_H_

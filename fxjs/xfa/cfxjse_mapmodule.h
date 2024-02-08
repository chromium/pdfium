// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXJS_XFA_CFXJSE_MAPMODULE_H_
#define FXJS_XFA_CFXJSE_MAPMODULE_H_

#include <stdint.h>

#include <map>
#include <optional>

#include "core/fxcrt/widestring.h"

class CXFA_Measurement;

class CFXJSE_MapModule {
 public:
  CFXJSE_MapModule();
  ~CFXJSE_MapModule();

  CFXJSE_MapModule(const CFXJSE_MapModule& that) = delete;
  CFXJSE_MapModule& operator=(const CFXJSE_MapModule& that) = delete;

  void SetValue(uint32_t key, int32_t value);
  void SetString(uint32_t key, const WideString& wsString);
  void SetMeasurement(uint32_t key, const CXFA_Measurement& measurement);
  std::optional<int32_t> GetValue(uint32_t key) const;
  std::optional<WideString> GetString(uint32_t key) const;
  std::optional<CXFA_Measurement> GetMeasurement(uint32_t key) const;
  bool HasKey(uint32_t key) const;
  void RemoveKey(uint32_t key);
  void MergeDataFrom(const CFXJSE_MapModule* pSrc);

 private:
  // keyed by result of GetMapKey_*().
  std::map<uint32_t, int32_t> m_ValueMap;
  std::map<uint32_t, WideString> m_StringMap;
  std::map<uint32_t, CXFA_Measurement> m_MeasurementMap;
};

#endif  // FXJS_XFA_CFXJSE_MAPMODULE_H_

// Copyright 2020 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#include "fxjs/xfa/cfxjse_mapmodule.h"

#include "core/fxcrt/containers/contains.h"
#include "xfa/fxfa/parser/cxfa_measurement.h"

CFXJSE_MapModule::CFXJSE_MapModule() = default;

CFXJSE_MapModule::~CFXJSE_MapModule() = default;

void CFXJSE_MapModule::SetValue(uint32_t key, int32_t value) {
  string_map_.erase(key);
  measurement_map_.erase(key);
  value_map_[key] = value;
}

void CFXJSE_MapModule::SetString(uint32_t key, const WideString& wsString) {
  value_map_.erase(key);
  measurement_map_.erase(key);
  string_map_[key] = wsString;
}

void CFXJSE_MapModule::SetMeasurement(uint32_t key,
                                      const CXFA_Measurement& measurement) {
  value_map_.erase(key);
  string_map_.erase(key);
  measurement_map_[key] = measurement;
}

std::optional<int32_t> CFXJSE_MapModule::GetValue(uint32_t key) const {
  auto it = value_map_.find(key);
  if (it == value_map_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::optional<WideString> CFXJSE_MapModule::GetString(uint32_t key) const {
  auto it = string_map_.find(key);
  if (it == string_map_.end()) {
    return std::nullopt;
  }
  return it->second;
}

std::optional<CXFA_Measurement> CFXJSE_MapModule::GetMeasurement(
    uint32_t key) const {
  auto it = measurement_map_.find(key);
  if (it == measurement_map_.end()) {
    return std::nullopt;
  }
  return it->second;
}

bool CFXJSE_MapModule::HasKey(uint32_t key) const {
  return pdfium::Contains(value_map_, key) ||
         pdfium::Contains(string_map_, key) ||
         pdfium::Contains(measurement_map_, key);
}

void CFXJSE_MapModule::RemoveKey(uint32_t key) {
  value_map_.erase(key);
  string_map_.erase(key);
  measurement_map_.erase(key);
}

void CFXJSE_MapModule::MergeDataFrom(const CFXJSE_MapModule* pSrc) {
  for (const auto& pair : pSrc->value_map_) {
    SetValue(pair.first, pair.second);
  }

  for (const auto& pair : pSrc->string_map_) {
    SetString(pair.first, pair.second);
  }

  for (const auto& pair : pSrc->measurement_map_) {
    SetMeasurement(pair.first, pair.second);
  }
}

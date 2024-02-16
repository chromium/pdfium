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
  m_StringMap.erase(key);
  m_MeasurementMap.erase(key);
  m_ValueMap[key] = value;
}

void CFXJSE_MapModule::SetString(uint32_t key, const WideString& wsString) {
  m_ValueMap.erase(key);
  m_MeasurementMap.erase(key);
  m_StringMap[key] = wsString;
}

void CFXJSE_MapModule::SetMeasurement(uint32_t key,
                                      const CXFA_Measurement& measurement) {
  m_ValueMap.erase(key);
  m_StringMap.erase(key);
  m_MeasurementMap[key] = measurement;
}

std::optional<int32_t> CFXJSE_MapModule::GetValue(uint32_t key) const {
  auto it = m_ValueMap.find(key);
  if (it == m_ValueMap.end())
    return std::nullopt;
  return it->second;
}

std::optional<WideString> CFXJSE_MapModule::GetString(uint32_t key) const {
  auto it = m_StringMap.find(key);
  if (it == m_StringMap.end())
    return std::nullopt;
  return it->second;
}

std::optional<CXFA_Measurement> CFXJSE_MapModule::GetMeasurement(
    uint32_t key) const {
  auto it = m_MeasurementMap.find(key);
  if (it == m_MeasurementMap.end())
    return std::nullopt;
  return it->second;
}

bool CFXJSE_MapModule::HasKey(uint32_t key) const {
  return pdfium::Contains(m_ValueMap, key) ||
         pdfium::Contains(m_StringMap, key) ||
         pdfium::Contains(m_MeasurementMap, key);
}

void CFXJSE_MapModule::RemoveKey(uint32_t key) {
  m_ValueMap.erase(key);
  m_StringMap.erase(key);
  m_MeasurementMap.erase(key);
}

void CFXJSE_MapModule::MergeDataFrom(const CFXJSE_MapModule* pSrc) {
  for (const auto& pair : pSrc->m_ValueMap)
    SetValue(pair.first, pair.second);

  for (const auto& pair : pSrc->m_StringMap)
    SetString(pair.first, pair.second);

  for (const auto& pair : pSrc->m_MeasurementMap)
    SetMeasurement(pair.first, pair.second);
}

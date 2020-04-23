// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_DATAMATRIX_BC_SYMBOLINFO_H_
#define FXBARCODE_DATAMATRIX_BC_SYMBOLINFO_H_

#include "core/fxcrt/fx_string.h"
#include "core/fxcrt/fx_system.h"

class CBC_SymbolInfo {
 public:
  struct Data {
    int16_t data_capacity;
    int16_t error_codewords;
    int8_t matrix_width;
    int8_t matrix_height;
    int8_t data_regions;
    int16_t rs_block_data;
    int8_t rs_block_error;
  };

  virtual ~CBC_SymbolInfo();

  static void Initialize();
  static void Finalize();
  static void overrideSymbolSet(CBC_SymbolInfo* override);
  static const CBC_SymbolInfo* Lookup(size_t iDataCodewords,
                                      bool bAllowRectangular);

  int32_t getSymbolDataWidth() const;
  int32_t getSymbolDataHeight() const;
  int32_t getSymbolWidth() const;
  int32_t getSymbolHeight() const;
  size_t getCodewordCount() const;
  virtual size_t getInterleavedBlockCount() const;
  size_t getDataLengthForInterleavedBlock() const;
  size_t getErrorLengthForInterleavedBlock() const;

  size_t dataCapacity() const { return data_->data_capacity; }
  size_t errorCodewords() const { return data_->error_codewords; }
  int32_t matrixWidth() const { return data_->matrix_width; }
  int32_t matrixHeight() const { return data_->matrix_height; }

 protected:
  explicit CBC_SymbolInfo(const Data* data);

 private:
  int32_t getHorizontalDataRegions() const;
  int32_t getVerticalDataRegions() const;

  const Data* const data_;
  const bool rectangular_;
};

#endif  // FXBARCODE_DATAMATRIX_BC_SYMBOLINFO_H_

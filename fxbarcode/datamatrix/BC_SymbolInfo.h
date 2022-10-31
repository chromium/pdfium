// Copyright 2014 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef FXBARCODE_DATAMATRIX_BC_SYMBOLINFO_H_
#define FXBARCODE_DATAMATRIX_BC_SYMBOLINFO_H_

#include <stddef.h>
#include <stdint.h>

#include "core/fxcrt/unowned_ptr.h"

class CBC_SymbolInfo {
 public:
  struct Data {
    int16_t data_capacity;
    int16_t error_codewords;
    int16_t rs_block_data;
    int8_t rs_block_error;
    int8_t matrix_width;
    int8_t matrix_height;
    int8_t data_regions;
  };

  virtual ~CBC_SymbolInfo();

  static void Initialize();
  static void Finalize();
  static const CBC_SymbolInfo* Lookup(size_t data_codewords,
                                      bool allow_rectangular);

  int32_t GetSymbolDataWidth() const;
  int32_t GetSymbolDataHeight() const;
  int32_t GetSymbolWidth() const;
  int32_t GetSymbolHeight() const;
  virtual size_t GetInterleavedBlockCount() const;
  size_t GetDataLengthForInterleavedBlock() const;
  size_t GetErrorLengthForInterleavedBlock() const;

  size_t data_capacity() const { return data_->data_capacity; }
  size_t error_codewords() const { return data_->error_codewords; }
  int32_t matrix_width() const { return data_->matrix_width; }
  int32_t matrix_height() const { return data_->matrix_height; }

 protected:
  explicit CBC_SymbolInfo(const Data* data);

 private:
  int32_t GetHorizontalDataRegions() const;
  int32_t GetVerticalDataRegions() const;
  bool is_rectangular() const {
    return data_->matrix_width != data_->matrix_height;
  }

  UnownedPtr<const Data> const data_;
};

#endif  // FXBARCODE_DATAMATRIX_BC_SYMBOLINFO_H_

// Copyright 2016 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef CORE_FPDFAPI_PAGE_CPDF_PSENGINE_H_
#define CORE_FPDFAPI_PAGE_CPDF_PSENGINE_H_

#include <stdint.h>

#include <array>
#include <memory>
#include <vector>

#include "core/fxcrt/bytestring.h"
#include "core/fxcrt/span.h"

class CPDF_PSEngine;
class CPDF_PSProc;
class CPDF_SimpleParser;

enum PDF_PSOP : uint8_t {
  PSOP_ADD,
  PSOP_SUB,
  PSOP_MUL,
  PSOP_DIV,
  PSOP_IDIV,
  PSOP_MOD,
  PSOP_NEG,
  PSOP_ABS,
  PSOP_CEILING,
  PSOP_FLOOR,
  PSOP_ROUND,
  PSOP_TRUNCATE,
  PSOP_SQRT,
  PSOP_SIN,
  PSOP_COS,
  PSOP_ATAN,
  PSOP_EXP,
  PSOP_LN,
  PSOP_LOG,
  PSOP_CVI,
  PSOP_CVR,
  PSOP_EQ,
  PSOP_NE,
  PSOP_GT,
  PSOP_GE,
  PSOP_LT,
  PSOP_LE,
  PSOP_AND,
  PSOP_OR,
  PSOP_XOR,
  PSOP_NOT,
  PSOP_BITSHIFT,
  PSOP_TRUE,
  PSOP_FALSE,
  PSOP_IF,
  PSOP_IFELSE,
  PSOP_POP,
  PSOP_EXCH,
  PSOP_DUP,
  PSOP_COPY,
  PSOP_INDEX,
  PSOP_ROLL,
  PSOP_PROC,
  PSOP_CONST
};

class CPDF_PSOP {
 public:
  CPDF_PSOP();
  explicit CPDF_PSOP(PDF_PSOP op);
  explicit CPDF_PSOP(float value);
  ~CPDF_PSOP();

  bool Parse(CPDF_SimpleParser* parser, int depth);
  void Execute(CPDF_PSEngine* pEngine);
  float GetFloatValue() const;
  PDF_PSOP GetOp() const { return op_; }

 private:
  const PDF_PSOP op_;
  const float value_;
  std::unique_ptr<CPDF_PSProc> proc_;
};

class CPDF_PSProc {
 public:
  CPDF_PSProc();
  ~CPDF_PSProc();

  bool Parse(CPDF_SimpleParser* parser, int depth);
  bool Execute(CPDF_PSEngine* pEngine);

  // These methods are exposed for testing.
  void AddOperatorForTesting(ByteStringView word);
  const std::unique_ptr<CPDF_PSOP>& last_operator() {
    return operators_.back();
  }

 private:
  static constexpr int kMaxDepth = 128;

  void AddOperator(ByteStringView word);

  std::vector<std::unique_ptr<CPDF_PSOP>> operators_;
};

class CPDF_PSEngine {
 public:
  CPDF_PSEngine();
  ~CPDF_PSEngine();

  bool Parse(pdfium::span<const uint8_t> input);
  bool Execute();
  bool DoOperator(PDF_PSOP op);
  void Reset() { stack_count_ = 0; }
  void Push(float value);
  float Pop();
  int PopInt();
  uint32_t GetStackSize() const { return stack_count_; }

 private:
  static constexpr uint32_t kPSEngineStackSize = 100;

  uint32_t stack_count_ = 0;
  CPDF_PSProc main_proc_;
  std::array<float, kPSEngineStackSize> stack_ = {};
};

#endif  // CORE_FPDFAPI_PAGE_CPDF_PSENGINE_H_

// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef XFA_INCLUDE_FWL_CORE_FWL_ERROR_H_
#define XFA_INCLUDE_FWL_CORE_FWL_ERROR_H_

#include <stdint.h>

typedef int32_t FWL_ERR;

#define FWL_ERR_Succeeded 0
#define FWL_ERR_Indefinite -1
#define FWL_ERR_Parameter_Invalid -100
#define FWL_ERR_Property_Invalid -200
#define FWL_ERR_Intermediate_Value__Invalid -300
#define FWL_ERR_Method_Not_Supported -400
#define FWL_ERR_Out_Of_Memory -500

#endif  // XFA_INCLUDE_FWL_CORE_FWL_ERROR_H_

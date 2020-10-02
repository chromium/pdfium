// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef THIRD_PARTY_BASE_CHECK_OP_H_
#define THIRD_PARTY_BASE_CHECK_OP_H_

#include "third_party/base/check.h"

#define CHECK_EQ(x, y) CHECK((x) == (y))
#define CHECK_NE(x, y) CHECK((x) != (y))
#define DCHECK_EQ(x, y) DCHECK((x) == (y))
#define DCHECK_NE(x, y) DCHECK((x) != (y))

#endif  // THIRD_PARTY_BASE_CHECK_OP_H_

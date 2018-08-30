// Copyright 2018 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef TESTING_PSEUDO_RETAINABLE_H_
#define TESTING_PSEUDO_RETAINABLE_H_

class PseudoRetainable {
 public:
  PseudoRetainable() = default;
  void Retain() { ++retain_count_; }
  void Release() {
    if (++release_count_ == retain_count_)
      alive_ = false;
  }
  bool alive() const { return alive_; }
  int retain_count() const { return retain_count_; }
  int release_count() const { return release_count_; }

 private:
  bool alive_ = true;
  int retain_count_ = 0;
  int release_count_ = 0;
};

#endif  // TESTING_PSEUDO_RETAINABLE_H_

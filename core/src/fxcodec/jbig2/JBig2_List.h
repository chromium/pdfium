// Copyright 2014 PDFium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

// Original code copyright 2014 Foxit Software Inc. http://www.foxitsoftware.com

#ifndef _JBIG2_LIST_H_
#define _JBIG2_LIST_H_

#include <vector>

template <class TYPE>
class CJBig2_List {
 public:
  CJBig2_List() {}

  ~CJBig2_List() {
    clear();
  }

  void clear() {
    for (size_t i = 0; i < m_vector.size(); ++i)
      delete m_vector[i];
    m_vector.clear();
  }

  void push_back(TYPE* pItem) { m_vector.push_back(pItem); }

  size_t size() const { return m_vector.size(); }
  void resize(size_t count) { m_vector.resize(count); }

  TYPE* get(size_t index) { return m_vector[index]; }

  TYPE* back() { return m_vector.back(); }

 private:
  std::vector<TYPE*> m_vector;
};

#endif

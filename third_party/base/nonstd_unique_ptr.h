// Copyright 2013 Google Inc. All Rights Reserved.
//
// Redistribution and use in source and binary forms, with or without
// modification, are permitted provided that the following conditions are
// met:
//
//     * Redistributions of source code must retain the above copyright
// notice, this list of conditions and the following disclaimer.
//     * Redistributions in binary form must reproduce the above
// copyright notice, this list of conditions and the following disclaimer
// in the documentation and/or other materials provided with the
// distribution.
//     * Neither the name of Google Inc. nor the names of its
// contributors may be used to endorse or promote products derived from
// this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
// LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR
// A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT
// OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
// SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT
// LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
// DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
// (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
// OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.

// This is a copy of breakpad's standalone scoped_ptr, which has been
// renamed to nonstd::unique_ptr, and from which more complicated classes
// have been removed. The reset() method has also been tweaked to more
// closely match c++11, and an implicit conversion to bool has been added.

// Scopers help you manage ownership of a pointer, helping you easily manage the
// a pointer within a scope, and automatically destroying the pointer at the
// end of a scope.
//
// A unique_ptr<T> is like a T*, except that the destructor of unique_ptr<T>
// automatically deletes the pointer it holds (if any).
// That is, unique_ptr<T> owns the T object that it points to.
// Like a T*, a unique_ptr<T> may hold either NULL or a pointer to a T object.
// Also like T*, unique_ptr<T> is thread-compatible, and once you
// dereference it, you get the thread safety guarantees of T.
//
// Example usage (unique_ptr):
//   {
//     unique_ptr<Foo> foo(new Foo("wee"));
//   }  // foo goes out of scope, releasing the pointer with it.
//
//   {
//     unique_ptr<Foo> foo;          // No pointer managed.
//     foo.reset(new Foo("wee"));    // Now a pointer is managed.
//     foo.reset(new Foo("wee2"));   // Foo("wee") was destroyed.
//     foo.reset(new Foo("wee3"));   // Foo("wee2") was destroyed.
//     foo->Method();                // Foo::Method() called.
//     foo.get()->Method();          // Foo::Method() called.
//     SomeFunc(foo.release());      // SomeFunc takes ownership, foo no longer
//                                   // manages a pointer.
//     foo.reset(new Foo("wee4"));   // foo manages a pointer again.
//     foo.reset();                  // Foo("wee4") destroyed, foo no longer
//                                   // manages a pointer.
//   }  // foo wasn't managing a pointer, so nothing was destroyed.
//
// The size of a unique_ptr is small: sizeof(unique_ptr<C>) == sizeof(C*)

#ifndef NONSTD_UNIQUE_PTR_H_
#define NONSTD_UNIQUE_PTR_H_

// This is an implementation designed to match the anticipated future TR2
// implementation of the unique_ptr class.

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

namespace nonstd {

// Common implementation for both pointers to elements and pointers to
// arrays. These are differentiated below based on the need to invoke
// delete vs. delete[] as appropriate.
template <class C>
class unique_ptr_base {
 public:

  // The element type
  typedef C element_type;

  explicit unique_ptr_base(C* p) : ptr_(p) { }

  // Accessors to get the owned object.
  // operator* and operator-> will assert() if there is no current object.
  C& operator*() const {
    assert(ptr_ != NULL);
    return *ptr_;
  }
  C* operator->() const  {
    assert(ptr_ != NULL);
    return ptr_;
  }
  C* get() const { return ptr_; }

  // Comparison operators.
  // These return whether two unique_ptr refer to the same object, not just to
  // two different but equal objects.
  bool operator==(C* p) const { return ptr_ == p; }
  bool operator!=(C* p) const { return ptr_ != p; }

  // Swap two scoped pointers.
  void swap(unique_ptr_base& p2) {
    C* tmp = ptr_;
    ptr_ = p2.ptr_;
    p2.ptr_ = tmp;
  }

  // Release a pointer.
  // The return value is the current pointer held by this object.
  // If this object holds a NULL pointer, the return value is NULL.
  // After this operation, this object will hold a NULL pointer,
  // and will not own the object any more.
  C* release() {
    C* retVal = ptr_;
    ptr_ = NULL;
    return retVal;
  }

  // Allow promotion to bool for conditional statements.
  operator bool() const { return ptr_ != NULL; }

 protected:
  C* ptr_;
};

// Implementation for ordinary pointers using delete.
template <class C>
class unique_ptr : public unique_ptr_base<C> {
 public:
  using unique_ptr_base<C>::ptr_;

  // Constructor. Defaults to initializing with NULL. There is no way
  // to create an uninitialized unique_ptr. The input parameter must be
  // allocated with new (not new[] - see below).
  explicit unique_ptr(C* p = NULL) : unique_ptr_base<C>(p) { }

  // Destructor.  If there is a C object, delete it.
  // We don't need to test ptr_ == NULL because C++ does that for us.
  ~unique_ptr() {
    enum { type_must_be_complete = sizeof(C) };
    delete ptr_;
  }

  // Reset.  Deletes the current owned object, if any.
  // Then takes ownership of a new object, if given.
  // this->reset(this->get()) works.
  void reset(C* p = NULL) {
    if (p != ptr_) {
      enum { type_must_be_complete = sizeof(C) };
      C* old_ptr = ptr_;
      ptr_ = p;
      delete old_ptr;
    }
  }

private:
  // Forbid comparison of unique_ptr types.  If C2 != C, it totally doesn't
  // make sense, and if C2 == C, it still doesn't make sense because you should
  // never have the same object owned by two different unique_ptrs.
  template <class C2> bool operator==(unique_ptr<C2> const& p2) const;
  template <class C2> bool operator!=(unique_ptr<C2> const& p2) const;

  // Disallow evil constructors
  unique_ptr(const unique_ptr&);
  void operator=(const unique_ptr&);
};

// Specialization for arrays using delete[].
template <class C>
class unique_ptr<C[]> : public unique_ptr_base<C> {
 public:
  using unique_ptr_base<C>::ptr_;

  // Constructor. Defaults to initializing with NULL. There is no way
  // to create an uninitialized unique_ptr. The input parameter must be
  // allocated with new[] (not new - see above).
  explicit unique_ptr(C* p = NULL) : unique_ptr_base<C>(p) { }

  // Destructor.  If there is a C object, delete it.
  // We don't need to test ptr_ == NULL because C++ does that for us.
  ~unique_ptr() {
    enum { type_must_be_complete = sizeof(C) };
    delete[] ptr_;
  }

  // Reset.  Deletes the current owned object, if any.
  // Then takes ownership of a new object, if given.
  // this->reset(this->get()) works.
  void reset(C* p = NULL) {
    if (p != ptr_) {
      enum { type_must_be_complete = sizeof(C) };
      C* old_ptr = ptr_;
      ptr_ = p;
      delete[] old_ptr;
    }
  }

  // Support indexing since it is holding array.
  C& operator[] (size_t i) { return ptr_[i]; }

private:
  // Forbid comparison of unique_ptr types.  If C2 != C, it totally doesn't
  // make sense, and if C2 == C, it still doesn't make sense because you should
  // never have the same object owned by two different unique_ptrs.
  template <class C2> bool operator==(unique_ptr<C2> const& p2) const;
  template <class C2> bool operator!=(unique_ptr<C2> const& p2) const;

  // Disallow evil constructors
  unique_ptr(const unique_ptr&);
  void operator=(const unique_ptr&);
};

// Free functions
template <class C>
void swap(unique_ptr<C>& p1, unique_ptr<C>& p2) {
  p1.swap(p2);
}

template <class C>
bool operator==(C* p1, const unique_ptr<C>& p2) {
  return p1 == p2.get();
}

template <class C>
bool operator!=(C* p1, const unique_ptr<C>& p2) {
  return p1 != p2.get();
}

}  // namespace nonstd

#endif  // NONSTD_UNIQUE_PTR_H_

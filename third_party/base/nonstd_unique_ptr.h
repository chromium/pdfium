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

// This is an implementation designed to match the anticipated future C++11
// implementation of the unique_ptr class.

#include <assert.h>
#include <stddef.h>
#include <stdlib.h>

#include <ostream>

#include "template_util.h"

namespace nonstd {

// Replacement for move, but doesn't allow things that are already
// rvalue references.
template <class T>
T&& move(T& t) {
  return static_cast<T&&>(t);
}

// Function object which deletes its parameter, which must be a pointer.
// If C is an array type, invokes 'delete[]' on the parameter; otherwise,
// invokes 'delete'. The default deleter for unique_ptr<T>.
template <class T>
struct DefaultDeleter {
  DefaultDeleter() {}
  template <typename U>
  DefaultDeleter(const DefaultDeleter<U>& other) {
    // IMPLEMENTATION NOTE: C++11 20.7.1.1.2p2 only provides this constructor
    // if U* is implicitly convertible to T* and U is not an array type.
    //
    // Correct implementation should use SFINAE to disable this
    // constructor. However, since there are no other 1-argument constructors,
    // using a static_assert() based on is_convertible<> and requiring
    // complete types is simpler and will cause compile failures for equivalent
    // misuses.
    //
    // Note, the is_convertible<U*, T*> check also ensures that U is not an
    // array. T is guaranteed to be a non-array, so any U* where U is an array
    // cannot convert to T*.
    enum { T_must_be_complete = sizeof(T) };
    enum { U_must_be_complete = sizeof(U) };
    static_assert((pdfium::base::is_convertible<U*, T*>::value),
                  "U_ptr_must_implicitly_convert_to_T_ptr");
  }
  inline void operator()(T* ptr) const {
    enum { type_must_be_complete = sizeof(T) };
    delete ptr;
  }
};

// Specialization of DefaultDeleter for array types.
template <class T>
struct DefaultDeleter<T[]> {
  inline void operator()(T* ptr) const {
    enum { type_must_be_complete = sizeof(T) };
    delete[] ptr;
  }

 private:
  // Disable this operator for any U != T because it is undefined to execute
  // an array delete when the static type of the array mismatches the dynamic
  // type.
  //
  // References:
  //   C++98 [expr.delete]p3
  //   http://cplusplus.github.com/LWG/lwg-defects.html#938
  template <typename U>
  void operator()(U* array) const;
};

template <class T, int n>
struct DefaultDeleter<T[n]> {
  // Never allow someone to declare something like unique_ptr<int[10]>.
  static_assert(sizeof(T) == -1, "do_not_use_array_with_size_as_type");
};

namespace internal {

// Common implementation for both pointers to elements and pointers to
// arrays. These are differentiated below based on the need to invoke
// delete vs. delete[] as appropriate.
template <class C, class D>
class unique_ptr_base {
 public:
  // The element type
  typedef C element_type;

  explicit unique_ptr_base(C* p) : data_(p) {}

  // Initializer for deleters that have data parameters.
  unique_ptr_base(C* p, const D& d) : data_(p, d) {}

  // Move constructor.
  unique_ptr_base(unique_ptr_base<C, D>&& that)
      : data_(that.release(), that.get_deleter()) {}

  ~unique_ptr_base() {
    enum { type_must_be_complete = sizeof(C) };
    if (data_.ptr != nullptr) {
      // Not using get_deleter() saves one function call in non-optimized
      // builds.
      static_cast<D&>(data_)(data_.ptr);
    }
  }

  void reset(C* p = nullptr) {
    C* old = data_.ptr;
    data_.ptr = p;
    if (old != nullptr)
      static_cast<D&>(data_)(old);
  }

  C* get() const { return data_.ptr; }
  D& get_deleter() { return data_; }
  const D& get_deleter() const { return data_; }

  // Comparison operators.
  // These return whether two unique_ptr refer to the same object, not just to
  // two different but equal objects.
  bool operator==(C* p) const { return data_.ptr == p; }
  bool operator!=(C* p) const { return data_.ptr != p; }

  // Swap two unique pointers.
  void swap(unique_ptr_base& p2) {
    Data tmp = data_;
    data_ = p2.data_;
    p2.data_ = tmp;
  }

  // Release a pointer.
  // The return value is the current pointer held by this object.
  // If this object holds a NULL pointer, the return value is NULL.
  // After this operation, this object will hold a NULL pointer,
  // and will not own the object any more.
  C* release() {
    C* ptr = data_.ptr;
    data_.ptr = nullptr;
    return ptr;
  }

  // Allow promotion to bool for conditional statements.
  explicit operator bool() const { return data_.ptr != nullptr; }

 protected:
  // Use the empty base class optimization to allow us to have a D
  // member, while avoiding any space overhead for it when D is an
  // empty class.  See e.g. http://www.cantrip.org/emptyopt.html for a good
  // discussion of this technique.
  struct Data : public D {
    explicit Data(C* ptr_in) : ptr(ptr_in) {}
    Data(C* ptr_in, const D& other) : D(other), ptr(ptr_in) {}
    C* ptr;
  };

  Data data_;
};

}  // namespace internal

// Implementation for ordinary pointers using delete.
template <class C, class D = DefaultDeleter<C>>
class unique_ptr : public internal::unique_ptr_base<C, D> {
 public:
  // Constructor.  Defaults to initializing with nullptr.
  unique_ptr() : internal::unique_ptr_base<C, D>(nullptr) {}

  // Constructor.  Takes ownership of p.
  explicit unique_ptr(C* p) : internal::unique_ptr_base<C, D>(p) {}

  // Constructor.  Allows initialization of a stateful deleter.
  unique_ptr(C* p, const D& d) : internal::unique_ptr_base<C, D>(p, d) {}

  // Constructor.  Allows construction from a nullptr.
  unique_ptr(decltype(nullptr)) : internal::unique_ptr_base<C, D>(nullptr) {}

  // Move constructor.
  unique_ptr(unique_ptr&& that)
      : internal::unique_ptr_base<C, D>(nonstd::move(that)) {}

  // operator=.  Allows assignment from a nullptr. Deletes the currently owned
  // object, if any.
  unique_ptr& operator=(decltype(nullptr)) {
    this->reset();
    return *this;
  }

  // Move assignment.
  unique_ptr<C>& operator=(unique_ptr<C>&& that) {
    this->reset(that.release());
    return *this;
  }

  // Accessors to get the owned object.
  // operator* and operator-> will assert() if there is no current object.
  C& operator*() const {
    assert(this->data_.ptr != nullptr);
    return *this->data_.ptr;
  }
  C* operator->() const {
    assert(this->data_.ptr != nullptr);
    return this->data_.ptr;
  }

  // Comparison operators.
  // These return whether two unique_ptr refer to the same object, not just to
  // two different but equal objects.
  bool operator==(const C* p) const { return this->get() == p; }
  bool operator!=(const C* p) const { return this->get() != p; }

 private:
  // Disallow evil constructors. It doesn't make sense to make a copy of
  // something that's allegedly unique.
  unique_ptr(const unique_ptr&) = delete;
  void operator=(const unique_ptr&) = delete;

  // Forbid comparison of unique_ptr types.  If U != C, it totally
  // doesn't make sense, and if U == C, it still doesn't make sense
  // because you should never have the same object owned by two different
  // unique_ptrs.
  template <class U>
  bool operator==(unique_ptr<U> const& p2) const;
  template <class U>
  bool operator!=(unique_ptr<U> const& p2) const;
};

// Specialization for arrays using delete[].
template <class C, class D>
class unique_ptr<C[], D> : public internal::unique_ptr_base<C, D> {
 public:
  // Constructor.  Defaults to initializing with nullptr.
  unique_ptr() : internal::unique_ptr_base<C, D>(nullptr) {}

  // Constructor. Stores the given array. Note that the argument's type
  // must exactly match T*. In particular:
  // - it cannot be a pointer to a type derived from T, because it is
  //   inherently unsafe in the general case to access an array through a
  //   pointer whose dynamic type does not match its static type (eg., if
  //   T and the derived types had different sizes access would be
  //   incorrectly calculated). Deletion is also always undefined
  //   (C++98 [expr.delete]p3). If you're doing this, fix your code.
  // - it cannot be const-qualified differently from T per unique_ptr spec
  //   (http://cplusplus.github.com/LWG/lwg-active.html#2118). Users wanting
  //   to work around this may use const_cast<const T*>().
  explicit unique_ptr(C* p) : internal::unique_ptr_base<C, D>(p) {}

  // Constructor.  Allows construction from a nullptr.
  unique_ptr(decltype(nullptr)) : internal::unique_ptr_base<C, D>(nullptr) {}

  // Move constructor.
  unique_ptr(unique_ptr&& that)
      : internal::unique_ptr_base<C, D>(nonstd::move(that)) {}

  // operator=.  Allows assignment from a nullptr. Deletes the currently owned
  // array, if any.
  unique_ptr& operator=(decltype(nullptr)) {
    this->reset();
    return *this;
  }

  // Move assignment.
  unique_ptr<C>& operator=(unique_ptr<C>&& that) {
    this->reset(that.release());
    return *this;
  }

  // Reset.  Deletes the currently owned array, if any.
  // Then takes ownership of a new object, if given.
  void reset(C* array = nullptr) {
    static_cast<internal::unique_ptr_base<C, D>*>(this)->reset(array);
  }

  // Support indexing since it is holding array.
  C& operator[](size_t i) { return this->data_.ptr[i]; }

  // Comparison operators.
  // These return whether two unique_ptr refer to the same object, not just to
  // two different but equal objects.
  bool operator==(C* array) const { return this->get() == array; }
  bool operator!=(C* array) const { return this->get() != array; }

 private:
  // Disable initialization from any type other than element_type*, by
  // providing a constructor that matches such an initialization, but is
  // private and has no definition. This is disabled because it is not safe to
  // call delete[] on an array whose static type does not match its dynamic
  // type.
  template <typename U>
  explicit unique_ptr(U* array);
  explicit unique_ptr(int disallow_construction_from_null);

  // Disable reset() from any type other than element_type*, for the same
  // reasons as the constructor above.
  template <typename U>
  void reset(U* array);
  void reset(int disallow_reset_from_null);

  // Disallow evil constructors.  It doesn't make sense to make a copy of
  // something that's allegedly unique.
  unique_ptr(const unique_ptr&) = delete;
  void operator=(const unique_ptr&) = delete;

  // Forbid comparison of unique_ptr types.  If U != C, it totally
  // doesn't make sense, and if U == C, it still doesn't make sense
  // because you should never have the same object owned by two different
  // unique_ptrs.
  template <class U>
  bool operator==(unique_ptr<U> const& p2) const;
  template <class U>
  bool operator!=(unique_ptr<U> const& p2) const;
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

template <typename T>
std::ostream& operator<<(std::ostream& out, const unique_ptr<T>& p) {
  return out << p.get();
}

}  // namespace nonstd

#endif  // NONSTD_UNIQUE_PTR_H_

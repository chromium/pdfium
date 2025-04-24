// Copyright 2024 The PDFium Authors
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#ifndef CORE_FXCRT_COMPILER_SPECIFIC_H_
#define CORE_FXCRT_COMPILER_SPECIFIC_H_

#include "build/build_config.h"

// A wrapper around `__has_cpp_attribute`, in case this is seen by
// a C (not C++) compiler, say.
#if defined(__has_cpp_attribute)
#define HAS_CPP_ATTRIBUTE(x) __has_cpp_attribute(x)
#else
#define HAS_CPP_ATTRIBUTE(x) 0
#endif

// A wrapper around `__has_attribute`, similar to HAS_CPP_ATTRIBUTE.
#if defined(__has_attribute)
#define HAS_ATTRIBUTE(x) __has_attribute(x)
#else
#define HAS_ATTRIBUTE(x) 0
#endif

// Annotate a function indicating it should not be inlined.
// Use like:
//   NOINLINE void DoStuff() { ... }
#if defined(__clang__) && HAS_ATTRIBUTE(noinline)
#define NOINLINE [[clang::noinline]]
#elif defined(COMPILER_GCC) && HAS_ATTRIBUTE(noinline)
#define NOINLINE __attribute__((noinline))
#else
#define NOINLINE
#endif

// Macro for hinting that an expression is likely to be false.
#if !defined(UNLIKELY)
#if defined(COMPILER_GCC) || defined(__clang__)
#define UNLIKELY(x) __builtin_expect(!!(x), 0)
#else
#define UNLIKELY(x) (x)
#endif  // defined(COMPILER_GCC)
#endif  // !defined(UNLIKELY)

#if !defined(LIKELY)
#if defined(COMPILER_GCC) || defined(__clang__)
#define LIKELY(x) __builtin_expect(!!(x), 1)
#else
#define LIKELY(x) (x)
#endif  // defined(COMPILER_GCC)
#endif  // !defined(LIKELY)

// Marks a type as being eligible for the "trivial" ABI despite having a
// non-trivial destructor or copy/move constructor. Such types can be relocated
// after construction by simply copying their memory, which makes them eligible
// to be passed in registers. The canonical example is std::unique_ptr.
//
// Use with caution; this has some subtle effects on constructor/destructor
// ordering and will be very incorrect if the type relies on its address
// remaining constant. When used as a function argument (by value), the value
// may be constructed in the caller's stack frame, passed in a register, and
// then used and destructed in the callee's stack frame. A similar thing can
// occur when values are returned.
//
// TRIVIAL_ABI is not needed for types which have a trivial destructor and
// copy/move constructors, such as base::TimeTicks and other POD.
//
// It is also not likely to be effective on types too large to be passed in one
// or two registers on typical target ABIs.
//
// See also:
//   https://clang.llvm.org/docs/AttributeReference.html#trivial-abi
//   https://libcxx.llvm.org/docs/DesignDocs/UniquePtrTrivialAbi.html
#if defined(__clang__) && HAS_ATTRIBUTE(trivial_abi)
#define TRIVIAL_ABI [[clang::trivial_abi]]
#else
#define TRIVIAL_ABI
#endif

#if defined(__clang__)
#define GSL_POINTER [[gsl::Pointer]]
#else
#define GSL_POINTER
#endif

// Annotates a pointer or reference parameter or return value for a member
// function as having lifetime intertwined with the instance on which the
// function is called. For parameters, the function is assumed to store the
// value into the called-on object, so if the referred-to object is later
// destroyed, the called-on object is also considered to be dangling. For return
// values, the value is assumed to point into the called-on object, so if that
// object is destroyed, the returned value is also considered to be dangling.
// Useful to diagnose some cases of lifetime errors.
//
// See also:
//   https://clang.llvm.org/docs/AttributeReference.html#lifetimebound
//
// Usage:
// ```
//   struct S {
//      S(int* p LIFETIME_BOUND);
//      int* Get() LIFETIME_BOUND;
//   };
//   S Func1() {
//     int i = 0;
//     // The following return will not compile; diagnosed as returning address
//     // of a stack object.
//     return S(&i);
//   }
//   int* Func2(int* p) {
//     // The following return will not compile; diagnosed as returning address
//     // of a local temporary.
//     return S(p).Get();
//   }
// ```
#if HAS_CPP_ATTRIBUTE(clang::lifetimebound)
#define LIFETIME_BOUND [[clang::lifetimebound]]
#else
#define LIFETIME_BOUND
#endif

#if defined(__clang__) && HAS_ATTRIBUTE(unsafe_buffer_usage)
#define UNSAFE_BUFFER_USAGE [[clang::unsafe_buffer_usage]]
#else
#define UNSAFE_BUFFER_USAGE
#endif

// clang-format off
// Formatting is off so that we can put each _Pragma on its own line, as
// recommended by the gcc docs.
#if defined(UNSAFE_BUFFERS_BUILD)
#define UNSAFE_BUFFERS(...)                  \
  _Pragma("clang unsafe_buffer_usage begin") \
  __VA_ARGS__                                \
  _Pragma("clang unsafe_buffer_usage end")
#else
#define UNSAFE_BUFFERS(...) __VA_ARGS__
#endif
// clang-format on

// Like UNSAFE_BUFFERS(), but indicates there is a TODO() task to
// investigate safety,
// TODO(crbug.com/pdfium/2155): remove all usage.
#define UNSAFE_TODO(...) UNSAFE_BUFFERS(__VA_ARGS__)

// Annotates a function restricting its availability based on compile-time
// information in the evaluated context. Useful to convert runtime errors to
// compile-time errors if functions' arguments are always known at compile time.
//
// SFINAE and `requires` clauses can restrict function availability based on the
// unevaluated context (type information and syntactic correctness). This
// provides a similar capability based on the evaluated context (variable
// values). If the condition fails, or cannot be determined at compile time, the
// function is excluded from the overload set.
//
// Some use cases could be satisfied without this by marking the function
// `consteval` and breaking compile when the condition fails (e.g. via
// `CHECK()`/`assert()`). However, `ENABLE_IF_ATTR()` is generally superior:
//   - Not all desired functions can be made `consteval`; e.g. most
//     constructors.
//   - The error message in the macro case is clearer and more actionable.
//   - `ENABLE_IF_ATTR()` interacts better with template metaprogramming.
//
// See also:
//   https://clang.llvm.org/docs/AttributeReference.html#enable-if
//   https://github.com/chromium/subspace/issues/266
//
// Usage:
// ```
//   void NotConsteval(int a) {
//     assert(a > 0);
//   }
//   consteval void WithoutEnableIf(int a) {
//     assert(a > 0);
//   }
//   void WithEnableIf(int a) ENABLE_IF_ATTR(a > 0, "arg must be positive") {}
//   void Func(int i) {
//     // Compiles; assertion fails at runtime.
//     NotConsteval(-1);
//
//     // Will not compile; diagnosed as not a constant expression.
//     WithoutEnableIf(-1);
//
//     // Will not compile; diagnosed as no matching function call with
//     // "note: candidate disabled: arg must be positive".
//     WithEnableIf(-1);
//
//     // Will not compile (same reason). Marking `Func()` as
//     // `ENABLE_IF_ATTR(i > 0, ...)` will not help; the compiler's analysis is
//     // not sufficiently sophisticated to propagate this constraint.
//     WithEnableIf(i);
//   }
// ```
#if HAS_ATTRIBUTE(enable_if)
#define ENABLE_IF_ATTR(cond, msg) __attribute__((enable_if(cond, msg)))
#else
#define ENABLE_IF_ATTR(cond, msg)
#endif

#endif  // CORE_FXCRT_COMPILER_SPECIFIC_H_

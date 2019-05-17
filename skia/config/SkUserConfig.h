
/*
 * Copyright 2006 The Android Open Source Project
 *
 * Use of this source code is governed by a BSD-style license that can be
 * found in the LICENSE file.
 */


#ifndef SKIA_CONFIG_SKUSERCONFIG_H_
#define SKIA_CONFIG_SKUSERCONFIG_H_

/*  SkTypes.h, the root of the public header files, does the following trick:

    #include "SkPreConfig.h"
    #include "SkUserConfig.h"
    #include "SkPostConfig.h"

    SkPreConfig.h runs first, and it is responsible for initializing certain
    skia defines.

    SkPostConfig.h runs last, and its job is to just check that the final
    defines are consistent (i.e. that we don't have mutually conflicting
    defines).

    SkUserConfig.h (this file) runs in the middle. It gets to change or augment
    the list of flags initially set in preconfig, and then postconfig checks
    that everything still makes sense.

    Below are optional defines that add, subtract, or change default behavior
    in Skia. Your port can locally edit this file to enable/disable flags as
    you choose, or these can be delared on your command line (i.e. -Dfoo).

    By default, this include file will always default to having all of the flags
    commented out, so including it will have no effect.
*/

///////////////////////////////////////////////////////////////////////////////

/*  Skia has lots of debug-only code. Often this is just null checks or other
    parameter checking, but sometimes it can be quite intrusive (e.g. check that
    each 32bit pixel is in premultiplied form). This code can be very useful
    during development, but will slow things down in a shipping product.

    By default, these mutually exclusive flags are defined in SkPreConfig.h,
    based on the presence or absence of NDEBUG, but that decision can be changed
    here.
 */
// #define SK_DEBUG
// #define SK_RELEASE

/*  Skia has certain debug-only code that is extremely intensive even for debug
    builds.  This code is useful for diagnosing specific issues, but is not
    generally applicable, therefore it must be explicitly enabled to avoid
    the performance impact. By default these flags are undefined, but can be
    enabled by uncommenting them below.
 */
// #define SK_DEBUG_GLYPH_CACHE
// #define SK_DEBUG_PATH

/*  preconfig will have attempted to determine the endianness of the system,
    but you can change these mutually exclusive flags here.
 */
// #define SK_CPU_BENDIAN
// #define SK_CPU_LENDIAN

/*  Most compilers use the same bit endianness for bit flags in a byte as the
    system byte endianness, and this is the default. If for some reason this
    needs to be overridden, specify which of the mutually exclusive flags to
    use. For example, some atom processors in certain configurations have big
    endian byte order but little endian bit orders.
*/
// #define SK_UINT8_BITFIELD_BENDIAN
// #define SK_UINT8_BITFIELD_LENDIAN


/*  To write debug messages to a console, skia will call SkDebugf(...) following
    printf conventions (e.g. const char* format, ...). If you want to redirect
    this to something other than printf, define yours here
 */
// #define SkDebugf(...)  MyFunction(__VA_ARGS__)

/*
 *  To specify a different default font cache limit, define this. If this is
 *  undefined, skia will use a built-in value.
 */
// #define SK_DEFAULT_FONT_CACHE_LIMIT   (1024 * 1024)

/*
 *  To specify the default size of the image cache, undefine this and set it to
 *  the desired value (in bytes). SkGraphics.h as a runtime API to set this
 *  value as well. If this is undefined, a built-in value will be used.
 */
// #define SK_DEFAULT_IMAGE_CACHE_LIMIT (1024 * 1024)

/*  Define this to set the upper limit for text to support LCD. Values that
    are very large increase the cost in the font cache and draw slower, without
    improving readability. If this is undefined, Skia will use its default
    value (e.g. 48)
 */
// #define SK_MAX_SIZE_FOR_LCDTEXT     48

/*  Change the ordering to work in X windows.
 */
// #ifdef SK_SAMPLES_FOR_X
//         #define SK_R32_SHIFT    16
//         #define SK_G32_SHIFT    8
//         #define SK_B32_SHIFT    0
//         #define SK_A32_SHIFT    24
// #endif


/* Determines whether to build code that supports the GPU backend. Some classes
   that are not GPU-specific, such as SkShader subclasses, have optional code
   that is used allows them to interact with the GPU backend. If you'd like to
   omit this code set SK_SUPPORT_GPU to 0. This also allows you to omit the gpu
   directories from your include search path when you're not building the GPU
   backend. Defaults to 1 (build the GPU code).
 */
// #define SK_SUPPORT_GPU 1

/* Skia makes use of histogram logging macros to trace the frequency of
 * events. By default, Skia provides no-op versions of these macros.
 * Skia consumers can provide their own definitions of these macros to
 * integrate with their histogram collection backend.
 */
// #define SK_HISTOGRAM_BOOLEAN(name, value)
// #define SK_HISTOGRAM_ENUMERATION(name, value, boundary_value)

// ===== Begin Chrome-specific definitions =====

#define SK_MSCALAR_IS_FLOAT
#undef SK_MSCALAR_IS_DOUBLE

#define GR_MAX_OFFSCREEN_AA_DIM 512

// Log the file and line number for assertions.
#define SkDebugf(...) SkDebugf_FileLine(__FILE__, __LINE__, __VA_ARGS__)
SK_API void SkDebugf_FileLine(const char* file,
                              int line,
                              const char* format,
                              ...);

#if !defined(ANDROID)  // On Android, we use the skia default settings.
#define SK_A32_SHIFT 24
#define SK_R32_SHIFT 16
#define SK_G32_SHIFT 8
#define SK_B32_SHIFT 0
#endif

#if defined(SK_BUILD_FOR_WIN32)

#define SK_BUILD_FOR_WIN

// Skia uses this deprecated bzero function to fill zeros into a string.
#define bzero(str, len) memset(str, 0, len)

#elif defined(SK_BUILD_FOR_MAC)

#define SK_CPU_LENDIAN
#undef SK_CPU_BENDIAN

#elif defined(SK_BUILD_FOR_UNIX) || defined(SK_BUILD_FOR_ANDROID)

// Prefer FreeType's emboldening algorithm to Skia's
// TODO: skia used to just use hairline, but has improved since then, so
// we should revisit this choice...
#define SK_USE_FREETYPE_EMBOLDEN

#if defined(SK_BUILD_FOR_UNIX) && defined(SK_CPU_BENDIAN)
// Above we set the order for ARGB channels in registers. I suspect that, on
// big endian machines, you can keep this the same and everything will work.
// The in-memory order will be different, of course, but as long as everything
// is reading memory as words rather than bytes, it will all work. However, if
// you find that colours are messed up I thought that I would leave a helpful
// locator for you. Also see the comments in
// base/gfx/bitmap_platform_device_linux.h
#error Read the comment at this location
#endif

#endif

// These flags are no longer defined in Skia, but we have them (temporarily)
// until we update our call-sites (typically these are for API changes).
//
// Remove these as we update our sites.
//
#ifndef SK_SUPPORT_LEGACY_GETTOPDEVICE
#define SK_SUPPORT_LEGACY_GETTOPDEVICE
#endif

#ifndef SK_SUPPORT_EXOTIC_CLIPOPS
#define SK_SUPPORT_EXOTIC_CLIPOPS
#endif

#ifndef SK_SUPPORT_LEGACY_GETDEVICE
#define SK_SUPPORT_LEGACY_GETDEVICE
#endif

// Workaround for poor anisotropic mipmap quality,
// pending Skia ripmap support.
// (https://bugs.chromium.org/p/skia/issues/detail?id=4863)
#ifndef SK_SUPPORT_LEGACY_ANISOTROPIC_MIPMAP_SCALE
#define SK_SUPPORT_LEGACY_ANISOTROPIC_MIPMAP_SCALE
#endif

#ifndef SK_SUPPORT_LEGACY_REFENCODEDDATA_NOCTX
#define SK_SUPPORT_LEGACY_REFENCODEDDATA_NOCTX
#endif

#ifndef SK_IGNORE_ETC1_SUPPORT
#define SK_IGNORE_ETC1_SUPPORT
#endif

#ifndef SK_IGNORE_GPU_DITHER
#define SK_IGNORE_GPU_DITHER
#endif

#ifndef SK_SUPPORT_LEGACY_EVAL_CUBIC
#define SK_SUPPORT_LEGACY_EVAL_CUBIC
#endif

///////////////////////// Imported from BUILD.gn

/* In some places Skia can use static initializers for global initialization,
 *  or fall back to lazy runtime initialization. Chrome always wants the latter.
 */
#define SK_ALLOW_STATIC_GLOBAL_INITIALIZERS 0

/* This flag forces Skia not to use typographic metrics with GDI.
 */
#define SK_GDI_ALWAYS_USE_TEXTMETRICS_FOR_FONT_METRICS

#define SK_IGNORE_BLURRED_RRECT_OPT
#define SK_USE_DISCARDABLE_SCALEDIMAGECACHE
#define SK_WILL_NEVER_DRAW_PERSPECTIVE_TEXT

#define SK_ATTR_DEPRECATED SK_NOTHING_ARG1
#define SK_ENABLE_INST_COUNT 0
#define GR_GL_CUSTOM_SETUP_HEADER "GrGLConfig_chrome.h"

// Blink layout tests are baselined to Clang optimizing through the UB in
// SkDivBits.
#define SK_SUPPORT_LEGACY_DIVBITS_UB

// mtklein's fiddling with Src / SrcOver.  Will rebaseline these only once when
// done.
#define SK_SUPPORT_LEGACY_X86_BLITS

#define SK_DISABLE_TILE_IMAGE_FILTER_OPTIMIZATION

// ===== End Chrome-specific definitions =====

#endif  // SKIA_CONFIG_SKUSERCONFIG_H_

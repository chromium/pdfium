PROJECT (pdfium)
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.12)

set(USE_BINUTILS_WORKAROUND ON CACHE BOOL "use gcc-ar and gcc-ranlib instead ar and ranlib (is needed for LTO)")
if(USE_BINUTILS_WORKAROUND)
    set(CMAKE_AR gcc-ar)
    set(CMAKE_RANLIB gcc-ranlib)
endif(USE_BINUTILS_WORKAROUND)

if(APPLE OR LINUX)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -fno-exceptions -Wc++0x-compat  -fvisibility=hidden")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fvisibility=hidden")
endif(APPLE OR LINUX)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/public)

if(WIN32)
  add_definitions(-D_WIN32 -DWIN32_LEAN_AND_MEAN -DNOMINMAX)
  file(GLOB PLATFORM_SOURCES
    "core/fxge/win32/*.cpp"
    "core/fxge/win32/*.h"
  )
elseif(APPLE)
  file(GLOB PLATFORM_SOURCES
    "core/fxge/apple/*.cpp"
    "core/fxge/apple/*.h"
  )
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -pthread")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC -pthread")
  set(CMAKE_SHARED_LINKER_FLAGS "${CMAKE_LINKER_FLAGS} -Wl,-z,defs")
elseif(UNIX)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
  set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -fPIC")
  file(GLOB PLATFORM_SOURCES
    "core/fxge/linux/*.cpp"
    "core/fxge/linix/*.h"
  )
endif(WIN32)

set(USE_STATIC_LIBRARY ON CACHE BOOL "build as static library. Otherwise - shared")

#hieroglyphs support
set(USE_GB1_MAPS OFF CACHE BOOL "use GB1 maps")
set(USE_CNS1_MAPS OFF CACHE BOOL "use CNS1 maps")
set(USE_JAPAN1_MAPS OFF CACHE BOOL "use JAPAN1 maps")
set(USE_KOREA_MAPS OFF CACHE BOOL "use KOREA maps")
# set(USE_JPX_DECODER OFF CACHE BOOL "use jpx-decoder")

if(USE_CNS1_MAPS)
  add_definitions(-DUSE_CNS1_MAPS)
  set(FPDFAPI_CNS1_SOURCES
core/ffpdfapi/cmaps/CNS1/Adobe-CNS1-UCS2_5.cpp
core/ffpdfapi/cmaps/CNS1/B5pc-H_0.cpp
core/ffpdfapi/cmaps/CNS1/B5pc-V_0.cpp
core/ffpdfapi/cmaps/CNS1/cmaps_cns1.cpp
core/ffpdfapi/cmaps/CNS1/cmaps_cns1.h
core/ffpdfapi/cmaps/CNS1/CNS-EUC-H_0.cpp
core/ffpdfapi/cmaps/CNS1/CNS-EUC-V_0.cpp
core/ffpdfapi/cmaps/CNS1/ETen-B5-H_0.cpp
core/ffpdfapi/cmaps/CNS1/ETen-B5-V_0.cpp
core/ffpdfapi/cmaps/CNS1/ETenms-B5-H_0.cpp
core/ffpdfapi/cmaps/CNS1/ETenms-B5-V_0.cpp
core/ffpdfapi/cmaps/CNS1/HKscs-B5-H_5.cpp
core/ffpdfapi/cmaps/CNS1/HKscs-B5-V_5.cpp
core/ffpdfapi/cmaps/CNS1/UniCNS-UCS2-H_3.cpp
core/ffpdfapi/cmaps/CNS1/UniCNS-UCS2-V_3.cpp
core/ffpdfapi/cmaps/CNS1/UniCNS-UTF16-H_0.cpp
)
endif(USE_CNS1_MAPS)

if(USE_GB1_MAPS)
  add_definitions(-DUSE_GB1_MAPS)
  set(FPDFAPI_GB1_SOURCES
core/ffpdfapi/cmaps/GB1/Adobe-GB1-UCS2_5.cpp
core/ffpdfapi/cmaps/GB1/cmaps_gb1.cpp
core/ffpdfapi/cmaps/GB1/cmaps_gb1.h
core/ffpdfapi/cmaps/GB1/GB-EUC-H_0.cpp
core/ffpdfapi/cmaps/GB1/GB-EUC-V_0.cpp
core/ffpdfapi/cmaps/GB1/GBK2K-H_5.cpp
core/ffpdfapi/cmaps/GB1/GBK2K-V_5.cpp
core/ffpdfapi/cmaps/GB1/GBK-EUC-H_2.cpp
core/ffpdfapi/cmaps/GB1/GBK-EUC-V_2.cpp
core/ffpdfapi/cmaps/GB1/GBKp-EUC-H_2.cpp
core/ffpdfapi/cmaps/GB1/GBKp-EUC-V_2.cpp
core/ffpdfapi/cmaps/GB1/GBpc-EUC-H_0.cpp
core/ffpdfapi/cmaps/GB1/GBpc-EUC-V_0.cpp
core/ffpdfapi/cmaps/GB1/UniGB-UCS2-H_4.cpp
core/ffpdfapi/cmaps/GB1/UniGB-UCS2-V_4.cpp )
endif(USE_GB1_MAPS)

if(USE_JAPAN1_MAPS)
  set(FPDFAPI_JAPAN1_SOURCES
core/ffpdfapi/cmaps/Japan1/83pv-RKSJ-H_1.cpp
core/ffpdfapi/cmaps/Japan1/90msp-RKSJ-H_2.cpp
core/ffpdfapi/cmaps/Japan1/90msp-RKSJ-V_2.cpp
core/ffpdfapi/cmaps/Japan1/90ms-RKSJ-H_2.cpp
core/ffpdfapi/cmaps/Japan1/90ms-RKSJ-V_2.cpp
core/ffpdfapi/cmaps/Japan1/90pv-RKSJ-H_1.cpp
core/ffpdfapi/cmaps/Japan1/Add-RKSJ-H_1.cpp
core/ffpdfapi/cmaps/Japan1/Add-RKSJ-V_1.cpp
core/ffpdfapi/cmaps/Japan1/Adobe-Japan1-UCS2_4.cpp
core/ffpdfapi/cmaps/Japan1/cmaps_japan1.cpp
core/ffpdfapi/cmaps/Japan1/cmaps_japan1.h
core/ffpdfapi/cmaps/Japan1/EUC-H_1.cpp
core/ffpdfapi/cmaps/Japan1/EUC-V_1.cpp
core/ffpdfapi/cmaps/Japan1/Ext-RKSJ-H_2.cpp
core/ffpdfapi/cmaps/Japan1/Ext-RKSJ-V_2.cpp
core/ffpdfapi/cmaps/Japan1/H_1.cpp
core/ffpdfapi/cmaps/Japan1/UniJIS-UCS2-H_4.cpp
core/ffpdfapi/cmaps/Japan1/UniJIS-UCS2-HW-H_4.cpp
core/ffpdfapi/cmaps/Japan1/UniJIS-UCS2-HW-V_4.cpp
core/ffpdfapi/cmaps/Japan1/UniJIS-UCS2-V_4.cpp
core/ffpdfapi/cmaps/Japan1/V_1.cpp  )
  add_definitions(-DUSE_JAPAN1_MAPS)
endif(USE_JAPAN1_MAPS)

if(USE_KOREA_MAPS)
  add_definitions(-DUSE_KOREA_MAPS)
  set(FPDFAPI_KOREA_SOURCES
core/ffpdfapi/cmaps/Korea1/Adobe-Korea1-UCS2_2.cpp
core/ffpdfapi/cmaps/Korea1/cmaps_korea1.cpp
core/ffpdfapi/cmaps/Korea1/cmaps_korea1.h
core/ffpdfapi/cmaps/Korea1/KSC-EUC-H_0.cpp
core/ffpdfapi/cmaps/Korea1/KSC-EUC-V_0.cpp
core/ffpdfapi/cmaps/Korea1/KSCms-UHC-H_1.cpp
core/ffpdfapi/cmaps/Korea1/KSCms-UHC-HW-H_1.cpp
core/ffpdfapi/cmaps/Korea1/KSCms-UHC-HW-V_1.cpp
core/ffpdfapi/cmaps/Korea1/KSCms-UHC-V_1.cpp
core/ffpdfapi/cmaps/Korea1/KSCpc-EUC-H_0.cpp
core/ffpdfapi/cmaps/Korea1/UniKS-UCS2-H_1.cpp
core/ffpdfapi/cmaps/Korea1/UniKS-UCS2-V_1.cpp
core/ffpdfapi/cmaps/Korea1/UniKS-UTF16-H_0.cpp
  )
endif(USE_KOREA_MAPS)
find_package(Freetype REQUIRED)

#Use own jbig2 in anytime
add_subdirectory(core/fxcodec/jbig2)

######################### FPDFSDK #############################################
set(FPDFSDK_SOURCES
    "fpdfsdk/cpdfsdk_actionhandler.cpp"
    "fpdfsdk/cpdfsdk_actionhandler.h"
    "fpdfsdk/cpdfsdk_annot.cpp"
    "fpdfsdk/cpdfsdk_annot.h"
    "fpdfsdk/cpdfsdk_annothandlermgr.cpp"
    "fpdfsdk/cpdfsdk_annothandlermgr.h"
    "fpdfsdk/cpdfsdk_annotiteration.cpp"
    "fpdfsdk/cpdfsdk_annotiteration.h"
    "fpdfsdk/cpdfsdk_annotiterator.cpp"
    "fpdfsdk/cpdfsdk_annotiterator.h"
    "fpdfsdk/cpdfsdk_appstream.cpp"
    "fpdfsdk/cpdfsdk_appstream.h"
    "fpdfsdk/cpdfsdk_baannot.cpp"
    "fpdfsdk/cpdfsdk_baannot.h"
    "fpdfsdk/cpdfsdk_baannothandler.cpp"
    "fpdfsdk/cpdfsdk_baannothandler.h"
    "fpdfsdk/cpdfsdk_customaccess.cpp"
    "fpdfsdk/cpdfsdk_customaccess.h"
    "fpdfsdk/cpdfsdk_filewriteadapter.cpp"
    "fpdfsdk/cpdfsdk_filewriteadapter.h"
    "fpdfsdk/cpdfsdk_formfillenvironment.cpp"
    "fpdfsdk/cpdfsdk_formfillenvironment.h"
    "fpdfsdk/cpdfsdk_helpers.cpp"
    "fpdfsdk/cpdfsdk_helpers.h"
    "fpdfsdk/cpdfsdk_interactiveform.cpp"
    "fpdfsdk/cpdfsdk_interactiveform.h"
    "fpdfsdk/cpdfsdk_pageview.cpp"
    "fpdfsdk/cpdfsdk_pageview.h"
    "fpdfsdk/cpdfsdk_pauseadapter.cpp"
    "fpdfsdk/cpdfsdk_pauseadapter.h"
    "fpdfsdk/cpdfsdk_renderpage.cpp"
    "fpdfsdk/cpdfsdk_renderpage.h"
    "fpdfsdk/cpdfsdk_widget.cpp"
    "fpdfsdk/cpdfsdk_widget.h"
    "fpdfsdk/cpdfsdk_widgethandler.cpp"
    "fpdfsdk/cpdfsdk_widgethandler.h"
    "fpdfsdk/fpdf_annot.cpp"
    "fpdfsdk/fpdf_attachment.cpp"
    "fpdfsdk/fpdf_catalog.cpp"
    "fpdfsdk/fpdf_dataavail.cpp"
    "fpdfsdk/fpdf_doc.cpp"
    "fpdfsdk/fpdf_editimg.cpp"
    "fpdfsdk/fpdf_editpage.cpp"
    "fpdfsdk/fpdf_editpath.cpp"
    "fpdfsdk/fpdf_edittext.cpp"
    "fpdfsdk/fpdf_ext.cpp"
    "fpdfsdk/fpdf_flatten.cpp"
    "fpdfsdk/fpdf_formfill.cpp"
    "fpdfsdk/fpdf_javascript.cpp"
    "fpdfsdk/fpdf_ppo.cpp"
    "fpdfsdk/fpdf_progressive.cpp"
    "fpdfsdk/fpdf_save.cpp"
    "fpdfsdk/fpdf_searchex.cpp"
    "fpdfsdk/fpdf_signature.cpp"
    "fpdfsdk/fpdf_structtree.cpp"
    "fpdfsdk/fpdf_sysfontinfo.cpp"
    "fpdfsdk/fpdf_text.cpp"
    "fpdfsdk/fpdf_thumbnail.cpp"
    "fpdfsdk/fpdf_transformpage.cpp"
    "fpdfsdk/fpdf_view.cpp"
    "fpdfsdk/ipdfsdk_annothandler.cpp"
    "fpdfsdk/ipdfsdk_annothandler.h"
    )
######################## FDRM #############################################
set(FDRM_SOURCES
core/fdrm/fx_crypt_aes.cpp
core/fdrm/fx_crypt.cpp
core/fdrm/fx_crypt.h
core/fdrm/fx_crypt_sha.cpp
#core/fdrm/crypt/fx_crypt_unittest.cpp
)

######################## FPDFDOC ###########################################
set(FPDFDOC_SOURCES
core/fpdfdoc/cpdf_aaction.cpp
core/fpdfdoc/cpdf_aaction.h
core/fpdfdoc/cpdf_action.cpp
#core/fpdfdoc/cpdf_actionfields.cpp
#core/fpdfdoc/cpdf_actionfields.h
core/fpdfdoc/cpdf_action.h
core/fpdfdoc/cpdf_annot.cpp
core/fpdfdoc/cpdf_annot.h
core/fpdfdoc/cpdf_annotlist.cpp
core/fpdfdoc/cpdf_annotlist.h
core/fpdfdoc/cpdf_apsettings.cpp
core/fpdfdoc/cpdf_apsettings.h
core/fpdfdoc/cpdf_bookmark.cpp
core/fpdfdoc/cpdf_bookmark.h
core/fpdfdoc/cpdf_bookmarktree.cpp
core/fpdfdoc/cpdf_bookmarktree.h
core/fpdfdoc/cpdf_defaultappearance.cpp
core/fpdfdoc/cpdf_defaultappearance.h
core/fpdfdoc/cpdf_dest.cpp
core/fpdfdoc/cpdf_dest.h
#core/fpdfdoc/cpdf_dest_unittest.cpp
#core/fpdfdoc/cpdf_docjsactions.cpp
#core/fpdfdoc/cpdf_docjsactions.h
core/fpdfdoc/cpdf_filespec.cpp
core/fpdfdoc/cpdf_filespec.h
#core/fpdfdoc/cpdf_filespec_unittest.cpp
core/fpdfdoc/cpdf_formcontrol.cpp
core/fpdfdoc/cpdf_formcontrol.h
core/fpdfdoc/cpdf_formfield.cpp
core/fpdfdoc/cpdf_formfield.h
#core/fpdfdoc/cpdf_formfield_unittest.cpp
core/fpdfdoc/cpdf_iconfit.cpp
core/fpdfdoc/cpdf_iconfit.h
#core/fpdfdoc/cpdf_interform.cpp
#core/fpdfdoc/cpdf_interform.h
core/fpdfdoc/cpdf_link.cpp
core/fpdfdoc/cpdf_link.h
core/fpdfdoc/cpdf_linklist.cpp
core/fpdfdoc/cpdf_linklist.h
core/fpdfdoc/cpdf_metadata.cpp
core/fpdfdoc/cpdf_metadata.h
core/fpdfdoc/cpdf_nametree.cpp
core/fpdfdoc/cpdf_nametree.h
core/fpdfdoc/cpdf_numbertree.cpp
core/fpdfdoc/cpdf_numbertree.h
#core/fpdfdoc/cpdf_occontext.cpp
#core/fpdfdoc/cpdf_occontext.h
core/fpdfdoc/cpdf_pagelabel.cpp
core/fpdfdoc/cpdf_pagelabel.h
#core/fpdfdoc/cpdf_variabletext.cpp
#core/fpdfdoc/cpdf_variabletext.h
core/fpdfdoc/cpdf_viewerpreferences.cpp
core/fpdfdoc/cpdf_viewerpreferences.h
#core/fpdfdoc/cpvt_arraytemplate.h
#core/fpdfdoc/cpvt_color.cpp
#core/fpdfdoc/cpvt_color.h
#core/fpdfdoc/cpvt_dash.h
core/fpdfdoc/cpvt_floatrect.h
core/fpdfdoc/cpvt_fontmap.cpp
core/fpdfdoc/cpvt_fontmap.h
#core/fpdfdoc/cpvt_generateap.cpp
#core/fpdfdoc/cpvt_generateap.h
core/fpdfdoc/cpvt_line.h
core/fpdfdoc/cpvt_lineinfo.h
#core/fpdfdoc/cpvt_secprops.h
core/fpdfdoc/cpvt_section.h
#core/fpdfdoc/cpvt_sectioninfo.cpp
#core/fpdfdoc/cpvt_sectioninfo.h
core/fpdfdoc/cpvt_word.h
core/fpdfdoc/cpvt_wordinfo.cpp
core/fpdfdoc/cpvt_wordinfo.h
core/fpdfdoc/cpvt_wordplace.h
#core/fpdfdoc/cpvt_wordprops.h
core/fpdfdoc/cpvt_wordrange.h
#core/fpdfdoc/csection.cpp
#core/fpdfdoc/csection.h
#core/fpdfdoc/ctypeset.cpp
#core/fpdfdoc/ctypeset.h
#core/fpdfdoc/#doc_tagged.cpp
#core/fpdfdoc/fpdf_tagged.h
#core/fpdfdoc/ipdf_formnotify.h
core/fpdfdoc/ipvt_fontmap.h
#core/fpdfdoc/tagged_int.h
)

######################## FPDFAPI ###########################################
file (GLOB_RECURSE FPDFAPI_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core/fpdfapi/*)

######################## FPDFTEXT #############################################
set(FPDFTEXT_SOURCES
core/fpdftext/cpdf_linkextract.cpp
core/fpdftext/cpdf_linkextract.h
core/fpdftext/cpdf_textpage.cpp
core/fpdftext/cpdf_textpagefind.cpp
core/fpdftext/cpdf_textpagefind.h
core/fpdftext/cpdf_textpage.h
#core/fpdftext/fpdf_text_int_unittest.cpp
core/fpdftext/unicodenormalizationdata.cpp
core/fpdftext/unicodenormalizationdata.h
)

######################## FXCODEC #############################################
# TODO rewrite 
file (GLOB_RECURSE FXCODEC_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core/fxcodec/*)
######################## FXCRT #############################################
file (GLOB_RECURSE FXCRT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core/fxcrt/*)
file(GLOB_RECURSE platform_sources
    "core/fxcrt/*posix*"
    )
if(NOT UNIX)
    list(REMOVE_ITEM FXCRT_SOURCES ${platform_sources})
endif()

######################## FXGE #############################################
file (GLOB_RECURSE FXGE_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/core/fxge/*)
file(GLOB_RECURSE to_remove 
    "core/fxge/*test.cpp"
    "core/fxge/skia/*"
    "core/fxge/win32/*"
    )
list(REMOVE_ITEM FXGE_SOURCES ${to_remove})
if(WIN32)
	file(GLOB_RECURSE to_remove 
	    "core/fxge/linux/*"
	    "core/fxge/apple/*"
	    "core/fxge/android/*"
	    )
elseif(APPLE)
	file(GLOB_RECURSE to_remove 
        "core/fxge/win32/*"
	    "core/fxge/linux/*"
	    "core/fxge/android/*"
	    )
elseif(UNIX)
	file(GLOB_RECURSE to_remove 
        "core/fxge/win32/*"
	    "core/fxge/apple/*"
	    "core/fxge/android/*"
	    )
endif()
list(REMOVE_ITEM FXGE_SOURCES ${to_remove})
######################## FXEDIT #############################################
file (GLOB_RECURSE FXEDIT_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/fpdfsdk/pwl/*)

######################## PDFWINDOW #############################################
file (GLOB_RECURSE PDFWINDOW_SOURCES ${CMAKE_CURRENT_SOURCE_DIR}/fpdfsdk/pwl/*)

######################## FORMFILLER #############################################
file (GLOB_RECURSE FORMFILLER_SOURCES 
    "core/fpdfdoc/*"
    "fpdfsdk/formfiller/*.cpp"   
    "fpdfsdk/formfiller/*.h"
    )

######################## JS #############################################
file (GLOB JS_STUB_SOURCES 
    #fxjs/fpdf_javascript.cpp
    #fxjs/fpdf_javascript_embeddertest.cpp
    "fxjs/cjs_event_context_stub.cpp"
    "fxjs/cjs_event_context_stub.h"
    "fxjs/cjs_runtimestub.cpp"
    "fxjs/cjs_runtimestub.h"
    "fxjs/ijs_event_context.h"
    "fxjs/ijs_runtime.cpp"
    "fxjs/ijs_runtime.h"
    )
#file(GLOB_RECURSE to_remove 

#    "fxjs/*test.cpp"
#    "core/fxge/skia/*"
#    "core/fxge/win32/*"
#    )
#list(REMOVE_ITEM FXGE_SOURCES ${to_remove})

######################## BIGINT #############################################
set(BIGINT_SOURCES
third_party/bigint/BigInteger.cc
third_party/bigint/BigInteger.hh
third_party/bigint/BigIntegerLibrary.hh
third_party/bigint/BigIntegerUtils.cc
third_party/bigint/BigIntegerUtils.hh
third_party/bigint/BigUnsigned.cc
third_party/bigint/BigUnsigned.hh
third_party/bigint/BigUnsignedInABase.cc
third_party/bigint/BigUnsignedInABase.hh
third_party/bigint/NumberlikeArray.hh
)

######################## AGG #############################################
set(AGG_SOURCES
third_party/agg23/agg_array.h
third_party/agg23/agg_basics.h
third_party/agg23/agg_clip_liang_barsky.h
third_party/agg23/agg_color_gray.h
third_party/agg23/agg_conv_adaptor_vcgen.h
third_party/agg23/agg_conv_dash.h
third_party/agg23/agg_conv_stroke.h
third_party/agg23/agg_curves.cpp
third_party/agg23/agg_curves.h
third_party/agg23/agg_math.h
third_party/agg23/agg_math_stroke.h
third_party/agg23/agg_path_storage.cpp
third_party/agg23/agg_path_storage.h
third_party/agg23/agg_pixfmt_gray.h
third_party/agg23/agg_rasterizer_scanline_aa.cpp
third_party/agg23/agg_rasterizer_scanline_aa.h
third_party/agg23/agg_renderer_base.h
third_party/agg23/agg_renderer_scanline.h
third_party/agg23/agg_rendering_buffer.h
third_party/agg23/agg_render_scanlines.h
third_party/agg23/agg_scanline_u.h
third_party/agg23/agg_shorten_path.h
third_party/agg23/agg_vcgen_dash.cpp
third_party/agg23/agg_vcgen_dash.h
third_party/agg23/agg_vcgen_stroke.cpp
third_party/agg23/agg_vcgen_stroke.h
third_party/agg23/agg_vertex_sequence.h
)
file(GLOB BASE_SOURCES
    third_party/base/debug/alias.h
    third_party/base/debug/alias.cc
    third_party/base/allocator/partition_allocator/*.cc
    third_party/base/allocator/partition_allocator/*.h
    )

file(GLOB OTHER_SOURCES 
    "constants/*.cpp"
    "constants/*.h"
    )
if(WIN32)
    file(GLOB to_add 
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/base/win/*
    )
endif()
list(APPEND OTHER_SOURCES ${to_add})
           
set(ALL_SOURCES
    ${FDRM_SOURCES}
    ${FPDFDOC_SOURCES}
    ${FPDFAPI_SOURCES} ${FPDFAPI_GB1_SOURCES} ${FPDFAPI_CNS1_SOURCES} ${FPDFAPI_JAPAN1_SOURCES} ${FPDFAPI_KOREA_SOURCES}
    ${FPDFTEXT_SOURCES}
    ${FXCODEC_SOURCES}
    ${FXCRT_SOURCES}
    ${FXGE_SOURCES}
    ${FXEDIT_SOURCES}
    ${PDFWINDOW_SOURCES}
    ${FORMFILLER_SOURCES}
    ${FPDFSDK_SOURCES}
    ${PLATFORM_SOURCES}
    ${JS_STUB_SOURCES}
    #${BIGINT_SOURCES}
    ${AGG_SOURCES}
    ${OTHER_SOURCES}
    ${BASE_SOURCES}
    ${FORMFILLER_SOURCES}
)

file( GLOB WINDOWS_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fpdfapi/render/cpdf_windowsrenderdevice.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxge/cfx_windows*"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxcrt/*windows.cpp"
    )
if(NOT WIN32)
    list (REMOVE_ITEM ALL_SOURCES ${WINDOWS_SOURCES})
endif()
file( GLOB APPLE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxge/apple/*"
    )
if(NOT APPLE)
    list(REMOVE_ITEM ALL_SOURCES 
        ${APPLE_SOURCES}
        )
endif()

file(GLOB_RECURSE to_remove "${CMAKE_CURRENT_SOURCE_DIR}/core/fpdfapi/*test.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fpdfapi/edit/*test.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fpdfapi/*_module_*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fpdfapi/*_module.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*unittest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxcodec/bmp/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxcodec/gif/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxcodec/jpeg/jpeg_progressive_decoder*"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxcodec/png/*"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxcodec/progressive_decoder*"
    "${CMAKE_CURRENT_SOURCE_DIR}/core/fxcodec/tiff_decoder*"
    #"${CMAKE_CURRENT_SOURCE_DIR}/core/fxcodec/jpx/*"
    "*test.cpp"
    )
list(REMOVE_ITEM ALL_SOURCES ${to_remove})
if(USE_STATIC_LIBRARY)
    add_library(${PROJECT_NAME} STATIC ${ALL_SOURCES})
else(USE_STATIC_LIBRARY)
    add_library(${PROJECT_NAME} SHARED ${ALL_SOURCES})
endif(USE_STATIC_LIBRARY)
set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "parsing")

target_include_directories(${PROJECT_NAME}
    PUBLIC
        ${CMAKE_CURRENT_SOURCE_DIR}
        ${CMAKE_CURRENT_SOURCE_DIR}/public
        ./fpdfsdk/include
)

target_compile_definitions(${PROJECT_NAME} PRIVATE 
    USE_SYSTEM_LIBJPEG
    USE_SYSTEM_ZLIB
    USE_SYSTEM_ICUUC
	USE_SYSTEM_LIBOPENJPEG2
    )  

target_include_directories(${PROJECT_NAME} PRIVATE  
    ${CONAN_INCLUDE_DIRS_ABSEIL}
    ${CONAN_INCLUDE_DIRS_FREETYPE}
    ${CONAN_INCLUDE_DIRS_ICU}
    ${CONAN_INCLUDE_DIRS_LIBJPEG}
    ${CONAN_INCLUDE_DIRS_ZLIB}
    )

target_link_libraries(${PROJECT_NAME}
    PUBLIC
        CONAN_PKG::openjpeg
        CONAN_PKG::libjpeg
)

#Install
file(GLOB_RECURSE INSTALL_HEADERS ${CMAKE_CURRENT_SOURCE_DIR}/public/*)
install(FILES ${INSTALL_HEADERS} DESTINATION include/pdfium)

install (TARGETS ${PROJECT_NAME} DESTINATION lib)

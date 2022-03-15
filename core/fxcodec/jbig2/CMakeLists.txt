project(internal_jbig2 DESCRIPTION "Internal jbig2 library of pdfium.")
CMAKE_MINIMUM_REQUIRED (VERSION 2.8.12)

if(WIN32)
  add_definitions(-D_WIN32)
else(WIN32)
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC")
endif(WIN32)

file(GLOB jbig2_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*.cpp")

file(GLOB to_remove "${CMAKE_CURRENT_SOURCE_DIR}/*unittest.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/*test.cpp")

list(REMOVE_ITEM jbig2_SOURCES ${to_remove})

add_library(${PROJECT_NAME} STATIC ${jbig2_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE  
     ${CONAN_INCLUDE_DIRS}
    )

set_property(TARGET ${PROJECT_NAME} PROPERTY FOLDER "parsing")

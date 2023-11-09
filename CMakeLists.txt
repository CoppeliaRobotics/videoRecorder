cmake_minimum_required(VERSION 2.6)

set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)

macro(add_precompiled_header PrecompiledHeader PrecompiledSource SourcesVar)
  if(MSVC)
    set(Sources ${${SourcesVar}})

    set_source_files_properties(${PrecompiledSource} PROPERTIES COMPILE_FLAGS "/Yc\"${PrecompiledHeader}\"")

    set_source_files_properties(${Sources} PROPERTIES COMPILE_FLAGS "/Yu\"${PrecompiledHeader}\"")

    list(APPEND ${SourcesVar} ${PrecompiledHeader} ${PrecompiledSource})
  endif(MSVC)
endmacro(add_precompiled_header)

project(VideoCompression)

set(CMAKE_MODULE_PATH ${PROJECT_SOURCE_DIR}/cmake ${CMAKE_MODULE_PATH})

find_package(FFMPEG COMPONENTS AVCODEC AVFORMAT AVUTIL SWSCALE REQUIRED)

if(FFMPEG_FOUND)
#  FFMPEG_INCLUDE_DIRS  - Include directory necessary for using the required components headers.
#  FFMPEG_LIBRARIES     - Link these to use the required ffmpeg components.
#  FFMPEG_DEFINITIONS   - Compiler switches required for using the required ffmpeg components.
    message("FFMPEG_INCLUDE_DIRS = ${FFMPEG_INCLUDE_DIRS} ")
    message("FFMPEG_LIBRARIES = ${FFMPEG_LIBRARIES} ")
    message("FFMPEG_DEFINITIONS = ${FFMPEG_DEFINITIONS} ")

    include_directories(${FFMPEG_INCLUDE_DIRS})
else()
    message(FATAL_ERROR "FFMPEG not found")
endif()

if(MSVC)
add_definitions(-D_CRT_NONSTDC_NO_DEPRECATE -D_CRT_SECURE_NO_DEPRECATE -D_CRT_NON_CONFORMING_SWPRINTFS)
endif(MSVC)

if(WIN32)
add_definitions(-D_WIN32 -D_WIN32_WINNT=0x0501)
endif(WIN32)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE RelWithDebInfo)
endif(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)

if(MSVC)
set(CMAKE_CXX_FLAGS "/W3 /nologo /EHsc")
set(CMAKE_CXX_FLAGS_DEBUG "/Od /Gm /RTC1 /MTd /ZI /D _DEBUG ")
set(CMAKE_CXX_FLAGS_RELEASE "/O2 /Ob2 /Oi /Ot /GL /GF /FD /MT /GS- /GR- /D NDEBUG")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELEASE} /Zi")
endif(MSVC)

set(CMAKE_C_FLAGS ${CMAKE_CXX_FLAGS})
set(CMAKE_C_FLAGS_DEBUG ${CMAKE_CXX_FLAGS_DEBUG})
set(CMAKE_C_FLAGS_RELEASE ${CMAKE_CXX_FLAGS_RELEASE})
set(CMAKE_C_FLAGS_RELWITHDEBINFO ${CMAKE_CXX_FLAGS_RELWITHDEBINFO})

if(MSVC)
set(CMAKE_EXE_LINKER_FLAGS "/MANIFEST")
set(CMAKE_EXE_LINKER_FLAGS_DEBUG "/DEBUG")
set(CMAKE_EXE_LINKER_FLAGS_RELEASE "/LTCG")
set(CMAKE_EXE_LINKER_FLAGS_RELWITHDEBINFO "/LTCG /DEBUG")

set(CMAKE_SHARED_LINKER_FLAGS "/MANIFEST")
set(CMAKE_SHARED_LINKER_FLAGS_DEBUG "/DEBUG")
set(CMAKE_SHARED_LINKER_FLAGS_RELEASE "/LTCG")
set(CMAKE_SHARED_LINKER_FLAGS_RELWITHDEBINFO "/LTCG /DEBUG")
endif(MSVC)

set(EXECUTABLE_OUTPUT_PATH ${VideoCompression_BINARY_DIR}/bin)
set(LIBRARY_OUTPUT_PATH ${VideoCompression_BINARY_DIR}/lib)

add_subdirectory(vvcl)
add_subdirectory(VideoCompressionTest)

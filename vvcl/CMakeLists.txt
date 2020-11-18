cmake_minimum_required(VERSION 2.6)

set(vvcl_INCLUDES ${vvcl_SOURCE_DIR})

add_definitions(-D__VCL_BUILD__)

if(WIN32)
add_definitions(-D_USRDLL)
endif(WIN32)

set(vvcl_SOURCES
    vvcl.h
    vvcl.cpp
)

add_library(vvcl SHARED ${vvcl_SOURCES})

set_property(TARGET vvcl PROPERTY RUNTIME_OUTPUT_DIRECTORY ${EXECUTABLE_OUTPUT_PATH})
set_property(TARGET vvcl PROPERTY ACRHIVE_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})
set_property(TARGET vvcl PROPERTY LIBRARY_OUTPUT_DIRECTORY ${LIBRARY_OUTPUT_PATH})

target_link_libraries(vvcl ${FFMPEG_LIBRARIES})

if(INSTALL_DIR)
    install(TARGETS vvcl DESTINATION "${INSTALL_DIR}")
endif()

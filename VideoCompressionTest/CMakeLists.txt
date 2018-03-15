cmake_minimum_required(VERSION 2.6)

include_directories(${VideoCompression_SOURCE_DIR}/vvcl)

link_directories(${LIBRARY_OUTPUT_PATH})

set(VideoCompressionTest_SOURCES
    VideoCompressionTest.cpp
)

add_executable(VideoCompressionTest ${VideoCompressionTest_SOURCES})

add_dependencies(VideoCompressionTest vvcl)

if(WIN32)
set(VideoCompressionTest_LIBRARIES
    vvcl.lib
)
else(WIN32)
set(VideoCompressionTest_LIBRARIES
    vvcl
)
endif(WIN32)

target_link_libraries(VideoCompressionTest ${VideoCompressionTest_LIBRARIES})

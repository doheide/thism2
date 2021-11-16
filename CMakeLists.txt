cmake_minimum_required(VERSION 3.5)

project(Thism2)

add_subdirectory(main)

add_library(libthism2 STATIC ${LIBSOURCE})

target_include_directories(libthism2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main/include)

add_library(Thism2::libthism2 ALIAS libthism2)

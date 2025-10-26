cmake_minimum_required(VERSION 3.5)

project(Thism2)
set(CMAKE_CXX_STANDARD 11)

add_definitions(-D__useNames -D__useDescription)

# ***********************************************************************************************
# lib
add_subdirectory(main)

add_library(libthism2 STATIC ${LIBSOURCE})

target_include_directories(libthism2 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main/include)

add_library(Thism2::libthism2 ALIAS libthism2)


# ***********************************************************************************************
# example
add_subdirectory(example)

add_executable(example_exe ${EX_CODE})

#target_compile_definitions(example_exe PRIVATE __useNames __noDescription)

target_include_directories(example_exe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main/include example/src)

target_link_libraries (example_exe LINK_PUBLIC libthism2)


# ***********************************************************************************************
# example2
add_subdirectory(example2)

add_executable(example2_exe ${EX2_CODE})

#target_compile_definitions(example_exe PRIVATE __useNames __noDescription)

target_include_directories(example2_exe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main/include example2/src)

target_link_libraries (example2_exe LINK_PUBLIC libthism2)


# ***********************************************************************************************
# diagrams
find_package(Qt5Core)
#if (DEFINED Qt5Core)
    add_executable(diagrams example/src/sm_sys.cpp main/src/make_diagrams.cpp)
    #target_compile_definitions(diagrams PRIVATE __useNames __useDescription)
    target_include_directories(diagrams PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main/include example/src)
    target_link_libraries (diagrams LINK_PUBLIC libthism2 Qt5::Core)
#endif ()


# ***********************************************************************************************
# test
add_executable(test_exe tests/main.cpp example/src/sm_sys.cpp)
#target_compile_definitions(test_exe PRIVATE __useNames __useDescription)
#add_definitions(-D__useNames -D__useDescription)
target_include_directories(test_exe PRIVATE tests/Catch2/single_include)
target_include_directories(test_exe PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/main/include example/src)
target_link_libraries (test_exe LINK_PUBLIC libthism2)

#add_subdirectory(tests/Catch2)
#add_executable(tests tests/main.cpp)
#target_include_directories(tests PRIVATE tests/Catch2/include)
#target_link_libraries(tests PRIVATE Catch2::Catch2WithMain)



cmake_minimum_required(VERSION 3.5)

project(B3L CXX)

file(GLOB SRC_FILES src/*.cpp src/*.h include/B3L/*.h)

add_library(B3L
  ${SRC_FILES}
)

target_compile_options(B3L PRIVATE /W4 /WX)

set_target_properties(B3L PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(B3L PROPERTIES CXX_STANDARD 20)

target_include_directories(B3L PRIVATE include/B3L)
target_include_directories(B3L PUBLIC include)
target_include_directories(B3L PUBLIC thirdparty/keystone/include)
target_include_directories(B3L PUBLIC thirdparty/capstone/include)

target_link_libraries(B3L PRIVATE keystone)
target_link_libraries(B3L PRIVATE capstone-static)

set(BUILD_LIBS_ONLY ON CACHE BOOL "Only build keystone library" FORCE)
add_subdirectory(thirdparty/keystone)

set(CAPSTONE_BUILD_SHARED OFF CACHE BOOL "Build shared library" FORCE)
set(CAPSTONE_BUILD_TESTS OFF CACHE BOOL "Build tests" FORCE)
set(CAPSTONE_BUILD_CSTOOL OFF CACHE BOOL "Build cstool" FORCE)
add_subdirectory(thirdparty/capstone)

option(B3L_ENABLE_TESTS "Enable unit tests for B3L" OFF)

if(B3L_ENABLE_TESTS)
  add_subdirectory(thirdparty/googletest)
  add_subdirectory(test)
endif()

cmake_minimum_required(VERSION 3.5)

project(B3L CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX20_STANDARD_COMPILE_OPTION "-std:c++latest")
set(CMAKE_CXX20_EXTENSION_COMPILE_OPTION "-std:c++latest")
SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /D_UNICODE /DUNICODE")

file(GLOB SRC_FILES src/*.cpp src/*.h include/B3L/*.h)

add_library(B3L
  ${SRC_FILES}
)

target_compile_options(B3L PRIVATE /W4 /WX)

option(B3L_BUILD_WITH_ASSEMBLERS "Build with key- and capstone" ON)

set_target_properties(B3L PROPERTIES LINKER_LANGUAGE CXX)
set_target_properties(B3L PROPERTIES CXX_STANDARD 20)

target_include_directories(B3L PRIVATE include/B3L)
target_include_directories(B3L PUBLIC include)

if(B3L_BUILD_WITH_ASSEMBLERS)
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

  add_compile_definitions(B3L_HAVE_ASSEMBLERS)
endif()

option(B3L_ENABLE_TESTS "Enable unit tests for B3L" OFF)

if(B3L_ENABLE_TESTS)
  add_subdirectory(thirdparty/googletest)
  add_subdirectory(test)
endif()

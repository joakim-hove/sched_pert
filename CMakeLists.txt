cmake_minimum_required(VERSION 3.16)
project(sched_pert C CXX)

set(CMAKE_C_STANDARD 99)
set(CMAKE_CXX_STANDARD 17)

set(CMAKE_BUILD_TYPE
  "RelWithDebInfo"
  CACHE STRING "CMake build type" FORCE)
message(
  STATUS
  "Setting CMake build type to ${CMAKE_BUILD_TYPE} as none was specified")

if(NOT EXISTS "${CMAKE_BINARY_DIR}/conan.cmake")
  message(
    STATUS
    "Downloading conan.cmake from https://github.com/conan-io/cmake-conan")
  file(
    DOWNLOAD
    "https://raw.githubusercontent.com/conan-io/cmake-conan/master/conan.cmake"
    "${CMAKE_BINARY_DIR}/conan.cmake")
endif()

include(${CMAKE_BINARY_DIR}/conan.cmake)

conan_cmake_run(
  # Packages
  REQUIRES
  fmt/8.0.1
  # Options
  OPTIONS
  catch2:with_main=True
  ${conan_opts}
  # Force cppstd to be the same as this CMakeLists.txt's
  SETTINGS
  compiler.cppstd=${CMAKE_CXX_STANDARD}
  # Build from source if there are no pre-compiled binaries
  BUILD
  missing
  GENERATORS
  cmake_find_package)

include(GNUInstallDirs)
enable_testing()


find_package(opm-common REQUIRED)
find_package(OpenMP REQUIRED)

add_executable(sched_pert sched_pert.cpp WellPert.cpp)
target_include_directories(sched_pert PUBLIC ${opm-common_INCLUDE_DIRS})
target_link_libraries(sched_pert PRIVATE ${opm-common_LIBRARY} fmt OpenMP::OpenMP_CXX)

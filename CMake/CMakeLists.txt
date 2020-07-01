# CMakeList.txt : CMake project for RTDS, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)

set(RTDS_MAJOR 1)
set(RTDS_MINOR 0)
set(RTDS_PATCH 0)
set(RTDS_VERSION ${RTDS_MAJOR}.${RTDS_MINOR}.${RTDS_PATCH})

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
#set(CMAKE_BUILD_TYPE Release)

project ("RTDS" VERSION ${RTDS_VERSION})

#set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)
  find_package(OpenSSL REQUIRED)

include_directories(${PROJECT_SOURCE_DIR}/include ${PROJECT_SOURCE_DIR}/src)
configure_file (
  "${PROJECT_SOURCE_DIR}/include/common.h.in"
  "${PROJECT_SOURCE_DIR}/include/common.h"  )

file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/include/*.h" "${PROJECT_SOURCE_DIR}/include/*.hpp"
        "${PROJECT_SOURCE_DIR}/src/*.cpp" "${PROJECT_SOURCE_DIR}/src/*.c")

# Add source to this project's executable.
add_executable(rtds ${all_SRCS})

target_link_libraries(rtds Threads::Threads)
target_link_libraries(rtds asio asio::asio)
target_link_libraries(rtds OpenSSL::SSL OpenSSL::Crypto)
# TODO: Add tests and install targets if needed.

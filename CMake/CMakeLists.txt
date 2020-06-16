# CMakeList.txt : CMake project for RTDS, include source and define
# project specific logic here.
#
cmake_minimum_required (VERSION 3.8)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

project ("RTDS" VERSION 1.0)

#set(CMAKE_THREAD_PREFER_PTHREAD TRUE)
#set(THREADS_PREFER_PTHREAD_FLAG TRUE)
find_package(Threads REQUIRED)
find_package(asio CONFIG REQUIRED)

# Add source to this project's executable.
add_executable (rtds "common.h" "sapair.h" "sapair.cpp" "log.cpp" "log.h" "peer.h" "peer.cpp" 
"rtds.cpp" "rtds.h" "main.cpp" "message.h" "message.cpp" "cmd_processor.h" "cmd_processor.cpp" 
"bg_controller.h" "bg_controller.cpp")

target_link_libraries(rtds Threads::Threads)
target_link_libraries(rtds asio asio::asio)
# TODO: Add tests and install targets if needed.

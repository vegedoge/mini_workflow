# Cmake version
cmake_minimum_required(VERSION 3.10)

# proj name
project(mini_workflow)

# cpp17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# debug 
set(CMAKE_BUILD_TYPE Debug)
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0")

# include
include_directories(include)

# find
find_package(Threads REQUIRED)
find_package(OpenSSL REQUIRED)

# exec
add_executable(mini_workflow_exe 
            src/main.cc 
            src/MyScheduler.cc 
            src/MySeriesWork.cc
            src/MyEpollPoller.cc 
            src/MyHttpClientTask.cc 
            src/MyTaskFactory.cc
            src/MyParallelWork.cc
            src/HttpMessages.cc 
            src/MyHttpServer.cc)

# test
add_executable(test_http_server 
            src/test_http_server.cc
            src/MyScheduler.cc 
            src/MySeriesWork.cc
            src/MyEpollPoller.cc 
            src/MyHttpClientTask.cc 
            src/MyTaskFactory.cc
            src/MyParallelWork.cc
            src/HttpMessages.cc 
            src/MyHttpServer.cc)


# linkers
target_link_libraries(mini_workflow_exe PRIVATE Threads::Threads OpenSSL::SSL OpenSSL::Crypto)
# test
target_link_libraries(test_http_server PRIVATE Threads::Threads OpenSSL::SSL OpenSSL::Crypto)

message(STATUS "Executable is located at: ${CMAKE_BINARY_DIR}/mini_workflow_exe")
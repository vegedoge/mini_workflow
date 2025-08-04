# Cmake version
cmake_minimum_required(VERSION 3.10)

# proj name
project(mini_workflow)

# cpp17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# include
include_directories(include)

# find
find_package(Threads REQUIRED)

# exec
add_executable(mini_workflow_exe src/main.cc src/MyScheduler.cc src/MySeriesWork.cc
                src/MyEpollPoller.cc src/MyHttpClientTask.cc)

# linker
target_link_libraries(mini_workflow_exe PRIVATE Threads::Threads)

message(STATUS "Executable is located at: ${CMAKE_BINARY_DIR}/mini_workflow_exe")
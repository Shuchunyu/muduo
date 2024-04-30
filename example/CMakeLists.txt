cmake_minimum_required(VERSION 3.10)
project(MyProject)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 获取当前目录下所有的.cc文件
file(GLOB SOURCES *.cc)

add_executable(myHttpServer ${SOURCES})
target_link_libraries(myHttpServer mymuduo pthread)

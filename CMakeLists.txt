cmake_minimum_required(VERSION 2.5) # 最小cmake版本
project(myreactor) # 项目名称

set(LIBRARY_OUTPUT_DIRECTORY ${PROJECT_SOURCE_DIR}/lib)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -std=c++11") # 编译选项

aux_source_directory(. SRC_LIST) # 参与编译的源文件
add_library(myreactor SHARED ${SRC_LIST})

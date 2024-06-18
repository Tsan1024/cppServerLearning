# 设置项目名称
project(step9)

# 指定 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED True)

cmake_minimum_required(VERSION 3.22)
# 设置可执行文件输出目录为 step6/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 查找spdlog包
find_package(spdlog REQUIRED)
if(NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found")
else()
    message(STATUS "spdlog found: ${spdlog_INCLUDE_DIRS}")
endif()

# 添加 server 可执行文件
add_executable(step9_server ${CMAKE_CURRENT_SOURCE_DIR}/src/server.cpp ${CMAKE_CURRENT_SOURCE_DIR}/src/epollManager.cpp)

# 添加 client 可执行文件
add_executable(step9_client ${CMAKE_CURRENT_SOURCE_DIR}/src/client.cpp)

# 链接spdlog库到server可执行文件
target_link_libraries(step9_server spdlog::spdlog)
target_link_libraries(step9_client spdlog::spdlog)

# 为可执行文件添加调试编译选项（可选）
target_compile_options(step9_server PRIVATE -g)
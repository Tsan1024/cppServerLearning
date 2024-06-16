# 设置项目名称
project(step6)

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
add_executable(step6_tcp_server ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.cpp)

# 添加 client 可执行文件
add_executable(step6_tcp_client ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.cpp)

# 为可执行文件添加调试编译选项（可选）
target_compile_options(step6_tcp_server PRIVATE -g)

# 链接spdlog库到server可执行文件
target_link_libraries(step6_tcp_server spdlog::spdlog)
target_link_libraries(step6_tcp_client spdlog::spdlog)
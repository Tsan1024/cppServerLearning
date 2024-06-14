# 设置项目名称
project(step3)

cmake_minimum_required(VERSION 3.22)
# 设置可执行文件输出目录为 step2/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 查找spdlog包
find_package(spdlog REQUIRED)
if(NOT spdlog_FOUND)
    message(FATAL_ERROR "spdlog not found")
else()
    message(STATUS "spdlog found: ${spdlog_INCLUDE_DIRS}")
endif()

# 添加 server 可执行文件
add_executable(step3_tcp_server ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.cpp)

# 添加 client 可执行文件
add_executable(step3_tcp_client ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.cpp)

# 链接spdlog库到server可执行文件
target_link_libraries(step3_tcp_server spdlog::spdlog)
target_link_libraries(step3_tcp_client spdlog::spdlog)
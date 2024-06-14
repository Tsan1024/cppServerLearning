# 在 step1 目录下的 CMakeLists.txt 文件中
# 设置可执行文件输出目录为 step1/bin
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}/bin)

# 添加 server 可执行文件
add_executable(tcp_server ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_server.cpp)

add_executable(udp_server ${CMAKE_CURRENT_SOURCE_DIR}/src/udp_server.cpp)

# 添加 client 可执行文件
add_executable(tcp_client ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_client.cpp)

add_executable(udp_client ${CMAKE_CURRENT_SOURCE_DIR}/src/udp_client.cpp)

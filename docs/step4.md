# doing

多线程处理客户端连接：

在 Server::run 方法中，每当接受一个新的客户端连接时，创建一个新的线程来处理该客户端连接。
使用 std::thread 创建新线程，并使用 detach 方法使线程在后台运行，处理完成后自动清理。
处理客户端的线程：

Server::handle_client 方法在独立线程中运行，处理每个客户端的请求。





valgrind 排查内存泄漏
valgrind --leak-check=full --show-leak-kinds=all --track-origins=yes --log-file=valgrind.out ./yourProgram
# doing

## epoll

要将当前的 Server 类换成使用 epoll，我们需要修改 Server::run 和 Server::handle_client 方法来使用 epoll 进行事件驱动的 I/O 操作。

主要改动点：
添加 epoll 实例 (epoll_fd) 的创建和关闭。
修改 Server::run 方法，使用 epoll_wait 监控事件。
将新连接的处理和客户端数据的处理分开，使用 epoll_ctl 添加新的客户端 socket 到 epoll 实例。
在 handle_client 方法中处理客户端数据，确保正确读取和写入数据，并检查退出条件。
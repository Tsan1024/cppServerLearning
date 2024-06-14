## 以tcp为例

添加spdlog

### spdlog
spdlog 是一个非常快速、线程安全的 C++ 日志库，设计简洁并且易于使用。它支持各种日志目标，如控制台、文件、syslog 等，同时也提供了格式化日志信息的功能，支持异步和同步日志记录。

|  特点	 | 描述 |
|  ---- | ---- |
| 高性能 |	spdlog 是一个非常高效的日志库，能够满足高性能应用的需求。|
| 轻量级 |	spdlog 的体积非常小，仅 1.5MB 左右，适合于各种场景的轻量级日志库。|
| 线程安全 |	spdlog 是线程安全的，可以在多线程环境中使用，确保日志记录的正确性和一致性。|
|易于使用	| 提供简单直观的接口，便于快速上手和使用。|
|灵活的日志目标 |	支持多种日志目标，如控制台输出、文件输出、syslog 等，用户可以根据需求选择适合的日志目标。|
|异步日志	| 支持异步日志记录，提高日志记录性能，适用于高并发场景。|
|格式化日志	|集成 fmt 库，支持丰富的日志格式化功能，用户可以方便地格式化日志信息。|

将标准输入输出换成日志方式。

spdlog使用介绍

引入头文件
```
#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
```

基本日志
```
int main() {
    spdlog::info("Welcome to spdlog!");
    spdlog::warn("This is a warning message.");
    spdlog::error("This is an error message.");

    // 格式化日志
    spdlog::info("Hello, {}!", "world");
    
    return 0;
}
```

文件日志
```
int main() {
    auto file_logger = spdlog::basic_logger_mt("basic_logger", "logs/basic-log.txt");
    spdlog::set_default_logger(file_logger);

    spdlog::info("This will be logged to a file.");

    return 0;
}
```

异步日志

```
#include <spdlog/async.h>
#include <spdlog/sinks/basic_file_sink.h>

int main() {
    spdlog::init_thread_pool(8192, 1); // queue with 8192 items and 1 backing thread.

    auto async_file_logger = spdlog::basic_logger_mt<spdlog::async_factory>("async_file_logger", "logs/async-log.txt");
    spdlog::set_default_logger(async_file_logger);

    spdlog::info("This is an async log message.");

    return 0;
}
```

高级使用

日志级别

spdlog 提供了多种日志级别，可以通过以下方式设置：

```
spdlog::set_level(spdlog::level::debug); // 设置全局日志级别为 debug
spdlog::debug("This is a debug message.");
spdlog::set_level(spdlog::level::info);  // 设置全局日志级别为 info
spdlog::info("This is an info message.");
```

自定义格式

可以通过以下方式设置日志格式：

```
spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%^%l%$] %v");

spdlog::info("This is a log message with a custom format.");
```

多日志目标
可以创建多种日志目标，并将它们组合在一起：

```
#include <memory>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/spdlog.h>

int main() {
    // 创建控制台日志接收器
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    // 创建文件日志接收器
    auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(
        "logs/multisink-log.txt", true);

    // 设置日志接收器集合
    std::vector<spdlog::sink_ptr> sinks{console_sink, file_sink};

    // 创建组合日志记录器
    auto combined_logger = std::make_shared<spdlog::logger>(
        "multi_sink", sinks.begin(), sinks.end());

    // 设置默认的日志记录器
    spdlog::set_default_logger(combined_logger);

    // 日志消息
    spdlog::info("This message should appear in both console and file.");

    return 0;
}

```
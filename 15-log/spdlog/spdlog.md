
# 介绍
## spdlog特点
支持异步日志
发起打印日志后不直接输出日志，而是交给另一个线程完成日志输出或者使用aio完成日志的输出，因为异步日志不会暂停当前线程，所以异步日志的效率高，
提供很多日志级别，支持设置最小打印日志级别
支持文件，控制台，自定义接收器(数据库/syslog/...)
高性能，通过模板和内联函数
高效格式化，使用fmt库

极高的日志记录效率，每秒百万条日志
极低的内存占用
灵活的用户配置，选择异步或同步日志，设置日志级别输出目标

输出控制
多种日志级别
多种输出目标，输出到控制台，文件，网络服务器
格式化输出，允许以用户结构化方式输出到日志


## spdlog的处理逻辑
抽象模型


                                                         ┌───────────┐
                                                         │ registry  │
                                                         └───────────┘
                                                               ▲
                             ┌─────────────────────────────────┴───────────────────────────────┐
                             │                                                                 │
                        ┌────┴────┐                                                        ┌───┴────┐
                        │ sync    │                                                        │ async  │
                        │ logger  │                              ┌───────────────────────► │ logger │ ◄─────────────────────────────────────────┐
                        └─────────┘◄─────────────────────────────┼───────┐                 └────────┘                                           │
                             ▲                                   │       │                     ▲                                                │
                             │                                   │       │                     │                                                │
                  ┌──────────┴────┬──────────────┐               │       │        ┌────────────┴───┬─────────────────┐                          │
                  │    ┌──────────┼──────────────┼───────────────┘       └────────┼────────────────┼───────────────┐ │                          │
            ┌─────┴────┴┐   ┌─────┴─────┐   ┌────┴──────┐                   ┌─────┴─────┐    ┌─────┴─────┐      ┌──┴─┴──────┐            ┌──────┴───────┐
            │ sink      │   │ sink      │   │ sink      │                   │ sink      │    │ sink      │      │ sink      │            │ thread pool  │
            │┌─────────┐│   │┌─────────┐│   │┌─────────┐│                   │┌─────────┐│    │┌─────────┐│      │┌─────────┐│            └──────────────┘
            ││formatter││   ││formatter││   ││formatter││                   ││formatter││    ││formatter││      ││formatter││
            │└─────────┘│   │└─────────┘│   │└─────────┘│                   │└─────────┘│    │└─────────┘│      │└─────────┘│
            └───────────┘   └───────────┘   └───────────┘                   └───────────┘    └───────────┘      └───────────┘

- 第一层
  - registry (皇上)
    - 单例模式
    - 提供全局访问点
    - 负责管理所有logger
    ```cc
    // 注册logger到 registry
    spdlog::register_logger(std::shared_ptr<logger> new_logger);
    // 根据日志名称获得logger
    spdlog::get(logger_name);
    工厂方法快速创建logger
    ```
- 第二层
  - logger (某部主管, 负责脚本) 
    - 可以是同步logger 或 异步logger
    ```cc
    // 1. 配置flush策略
    手动刷，周期刷，遇到error时刷...
    // 2. 输出日志
    logger->log(...)
    // 2. 设置日志级别
    set_level
    // 
    设置线程池
    ```
- 第三层
  - sink(接收器) (技术工，真正干活)
    - 负责将日志数据写入到具体目标
    - sink可以是指向文件，指向控制台，指向网络服务..
    ```cc
    // 1. 提供具体写操作的实现
    // 对应接收器的 write 操作
    sink_it_xx
    // 对应接收器的 flush 操作
    flush_xx
    set_level
    // 设置前缀格式, 比如 [级别 时间]
    set_pattern
    // 创建前缀自定义格式
    set_formatter
    ```
  - thread pool
    - 为异步日志提供读线程


# 使用spdlog
## 如何创建logger
### spdlog工程模式创建
创建的logger分为线程安全`_mt`和单线程`_st`.
通过指定模板参数可以创建同步和异步日志。
如下面示例
```cc
// 创建多线程同步日志
std::shared_ptr<spdlog::logger> slogger_st = spdlog::stdout_color_st<spdlog::synchronous_factory>("thread unsafe, sync logger");
slogger_st->info("hello");

// 创建异步日志
std::shared_ptr<spdlog::logger> alogger_mt = spdlog::stdout_color_mt<spdlog::async_factory>("thread safe, async logger");
alogger_mt->info("hello");
```
原理是
```cc
// 1.
template <typename Factory>
SPDLOG_INLINE std::shared_ptr<logger> stdout_color_mt(const std::string &logger_name,
                                                      color_mode mode) {
    // 实际调用的是模板参数的 create
    return Factory::template create<sinks::stdout_color_sink_mt>(logger_name, mode);
}

// 2.
// 当模板参数为 spdlog::synchronous_factory 时
struct synchronous_factory {
    template <typename Sink, typename... SinkArgs>
    // Sink = sinks::stdout_color_mt
    static std::shared_ptr<spdlog::logger> create(std::string logger_name, SinkArgs &&...args) {
        // 构造sink
        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        // 构造logger, 并绑定sink
        auto new_logger = std::make_shared<spdlog::logger>(std::move(logger_name), std::move(sink));
        // 将logger注册到 registry(此为单例模式)
        details::registry::instance().initialize_logger(new_logger);
        // 返回创建的logger
        return new_logger;
    }
};

// 当模板参数为 spdlog::async_factory 时
using async_factory = async_factory_impl<async_overflow_policy::block>;
template <async_overflow_policy OverflowPolicy = async_overflow_policy::block>
struct async_factory_impl {
    // 也是调用 create方法
    template <typename Sink, typename... SinkArgs>
    static std::shared_ptr<async_logger> create(std::string logger_name, SinkArgs &&...args) {
        auto &registry_inst = details::registry::instance();

        // 创建线程池
        auto &mutex = registry_inst.tp_mutex();
        std::lock_guard<std::recursive_mutex> tp_lock(mutex);
        auto tp = registry_inst.get_tp();
        if (tp == nullptr) {
            tp = std::make_shared<details::thread_pool>(details::default_async_q_size, 1U);
            registry_inst.set_tp(tp);
        }

        // 构造 sink, logger 
        auto sink = std::make_shared<Sink>(std::forward<SinkArgs>(args)...);
        auto new_logger = std::make_shared<async_logger>(std::move(logger_name), std::move(sink),
                                                         std::move(tp), OverflowPolicy);
        // 注册logger到 registry
        registry_inst.initialize_logger(new_logger);
        return new_logger;
    }
};
```
所以使用工厂方法创建logger有个缺点，即logger只有一个sink

### 手动创建logger
若要让logger有多个sink，需要手动创建.
```cc
// 3. 手动创建logger，指定多个sink
// 创建指向控制台多线程安全的sink1
auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
// 创建指向1.log文件的线程安全sink2 追加写
auto sink2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>("./1.log");
// 创建logger并设置sinks
auto logger = std::make_shared<spdlog::logger>("two sinks");
logger->sinks().push_back(sink1);
logger->sinks().push_back(sink2);

spdlog::register_logger(logger);
logger->info("hello");
```


## 如何创建sink
内部可用的sink
```bash
# spdlog内部定义了各种特点的sink, 使用时可以直接包含相关头文件
./_install/include/spdlog/sinks/xxx_sink.h
```

```cc
#include "spdlog/sinks/basic_file_sink.h"
auto sink1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>("3.log");
```

## 格式化

| 模式标记        | 描述                               |
|----------------|------------------------------------|
| `%v`           | 实际日志消息文本                   |
| `%l`           | 日志级别（如：info, error）        |
| `%L`           | 日志级别短形式（如：I, E）         |
| `%n`           | 记录器名称                         |
| `%P`           | 进程 ID                            |
| `%t`           | 线程 ID                            |
| `%a`           | 星期几的缩写（如：Mon）            |
| `%A`           | 星期几的全称（如：Monday）         |
| `%b`           | 月份的缩写（如：Jan）              |
| `%B`           | 月份的全称（如：January）          |
| `%c`           | 日期和时间（如：Thu Dec 4 18:56）  |
| `%C`           | 年份的后两位（如：25）             |
| `%Y`           | 四位年份（如：2025）               |
| `%D` 或 `%x`   | 日期（如：10/01/25）               |
| `%m`           | 月份（01-12）                      |
| `%d`           | 日（01-31）                        |
| `%H`           | 小时（00-23）                      |
| `%I`           | 小时（01-12）                      |
| `%M`           | 分钟（00-59）                      |
| `%S`           | 秒（00-59）                        |
| `%e`           | 毫秒（000-999）                    |
| `%f`           | 微秒（000000-999999）              |
| `%F`           | 纳秒（000000000-999999999）        |
| `%p`           | AM/PM                              |
| `%r`           | 12 小时制时间（如：06:55:15 PM）   |
| `%R`           | 24 小时制时间（如：18:55）         |
| `%T` 或 `%X`   | 时间（如：18:55:15）               |
| `%z`           | UTC 偏移（如：+0800）              |
| `%+`           | 完整 ISO8601 时间戳                |
| `%%`           | 百分号（%）                        |

你还可以使用格式修饰符（如 `%5l` 表示日志级别宽度为 5，左对齐；`%-5l` 为右对齐）。

如果需要自定义 pattern，可以在创建 logger 或 sink 时指定：

```cpp
// filepath: example.cpp
#include "spdlog/spdlog.h"
#include "spdlog/sinks/stdout_color_sinks.h"

int main() {
    auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
    console_sink->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    auto logger = std::make_shared<spdlog::logger>("my_logger", console_sink);
    spdlog::register_logger(logger);
    logger->info("这是一条自定义格式的日志");
    return 0;
}
```


## 源文件定位
```c
// 源文件定位
SPDLOG_INFO("hello");
// [2025-10-01 12:07:27.367] [info] [main.cc:58] hello

SPDLOG_LOGGER_INFO(spdlog::get("two sinks"), "1111111111");
// [2025-10-01 12:07:27.367] [two sinks] [info] [main.cc:60] 1111111111
```

## 异步日志
```cc
// 异步日志
// 工厂方法
// 优点方便，缺点只能绑定一个sink，且sink类型和工厂类型绑定
auto logger1 = spdlog::stdout_color_mt<spdlog::async_factory>("Factory");
logger1->info("------1------");

// 手动创建
// 只能绑定一个sink，且sink类型可以任意
auto logger2 = spdlog::create_async<spdlog::sinks::basic_file_sink_mt>("async", "2.log");
logger2->info("-------2--------");

// 手动创建
// spdlog::init_thread_pool(1024, 1);
auto sink1 = std::make_shared<spdlog::sinks::basic_file_sink_mt>("3.log");
auto sink2 = std::make_shared<spdlog::sinks::basic_file_sink_mt>("4.log");
std::vector<spdlog::sink_ptr> sinks = {sink1, sink2};
// logger3为异步日志，指向 sink1 sink2 , 当缓存队列满了，选择丢弃最老的日志，而非阻塞
auto logger3 = std::make_shared<spdlog::async_logger>("async2", sinks.begin(), sinks.end(), spdlog::thread_pool(), spdlog::async_overflow_policy::overrun_oldest);
logger3->info("--------3-------");
```


## 定制刷新策略
默认情况:sink直接写到用户缓存区，刷新时才写入真正目标。
```c
// logger1手动触发立即刷新
logger1->flush();
// logger1至少出现err级别的消息才立即刷新
logger1->flush_on(spdlog::level::err);
// registry管理的所有logger，每4秒刷新一次
spdlog::flush_every(std::chrono::seconds(4));
```



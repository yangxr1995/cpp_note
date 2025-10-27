#include "spdlog/common.h"
#include "spdlog/details/synchronous_factory.h"
#include "spdlog/logger.h"
#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include <chrono>
#include <memory>
#include <vector>

int main (int argc, char *argv[]) {

    // 1. 使用默认logger
    spdlog::info("hello world");

    {
        // 2. 使用工厂方法创建logger
        std::shared_ptr<spdlog::logger> slogger_mt = spdlog::stdout_color_mt("thread safe, sync logger");
        slogger_mt->info("hello");

        std::shared_ptr<spdlog::logger> slogger_st = spdlog::stdout_color_st<spdlog::synchronous_factory>("thread unsafe, sync logger");
        slogger_st->info("hello");

        // 创建异步日志
        std::shared_ptr<spdlog::logger> alogger_mt = spdlog::stdout_color_mt<spdlog::async_factory>("thread safe, async logger");

        spdlog::get("thread safe, sync logger")->info("0000000000");
    }

    {
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
    }

    {
        // 4. 自定义格式化
        auto sink1 = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
        sink1->set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");
        // 创建logger并设置sinks
        auto logger = std::make_shared<spdlog::logger>("show pattern");
        logger->sinks().push_back(sink1);

        spdlog::register_logger(logger);
        logger->info("hello");
    }
    
    {
        // 源文件定位
        SPDLOG_INFO("hello");
        SPDLOG_LOGGER_INFO(spdlog::get("two sinks"), "1111111111");
    }

    {
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

        // logger1手动触发立即刷新
        logger1->flush();
        // logger1至少出现err级别的消息才立即刷新
        logger1->flush_on(spdlog::level::err);
        // registry管理的所有logger，每4秒刷新一次
        spdlog::flush_every(std::chrono::seconds(4));

    }

    
    return 0;
}

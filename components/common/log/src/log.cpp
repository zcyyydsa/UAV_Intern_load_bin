#include "log.h"

//#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO
#include <spdlog/sinks/stdout_sinks.h>
#include <spdlog/sinks/rotating_file_sink.h>

namespace hgdf {

int log_init()
{
    auto console_sink = std::make_shared<spdlog::sinks::stdout_sink_mt>();
    // Create a file rotating logger with 5mb size max and 3 rotated files.
    auto max_size = 1048576 * 5;
    auto max_file = 3;
    auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>("/userdata/d04.log", max_size, max_file);

    spdlog::sinks_init_list log_sinks = {console_sink, file_sink};
    auto logger = std::make_shared<spdlog::logger>("D04", log_sinks);
    spdlog::set_default_logger(logger);
    spdlog::set_pattern("[%Y-%m-%d %H:%M:%S.%e] [%l] %v");

    // trace < debug < info < warn < error < critical
    spdlog::flush_on(spdlog::level::info);
    //spdlog::flush_every(std::chrono::seconds(60));

    return 0;
}

void log_deinit()
{
    spdlog::shutdown();
}

}
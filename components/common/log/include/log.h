#ifndef __LOG_H_
#define __LOG_H_

#define SPDLOG_ACTIVE_LEVEL SPDLOG_LEVEL_INFO

#include <spdlog/spdlog.h>

namespace hgdf {

int log_init();
void log_deinit();

}

#define HGLOG_TRACE(...) SPDLOG_TRACE(__VA_ARGS__)
#define HGLOG_DEBUG(...) SPDLOG_DEBUG(__VA_ARGS__)
#define HGLOG_INFO(...) SPDLOG_INFO(__VA_ARGS__)
#define HGLOG_WARN(...) SPDLOG_WARN(__VA_ARGS__)
#define HGLOG_ERROR(...) SPDLOG_ERROR(__VA_ARGS__)
#define HGLOG_CRITICAL(...) SPDLOG_CRITICAL(__VA_ARGS__)

#endif // __HGMEDIA_H_

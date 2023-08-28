#include <stdio.h>

#include "log.h"

int main()
{
    hgdf::log_init();

    // log level
    // trace < debug < info < warn < err < critical
    HGLOG_TRACE("log trace.");
    HGLOG_DEBUG("log debug.");
    HGLOG_INFO("log info.");
    HGLOG_WARN("log warn.");
    HGLOG_ERROR("log err.");
    HGLOG_CRITICAL("log critical.");

    int num = 100;
    HGLOG_INFO("num = {}", num);
    float val = 9.8;
    HGLOG_INFO("val = {}", val);

    hgdf::log_deinit();
    return 0;
}


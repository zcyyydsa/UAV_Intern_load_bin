
if(CONFIG_LOG_ENABLED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(log REQUIRED spdlog)
list(APPEND ADD_INCLUDE ${log_INCLUDE_DIRS})
list(APPEND ADD_LINK_SEARCH_PATH ${log_LIBRARY_DIRS})
list(APPEND ADD_REQUIREMENTS ${log_LIBRARIES})


list(APPEND ADD_INCLUDE "include" ${log_INCLUDE_DIRS})
list(APPEND ADD_SRCS  "src/log.cpp")

register_component()

endif()

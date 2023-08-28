
if(CONFIG_HGMUXER_ENABLED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(muxer REQUIRED hgflv hgmov)
list(APPEND ADD_INCLUDE ${muxer_INCLUDE_DIRS})
list(APPEND ADD_LINK_SEARCH_PATH ${muxer_LIBRARY_DIRS})
list(APPEND ADD_REQUIREMENTS ${muxer_LIBRARIES})

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_STATIC_LIB "lib/libhgmuxer.a")

register_component()

endif()

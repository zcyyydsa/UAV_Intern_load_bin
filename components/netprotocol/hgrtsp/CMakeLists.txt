
if(CONFIG_HGRTSP_ENABLED)

find_package(PkgConfig REQUIRED)
pkg_check_modules(rtsp REQUIRED hgrtsp hgrtp hghttp hgtoolkit hgaio hgflv hgmov hgavcodec)
list(APPEND ADD_INCLUDE ${rtsp_INCLUDE_DIRS})
list(APPEND ADD_LINK_SEARCH_PATH ${rtsp_LIBRARY_DIRS})
list(APPEND ADD_REQUIREMENTS ${rtsp_LIBRARIES})

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_STATIC_LIB "lib/libhgrtsp.a")

register_component()

endif()

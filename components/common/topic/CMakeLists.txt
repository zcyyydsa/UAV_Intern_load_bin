
if(CONFIG_HGTOPIC_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_STATIC_LIB "lib/libhgtopic.a")

register_component()

endif()

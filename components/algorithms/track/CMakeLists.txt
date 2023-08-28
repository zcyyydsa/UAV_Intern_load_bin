
if(CONFIG_HGTRACK_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_STATIC_LIB "lib/libhgtrack.a")

register_component()

endif()

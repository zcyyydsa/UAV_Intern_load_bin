
if(CONFIG_PTZ_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_STATIC_LIB "lib/libhgptz.a")

register_component()

endif()


if(CONFIG_FLIGHTCTL_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_STATIC_LIB "lib/libhgflightctrl.a")

register_component()

endif()


if(CONFIG_DETECT_DIGITAL_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_STATIC_LIB "lib/libhgddigital.a")

register_component()

endif()

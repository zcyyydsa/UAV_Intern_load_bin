
if(CONFIG_HGMEDIA_ENABLED)

list(APPEND ADD_INCLUDE "include")
#list(APPEND ADD_DYNAMIC_LIB "lib/libhgmedia.so")
list(APPEND ADD_STATIC_LIB "lib/libhgmedia.a")
list(APPEND ADD_DYNAMIC_LIB "lib/libeasymedia.so")

register_component()

endif()

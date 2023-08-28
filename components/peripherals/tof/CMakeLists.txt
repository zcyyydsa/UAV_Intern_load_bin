
if(CONFIG_TOF_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_SRCS  "src/hg_tof.cpp")

#list(APPEND ADD_REQUIREMENTS  uart)
register_component()

endif()

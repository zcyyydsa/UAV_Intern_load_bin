
if(CONFIG_RC_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_SRCS  "src/hg_rc.cpp")

register_component()

endif()

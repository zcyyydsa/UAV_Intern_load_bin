
if(CONFIG_HGPROTOCOL_ENABLED)

list(APPEND ADD_INCLUDE "include")

list(APPEND ADD_SRCS  "src/hg_protocol.cpp")

register_component()

endif()

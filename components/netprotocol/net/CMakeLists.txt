
if(CONFIG_HGNET_ENABLED)

list(APPEND ADD_INCLUDE "include")

list(APPEND ADD_SRCS  "src/hg_net_protocol.cpp" "src/udp_client.cpp")

list(APPEND ADD_REQUIREMENTS  protocol)
register_component()

endif()


if(CONFIG_WIFI_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_SRCS  "src/hg_wifi.cpp" "src/hgwifi.cpp")

register_component()

endif()

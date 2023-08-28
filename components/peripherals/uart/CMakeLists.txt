
if(CONFIG_UART_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_SRCS  "src/uart.cpp" "src/hg_uart.cpp")

register_component()

endif()

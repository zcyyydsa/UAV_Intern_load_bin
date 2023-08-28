
if(CONFIG_SERVO_ENABLED)

list(APPEND ADD_INCLUDE "include")
list(APPEND ADD_SRCS  "src/hg_steering_engine.cpp")

###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS uart)
register_component()

endif()

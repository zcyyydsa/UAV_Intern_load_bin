############### Add include ###################
list(APPEND ADD_INCLUDE ""
    )
list(APPEND ADD_PRIVATE_INCLUDE "")

############ Add source files #################
append_srcs_dir(ADD_SRCS "src")

###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS log tof uart)


###### Add link search path for requirements/libs ######
#list(APPEND ADD_REQUIREMENTS pthread m)  # add system libs, pthread and math lib for example here

register_component()



############### Add include ###################
list(APPEND ADD_INCLUDE ""
    )
list(APPEND ADD_PRIVATE_INCLUDE "")

############ Add source files #################
append_srcs_dir(ADD_SRCS "src")

###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS net)

###### Add link search path for requirements/libs ######
list(APPEND ADD_REQUIREMENTS pthread m drm)

register_component()


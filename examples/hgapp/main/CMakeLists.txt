#set(HGAPP ${CMAKE_CURRENT_SOURCE_DIR})
#set(LIB_FILE ${HGAPP}/lib/libhgprotocol.a)
#set(LIB_FILE ${HGAPP}/lib/libapp.a)

add_compile_options(-std=c++11)

############### Add include ###################
#list(APPEND ADD_INCLUDE "${HGAPP}/include")
list(APPEND ADD_INCLUDE "")
list(APPEND ADD_PRIVATE_INCLUDE "")

############ Add source files #################
append_srcs_dir(ADD_SRCS "src")

find_package(PkgConfig REQUIRED)
pkg_check_modules(opencv REQUIRED opencv)
list(APPEND ADD_INCLUDE ${opencv_INCLUDE_DIRS})
list(APPEND ADD_LINK_SEARCH_PATH ${opencv_LIBRARY_DIRS})
list(APPEND ADD_REQUIREMENTS ${opencv_LIBRARIES})

# src dir
set(SRC_SUB_DIR_LIST "commom" "media" "algo" "msg" "dev")
foreach(SUB_DIR ${SRC_SUB_DIR_LIST})
	list(APPEND ADD_INCLUDE src/${SUB_DIR})
	aux_source_directory(src/${SUB_DIR} ADD_SRCS)
endforeach()


###### Add required/dependent components ######
list(APPEND ADD_REQUIREMENTS #${LIB_FILE} 
	hgmedia hgmuxer hgrtsp wifi log topic protocol
	flightctrl ptz uart tof servo rc track detect_digital -lfftw3f
)

###### Add link search path for requirements/libs ######
list(APPEND ADD_REQUIREMENTS pthread m drm)  # add system libs, pthread and math lib for example here

register_component()


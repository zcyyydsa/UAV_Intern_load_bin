set(CMAKE_SYSTEM_NAME Linux)
set(CMAKE_SYSTEM_PROCESSOR aarch64)

set(TOOLCHAIN "/opt/rk3399pro_host")
set(CMAKE_C_COMPILER ${TOOLCHAIN}/bin/aarch64-linux-gcc)
set(CMAKE_CXX_COMPILER ${TOOLCHAIN}/bin/aarch64-linux-g++)
set(CAMKE_FIND_ROOT_PATH ${TOOLCHAIN}/aarch64-buildroot-linux-gnu/sysroot/)

set(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY ONLY)
set(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE ONLY)

set(SYSROOT_PKGCONFIG "/opt/rk3399pro_host/aarch64-buildroot-linux-gnu/sysroot/usr/lib/pkgconfig")
set(ENV{PKG_CONFIG_PATH} "${SYSROOT_PKGCONFIG}:ENV{PKG_CONFIG_PATH}")

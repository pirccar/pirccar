cmake_minimum_required(VERSION 2.8)
project( carserver )
SET(COMPILE_DEFINITIONS -Werror)
include_directories(/opt/vc/include)
include_directories(/opt/vc/userland-master/interface/vcos)
include_directories(/opt/vc/userland-master)
include_directories(/opt/vc/userland-master/interface/vcos/pthreads)
include_directories(/opt/vc/userland-master/interface/vmcs_host/linux)
include_directories(/opt/vc/userland-master/host_applications/linux/libs/bcm_host/include)
include_directories(/usr/local/include)
include_directories(/home/pi/test)
link_directories(/opt/vc/lib)
link_directories(/usr/local/lib)

add_executable(carserver carserver.cpp socket.cpp camera.cpp)

#target_link_libraries(carserver ${raspicam_LIBS})
target_link_libraries(carserver libjpeg.so raspicam)


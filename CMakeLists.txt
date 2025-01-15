cmake_minimum_required(VERSION 3.10)
project(voir_project)
set(CMAKE_CXX_FLAGS "-O3 -Wall -Wextra")


# Recherche d'OpenCV
find_package(PkgConfig REQUIRED)
find_package(OpenCV REQUIRED)

# Recherche de LibCamera
pkg_check_modules(LIBCAMERA REQUIRED libcamera)
message(STATUS "libcamera library found:")
message(STATUS "    version: ${LIBCAMERA_VERSION}")
message(STATUS "    libraries: ${LIBCAMERA_LINK_LIBRARIES}")
message(STATUS "    include path: ${LIBCAMERA_INCLUDE_DIRS}")

# Recherche de RPiCamApp
pkg_check_modules(RPICAM_APPS REQUIRED rpicam_app)
message(STATUS "rpicam-app library found:")
message(STATUS "    version: ${RPICAM_APPS_VERSION}")
message(STATUS "    libraries: ${RPICAM_APPS_LINK_LIBRARIES}")
message(STATUS "    include path: ${RPICAM_APPS_INCLUDE_DIRS}")
include_directories(${CMAKE_SOURCE_DIR} ${RPICAM_APPS_INCLUDE_DIRS})

# Recherche de RPiCamOPENCV
pkg_check_modules(RPICAMOPENCV REQUIRED rpicamopencv)

pkg_check_modules(RPICAMOPENCV REQUIRED rpicamopencv)
message(STATUS "rpicamopencv library found:")
message(STATUS "    version: ${RPICAMOPENCV_VERSION}")
message(STATUS "    libraries: ${RPICAMOPENCV_LINK_LIBRARIES}")
message(STATUS "    include path: ${RPICAMOPENCV_INCLUDE_DIRS}")

# Rechercher de Boost et program_options
find_package(Boost REQUIRED COMPONENTS program_options)
message(STATUS "Boost version: ${Boost_VERSION}")
message(STATUS "Boost libraries: ${Boost_LIBRARIES}")
message(STATUS "Boost include dir: ${Boost_INCLUDE_DIRS}")

execute_process (
    COMMAND bash -c "pkg-config rpicam_app --libs"
    OUTPUT_VARIABLE rpicam_app_lib
)
#string(REGEX REPLACE "\n$" "" rpicam_app_lib "${rpicam_app_lib}")
string(STRIP "${rpicam_app_lib}" rpicam_app_lib)

add_executable(voir src/main.cc src/picam.cc)
add_executable(master src/master.cc src/picam.cc src/sgbm.cc)
add_executable(slave src/slave.cc src/picam.cc)

target_include_directories(voir PRIVATE ${OpenCV_INCLUDE_DIRS} ${LIBCAMERA_INCLUDE_DIRS}  ${RPICAM_APPS_INCLUDE_DIRS} ${RPICAMOPENCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

target_include_directories(master PRIVATE ${OpenCV_INCLUDE_DIRS} ${LIBCAMERA_INCLUDE_DIRS}  ${RPICAM_APPS_INCLUDE_DIRS} ${RPICAMOPENCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})

target_include_directories(slave PRIVATE ${OpenCV_INCLUDE_DIRS} ${LIBCAMERA_INCLUDE_DIRS}  ${RPICAM_APPS_INCLUDE_DIRS} ${RPICAMOPENCV_INCLUDE_DIRS} ${Boost_INCLUDE_DIRS})
# Lien avec les bibliothèques OpenCV

target_link_libraries(voir PRIVATE  ${OpenCV_LIBS} ${rpicam_app_lib} ${RPICAMOPENCV_LINK_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(master PRIVATE ${OpenCV_LIBS} ${rpicam_app_lib} ${RPICAMOPENCV_LINK_LIBRARIES} ${Boost_LIBRARIES} ${OpenCV_LIBS} ${rpicam_app_lib} ${RPICAMOPENCV_LINK_LIBRARIES} ${Boost_LIBRARIES})
target_link_libraries(slave PRIVATE  ${OpenCV_LIBS} ${rpicam_app_lib} ${RPICAMOPENCV_LINK_LIBRARIES} ${Boost_LIBRARIES})
# Inclure les répertoires d'en-têtes d'OpenCV LIBCAMERA et RPICAM

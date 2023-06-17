cmake_minimum_required(VERSION 2.8.3)
project(imu_gps_localizer_yq)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++14")

find_package(Eigen3  REQUIRED)

# Add GeographicLib.
add_subdirectory(third_party/GeographicLib_yq)

include_directories(
  include
  ${EIGEN3_INCLUDE_DIR}
  third_party/GeographicLib_yq/include
)

add_library(imu_gps_localizer_lib_yq
	src/imu_gps_localizer.cpp
  src/initializer.cpp
  src/imu_processor.cpp
  src/gps_processor.cpp
)

target_link_libraries(imu_gps_localizer_lib_yq
	${EIGEN3_LIBS}
  GeographicLib_yq
  glog
)
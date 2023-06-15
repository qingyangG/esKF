cmake_minimum_required(VERSION 2.8.3)
project(ekf2d)

add_compile_options(-std=c++14)

## Find catkin macros and libraries
find_package(catkin REQUIRED COMPONENTS
  roscpp
  nav_msgs
)

catkin_package()

add_subdirectory(imu_gps_localizer_yq)

include_directories(
  ${catkin_INCLUDE_DIRS}
  ${EIGEN3_INCLUDE_DIR}
  imu_gps_localizer_yq/include
  ros_wrapper_yq/include
  imu_gps_localizer_yq/third_party/GeographicLib_yq/include
)

add_library(ros_wrapper_lib_yq
	ros_wrapper_yq/src/localization_wrapper.cpp
)
target_link_libraries(ros_wrapper_lib_yq
  ${catkin_LIBRARIES}
  imu_gps_localizer_lib_yq
  glog
)

## Localization node.
add_executable(${PROJECT_NAME}_node ros_wrapper_yq/src/localization_node.cpp)
target_link_libraries(${PROJECT_NAME}_node
  ros_wrapper_lib_yq
)

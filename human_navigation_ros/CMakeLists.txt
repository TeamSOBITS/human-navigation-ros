cmake_minimum_required(VERSION 3.8)
project(human_navigation_ros)

if(CMAKE_COMPILER_IS_GNUCXX OR CMAKE_CXX_COMPILER_ID MATCHES "Clang")
  add_compile_options(-Wall -Wextra -Wpedantic)
endif()

# find dependencies
find_package(ament_cmake REQUIRED)
find_package(rclcpp REQUIRED)
find_package(std_msgs REQUIRED)
find_package(sensor_msgs REQUIRED)
find_package(geometry_msgs REQUIRED)
find_package(trajectory_msgs REQUIRED)
find_package(tf2 REQUIRED)
find_package(tf2_ros REQUIRED)
find_package(tf2_geometry_msgs REQUIRED)
find_package(human_navigation REQUIRED)

# uncomment the following section in order to fill in
# further dependencies manually.
# find_package(<dependency> REQUIRED)
include_directories(
  /usr/local/include
)

#-- for Launch
install(DIRECTORY launch/
DESTINATION share/${PROJECT_NAME}/launch)
#--- end Launch

if(BUILD_TESTING)
  find_package(ament_lint_auto REQUIRED)
  # the following line skips the linter which checks for copyrights
  # comment the line when a copyright and license is added to all source files
  set(ament_cmake_copyright_FOUND TRUE)
  # the following line skips cpplint (only works in a git repo)
  # comment the line when this package is in a git repo and when
  # a copyright and license is added to all source files
  set(ament_cmake_cpplint_FOUND TRUE)
  ament_lint_auto_find_test_dependencies()
endif()

include_directories(include)
link_directories(/usr/local/lib)
add_executable(human_navigation_sample src/human_navigation_sample.cpp)
add_executable(hsr_key_teleop src/hsr_key_teleop.cpp)

ament_target_dependencies(human_navigation_sample
  rclcpp
  std_msgs
  human_navigation
)

ament_target_dependencies(hsr_key_teleop
  rclcpp
  geometry_msgs
  sensor_msgs
  trajectory_msgs
  tf2
  tf2_ros
  tf2_geometry_msgs
  human_navigation
)

install(TARGETS
  human_navigation_sample
  hsr_key_teleop
  DESTINATION lib/${PROJECT_NAME}
)
#--- end C++

target_link_libraries(human_navigation_sample)
target_link_libraries(hsr_key_teleop)
ament_package()

# ROS2 Package for Human Navigation

This project is ROS2 package for the Human Navigation task of the RoboCup@Home Simulation.

See also [wiki page](https://github.com/RoboCupatHomeSim/human-navigation-ros/wiki).

## Prerequisites

Same as below for OS and ROS2 version.  
https://github.com/RoboCupatHomeSim/documents/blob/master/SoftwareManual/Environment.md#ubuntu-pc

## How to Install

### Install Rosbridge Server

Please see below.  
http://wiki.ros.org/rosbridge_suite

### Install SIGVerse Rosbridge Server

Please see below.  
https://github.com/SIGVerse/ros_package/tree/humble-devel/sigverse_ros_bridge

### Install ROS Package of Human Navigation

```bash:
$ cd ~/colcon_ws/src
$ git clone -b humble-devel https://github.com/RoboCupatHomeSim/human-navigation-ros.git
$ cd ..
$ colcon build
```

## How to Execute

### How to Execute Sample ROS2 Node

This sample ROS2 node communicates with the Unity application of Human Navigation.  
HSR can be operated with keyboard operation.

```bash:
$ ros2 launch human_navigation_ros sample.launch.py
```

## License

This project is licensed under the SIGVerse License - see the LICENSE.txt file for details.

import launch
from launch import LaunchDescription
from launch_ros.actions import Node
from launch.actions import DeclareLaunchArgument, IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import LaunchConfiguration
from ament_index_python.packages import get_package_share_directory
import os

def generate_launch_description():
    sub_msg_to_robot_topic_name = LaunchConfiguration('sub_msg_to_robot_topic_name', default='/human_navigation/message/to_robot')
    sum_msg_task_info_topic_name = LaunchConfiguration('sum_msg_task_info_topic_name', default='/human_navigation/message/task_info')
    sub_msg_avatar_status_topic_name = LaunchConfiguration('sub_msg_avatar_status_topic_name', default='/human_navigation/message/avatar_status')
    sum_msg_object_status_topic_name = LaunchConfiguration('sum_msg_object_status_topic_name', default='/human_navigation/message/object_status')
    pub_msg_to_moderator_topic_name = LaunchConfiguration('pub_msg_to_moderator_topic_name', default='/human_navigation/message/to_moderator')
    pub_msg_guidance_message_topic_name = LaunchConfiguration('pub_msg_guidance_message_topic_name', default='/human_navigation/message/guidance_message')
    ros_bridge_port = LaunchConfiguration('ros_bridge_port', default='9090')

    sub_joint_state_topic_name = LaunchConfiguration('sub_joint_state_topic_name', default='sub_joint_state_topic')
    pub_base_twist_topic_name = LaunchConfiguration('pub_base_twist_topic_name', default='pub_base_twist_topic')
    pub_arm_trajectory_topic_name = LaunchConfiguration('pub_arm_trajectory_topic_name', default='pub_arm_trajectory_topic')
    pub_gripper_trajectory_topic_name = LaunchConfiguration('pub_gripper_trajectory_topic_name', default='pub_gripper_trajectory_topic')
    
    human_navigation_node = Node(
        package='human_navigation_ros',
        executable='human_navigation_sample',
        name='human_navigation_sample',
        output='screen',
        parameters=[{
            'sub_msg_to_robot_topic_name': sub_msg_to_robot_topic_name,
            'sum_msg_task_info_topic_name': sum_msg_task_info_topic_name,
            'sub_msg_avatar_status_topic_name': sub_msg_avatar_status_topic_name,
            'sum_msg_object_status_topic_name': sum_msg_object_status_topic_name,
            'pub_msg_to_moderator_topic_name': pub_msg_to_moderator_topic_name,
            'pub_msg_guidance_message_topic_name': pub_msg_guidance_message_topic_name
        }]
    )

    human_navigation_hsr_key_teleop_node = Node(
        package='human_navigation_ros',
        executable='hsr_key_teleop',
        name='human_navigation_hsr_key_teleop',
        output='screen',
        prefix='xterm -font r16 -fg floralwhite -bg darkslateblue -e',
        parameters=[{
            'sub_joint_state_topic_name': sub_joint_state_topic_name,
            'pub_base_twist_topic_name': pub_base_twist_topic_name,
            'pub_arm_trajectory_topic_name': pub_arm_trajectory_topic_name,
            'pub_gripper_trajectory_topic_name': pub_gripper_trajectory_topic_name
        }]
    )

    sigverse_ros_bridge_launch_file = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(
                get_package_share_directory('sigverse_ros_bridge'),
                'launch',
                'sigverse_ros_bridge.launch.py'
            )
        ),
        launch_arguments={'port': ros_bridge_port}.items()
    )

    rviz_node = Node(
        package='rviz2',
        executable='rviz2',
        name='rviz',
        arguments=['-d', os.path.join(get_package_share_directory('human_navigation_ros'), 'launch', 'hsr.rviz')]
    )

    return LaunchDescription([
        human_navigation_node,
        human_navigation_hsr_key_teleop_node,
        sigverse_ros_bridge_launch_file,
        rviz_node
    ])

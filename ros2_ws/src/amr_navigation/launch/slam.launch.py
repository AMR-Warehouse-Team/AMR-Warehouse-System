from pathlib import Path

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import DeclareLaunchArgument
from launch.substitutions import LaunchConfiguration
from launch_ros.actions import Node


def generate_launch_description():
    pkg_share = Path(get_package_share_directory("amr_navigation"))
    default_params = pkg_share / "config" / "mapper_params_online_async.yaml"

    use_sim_time = LaunchConfiguration("use_sim_time")
    slam_params = LaunchConfiguration("slam_params_file")

    return LaunchDescription([
        DeclareLaunchArgument(
            "use_sim_time",
            default_value="false",
            description="Use simulation clock (true for Gazebo, false for hardware)",
        ),
        DeclareLaunchArgument(
            "slam_params_file",
            default_value=str(default_params),
            description="Full path to the slam_toolbox parameters file",
        ),
        Node(
            package="slam_toolbox",
            executable="async_slam_toolbox_node",
            name="slam_toolbox",
            output="screen",
            parameters=[slam_params, {"use_sim_time": use_sim_time}],
        ),
    ])

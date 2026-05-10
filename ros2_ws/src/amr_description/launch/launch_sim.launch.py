import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.actions import IncludeLaunchDescription
from launch.launch_description_sources import PythonLaunchDescriptionSource
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    pkg_share     = get_package_share_directory('amr_description')
    gazebo_share  = get_package_share_directory('gazebo_ros')

    xacro_file = os.path.join(pkg_share, 'urdf', 'amr_robot.urdf.xacro')

    robot_description = ParameterValue(Command(['xacro ', xacro_file]), value_type=str)

    # ── Gazebo server only (headless — no gzclient, safe without a GPU) ───────
    gazebo = IncludeLaunchDescription(
        PythonLaunchDescriptionSource(
            os.path.join(gazebo_share, 'launch', 'gzserver.launch.py')
        )
    )

    # ── Robot State Publisher (sim time ON) ─────────────────────────────────
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{
            'robot_description': robot_description,
            'use_sim_time': True,
        }],
    )

    # ── Spawn URDF entity into Gazebo ───────────────────────────────────────
    spawn_entity = Node(
        package='gazebo_ros',
        executable='spawn_entity.py',
        arguments=[
            '-topic', 'robot_description',
            '-entity', 'amr_robot',
        ],
        output='screen',
    )

    # ── RViz2 (sim time ON, uses the existing display config) ──────────────
    rviz2 = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        arguments=['-d', os.path.join(pkg_share, 'rviz', 'display.rviz')],
        parameters=[{'use_sim_time': True}],
    )

    return LaunchDescription([
        gazebo,
        robot_state_publisher,
        spawn_entity,
        rviz2,
    ])

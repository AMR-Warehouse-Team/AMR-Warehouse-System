import os

from ament_index_python.packages import get_package_share_directory
from launch import LaunchDescription
from launch.substitutions import Command
from launch_ros.actions import Node
from launch_ros.parameter_descriptions import ParameterValue


def generate_launch_description():
    amr_description_share = get_package_share_directory('amr_description')

    xacro_file  = os.path.join(amr_description_share, 'urdf', 'amr_robot.urdf.xacro')
    rviz_config = os.path.join(amr_description_share, 'rviz', 'display.rviz')

    robot_description = ParameterValue(Command(['xacro ', xacro_file]), value_type=str)

    # ── Robot State Publisher ───────────────────────────────────────────────
    # Publishes /robot_description and all static TF transforms from the URDF
    # (base_footprint → base_link → laser, camera_link, etc.)
    robot_state_publisher = Node(
        package='robot_state_publisher',
        executable='robot_state_publisher',
        output='screen',
        parameters=[{'robot_description': robot_description}],
    )

    # ── RPLIDAR C1 ──────────────────────────────────────────────────────────
    # frame_id overridden to 'laser' so /scan messages match the URDF TF
    # tree without needing an extra static_transform_publisher.
    #
    # Alternative (if you cannot set frame_id here): add a
    # static_transform_publisher node bridging the frames with a zero transform:
    #   Node(package='tf2_ros', executable='static_transform_publisher',
    #        arguments=['0','0','0','0','0','0','laser','laser'])
    sllidar = Node(
        package='sllidar_ros2',
        executable='sllidar_node',
        name='sllidar_node',
        output='screen',
        parameters=[{
            'serial_port':      '/dev/ttyUSB0',
            'serial_baudrate':  460800,          # RPLIDAR C1 baud rate
            'frame_id':         'laser',    # ← matches URDF link name
            'inverted':         False,
            'angle_compensate': True,
        }],
    )

    # ── RViz2 ───────────────────────────────────────────────────────────────
    rviz2 = Node(
        package='rviz2',
        executable='rviz2',
        output='screen',
        arguments=['-d', rviz_config],
    )

    return LaunchDescription([
        robot_state_publisher,
        sllidar,
        rviz2,
    ])

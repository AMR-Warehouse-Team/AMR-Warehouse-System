# ROS2 Humble — Laptop Install (Developer Laptop)

Installed on: May 3, 2026
Ubuntu version: 22.04.5 LTS
Hostname: developer-laptop

## Verified working
- Ran `source /opt/ros/humble/setup.bash` in the shell used for verification.
- Ran `echo $ROS_DISTRO`; output was `humble`.
- Ran `ros2 run demo_nodes_cpp talker` in one terminal and `ros2 run demo_nodes_py listener` in another; listener output included `I heard: [Hello World: ...]`.

## Issues encountered
- Resolved initial apt configuration conflicts.

## Time taken
1 hour

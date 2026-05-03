# AMR Warehouse System

Autonomous Mobile Robot for medicine warehouse picking.
Graduation Project — University of Technology Bahrain — 2026.

## Team

| Name | Role | Owns |
|------|------|------|
| Nathalie Alomari | Team Lead, Autonomy | `ros2_ws/` (navigation, state machine) |
| Abdalla Elradi   | Vision & Web GUI    | `web_app/`, `ros2_ws/vision` |
| Mahanna Al-Hamimi | Motion & Power     | `firmware/` (Teensy 4.1) |
| Zahra Hasan      | Payload & Docs      | Lift/gripper code, `docs/` |

## Repo Structure

- `web_app/`   — Flask dashboard + MQTT (high-level control)
- `ros2_ws/`   — ROS2 workspace (navigation, vision, state machine)
- `firmware/`  — PlatformIO project for Teensy 4.1 (low-level)
- `docs/`      — Proposals, CAD, diagrams, BOM, thesis

## Workflow

1. Create a branch off `main` for your task: `git checkout -b yourname-taskID`
2. Commit with the task ID: `git commit -m "T0.5: flash Jetson with JetPack"`
3. Push and open a Pull Request to `main`.
4. At least one teammate reviews before merge.

## Demo target

July 15, 2026.

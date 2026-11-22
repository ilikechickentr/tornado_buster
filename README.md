# Tornado Buster

Tornado Buster is a drone-based disaster assessment project focused on using aerial imaging, sensors, and computer vision to help identify and assess damage caused by natural disasters.

The project aims to develop a drone system capable of collecting information from disaster-affected areas and presenting it through a custom desktop application. The application is written in C++ using Raylib, with a user interface designed for monitoring camera feeds, map data, sensor readings, and system status.

## Website

**[Visit the Tornado Buster Website](https://aarav825.github.io/Tornado-Buster-Site/)**

## Goals

- Collect aerial imagery and sensor data from disaster-affected areas.
- Explore computer vision and AI for detecting damage and hazards.
- Rank affected areas by how severely they were damaged and show the results on a map.
- Develop a custom ground-control interface for monitoring the drone.
- Display camera feeds, map information, telemetry, and sensor readings.
- Help make disaster assessment faster and more efficient.

## Ground Station App

The desktop app is a resizable window with a sidebar (drag its edge to change the width) and four tabs:

| Tab | What it shows |
| --- | --- |
| **Overview** | Console, mini map, four small camera feeds plus a large 16:9 feed, a sensor summary, and drone connection and battery status |
| **Camera Feed** | All camera feeds at full size |
| **Sensors** | One panel per sensor with a live graph |
| **Console** | Full-size log with a command line |

The interface has a Windows XP-inspired, modern "gamey" look with a light/dark mode switch.

> **Note:** the app currently runs on placeholder data. Camera feeds, map content, sensor readings, battery level, and connection status are simulated until the drone link is built.

## Planned Features

- **Drone link:** telemetry, video, and commands, most likely over WiFi (hardware and protocol not decided yet).
- **Damage detection:** a trained vision model that scores damage in each video frame.
- **Damage map:** the drone's trajectory drawn on the map, with areas colored and ranked by how destroyed they are.
- **Full Map tab:** a larger map with a ranked list of the worst-hit areas.
- **Console commands:** a working command line for controlling the app and drone.
- **Settings:** saving preferences such as theme and sidebar width.

## Tech Stack

- **C++23**: core application development
- **Raylib 5.5**: graphics, input, and window management
- **CMake**: build system (Raylib is fetched automatically)
- **Computer Vision & AI**: damage detection and image analysis (planned)
- **Drone Communication**: telemetry and control integration (planned)

## Project Structure

```text
tornado_buster/
├── CMakeLists.txt
├── main.cpp      # window, sidebar, pages, overview layout
├── sensors.cpp   # sensor data, summary view, and Sensors tab
├── sensors.h
└── theme.h       # colors and shared UI drawing helpers
```

## Building

Requirements: a C++23 compiler and CMake. Raylib is downloaded automatically on the first configure, so an internet connection is needed for that step.

```bash
git clone https://github.com/ilikechickentr/tornado_buster/
cd tornado_buster
cmake -S . -B build
cmake --build build
./build/tornado_buster
```

On Windows the executable is `build\tornado_buster.exe` (or inside a `Debug` / `Release` folder, depending on your generator).

## Controls

- **Click a sidebar item** to switch tabs.
- **Drag the sidebar's right edge** to resize it.
- **Light/Dark Mode button** (bottom of the sidebar) switches the theme.
- **Mouse wheel** scrolls the Sensors tab.

## License

Licensed under the MIT License. See [LICENSE](LICENSE) for details.

## Status

Currently in development. Features and technologies may change as the project evolves.

---

**Built by 4 friends :)**

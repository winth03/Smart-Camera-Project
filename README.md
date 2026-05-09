# Smart Camera with AI Hand Tracking & Node-RED Integration

![GitHub last commit](https://img.shields.io/github/last-commit/winth03/Smart-Camera-Project)
![GitHub stars](https://img.shields.io/github/stars/winth03/Smart-Camera-Project?style=social)
![GitHub top language](https://img.shields.io/github/languages/top/winth03/Smart-Camera-Project)
![GitHub language count](https://img.shields.io/github/languages/count/winth03/Smart-Camera-Project)

## Overview

The Smart Camera Project is a comprehensive, integrated system designed for real-time video streaming, AI-powered hand tracking, and interactive data visualization. It leverages an ESP32-CAM module for capturing and streaming video, a robust Python backend for advanced image processing (specifically real-time hand detection and finger counting using `cvzone`), and Node-RED for orchestrating data flows, providing a user-friendly interface, and enabling further automation. This project offers a modular and powerful framework for building smart camera applications with edge AI capabilities.

## Features

*   **Real-time Video Streaming**: Seamless acquisition and transmission of video feeds from an ESP32-CAM module via WebSockets.
*   **AI Hand Tracking**: Utilizes the `cvzone` library for accurate, real-time hand detection and finger counting in the video stream.
*   **WebSocket Communication**: Efficient, asynchronous data transfer between the ESP32-CAM, Python processing backend, and Node-RED.
*   **Node-RED Integration**: Provides a low-code environment for visualizing processed data, automating actions based on hand gestures, and extending system functionality.
*   **Modular Architecture**: Clearly separates embedded firmware, backend AI processing, and visualization/automation layers for enhanced flexibility, maintainability, and scalability.
*   **Configurable Settings**: Includes a `settings.conf` file for easy adjustment of project-wide parameters.

## Tech Stack

*   **Embedded Firmware**: C++ (ESP-IDF/Arduino Framework via PlatformIO) for the ESP32-CAM.
*   **Backend Processing**: Python 3.x with:
    *   `websockets`: For asynchronous WebSocket server communication.
    *   `cvzone`: For advanced computer vision tasks, specifically hand tracking.
    *   `opencv-python`: Core image processing functionalities.
    *   `numpy`: Numerical operations for image data.
*   **Automation & UI**: Node-RED (JavaScript) for flow-based programming, data visualization, and web dashboard capabilities.
*   **System Orchestration**: Shell Scripting (`bin/start.sh`) for launching and managing multiple services.

## Prerequisites

Before setting up the project, ensure you have the following hardware and software installed:

*   **Hardware**:
    *   An ESP32-CAM development board (e.g., AI-Thinker ESP32-CAM).
    *   A micro-USB cable for flashing the ESP32-CAM.
*   **Software**:
    *   **Python 3.8+**: Download from [python.org](https://www.python.org/downloads/).
    *   **Node.js (LTS recommended) and npm**: Download from [nodejs.org](https://nodejs.org/en/download/). Required for Node-RED.
    *   **PlatformIO Core CLI or VS Code Extension**: For compiling and uploading firmware to the ESP32-CAM. Install from [platformio.org](https://platformio.org/install).
    *   **Git**: For cloning the repository.

## Installation

Follow these steps to set up and run the Smart Camera Project:

1.  **Clone the Repository**:
    ```bash
    git clone https://github.com/winth03/Smart-Camera-Project.git
    cd Smart-Camera-Project
    ```

2.  **ESP32-CAM Firmware Setup**:
    *   Ensure PlatformIO CLI or the VS Code extension is installed.
    *   Navigate to the firmware directory:
        ```bash
        cd src/platform-io/Esp32CamWebserver
        ```
    *   Build and upload the firmware to your ESP32-CAM board. Connect your ESP32-CAM, select the correct serial port, and run:
        ```bash
        pio run -t upload
        ```
    *   The `CameraWebsocket.cpp` in this project configures the ESP32-CAM to act as a WebSocket client, sending image data to the Python backend. You might need to adjust network credentials or the Python backend's IP address within the firmware source code (e.g., `src/CameraWebsocket.cpp`) before uploading, depending on its implementation details. After upload, the ESP32-CAM should connect to your Wi-Fi and start streaming.

3.  **Python Backend Setup**:
    *   Navigate back to the project root and then into the Python directory:
        ```bash
        cd ../../python
        ```
    *   Create and activate a Python virtual environment (recommended):
        ```bash
        python3 -m venv venv
        source venv/bin/activate # On Windows, use `venv\Scripts\activate`
        ```
    *   Install the required Python packages:
        ```bash
        pip install -r requirements.txt
        ```
    *   The `main.py` script will run a WebSocket server on port `8000`, expecting raw image data from the ESP32-CAM.

4.  **Node-RED Setup**:
    *   Install Node-RED globally if you haven't already:
        ```bash
        npm install -g --unsafe-perm node-red
        ```
    *   Start Node-RED to generate its user directory:
        ```bash
        node-red
        ```
        You can stop it with `Ctrl+C` after it starts.
    *   Access the Node-RED editor (typically `http://localhost:1880`) and go to `Menu` (top right) -> `Import` -> `Select a file to import`.
    *   Navigate to `src/node-red/NodeRedCamera.json` and import the flow.
    *   You may need to install additional Node-RED nodes if prompted by the imported flow (e.g., `node-red-node-ui-dashboard` for visualization or `node-red-node-websocket` if the flow uses a client/server websocket node). Deploy the imported flow.

## Usage

The system is designed to be orchestrated via a single startup script:

1.  **Configure `settings.conf`**:
    *   Review `settings.conf` in the project root. This file likely contains crucial network configurations (e.g., IP addresses for the Python backend or Node-RED) that need to be aligned with your environment.

2.  **Start the System**:
    *   From the project root directory, execute the startup script:
        ```bash
        ./bin/start.sh
        ```
        This script is intended to:
        *   Ensure the ESP32-CAM is powered on and actively streaming image data via WebSocket to the Python backend.
        *   Launch the Python backend (`src/python/main.py`), which listens for image data on port `8000`, performs hand detection and finger counting, and prints JSON data (processed image + finger count) to standard output.
        *   Start Node-RED, which is configured by `NodeRedCamera.json` to capture the JSON output from the Python script (likely via an `exec` node or another form of inter-process communication), process it further, and display it on a dashboard.

3.  **Access Node-RED Dashboard**:
    *   Once all services are running, open your web browser to `http://localhost:1880` (or the configured URL for your Node-RED instance).
    *   The imported Node-RED flow should display the live camera feed with real-time hand detection overlays and the detected finger count. Interact with the camera by showing your hand, and observe the immediate updates on the dashboard.

## Project Structure

```
.
├── .gitignore               # Standard Git ignore file
├── README.md                # This documentation file
├── bin/                     # Contains startup scripts for project orchestration
│   └── start.sh             # Main script to launch all project components
├── requirements.txt         # List of Python dependencies
├── settings.conf            # Project-wide configuration settings
└── src/                     # Source code directory for different components
    ├── node-red/            # Node-RED flows and configuration files
    │   ├── .NodeRedCamera.json.backup # Backup of Node-RED flow
    │   ├── NodeRedCamera.json # Main Node-RED flow definition for import
    │   └── settings.js      # Node-RED custom runtime settings
    ├── platform-io/         # PlatformIO projects for embedded systems development
    │   └── Esp32CamWebserver/ # ESP32-CAM firmware project
    │       ├── .gitignore   # PlatformIO specific ignore file
    │       ├── include/     # Header files for ESP32 project
    │       ├── lib/         # Project-specific libraries for ESP32
    │       ├── platformio.ini # PlatformIO configuration for ESP32-CAM build
    │       └── src/         # ESP32-CAM C++ source code
    │           └── CameraWebsocket.cpp # WebSocket client implementation for camera streaming
    └── python/              # Python backend for AI processing and WebSocket server
        └── main.py          # Main Python script for hand detection and image processing
```

## License

This project is currently unlicensed. Please contact the repository owner for licensing information.
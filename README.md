# **Smart Camera Project**

A distributed smart camera solution featuring an ESP32-CAM video streaming client, a central processing server, and Node-RED for visual flow management. The project captures live video frames and streams them over WebSockets for backend computer vision analysis and IoT integration. The embedded camera software is built and managed using PlatformIO, ensuring streamlined dependency management and cross-platform flashing.

## **Project Architecture**

The system is distributed across three primary components:

* **ESP32-CAM Client (C++):** Built using PlatformIO, this component runs on an AI Thinker ESP32-CAM module. It acts as a dedicated hardware client that continuously captures VGA (640x480) JPEG frames and pushes them as a binary payload over a WebSocket connection to the central server or Node-RED instance.  
* **Processing Backend (Python/C++):** A central server environment that handles the intensive "smart" computational tasks and computer vision logic.  
* **Node-RED Integration:** Acts as the IoT wiring and dashboarding layer. It can receive the WebSocket streams, route data to the processing backend, handle automation triggers (like motion detection alerts), and display the camera feed on a user-friendly UI.

## **Key Features**

* **Visual IoT Workflows:** Leverages Node-RED for easy drag-and-drop routing of the video stream, enabling quick integrations with other smart home devices or MQTT brokers.  
* **Smart Network Provisioning:** On first boot, or if the network is unavailable, the camera switches into Access Point (AP) mode. Users can connect to the camera's hotspot to configure local WiFi credentials and the target WebSocket Server IP via a local web interface. Settings are saved to persistent flash memory.  
* **Low-Latency Streaming:** Bypasses traditional MJPEG HTTP servers in favor of pushing raw binary JPEGs directly over WebSockets for faster processing.  
* **Self-Healing Design:** The camera firmware is highly resilient, built to automatically hardware-restart itself if the camera fails to capture a frame or if the WebSocket connection drops.  
* **PlatformIO Integration:** Simplifies building, environment configuration, and uploading for embedded development.

## **Hardware Requirements**

* ESP32-CAM module (AI Thinker board pinout)  
* FTDI programmer (for initial firmware flashing)  
* A host machine running the processing backend and Node-RED

## **Setup and Installation**

### **1\. Backend & Node-RED Setup**

1. **Python Dependencies:** Navigate to the root directory and install the required Python dependencies using pip install \-r requirements.txt. Configure your settings.conf file to match your hardware setup.  
2. **Node-RED Setup:** Ensure Node-RED is installed on your host machine. Start the Node-RED server, open the web interface, and import any provided project flows. Ensure the WebSocket input nodes are configured to listen on the correct port (default is usually 8000).

### **2\. Flashing the ESP32-CAM**

1. Open the src/platform-io/Esp32CamWebserver directory using the PlatformIO IDE (such as the VSCode extension) or the PlatformIO Core CLI.  
2. Connect your ESP32-CAM to your computer using an FTDI adapter.  
3. Build and upload the firmware to the ESP32-CAM using PlatformIO (pio run \-t upload).

### **3\. Camera Provisioning**

1. Power on the ESP32-CAM without the FTDI programmer.  
2. On your computer or smartphone, look for a new WiFi network named **ESP32-CAM** and connect to it using the password 12345678\.  
3. Navigate to the captive portal IP address in your web browser.  
4. Enter your home/office WiFi SSID, Password, and the local IP address of the machine running your Node-RED/WebSocket backend.  
5. Submit the form to restart the camera. It will connect to your network and instantly begin streaming video frames.

## **Usage**

Once the hardware is flashed and the network is provisioned, you can start tracking hand gestures and using them in Node-RED.

1. **Start the Video Stream:** Power on your ESP32-CAM. It will automatically connect to your WiFi and begin pushing frames to the target WebSocket IP.  
2. **Run the Backend:** Start the Python processing script to ingest the stream and begin computer vision processing:  
   python src/python/main.py

3. **Perform Gestures:** Position your hand in front of the ESP32-CAM. The backend uses OpenCV and cvzone to track your hand landmarks and determine exactly which fingers are held up.  
4. **Capture in Node-RED:** In your Node-RED flow, use an exec node to run the main.py script as a daemon (instead of running it manually in the terminal). The script prints structured JSON directly to stdout containing:  
   * A Base64 encoded image string (the current frame with drawn hand-tracking markers).  
   * An array representing your raised fingers.  
5. **Trigger Automations:** Parse the JSON output using a json node in Node-RED. You can feed the Base64 string to a dashboard template node to view the live annotated feed, and route the finger-count array to switch nodes to trigger specific IoT automations based on your hand gestures.

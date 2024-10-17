ESP8266 Network Server with mDNS and Python Requests

This project connects an ESP8266 to a Wi-Fi network using mDNS and starts an HTTP server to handle requests at http://espmine.local/data. You can send data to the ESP8266 using Python's requests module. Ensure all devices are connected to the same network.
Features

    mDNS: Access the ESP8266 via a friendly hostname (e.g., espmine.local).
    HTTP Server: ESP8266 listens for HTTP POST requests at /data.
    Python Requests: Send data to the ESP8266 from Python scripts.

Setup

    ESP8266 Setup: Use libraries like ESP8266WiFi.h, ESP8266mDNS.h, and ESP8266WebServer.h to connect the ESP8266 to a Wi-Fi network and enable the HTTP server.
    Python Requests: Install Python's requests module to send data.

Linux Users

If you encounter permission errors when uploading code, run:

bash

sudo chmod a+rw /dev/ttyUSB0

Or add your user to the dialout group for a permanent fix:

bash

sudo usermod -aG dialout $USER


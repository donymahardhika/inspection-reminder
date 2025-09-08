# inspection-reminder

This project uses a NodeMCU ESP8266 and a DFPlayer Mini to help standardize and validate the production inspection process. The system provides automatic audio reminders during the inspection process.

✨ Features
- Audio reminder playback using the DFPlayer Mini
- Volume, play, and stop control via the NodeMCU web server
- Customizable audio sequence, volume, and delays for inspection needs in 20 different recipe
- A simple web interface with level password (can be further developed)

🛠️ Hardware
- NodeMCU ESP8266
- DFPlayer Mini + Speaker or Active Speaker
- MicroSD Card (containing MP3 audio files)
- 5V Power Supply

💻 Software & Libraries
Arduino IDE
Required libraries to install:
- DFRobotDFPlayerMini
- ESP8266WiFi
- ESP8266WebServer
- EEPROM
- SoftwareSerial

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

⚙️ Wiring

<img width="1169" height="827" alt="inspection_reminder" src="https://github.com/user-attachments/assets/97d1112b-6127-4144-a52d-290e4ef66b45" /><br>

💻 Software & Libraries
Arduino IDE
Required libraries to install:
- DFRobotDFPlayerMini
- ESP8266WiFi
- ESP8266WebServer
- EEPROM
- SoftwareSerial

✅ How it Works
1. When powered on, plays the last recipe before shutdown
2. Plays Track 001 through Track 004 with a delay between tracks
3. Tracks 005 and 006 are for breaks and will play when the delay interval is reached
4. After finishing Track 006, it will return to step 2

🖼️ Gallery

  <table>
    <!-- Baris 1 -->
    <tr>
      <td>
        <figure>
          <img width="147" height="300" alt="Home Page" src="https://github.com/user-attachments/assets/9657dc8b-ab4a-488f-bbec-055e143fb786" /><br>
          <figcaption>Home Page</figcaption>
        </figure>
      </td>
      <td>
        <figure>
          <img width="147" height="300" alt="Login Page" src="https://github.com/user-attachments/assets/e61ad8e4-98d2-4499-ab79-78c8e815ec8b" /><br>
          <figcaption>Login Page</figcaption>
        </figure>
      </td>
      <td>
        <figure>
          <img width="147" height="300" alt="Role 1 Page" src="https://github.com/user-attachments/assets/cb3ca530-ac5b-4e8b-ba54-989cb46641bd" /><br>
          <figcaption>Role 1 Page</figcaption>
        </figure>
      </td>
    </tr>
    <!-- Baris 2 -->
    <tr>
      <td>
        <figure>
          <img width="147" height="300" alt="Role 2 Page" src="https://github.com/user-attachments/assets/679940ea-f396-4dfe-a0da-7fbccd6c9b7b" /><br>
          <figcaption>Role 2 Page</figcaption>
        </figure>
      </td>
      <td>
        <figure>
         <img width="147" height="300" alt="Role 3 Page" src="https://github.com/user-attachments/assets/40393a9d-7570-4ad6-ad00-305e0c63f094" /><br>
          <figcaption>Role 3 Page</figcaption>
        </figure>
      </td>
      <td>
        <figure>
          <img width="147" height="300" alt="Edit Page" src="https://github.com/user-attachments/assets/fddb037c-f8e6-4b4f-ba4a-3d718738fc3f" /><br>
          <figcaption>Edit Page</figcaption>
        </figure>
      </td>
    </tr>
    <!-- Baris 3 -->
    <tr>
      <td>
        <figure>
          <img width="147" height="300" alt="Change Password Page" src="https://github.com/user-attachments/assets/8f64bcf0-35c5-4848-ada7-277aa3c92b7a" /><br>
          <figcaption>Change Password Page</figcaption>
        </figure>
      </td>
      <td>
      </td>
      <td>
      </td>
    </tr>
  </table>

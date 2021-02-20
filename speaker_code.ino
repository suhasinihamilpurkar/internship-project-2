/*
Sources:
https://www.youtube.com/watch?v=kq2RLz65_w0
https://www.dfrobot.com/wiki/index.php/DFPlayer_Mini_SKU:DFR0299
https://github.com/pcbreflux/espressif/blob/master/esp32/arduino/sketchbook/ESP32_DFPlayer_full/ESP32_DFPlayer_full.ino
https://github.com/pcbreflux/espressif/blob/master/esp32/arduino/sketchbook/ESP32_DFPlayer_full/setup.png
https://randomnerdtutorials.com/esp32-web-server-arduino-ide/
*/

#include <Arduino.h>
#include "DFRobotDFPlayerMini.h"
#include <WiFi.h>

HardwareSerial hwSerial(1);
DFRobotDFPlayerMini dfPlayer;
int volume = 5;

const char* ssid     = "WiFi SSID";
const char* password = "WiFi password";
WiFiServer server(80); // Set web server port number to 80

String header;
String ledState = "";
const int ledPin = 26;

unsigned long timestamp = 0;


void setup()
{
  btStop(); // turn off bluetooth
  hwSerial.begin(9600, SERIAL_8N1, 18, 19);  // speed, type, TX, RX
  Serial.begin(115200);

// WiFi & LED ==================================================================
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);

  // Connect to Wi-Fi network with SSID and password
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  // Print local IP address and start web server
  Serial.println("");
  Serial.println("WiFi connected.");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  server.begin();
  delay(500);

  dfPlayer.begin(hwSerial);  //Use softwareSerial to communicate with mp3
  dfPlayer.setTimeOut(500); //Set serial communication time out 500ms
  dfPlayer.volume(volume);  //Set volume value (0~30).
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);
  
  ledState = "on";
  digitalWrite(ledPin, HIGH);
  timestamp = millis();
  dfPlayer.play(1);  //Play the first mp3

}

void loop() {

  WiFiClient client = server.available();   // Listen for incoming clients

  if (ledState == "on" && (millis() - timestamp) > 2000) {
    ledState = "off";
    digitalWrite(ledPin, LOW);
  }

  if (client) {
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println();
            
            // turns the GPIOs on and off
            if (header.indexOf("GET /bell/on") >= 0) {
              ledState = "on";
              digitalWrite(ledPin, HIGH);
              timestamp = millis();
              dfPlayer.play(1);  //Play the first mp3
            }
            else  if (header.indexOf("GET /volume/") >= 0) { // yes, I know this is not RESTful
              String str1 = header;
              str1 = str1.substring(header.indexOf("GET /volume/") + 12);
              volume = str1.substring(0, str1.indexOf(" ")).toInt();
              
              if (volume < 0) {
                volume = 0;
              }
              else if (volume > 30) {
                volume = 30;
              }
              
              dfPlayer.volume(volume);
              Serial.print("volume set to ");
              Serial.println(volume);
            }
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            client.println("<style>html { font-family: sans-serif; display: inline-block; margin: 0px auto; text-align: center; }");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px; text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 { background-color: #4CAF50; border: none; color: white; padding: 4px 10px; text-decoration: none; margin: 1px; cursor: pointer;}</style></head>");
            
            client.println("<body><h1>LeWe T&uuml;rklingel</h1>");
            client.println("<p>Volume: <a href=\"/volume/" + String(volume-1) + "\"><button class=\"button2\">&minus;</button></a> " + String(volume) + " <a href=\"/volume/" + String(volume+1) + "\"><button class=\"button2\">&plus;</button></a></p>");
            client.println("<p><a href=\"/bell/on\"><button class=\"button\">Klingeln</button></a></p>");
            client.println("<p style='margin-top: 40px; font-size: 8px;'>dfplayer status: " + printDetail(dfPlayer.readType(), dfPlayer.read()) + "</p>");
            client.println("<p style='font-size: 8px;'>LED - State " + ledState + "</p>");
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

String printDetail(uint8_t type, int value){
  switch (type) {
    case TimeOut:
      return "Time Out!";
      break;
    case WrongStack:
      return "Stack Wrong!";
      break;
    case DFPlayerCardInserted:
      return "Card Inserted!";
      break;
    case DFPlayerCardRemoved:
      return "Card Removed!";
      break;
    case DFPlayerCardOnline:
      return "Card Online!";
      break;
    case DFPlayerPlayFinished:
      return "Play Finished!";
      break;
    case DFPlayerError:
      switch (value) {
        case Busy:
          return "Error: Card not found";
          break;
        case Sleeping:
          return "Error: Sleeping";
          break;
        case SerialWrongStack:
          return "Error: Get Wrong Stack";
          break;
        case CheckSumNotMatch:
          return "Error: Check Sum Not Match";
          break;
        case FileIndexOut:
          return "Error: File Index Out of Bound";
          break;
        case FileMismatch:
          return "Error: Cannot Find File";
          break;
        case Advertise:
          return "Error: In Advertise";
          break;
        default:
          break;
      }
      break;
    default:
      break;
  }
}


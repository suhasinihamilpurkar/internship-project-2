#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <Espalexa.h>
#include "TM1637Display.h"
#include "DFRobotDFPlayerMini.h"


HardwareSerial hwSerial(1);
DFRobotDFPlayerMini dfPlayer;
int volume = 5;

unsigned long timestamp = 0;
//define the GPIO connected with relays

#define RelayPin1 4 //D2
#define CLK 3
#define DIO 2

//prototypes
boolean connectWifi();

TM1637 disp(CLK,DIO);

//callback functions
void firsLightChanged(uint8_t brightness);

//WiFi Credentials
const char* ssid = "WiFi Name";
const char* password = "WiFi Password";

//device name
String Device_1_Name = "Timer1";

boolean wifiConnected = false;

Espalexa espalexa;

void setup()
{
  btStop(); //turnoff bluetooth
  hwSerial.begin(9600, SERIAL_8N1,18,19); //speed, type, TX,RX
  Serial.begin(115200);

   pinMode(RelayPin1,OUTPUT);

   //Initialise wifi connection
   wificonnected = connectWifi();

   if(wificonnected)
   {
    //define your device here
    espalexa.addDevice(Device_1_Name, firstLightChanged); //simplest definition ,default state off
    disp.set(5);
    disp.init(D4056A);
    espalexa.begin();
   }
   else
   {
    while(1)
    {
      Serial.println("Cannot connect to Wifi.Please check data and reset the ESP.");
      delay(2500);
    }
   }
  dfPlayer.begin(hwSerial); //use software serial to communicate with mp3
  dfPlayer.setTimeout(500); //set serial communication timeout 500ms
  dfPlayer.volume(volume); //set volume value(0~30)
  dfPlayer.EQ(DFPLAYER_EQ_NORMAL);
  dfPlayer.outputDevice(DFPLAYER_DEVICE_SD);

  firstLightChanged();
}


void loop() 
{
  espalexa.loop();
  delay(1);
  if( v==0 &&(millis()- timestamp)>2000)
  {
  digitalWrite(RelayPin1, LOW);
  Serial.println("Timer1 OFF");
  }
}

//our callback functions
void firstLightChanged(uint8_t brightness)
{
  int v=60;
  //control the device
  if(brightness == 255)
  {
    while(v<=60)
    {
     digitalWrite(RelayPin1 , HIGH);
     Serial.println("Timer1 ON");
     timestamp = millis();
     dfPlayer.play(1); // play the first mp3
     disp.display(v);
     delay(1000);
     v=v-1;
    }
  }
  /*
  else
  {
    digitalWrite(RelayPin1,LOW);
    Serial.println("Timer1 OFF");
  }*/
}


//CONNECT TO WIFI - RETURNS TRUE IF SUCCESSFUL OR FALSE IF NOT
boolean connectWifi()
{
  boolean state = true;
  int i=0;

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid,password);
  Serial.println("");
  Serial.println("Connecting to Wifi");

  //wait for connection
  Serial.print("Connecting...");
  while(WiFi.status()!= WL_CONNECTED)
  {
    delay(500);
    Serial.print(".");
    if(i>20)
    {
      state = false; break;
    }
    i++;
  }
  Serial.println("");
  if(state)
  {
    Serial.print("Connected to");
    Serial.println(ssid);
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());
  }
  else
  {
    Serial.println("Connection Failed.");
  }
  return state;
}

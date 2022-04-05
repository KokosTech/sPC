//
// ESP8266 HTTP Post Program
//
// Uses an ESP8266 ESP-01, connected to an STM32 Blue Pill
//
// Must have 128k flash variant of the STM32!
//
//  Pins
//  STM32 pin PA2 Serial 2 (RX) to ESP8266 TX
//  Arduino pin PA3 Serial 2 to voltage divider then to ESP8266 RX
//  Connect GND from the STM32 to GND on the ESP8266
//  Connect 3.3V from the STM32 to VCC on the ESP8266
//  Pull ESP8266 CH_PD HIGH via jumper from ESP8266 3.3V line
//
// Original code credit / inspiration:
// https://community.wia.io/d/25-how-to-setup-an-arduino-uno-with-esp8266-and-publish-an-event-to-wia
// Alan - WIA community admin
//

#include <ArduinoJson.h> // Must use library version <= 5.13.4, 6.x.x is incompatible. Handles JSON formatting
#include <DHT_U.h>
#include <DHT.h>
#include "SoftwareSerial.h"

#include "arduino_secrets.h"

#define DHTPIN 7     // Digital pin connected to the DHT sensor
#define DHTYPE DHT11
// Relay Config
#define RELAY_PIN 12
// KY-037 (Noise Detector / Meter) Config
#define KY_DPIN 5
#define KY_APIN 0
// Global Config
#define DEBUG true

String wifi_ssid = SSID;                  // Wifi network SSID
String wifi_password = PASS;          // Wifi password
String host = "192.168.0.238";                           // Server hostname or IP
String path = "/post-test";                     // API Route
String port = "8080";                                       // HTTP Port

int outletId = 1;           // Unique, hard-coded ID for each outlet.
int loopDelay = 5000;       // 5 second delay between sensor reads
int countTrueCommand;       // Used in determining success or failure of serial commands
int countTimeCommand;       // Used in determining success or failure of serial commands
boolean found = false;      // Used in determining success or failure of serial commands

SoftwareSerial esp(3, 2);
DHT dht(DHTPIN, DHTYPE);

// Buffer to store JSON object
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

void setup() {
  digitalWrite(RELAY_PIN, HIGH);
  // Serial Config
  Serial.begin(115200);
  esp.begin(115200);  
  
  // ESP Config
  esp.println("AT");                                                                        // Poke the ESP8266             
  Serial.println(esp.read());   
  sendCommandToesp("AT+RST", 5, "OK");                                                            // Check that AT firmware on ESP8266 is responding
  sendCommandToesp("AT", 5, "OK");                                                          // If status is okay, set radio mode
  sendCommandToesp("AT+CWMODE=3", 5, "OK");                                                 // Set radio mode
  sendCommandToesp("AT+CWJAP=\"" + wifi_ssid + "\",\"" + wifi_password + "\"", 20, "OK");   // Connect to pre-defined wireless network
  sendCommandToesp("AT+CIPMUX=1\r\n", 5, "OK");
  sendCommandToesp("AT+CIPSERVER=1,80\r\n", 5, "OK");
  // Other I/O Config
  pinMode(RELAY_PIN, OUTPUT);

  dht.begin();
}

void loop() {
  if(esp.available())
  {
    String res = "" +
      String("HTTP/1.1 200 OK\r\n") +
      "Content-Type: none\r\n" +
      "Content-Length: 0\r\n" +
      "Connection: close\r\n\r\n";

    if(esp.find("+IPD,"))
    {
      Serial.println("AAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAAA");
      delay(1000);
      int connectionId = esp.read()-48;
      esp.find("command=");
      int command = (esp.read());
      delay(500);
      Serial.println("COMMAND: " + command); 
      Serial.println("CONNID: " + connectionId); 
      if(command == '1'){
        digitalWrite(RELAY_PIN, LOW);
        delay(1500);
        digitalWrite(RELAY_PIN, HIGH);

/*         String d = "200";

        String cipSend = " AT+CIPSEND=";
             cipSend += connectionId; 
             cipSend += ",";
             cipSend +=d.length();
             cipSend +="\r\n";
             sendCommandToesp(cipSend,2, "OK");
             sendCommandToesp(d,2, "OK"); */
        
        String cipSend = "AT+CIPSEND=" + String(connectionId) + "," + String(res.length());
        sendCommandToesp(cipSend, 4, ">");
        sendData(res);
        String closeCommand = "AT+CIPCLOSE=";
        closeCommand+=connectionId;
        closeCommand+="\r\n";
        sendCommandToesp(closeCommand,5,"OK");
      }
      else if(command == '2'){ 
        float h = dht.readHumidity();  
        float t = dht.readTemperature(); 
        // Delays sensor reads as desired
        // delay(loopDelay);

        // JSON Data - using ArduinoJson library object
        // TODO - Stubu in JSON arrays for remaining sensor values
        delay(500);
        root["humidity"] = String(h);
        root["temp"] =  String(t);
        String data;
        root.printTo(data);

        // HTTP post request
        String postRequest = "" + String("HTTP/1.1 200 OK\r\n") +
                            "Connection: close\r\n" +
                            "Content-Length: " + data.length() + "\r\n" +
                            "Content-Type: application/json\r\n" +
                            "\r\n" + data;

        // Send post request using AT Firmware
        String cipSend = "AT+CIPSEND=" + String(connectionId) + "," + String(postRequest.length());
        sendCommandToesp(cipSend, 4, ">");
        sendData(postRequest);
        String closeCommand = "AT+CIPCLOSE=";
        closeCommand+=connectionId;
        closeCommand+="\r\n";
        sendCommandToesp(closeCommand,5,"OK");
      }
      else if(command == '3'){
        digitalWrite(RELAY_PIN, LOW);
        delay(7500);
        digitalWrite(RELAY_PIN, HIGH);

/*         String d = "200";

        String cipSend = " AT+CIPSEND=";
             cipSend += connectionId; 
             cipSend += ",";
             cipSend +=d.length();
             cipSend +="\r\n";
             sendCommandToesp(cipSend,2, "OK");
             sendCommandToesp(d,2, "OK"); */
        
        String cipSend = "AT+CIPSEND=" + String(connectionId) + "," + String(res.length());
        sendCommandToesp(cipSend, 4, ">");
        sendData(res);
        String closeCommand = "AT+CIPCLOSE=";
        closeCommand+=connectionId;
        closeCommand+="\r\n";
        sendCommandToesp(closeCommand,5,"OK");
      }
    }

    
  }

  
}

void getGetRequest() {

}

String retardSend(String command, const int timeout, boolean debug)
{
    String response = "";                                             //initialize a String variable named "response". we will use it later.
    
    esp.print(command);                                           //send the AT command to the esp8266 (from ARDUINO to ESP8266).
    long int time = millis();                                         //get the operating time at this specific moment and save it inside the "time" variable.
    while( (time+timeout) > millis())                                 //excute only whitin 1 second.
    {      
      while(esp.available())                                      //is there any response came from the ESP8266 and saved in the Arduino input buffer?
      {
        char c = esp.read();                                      //if yes, read the next character from the input buffer and save it in the "response" String variable.
        response+=c;                                                  //append the next character to the response variabl. at the end we will get a string(array of characters) contains the response.
      }  
    }    
    if(debug)                                                         //if the "debug" variable value is TRUE, print the response on the Serial monitor.
    {
      Serial.print(response);
    }    
    return response;                                                  //return the String response.
}

// Function to determine success / failure of serial commands send to/from ESP8266
void sendCommandToesp(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while (countTimeCommand < (maxTime * 1))
  {
    esp.println(command);
    if (esp.find(readReplay))
    {
      found = true;
      break;
    }

    countTimeCommand++;
  }

  if (found == true)
  {
    Serial.println("Success");
    countTrueCommand++;
    countTimeCommand = 0;
  }

  if (found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }

  found = false;
}

// Send post request to Arduino and ESP8266
void sendData(String postRequest) {
  Serial.println(postRequest);
  esp.println(postRequest);
  delay(1500);
  countTrueCommand++;
}

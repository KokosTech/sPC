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

#include "../arduino_secrets.h"

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

// Buffer to store JSON object
StaticJsonBuffer<200> jsonBuffer;
JsonObject& root = jsonBuffer.createObject();

void setup() {
  Serial.begin(115200);                                                                           // Start STM32 Serial at 9600 Baud
  esp.begin(115200);                                                                          // Start ESP8266 Serial at 9600 Baud
  esp.println("AT");                                                                        // Poke the ESP8266             
  Serial.println(esp.read());                                                               // Check that AT firmware on ESP8266 is responding
  sendCommandToesp("AT", 5, "OK");                                                          // If status is okay, set radio mode
  sendCommandToesp("AT+CWMODE=1", 5, "OK");                                                 // Set radio mode
  sendCommandToesp("AT+CWJAP=\"" + wifi_ssid + "\",\"" + wifi_password + "\"", 20, "OK");   // Connect to pre-defined wireless network
}

void loop() {

  // Delays sensor reads as desired
  // delay(loopDelay);

  // JSON Data - using ArduinoJson library object
  // TODO - Stubu in JSON arrays for remaining sensor values
  root["outlet_id"] = 1;
  root["voltage"] =  2;
  root["vrms"] =  3;
  root["amps_rms"] =  4;
  String data;
  root.printTo(data);

  // HTTP post request
  String postRequest = "POST " + path  + " HTTP/1.1\r\n" +
                       "Host: " + host + "\r\n" +
                       "Accept: *" + "/" + "*\r\n" +
                       "Content-Length: " + data.length() + "\r\n" +
                       "Content-Type: application/json\r\n" +
                       "\r\n" + data;

  // Send post request using AT Firmware
  sendCommandToesp("AT+CIPMUX=1", 5, "OK");
  sendCommandToesp("AT+CIPSTART=0,\"TCP\",\"" + host + "\"," + port, 15, "OK");
  String cipSend = "AT+CIPSEND=0," + String(postRequest.length());
  sendCommandToesp(cipSend, 4, ">");
  sendData(postRequest);
  sendCommandToesp("AT+CIPCLOSE=0", 5, "OK");
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

#include <ArduinoJson.h> // ArduinoJson 6+ ONLy, btw
#include <DHT_U.h>
#include <DHT.h>
#include "SoftwareSerial.h"

#include "arduino_secrets.h"

// DHT Config Variables
#define DHTPIN 7    
#define DHTYPE DHT11
// Relay Config
#define RELAY_PIN 12
// KY-037 (Noise Detector / Meter) Config
#define KY_DPIN 5
#define KY_APIN 0
// Global Config
#define DEBUG true


// Wifi Credentials
String wifi_ssid = SSID;
String wifi_password = PASS;

// Debuggig (Fail / Succ) with esp8266 - kinda unstable
int countTrueCommand;
int countTimeCommand;
boolean found = false;


// Setting up global sensors x esp8266
SoftwareSerial esp(3, 2);
DHT dht(DHTPIN, DHTYPE);

// Buffer to store JSON object - ArduinoJSON 6+
StaticJsonDocument<200> root;

// Function Declarations

// Func to set tbe esp up
void esp_setup();

// Function to determine success / failure of serial commands send to/from ESP8266
void sendCommandToesp(String command, int maxTime, char readReplay[]);

// Send HTTP Response from the Arduino and the ESP8266 to the client (aka the React / Flutter app)
// Func could also be used to SEND GET / POST requests
void sendData(String postRequest);

void setup() {
	// Setting the relay to its default state (just in case) - relay connected with NO (Normally Open)
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, HIGH); 

	// Setting up the DHT
	dht.begin();

	// Serial Config (Arduino -> esp)
	Serial.begin(115200);
	esp.begin(115200);  

	// ESP Config  
	esp_setup();  
}

void loop() {
	if(esp.available()) {
		String def_res = "" + String("HTTP/1.1 200 OK\r\n") +
						"Content-Type: none\r\n" +
						"Content-Length: 0\r\n" +
						"Connection: close\r\n\r\n";

		if(esp.find("+IPD,")) {
			Serial.println("----------RECEIVED----------");
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

				String cipSend = "AT+CIPSEND=" + String(connectionId) + "," + String(def_res.length());
				sendCommandToesp(cipSend, 4, ">");
				sendData(def_res);
				String closeCommand = "AT+CIPCLOSE=";
				closeCommand+=connectionId;
				closeCommand+="\r\n";
				sendCommandToesp(closeCommand,5,"OK");
			} else if(command == '2') { 
				float h = dht.readHumidity();  
				float t = dht.readTemperature(); 

				delay(500);
				root["humidity"] = String(h);
				root["temp"] =  String(t);
				String data;
				serializeJson(root, data);

				String responce = "" + String("HTTP/1.1 200 OK\r\n") +
									"Connection: close\r\n" +
									"Content-Length: " + data.length() + "\r\n" +
									"Content-Type: application/json\r\n" +
									"\r\n" + data;

				String cipSend = "AT+CIPSEND=" + String(connectionId) + "," + String(responce.length());
				sendCommandToesp(cipSend, 4, ">");
				sendData(responce);
				String closeCommand = "AT+CIPCLOSE=";
				closeCommand+=connectionId;
				closeCommand+="\r\n";
				sendCommandToesp(closeCommand,5,"OK");
			} else if(command == '3'){
				digitalWrite(RELAY_PIN, LOW);
				delay(7500);
				digitalWrite(RELAY_PIN, HIGH);

				String cipSend = "AT+CIPSEND=" + String(connectionId) + "," + String(def_res.length());
				sendCommandToesp(cipSend, 4, ">");
				sendData(def_res);
				String closeCommand = "AT+CIPCLOSE=";
				closeCommand+=connectionId;
				closeCommand+="\r\n";
				sendCommandToesp(closeCommand,5,"OK");
			}
		}
		Serial.println("----------END RECEIVED----------");
	}
}

void esp_setup(){
	sendCommandToesp("AT+RST", 5, "OK");
	delay(1000);
	sendCommandToesp("AT", 5, "OK");
	sendCommandToesp("AT+CWMODE=3", 5, "OK");
	sendCommandToesp("AT+CWJAP=\"" + wifi_ssid + "\",\"" + wifi_password + "\"", 20, "OK");
	sendCommandToesp("AT+CIPMUX=1\r\n", 5, "OK");
	sendCommandToesp("AT+CIPSERVER=1,80\r\n", 5, "OK");
}

void sendCommandToesp(String command, int maxTime, char readReplay[]) {
	Serial.print(countTrueCommand);
	Serial.print(". at command => ");
	Serial.print(command);
	Serial.print(" ");

	while (countTimeCommand < (maxTime * 1)) {
		esp.println(command);
		if (esp.find(readReplay)) {
			found = true;
			break;
		}

		countTimeCommand++;
	}

	if (found == true) {
		Serial.println("Success");
		countTrueCommand++;
		countTimeCommand = 0;
	} else if (found == false) {
		Serial.println("Fail");
		countTrueCommand = 0;
		countTimeCommand = 0;
	}

	found = false;
}

void sendData(String postRequest) {
	Serial.println(postRequest);
	esp.println(postRequest);
	delay(1500);
	countTrueCommand++;
}

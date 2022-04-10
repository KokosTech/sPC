#include <ArduinoJson.h> // ArduinoJson 6+ only, btw -> works with the latest version of ArduinoJson
#include <DHT_U.h> // DHT library
#include <DHT.h> // DHT library
#include <SoftwareSerial.h> // SoftwareSerial library

#include "arduino_secrets.h" // Arduino secrets - Used to store Wi-Fi credentials
// [IMPORTANT]
// When cloning this project CREATE an "arduino_secrets.h" file with your Wi-Fi credentials
// [TEMPLATE] arduino_secrets.h
// [BEGINNING OF FILE]
// #define SSID "this is where you type your SSID"
// #define PASS "this is where you type your password"
// [END OF FILE]

// DHT Config Variables
#define DHTPIN 7 // DHT digital pin number    
#define DHTYPE DHT11 // Type of the DHT sensor (in this case - DHT11) - values could be DHT11 or DHT22
// Relay Config
#define RELAY_PIN 12 // DHT relay digital pin number

// Wifi Credentials
String wifi_ssid = SSID;
String wifi_password = PASS;

// Debuggig (Fail / Succ) with ESP8266
int countTrueCommand;
int countTimeCommand;
boolean found = false;


// Setting up global sensors x ESP8266
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
void sendData(String data);

void setup() {
	// Setting the relay to its default state (just in case) - relay connected with NO (Normally Open)
	pinMode(RELAY_PIN, OUTPUT);
	digitalWrite(RELAY_PIN, HIGH); 

	// Setting up the DHT
	dht.begin();

	// Serial Config (Arduino -> esp) - Baudrate @ 115 200
	Serial.begin(115200);
	esp.begin(115200);  

	// ESP Config  
	esp_setup();  
}

void loop() {
	if(esp.available()) {
		// Default response
		String def_res = "" + String("HTTP/1.1 200 OK\r\n") +
						"Content-Type: none\r\n" +
						"Content-Length: 0\r\n" +
						"Access-Control-Allow-Origin: *\r\n" +
						"Connection: close\r\n\r\n";

		// Check if the ESP8266 is getting data
		if(esp.find("+IPD,")) {
			Serial.println("----------RECEIVED----------"); // [DEBUG]
			delay(1000);
			int connectionId = esp.read()-48; // Get Connection ID & convert it from ASCII to int

			// Look for the command in the query string
			if(esp.find("command=")){ 
				int command = (esp.read()); // Get Command
				delay(500);
				Serial.println("COMMAND: " + command); // [DEBUG]
				Serial.println("CONNID: " + connectionId); // [DEBUG]

				// [LIST] Command List
				// 1 -> Turn On / Off the computer
				// 2 -> Send sensor data (temp & humidity) to the client
				// 3 -> Force reset the computer (usually you have to short the power pins around 7 seconds to reset a computer)

				// Check which command to execute
				if(command == '1'){
					// Short the relay for 1.5 seconds
					digitalWrite(RELAY_PIN, LOW);
					delay(1500);
					digitalWrite(RELAY_PIN, HIGH);

					// Send HTTP Response to the client - to signal that the command was successfully executed
					String cipSend = "AT+CIPSEND=" + String(connectionId) + "," + String(def_res.length());
					sendCommandToesp(cipSend, 4, ">");
					sendData(def_res);
					String closeCommand = "AT+CIPCLOSE=";
					closeCommand+=connectionId;
					closeCommand+="\r\n";
					sendCommandToesp(closeCommand,5,"OK");
				} else if(command == '2') { 
					// Get data from the temp / humidity sensor via funcs form the DHT library
					float h = dht.readHumidity();  
					float t = dht.readTemperature(); 

					delay(500);

					// Add the data to the JSON buffer
					root["humidity"] = String(h);
					root["temp"] =  String(t);

					// Stringify the JSON buffer
					String data;
					serializeJson(root, data);

					// Send HTTP Response to the client - to send the data
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
					// Short the relay for 1.5 seconds
					digitalWrite(RELAY_PIN, LOW);
					delay(7500);
					digitalWrite(RELAY_PIN, HIGH);

					// Send HTTP Response to the client - to signal that the command was successfully executed
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
}


// Function to set-up the ESP8266
void esp_setup(){
	sendCommandToesp("AT+RST", 5, "OK"); // Reset the ESP8266 & wait for it to boot (set to 5 secs)
	delay(1000); // Added delay for safety
	sendCommandToesp("AT", 5, "OK"); // [TEST] Send AT command to ESP8266
	sendCommandToesp("AT+CWMODE=3", 5, "OK"); // Set the ESP8266 to STA mode
	sendCommandToesp("AT+CWJAP=\"" + wifi_ssid + "\",\"" + wifi_password + "\"", 20, "OK"); // Connect to the WIFI network
	sendCommandToesp("AT+CIPMUX=1\r\n", 5, "OK"); // Enable multiple connections
	sendCommandToesp("AT+CIPSERVER=1,80\r\n", 5, "OK"); // Start the server on port 80
}

// Function to send a command to the ESP8266
void sendCommandToesp(String command, int maxTime, char readReplay[]) {
	Serial.print(countTrueCommand); // [DEBUG] Print the number of successful commands
	Serial.print(". at command => "); // [DEBUG] text
	Serial.print(command); // [DEBUG] Print the command
	Serial.print(" "); // [DEBUG] text

	while (countTimeCommand < (maxTime * 1)) {
		esp.println(command); // Send command to the ESP8266
		if (esp.find(readReplay)) { // Find the reply from the ESP8266
			found = true;
			break;
		}

		countTimeCommand++; // Timer for timeout - increment by one
	}

	// Here we signal if the command was received successfully or not
	if (found == true) {
		Serial.println("Success"); // [DEBUG] Print success message and up the counter of successful commands
		countTrueCommand++;
		countTimeCommand = 0;
	} else if (found == false) {
		Serial.println("Fail"); // [DEBUG] Print error message and reset the counter
		countTrueCommand = 0;
		countTimeCommand = 0;
	}

	found = false;
}

// Function to send data to the client (aka the React / Flutter app)
void sendData(String data) {
	Serial.println(data); // [DEBUG] Print the data to the serial monitor 
	esp.println(data); // Send the data to the client
	delay(1500);
	countTrueCommand++;
}

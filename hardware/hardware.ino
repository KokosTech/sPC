// Date and time functions using a DS1307 RTC connected via I2C and Wire lib
#include <SoftwareSerial.h>
#include <DHT_U.h>
#include <DHT.h>

#include "arduino_secrets.h"

#define DHTPIN 5     // Digital pin connected to the DHT sensor
#define DEBUG true

SoftwareSerial esp8266(3, 2);

DHT dht(DHTPIN, DHT11);

const int relay = 13;
//const int LedIndicator = 9;

// char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

void setup()
{
    // coffeeDays[0][4] = 1;
    // coffeeHours[0] = 18;
    // coffeeMinutes[0] = 43;
    // coffeeCounter++;
    Serial.begin(115200);
    esp8266.begin(115200);

    pinMode(relay, OUTPUT);
    digitalWrite(relay, LOW);
    setupESP();
    dht.begin();
}

void loop()
{

    // Serial.print(now.year(), DEC);
    // Serial.print('/');
    // Serial.print(now.month(), DEC);
    // Serial.print('/');
    // Serial.print(now.day(), DEC);
    // Serial.print(" (");
    // Serial.print(daysOfTheWeek[now.dayOfTheWeek()]);
    // Serial.print(") ");
    // Serial.print(now.hour(), DEC);
    // Serial.print(':');
    // Serial.print(now.minute(), DEC);
    // Serial.print(':');
    // Serial.print(now.second(), DEC);
    // Serial.println();

    float t = dht.readTemperature(); 
    float h = dht.readHumidity();  

/*     if (isnan(h) || isnan(t)) {
        Serial.println(F("Failed to read from DHT sensor!"));
        delay(5000);
        return;
    }
    else {
        float hic = dht.computeHeatIndex(t, h, false);
            Serial.print(F(" Humidity: "));
            Serial.print(h);
            Serial.print(F("%  Temperature: "));
            Serial.print(t);
            Serial.print(F("°C "));
            Serial.print(hic);
            Serial.print(F("°C "));
    } */


    if (esp8266.available())
    {
        esp8266.find("command=");
        char command = (esp8266.read());
        Serial.print("\ncommand: ");
        Serial.println(command);
        Serial.println("\n");
        // digitalWrite(LedIndicator, HIGH);
        String content = "";
        if (command == '1')
        {
            command = (esp8266.read());            
            Serial.println("\ncommand: ");
            Serial.println(command);
            Serial.println("\n");
            digitalWrite(relay, HIGH);
            delay(2000);
            digitalWrite(relay, LOW);
            content = "heating";
            
        }
        else if (command == '2')
        {
            Serial.println("\n\nCOMMAND 2\n\n");
            int connectionId = esp8266.read()-48;
            Serial.print("\n\nCONNID -> ");
            Serial.println(connectionId);

            sendHTTPResponse(connectionId);

            String closeCommand = "AT+CIPCLOSE="; 
            closeCommand+=connectionId; // append connection id
            closeCommand+="\r\n";

            sendDataBTW(closeCommand,3000,DEBUG);
            
/*             String webpage = "<h1>Hello</h1>&lth2>World!</h2><button>LED1</button>";
            String cipSend = "AT+CIPSEND=";
            cipSend += connectionId;
            cipSend += ",";
            cipSend +=webpage.length();
            cipSend +="\r\n";
            
            sendDataBTW(cipSend,1000,DEBUG);
            sendDataBTW(webpage,1000,DEBUG);
            sendDataBTW(closeCommand,3000,DEBUG);

            
            webpage="<button>LED2</button>";
            
            cipSend = "AT+CIPSEND=";
            cipSend += connectionId;
            cipSend += ",";
            cipSend +=webpage.length();
            cipSend +="\r\n";
            
            sendDataBTW(cipSend,1000,DEBUG);
            sendDataBTW(webpage,1000,DEBUG);
            sendDataBTW(closeCommand,3000,DEBUG); */

        

            //server.send(200, "text/html", SendHTML(69,69,"THIS IS A FUCKING TIME", "AND THIS A DATE"));

/*             int command_int = (esp8266.read()) - 48;
            for (int i = 0; i < command_int; i++)
            {
                command_int = (esp8266.read()) - 48;
                coffeeDays[coffeeCounter][command_int] = true;
            }

            command_int = (esp8266.read()) - 48;
            if (command_int == 0)
            {
                command_int = (esp8266.read()) - 48;
                coffeeHours[coffeeCounter] = command_int;
            }
            else
            {
                command_int *= 10;
                command_int += (esp8266.read()) - 48;
                coffeeHours[coffeeCounter] = command_int;
            }

            command_int = (esp8266.read()) - 48;
            if (command_int == 0)
            {
                command_int = (esp8266.read()) - 48;
                coffeeMinutes[coffeeCounter] = command_int;
            }
            else
            {
                command_int *= 10;
                command_int += (esp8266.read()) - 48;
                coffeeMinutes[coffeeCounter] = command_int;
            } */
        }
        //Serial.println("send HTTP Request");
        //sendHTTPResponse(connectionId);
        // close connection
        //sendCommand("AT+CIPCLOSE=0\r\n", 1000, DEBUG);

        //digitalWrite(LedIndicator, LOW);
        //  delay(25000);
    }

    //    for (int i = 0; i < coffeeCounter; i++)
    //    {
    //        if (coffeeDays[now.dayOfTheWeek()] && (now.hour() == coffeeHours[i] && now.minute() == coffeeMinutes[i]))
    //        {
    //            digitalWrite(LedIndicator, HIGH);
    //            delay(1000);
    //            digitalWrite(LedIndicator, LOW);
    //            Serial.println("It worked?");
    //            delay(20000);
    //
    //            break;
    //        }
    //    }

    // calculate a date which is 7 days and 30 seconds into the future
    //  DateTime future (now + TimeSpan(7, 12, 30, 6));
    //
    //  Serial.print(" now + 7d + 30s: ");
    //  Serial.print(future.year(), DEC);
    //  Serial.print('/');
    //  Serial.print(future.month(), DEC);
    //  Serial.print('/');
    //  Serial.print(future.day(), DEC);
    //  Serial.print(' ');
    //  Serial.print(future.hour(), DEC);
    //  Serial.print(':');
    //  Serial.print(future.minute(), DEC);
    //  Serial.print(':');
    //  Serial.print(future.second(), DEC);
    //  Serial.println();
    //
    //  Serial.println();
    // delay(30000);
}

String sendDataBTW(String command, const int timeout, boolean debug)
{
    String response = "";
    
    esp8266.print(command); // send the read character to the esp8266
    
    long int time = millis();
    
    while( (time+timeout) > millis())
    {
      while(esp8266.available())
      {
        
        // The esp has data so display its output to the serial window 
        char c = esp8266.read(); // read the next character.
        response+=c;
      }  
    }
    
    if(debug)
    {
      Serial.print(response);
    }
    
    return response;
}

String SendHTML(float TemperatureWeb,float HumidityWeb, String TimeWeb,String DateWeb){
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<title>ESP8266 Global Server</title>\n";

  ptr +="</head>\n";
  ptr +="<body>\n";
  ptr +="<div id=\"webpage\">\n";
  ptr +="<h1>ESP8266 Global Server</h1>\n";

  ptr +="<p>Date: ";
  ptr +=(String)DateWeb;
  ptr +="</p>";
  ptr +="<p>Time: ";
  ptr +=(String)TimeWeb;
  ptr +="</p>";
  ptr +="<p>Temperature: ";
  ptr +=(int)TemperatureWeb;
  ptr +="C</p>";
  ptr +="<p>Humidity: ";
  ptr +=(int)HumidityWeb;
  ptr +="%</p>";
  
  ptr +="</div>\n";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}

void setupESP()
{
    sendCommand("AT+RST\r\n", 2000, DEBUG);
    sendCommand("AT+CWJAP=\"" + SSID +"\",\"" + PASS + "\"\r\n", 3000, DEBUG);
    sendCommand("AT+CWMODE=1\r\n", 1500, DEBUG);                                             //set the ESP8266 WiFi mode to station mode.
    sendCommand("AT+CIFSR\r\n", 1000, DEBUG);
    sendCommand("AT+CWMODE=3\r\n", 1000, DEBUG);
    sendCommand("AT+CIPMUX=1\r\n", 1000, DEBUG);       // configure for multiple connections
    sendCommand("AT+CIPSERVER=1,80\r\n", 1000, DEBUG); // set server
    Serial.println("Server Ready");
}

/*
   Name: sendData
   Description: Function used to send data to ESP8266.
   Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
   Returns: The response from the esp8266 (if there is a reponse)
*/
String sendData(String command, const int timeout, boolean debug)
{
    String response = "";
    Serial.println("Sending: " + command);
    esp8266.println(command);
    if (debug)
    {
        Serial.print("Command: ");
        Serial.println(command);
    }
    unsigned long start = millis();
    while (millis() - start < timeout)
    {
        if (esp8266.available())
        {
            char c = esp8266.read();
            response += c;
        }
    }
    if (debug)
    {
        Serial.println(response);
    }
    return response;

    // String response = "";
    // int dataSize = command.length();
    // char data[dataSize];
    // command.toCharArray(data, dataSize);
    // esp8266.write(data, dataSize); // send the read character to the esp8266
    // if (debug)
    // {
    //     Serial.println("\r\n====== HTTP Response From Arduino ======");
    //     Serial.write(data, dataSize);
    //     Serial.println("\r\n========================================");
    // }
    // long int time = millis();
    // while ((time + timeout) > millis())
    // {
    //     while (esp8266.available())
    //     {
    //         // The esp has data so display its output to the serial window
    //         char c = esp8266.read(); // read the next character.
    //         response += c;
    //     }
    // }
    // if (debug)
    // {
    //     Serial.print(response);
    // }
    // return response;
}

/*
   Name: sendHTTPResponse
   Description: Function that sends HTTP 200, HTML UTF-8 response
*/

void sendHTTPResponse(char connectionId)
{
    // build HTTP response
    String httpHeader;
    // HTTP Body

    // HTTP Header
    httpHeader = "HTTP/1.1 200 OK\r\n";
    httpHeader += "Content-Type: text/html; charset=UTF-8\r\n";
    httpHeader += "Connection: close\r\n";
    httpHeader += "Content-Length: ";
    httpHeader += 0;
    httpHeader += "\r\n\r\n";
    // HTTP Response
    // send the response
    Serial.println("send CIP Data");
    sendCIPData(connectionId, httpHeader);
}

/*
   Name: sendCIPDATA
   Description: sends a CIPSEND=<connectionId>,<data> command

*/
void sendCIPData(char connectionId, String data)
{
    String cipSend = "AT+CIPSEND=";
    cipSend += connectionId;
    cipSend += ",";
    cipSend += data.length();
    cipSend += "\r\n";
    Serial.println("CIPSEND Command");
    sendCommand(cipSend, 3000, DEBUG);
    sendData(data, 3000, DEBUG);
}

/*
   Name: sendCommand
   Description: Function used to send data to ESP8266.
   Params: command - the data/command to send; timeout - the time to wait for a response; debug - print to Serial window?(true = yes, false = no)
   Returns: The response from the esp8266 (if there is a reponse)
*/
String sendCommand(String command, const int timeout, boolean debug)
{
    String response = "";
    char c;
    esp8266.print(command); // send the read character to the esp8266
    long int time = millis();
    while ((time + timeout) > millis())
    {
        while (esp8266.available())
        {
            // The esp has data so display its output to the serial window
            c = esp8266.read(); // read the next character.
            response += c;
        }
        if (c == '>')
        {
            Serial.println("> found");
            break;
        }
    }
    if (debug)
    {
        Serial.println("Response: " + response);
    }
    return response;
}

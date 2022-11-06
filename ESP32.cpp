#include "HX710B.h"
HX710B pressure_sensor;
const int DOUT = 2;   //sensor data pin
const int SCLK  = 4;   //sensor clock pin
/*********
  Rui Santos
  Complete project details at http://randomnerdtutorials.com  
*********/

// Load Wi-Fi library
#include <WiFi.h>

// Replace with your network credentials
const char* ssid = "AndroidAPED3F";
const char* password = "ckkz7752";

// Set web server port number to 80
WiFiServer server(80);

// Variable to store the HTTP request
String header;

float pascal;

// Auxiliar variables to store the current output state
String output16State = "off";
String output17State = "off";

// Assign output variables to GPIO pins
const int output16 = 16;
const int output17 = 17;

// Current time
unsigned long currentTime = millis();
// Previous time
unsigned long previousTime = 0; 
// Define timeout time in milliseconds (example: 2000ms = 2s)
const long timeoutTime = 2000;

void setup() {
  Serial.begin(115200);
  pressure_sensor.begin(DOUT, SCLK);
  // Initialize the output variables as outputs
  pinMode(output16, OUTPUT);
  pinMode(output17, OUTPUT);
  // Set outputs to LOW
  digitalWrite(output16, HIGH);
  digitalWrite(output17, HIGH);

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
}

void loop(){
  //Serial.println(WiFi.localIP());
  if (pressure_sensor.is_ready()) {
    Serial.print("Pascal: ");
    Serial.println(pressure_sensor.pascal());
  }else {
    Serial.println("Pressure sensor not found.");
  }
  delay(1000);
  
  WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    currentTime = millis();
    previousTime = currentTime;
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected() && currentTime - previousTime <= timeoutTime) {  // loop while the client's connected
      currentTime = millis();
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
            if (header.indexOf("GET /16/on") >= 0) {
              Serial.println("GPIO 16 on");
              ok();
              output16State = "on";
              //digitalWrite(output16, HIGH);
            } else if (header.indexOf("GET /16/off") >= 0) {
              Serial.println("GPIO 16 off");
              output16State = "off";
              //digitalWrite(output16, LOW);
            } /*else if (header.indexOf("GET /17/on") >= 0) {
              Serial.println("GPIO 17 on");
              output17State = "on";
              digitalWrite(output17, HIGH);
            } else if (header.indexOf("GET /17/off") >= 0) {
              Serial.println("GPIO 17 off");
              output17State = "off";
              digitalWrite(output17, LOW);
            }*/
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the on/off buttons 
            // Feel free to change the background-color and font-size attributes to fit your preferences
            client.println("<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}");
            client.println(".button { background-color: #4CAF50; border: none; color: white; padding: 16px 40px;");
            client.println("text-decoration: none; font-size: 30px; margin: 2px; cursor: pointer;}");
            client.println(".button2 {background-color: #555555;}</style></head>");
            
            // Web Page Heading
            client.println("<body><h1>ESP32 Web Server</h1>");

            //open google
            client.println("<a href=\"http://google.com\" target=\"_blank\">open google</a>");

            //Pascal
            client.println("<p id=\"pascal01\" onclick=\"myFunction()\"> Pascal </p>");
            
            // Display current state, and ON/OFF buttons for GPIO 16  
            client.println("<p>GPIO 16 - State " + output16State + "</p>");
            // If the output16State is off, it displays the ON button       
            if (output16State=="off") {
              client.println("<p><a href=\"/16/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/16/off\"><button class=\"button button2\">OFF</button></a></p>");
            }
            
            /*// Display current state, and ON/OFF buttons for GPIO 17  
            client.println("<p>GPIO 17 - State " + output17State + "</p>");
            // If the output17State is off, it displays the ON button       
            if (output17State=="off") {
              client.println("<p><a href=\"/17/on\"><button class=\"button\">ON</button></a></p>");
            } else {
              client.println("<p><a href=\"/17/off\"><button class=\"button button2\">OFF</button></a></p>");
            }*/
            //client.println("<script>document.getElementById(\"pascal01\").innerHTML = \"Pascal =\"+pascal;</script>");

            client.println("<div id=\"demo\">");
            client.println("<button type=\"button\" onclick=\"myFunction()\">Change Content</button>");
            client.println("</div>");

            client.println("<sensorData>");
            client.println("Pascal = " + String(pressure_sensor.pascal()));
            client.println("</sensorData>");
            
            client.println("<script>");
            client.println("setInterval(updateValues, 1000, \"pascal01\");");
            client.println("function myFunction() {");
            //client.println("document.getElementById(\"pascal01\").innerHTML = \"currentTime;\"");
            client.println("  var xhttp = new XMLHttpRequest();");
            client.println("  xhttp.onreadystatechange = function() {");
            client.println("  if (this.readyState == 4 && this.status == 200) {");
            client.println("    document.getElementById(\"pascal01\").innerHTML =");
            client.println("    this.responseText;");
            client.println("  }");
            client.println("};");
            client.println("xhttp.open(\"GET\", \"pressure_sensor.pascal()\", true);");
            client.println("xhttp.send();");
            client.println("}");
            client.println("</script>");
            
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

void ok(){
while (pressure_sensor.is_ready()){
   if (pressure_sensor.pascal() < 444.40){
      digitalWrite(17,LOW);
      digitalWrite(16,LOW);
    }
    else{
      digitalWrite(17,HIGH);
      digitalWrite(16,HIGH);
      //break;
    }
    Serial.println("on");
    Serial.println(pressure_sensor.pascal());
    delay(200);
}
}

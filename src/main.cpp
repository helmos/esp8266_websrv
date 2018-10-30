#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino
#define BLYNK_PRINT Serial
// needed for library
#include <DNSServer.h>
#include <ESP8266WebServer.h>
#include <WiFiManager.h>         //https://github.com/tzapu/WiFiManager
#include <ESP8266mDNS.h>
#include <BlynkSimpleEsp8266.h>
//ADC_MODE(ADC_VCC);
// Servo lib
#include <Servo.h> 
Servo servo1; 
#define servo1Pin 12 
int val = 1;
bool flag = 0;
char auth[] = "c13bb5bc3a384fe181333cd3fcf01866";
//------------------------------------------
String valueString = String(5);
int pos1 = 0;
int pos2 = 0;
//----------------------------------------------

WiFiServer server(80);
void setup() {
    Blynk.config(auth);
    Blynk.connect();
    servo1.attach(servo1Pin);
    servo1.write(90);

    // put your setup code here, to run once:
    Serial.begin(9600);
    delay(10);

    // prepare GPIO2
    pinMode(2, OUTPUT);
    digitalWrite(2, 0);
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    //reset saved settings
    //wifiManager.resetSettings();

    //set custom ip for portal
    wifiManager.setAPStaticIPConfig(IPAddress(10,0,1,1), IPAddress(10,0,1,1), IPAddress(255,255,255,0));

    //fetches ssid and pass from eeprom and tries to connect
    //if it does not connect it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration
    //wifiManager.autoConnect("AutoConnectAP");
    //or use this for auto generated name ESP + ChipID
    wifiManager.autoConnect();

    //if you get here you have connected to the WiFi
    Serial.println("connected... :)");
    Serial.println(WiFi.localIP());
    // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp8266.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("esp8266")) {
    Serial.println("Error setting up MDNS responder!");
    while(1) {
      delay(1000);
    }
  }
Serial.println("mDNS responder started");
    // Start the server
    server.begin();
    Serial.println("HTTP Server started");
     MDNS.addService("http", "tcp", 80);
     Serial.println("mDNS service added");
}

BLYNK_WRITE(V1)
{
  int pinValue = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use:
  // String i = param.asStr();
  // double d = param.asDouble();
  servo1.write(pinValue);
  Serial.print("V1 Slider value is: ");
  Serial.println(pinValue);

}

BLYNK_WRITE(V0)
{
  int x = param[0].asInt();
  int y = param[1].asInt();
  int z = param[2].asInt();
  Serial.print("x value is: ");
  Serial.println(x);
  Serial.print("y value is: ");
  Serial.println(y);
  Serial.print("z value is: ");
  Serial.println(z);
  servo1.write(90 + z * 4);
}

void loop() {
    // put your main code here, to run repeatedly:
    Blynk.run();
  //Serial.println(analogRead(A0));
    // Check if a client has connected
  WiFiClient client = server.available();
  if (!client) {
    return;
  }

  // Wait until the client sends some data
  Serial.println("new client");
  while(!client.available()){ 
    delay(1);
  }

  // Read the first line of the request
  String req = client.readStringUntil('\r');
  Serial.println(req);
  client.flush();

//----------------------------------------------------------------------
  
  
   //----------------------------------------------------------------------

   // Match the request
   //int val;
   if (req.indexOf("/gpio/0") != -1)
   {
     val = 0;
     flag = 1;
   }
   else if (req.indexOf("/gpio/1") != -1)
   {
     val = 1;
     flag = 1;
   }
   else if (req.indexOf("/servo/") != -1)
   {
     servo1.write(req.substring(req.indexOf("/servo/") + 7).toInt());
   }
   else if (req.indexOf("/web/") != -1)
   {
     // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
     // and a content-type so the client knows what's coming, then a blank line:
     client.println("HTTP/1.1 200 OK");
     client.println("Content-type:text/html");
     client.println("Connection: close");
     client.println();
     // Display the HTML web page
     client.println("<!DOCTYPE html><html>");
     client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
     client.println("<link rel=\"icon\" href=\"data:,\">");
     // CSS to style the on/off buttons
     // Feel free to change the background-color and font-size attributes to fit your preferences
     client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial; margin-left:auto; margin-right:auto;}");
     client.println(".slider { width: 300px; }</style>");
     client.println("<script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.3.1/jquery.min.js\"></script>");
     // Web Page
     client.println("</head><body><h1>ESP8266 Servo</h1>");
     client.println("<p>Position: <span id=\"servoPos\"></span></p>");
     client.println("<input type=\"range\" min=\"10\" max=\"180\" class=\"slider\" id=\"servoSlider\" onchange=\"servo(this.value)\" value=\"" + valueString + "\"/>");
     client.println("<script>var slider = document.getElementById(\"servoSlider\");");
     client.println("var servoP = document.getElementById(\"servoPos\"); servoP.innerHTML = slider.value;");
     client.println("slider.oninput = function() { slider.value = this.value; servoP.innerHTML = this.value; }");
     client.println("$.ajaxSetup({timeout:1000}); function servo(pos) { ");
     client.println("$.get(\"/?value=\" + pos + \"&\"); {Connection: close};}</script>");
     client.println("</body></html>");
    }
  //-----------------------------------------------------------------------------
  //GET /?value=180& HTTP/1.1
   else if (req.indexOf("GET /?value=") >= 0)
   {
     pos1 = req.indexOf('=');
     pos2 = req.indexOf('&');
     valueString = req.substring(pos1 + 1, pos2);

     //Rotate the servo
     servo1.write(valueString.toInt());
     delay(20);
     Serial.println(valueString);
     return;
   }
  //-----------------------------------------------------------------------------
  else {
    Serial.println("invalid request");
    client.stop();
    return;
  }

  // Set GPIO2 according to the request
  digitalWrite(2, val);
  ///if (val == 0)
  /// servo1.write(170);
  ///else if (val == 1)
  /// servo1.write(10);


  client.flush();

  // Prepare the response
  String s = "HTTP/1.1 200 OK\r\nContent-Type: text/html\r\n\r\n<!DOCTYPE HTML>\r\n<html>\r\nGPIO is now ";
  s += (val)?"high":"low";
  s += analogRead(A0);
  s += "</html>\n";

  // Send the response to the client
  if (flag == 1) client.print(s);
  delay(1);
  Serial.println("Client disonnected");

  // The client will actually be disconnected
  // when the function returns and 'client' object is detroyed

}

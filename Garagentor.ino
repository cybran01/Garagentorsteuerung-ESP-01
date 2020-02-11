#include "ESP8266WiFi.h"
#include "ESP8266WebServer.h"
#include "EEPROM.h"

ESP8266WebServer server(80);

String spacer = "<div style=\"height:10%\"></div>";
String BUTTONText = "GARAGENTOR";
String BUTTON = "<button name=\"action\" value=\"TOGGLE\" class=\"myButton\">" + BUTTONText + "</button>";

String SSIDINPUT = "<label for=\"ssid\">SSID:<input type=\"text\" id=\"ssid\" name=\"ssid\"></label>";
String KEYINPUT = "<label for=\"key\">KEY:<input type=\"text\" id=\"key\" name=\"key\"></label>";
String SUBMITBUTTON = "<input type=\"submit\" value=\"Bestätigen\">";



String BUTTONStyle(String background_color, String border_color, String text_shadow, String hover_background_color) {
  return String(".myButton { ") +
         "width:60%;" +
         "height:150px;" +
         "background-color:" + background_color + ";" +
         "-moz-border-radius:28px;" +
         "-webkit-border-radius:28px;" +
         +"border-radius:28px;" +
         "border:1px solid " + border_color + ";" +
         "display:inline-block;" +
         "cursor:pointer;" +
         "color:#ffffff;" +
         "font-family:Arial;" +
         "font-size:70px;" +
         "padding:16px 31px;" +
         "text-decoration:none;" +
         "text-shadow:0px 1px 0px " + text_shadow + ";" +
         "}" +
         ".myButton:hover {" +
         "background-color:" + hover_background_color + ";" +
         "}" +
         ".myButton:active {" +
         "position:relative;" +
         "top:1px;" +
         "}";
}

bool toggleDoor = false, panicmode = false;
unsigned long prevMillis = 0;
const int controlPin = 3;
const int startupFixPin = 0;
const int readPin = 2;
//TODO: prints wegmachen, sonst mögliche steuerung des garagentors
void setup() {
  Serial.begin(9600);
  
  String ssid, key;
  EEPROM.begin(512);
  ssid = read_String(0);
  key = read_String(128);
  EEPROM.end();

  Serial.println("SSID: " + ssid);
  Serial.println("KEY: " + key);

  WiFi.begin(ssid, key);  //Connect to the WiFi network
  int errorCount;
  for (errorCount = 0; WiFi.status() != WL_CONNECTED && errorCount < 100; errorCount++) {  //Wait for connection

    delay(500);
    Serial.println("Waiting to connect…");

  }
  if (errorCount == 100) //Enter panic mode
  {
    panicmode = true;
    Serial.println("Entered Panicmode");
    WiFi.disconnect(true);
    WiFi.mode(WIFI_AP);
    WiFi.softAP("Garagentorsteuerung_Fallback", "123456789");
    server.on("/", handleRootPathPanic); //default IP: 192.168.4.1
    server.begin();
  }
  else {
    Serial.print("IP address: ");
    Serial.println(WiFi.localIP());  //Print the local IP

    //Initialize pins to be used as GPIOs
    pinMode(controlPin, FUNCTION_3);
    pinMode(readPin, FUNCTION_3);



    pinMode(startupFixPin, OUTPUT);
    digitalWrite(startupFixPin, LOW);
    pinMode(controlPin, OUTPUT);
    digitalWrite(controlPin, HIGH);
    pinMode(readPin, INPUT);
    //
    /*
      server.on("/other", []() {   //Define the handling function for the path

        String message = "Hello world!";

        server.send(200, "text/plain", message);

      });
    */
    server.on("/", handleRootPath);    //Associate the handler function to the path
    server.begin();                    //Start the server
    Serial.println("Server listening");
  }
}

void loop() {
  if (panicmode)
    server.handleClient();
  else {
    if (WiFi.status() != WL_CONNECTED)
      ESP.restart();

    server.handleClient();         //Handling of incoming requests

    if (toggleDoor) { //send 1sec puls to door
      prevMillis = millis();
      toggleDoor = false;
    }

    ((unsigned long)(millis() - prevMillis) > 1000) ? digitalWrite(controlPin, HIGH) : digitalWrite(controlPin, LOW);
  }
}

void handleRootPath() {            //Handler for the root path
  String action = server.arg("action");
  if (action != "") {
    Serial.println(action);
    if (action == "TOGGLE")
      toggleDoor = true;
  }

  String curBUTTONStyle = (digitalRead(readPin) == LOW) ? BUTTONStyle("#c74545", "#ab1919", "#662828", "#bd2a2a") /*rot*/ : BUTTONStyle("#44c767", "#18ab29", "#2f6627", "#5cbf2a");/*gruen*/;


  String PAGE_DATA = "<html><head> <meta http-equiv=\"refresh\" content=\"5\" /><style>" + curBUTTONStyle + "</style></head><body>" + spacer + "<form style=\"text-align:center\" method=\"post\">";
  PAGE_DATA += BUTTON;
  PAGE_DATA += "</form>";
  PAGE_DATA += "</body></html>";
  server.send(200, "text/html; charset=utf-8", PAGE_DATA);
}

void handleRootPathPanic() {            //Handler for the root path in case of panic mode
  String ssid = server.arg("ssid");
  String key = server.arg("key");

  if (ssid != "") {
    Serial.println(ssid);
    Serial.println(key);

    EEPROM.begin(512);
    write_String(0, ssid);
    write_String(128, key);
    EEPROM.end();
    ESP.restart();
  }

  String PAGE_DATA = "<html><body>" + spacer + "<form style=\"text-align:center\" method=\"post\">";
  PAGE_DATA += SSIDINPUT;
  PAGE_DATA += "<br><br>";
  PAGE_DATA += KEYINPUT;
  PAGE_DATA += "<br><br>";
  PAGE_DATA += SUBMITBUTTON;
  PAGE_DATA += "</form></body></html>";
  server.send(200, "text/html; charset=utf-8", PAGE_DATA);
}

void write_String(char add, String data)
{
  int _size = data.length();
  int i;
  for (i = 0; i < _size; i++)
  {
    EEPROM.write(add + i, data[i]);
  }
  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
}

String read_String(char add)
{
  int i;
  char data[100]; //Max 100 Bytes
  int len = 0;
  unsigned char k;
  k = EEPROM.read(add);
  while (k != '\0' && len < 100) //Read until null character or until 100 characters
  {
    k = EEPROM.read(add + len);
    data[len] = k;
    len++;
  }
  data[len] = '\0';
  return String(data);
}

#pragma once

#include <EEPROM.h>
#include <ESP8266WebServer.h>

#include "../CTML/include/ctml.hpp"

class Handler
{
	public:
		static const int controlPin = 3; //GPIO3 controls the relay
		static const int startupFixPin = 0; //Pin required to pull readPin low after startup to keep it from floating
		static const int readPin = 2; //GPIO2 reads the status of the door
		
		static void init(const int port, const bool panicmode, const int pulsetime)
		{
			toggleDoor = false;
			prevMillis = 0;
			pulseTime = pulsetime;
			
			server = new ESP8266WebServer(port);
			
			if (panicmode)		
				server->on("/", handleRootPathPanic);	
			else 
			{
				//Initialize pins to be used as GPIOs
				pinMode(controlPin, FUNCTION_3);
				pinMode(readPin, FUNCTION_3);

				pinMode(startupFixPin, OUTPUT);
				digitalWrite(startupFixPin, LOW);
				pinMode(controlPin, OUTPUT);
				digitalWrite(controlPin, HIGH);
				pinMode(readPin, INPUT);
				
				server->on("/", handleRootPath);
			}
			
			server->begin();
		}
		static void handleRequests()
		{
			server->handleClient();
			((unsigned long)(millis() - prevMillis) > pulseTime) ? digitalWrite(controlPin, HIGH) : digitalWrite(controlPin, LOW);
		}
		static bool toggleRequestPending()
		{
			return toggleDoor;
		}
		static void processToggleRequest()
		{
			prevMillis = millis();
			toggleDoor = false;
		}
		virtual ~Handler()
		{}
		

	private:
		Handler(){}
		static ESP8266WebServer* server;
		static bool toggleDoor;
		static unsigned long prevMillis;
		static int pulseTime;
		
		static void handleRootPath() //Handler for the root path for normal mode
		{    		
			if(server->arg("reset") != "")//Hidden parameter which allows one to reset the currently used credentials
			{
				EEPROM.begin(512);
				write_String(0, "");
				write_String(128, "");
				EEPROM.end();
				ESP.restart();
			}

			if (server->arg("action") == "toggle") //Handle incoming request to toggle state of door
			{
				toggleDoor = true;
			}
		
			String curToggleBtnStyle = (digitalRead(readPin) == LOW) ? toggleBtnStyle("#44c767", "#18ab29", "#2f6627", "#5cbf2a") /*green button when door closed*/ : toggleBtnStyle("#c74545", "#ab1919", "#662828", "#bd2a2a") /*red button when door open*/;

			CTML::Document document;
			document.AppendNodeToHead(CTML::Node("meta")
			.SetAttribute("http-equiv","refresh")
			.SetAttribute("content","5")
			.UseClosingTag(false));
			document.AppendNodeToHead(CTML::Node("style",curToggleBtnStyle.c_str()));
			document.AppendNodeToBody(CTML::Node("form")
			.SetAttribute("style","text-align:center")
			.SetAttribute("method","post")
			.AppendChild(CTML::Node("button.toggleBtn","Garagentor").SetAttribute("name","action").SetAttribute("value","toggle")));  

			server->send(200, "text/html; charset=utf-8", document.ToString().c_str());
		}
		static void handleRootPathPanic() //Handler for the root path in case of panic mode
		{
			String ssid = server->arg("ssid");
			String key = server->arg("key");
			
			if (ssid != "") {
				EEPROM.begin(512);
				write_String(0, ssid);
				write_String(128, key);
				EEPROM.end();
				ESP.restart();
			}
			
			CTML::Document document;
			CTML::Node form("form");
			form.SetAttribute("style","text-align:center").SetAttribute("method","post");
			form.AppendChild(CTML::Node("label","SSID:").AppendChild(CTML::Node("input").SetAttribute("type","text").SetAttribute("name","ssid").SetAttribute("maxlength","127").UseClosingTag(false)));//AppendChild returns child or parent?
			form.AppendChild(CTML::Node("br").UseClosingTag(false));
			form.AppendChild(CTML::Node("br").UseClosingTag(false));
			form.AppendChild(CTML::Node("label","KEY:").AppendChild(CTML::Node("input").SetAttribute("type","text").SetAttribute("name","key").SetAttribute("maxlength","127").UseClosingTag(false)));
			form.AppendChild(CTML::Node("br").UseClosingTag(false));
			form.AppendChild(CTML::Node("br").UseClosingTag(false));
			form.AppendChild(CTML::Node("input").SetAttribute("type","submit").SetAttribute("value","Bestätigen").UseClosingTag(false));
			document.AppendNodeToBody(form);
			
			server->send(200, "text/html; charset=utf-8", document.ToString().c_str());
		}	
		static inline String toggleBtnStyle(String background_color, String border_color, String text_shadow, String hover_background_color) 
		{
			return String(".toggleBtn{") +
			 "margin-top:10%;" + 
			 "width:60%;" +
			 "height:150px;" +
			 "background-color:" + background_color + ";" +
			 "-moz-border-radius:28px;" +
			 "-webkit-border-radius:28px;" +
			 "border-radius:28px;" +
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
			 ".toggleBtn:hover{" +
			 "background-color:" + hover_background_color + ";" +
			 "}" +
			 ".toggleBtn:active{" +
			 "position:relative;" +
			 "top:1px;" +
			 "}";
		}
		static void write_String(char add, String data) //write String to EEPROM
		{
		  int _size = data.length();
		  int i;
		  for (i = 0; i < _size; i++)
		  {
			EEPROM.write(add + i, data[i]);
		  }
		  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
		}
		static String read_String(char add) //read String from EEPROM
		{
		  int i;
		  char data[128]; //Max 128 Bytes
		  int len = 0;
		  unsigned char k;
		  k = EEPROM.read(add);
		  while (k != '\0' && len < 127) //Read until null character or until 127 characters
		  {
			k = EEPROM.read(add + len);
			data[len] = k;
			len++;
		  }
		  data[len] = '\0';
		  return String(data);
		}
		
};

ESP8266WebServer* Handler::server;
bool Handler::toggleDoor;
unsigned long Handler::prevMillis;
int Handler::pulseTime;
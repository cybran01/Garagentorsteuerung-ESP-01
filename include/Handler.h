#pragma once

#include <EEPROM.h>
#include "Connector.h"
#include "../CTML/include/ctml.hpp"

/*
#define MULTI_LINE_STRING(...) #__VA_ARGS__

MULTI_LINE_STRING(.toggleBtn 
			{ 
				background-color: + background_color + ;
				border:1px solid  + border_color + ; 
				text-shadow:0px 1px 0px  + text_shadow + ;
				width:60%;
				height:150px;
				-moz-border-radius:28px;
				-webkit-border-radius:28px;
				border-radius:28px;
				display:inline-block;
				cursor:pointer;
				color:#ffffff;
				font-family:Arial;
				font-size:70px;
				padding:16px 31px;
				text-decoration:none;
			}
			.toggleBtn:hover 
			{
				background-color: + hover_background_color + ;
			}
			.toggleBtn:active 
			{
				position:relative;
				top:1px;
			})
*/
class Handler
{
	public:
		Handler(Connector connect, int port, bool panicmode)
		{
			connector = connect;
			//initialize server
		}
		virtual ~Handler()
		{}
	private:
		//ESP8266WebServer server;
		Connector connector;
		bool toggleDoor = false;
		
		void handleRootPath() //Handler for the root path for normal mode
		{    
		
			String action = server.arg("action");//Handle incoming request to toggle state of door
			if (action == "toggle") 
			{
				toggleDoor = true;
			}
		
			String curToggleBtnStyle = connector.doorOpen() ? toggleBtnStyle("#c74545", "#ab1919", "#662828", "#bd2a2a") /*red button when door open*/ : toggleBtnStyle("#44c767", "#18ab29", "#2f6627", "#5cbf2a") /*green button when door closed*/;
			
			CTML::node toggleBtn("button.toggleBtn","Garagentor");
			toggleBtn.SetAttribute("name","action").SetAttribute("value","toggle");

			CTML::Document document;
			document.AppendNodeToHead(CTML::Node("meta")
			.SetAttribute("http-equiv","refresh")
			.SetAttribute("content","5")
			.UseClosingTag(false));
			document.AppendNodeToHead(CTML::Node("style",curToggleBtnStyle.c_str()));
			document.AppendNodeToBody(CTML::Node("div").SetAttribute("style","height:10%"));
			document.AppendNodeToBody(CTML::Node("form")
			.SetAttribute("style","text-align:center")
			.SetAttribute("method","post")
			.AppendChild(toggleBtn));  

			//server.send(200, "text/html; charset=utf-8", document.ToString());
		}

		void handleRootPathPanic() //Handler for the root path in case of panic mode
		{
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
			
			CTML::Document document;
			CTML::Node form;
			form.SetAttribute("style","text-align:center").SetAttribute("method","post");
			form.AppendChild(CTML::Node("label","SSID:")).AppendChild(CTML::Node("input")).SetAttribute("type","text").SetAttribute("name","ssid").UseClosingTag(false);
			form.AppendChild(CTML::Node("br").UseClosingTag(false));
			form.AppendChild(CTML::Node("br").UseClosingTag(false));
			form.AppendChild(CTML::Node("label","KEY:")).AppendChild(CTML::Node("input")).SetAttribute("type","text").SetAttribute("name","key").UseClosingTag(false);
			form.AppendChild(CTML::Node("br").UseClosingTag(false));
			form.AppendChild(CTML::Node("br").UseClosingTag(false));
			form.AppendChild(CTML::Node("input")).SetAttribute("type","sumbit").SetAttribute("value","Bestätigen").UseClosingTag(false);
			document.AppendNodeToBody(form);
			
			//server.send(200, "text/html; charset=utf-8", document.ToSring());
		}	
		
		inline String toggleBtnStyle(String background_color, String border_color, String text_shadow, String hover_background_color) 
		{
			return String(".myButton { ") +
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
			 ".myButton:hover {" +
			 "background-color:" + hover_background_color + ";" +
			 "}" +
			 ".myButton:active {" +
			 "position:relative;" +
			 "top:1px;" +
			 "}";
		}
		
		void write_String(char add, String data) //write String to EEPROM
		{
		  int _size = data.length();
		  int i;
		  for (i = 0; i < _size; i++)
		  {
			EEPROM.write(add + i, data[i]);
		  }
		  EEPROM.write(add + _size, '\0'); //Add termination null character for String Data
		}

		String read_String(char add) //read String from EEPROM
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


//#undef MULTI_LINE_STRING(...)
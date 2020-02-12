#pragma once

#include <ESP8266WiFi.h>
#include <EEPROM.h>

class Connector
{
	public:
		Connector(const int timeoutCountMax, const int retryDelay, const String fallbackSSID, const String fallbackWiFiKey)
		{
			String ssid, key;
			
			//Read previous ssid and key from EEPROM of ESP8266-Chip
			EEPROM.begin(512);
			ssid = read_String(0);
			key = read_String(128);
			EEPROM.end();
			
			WiFi.begin(ssid, key);  //Connect to the WiFi network
			
			int timeoutCount;
			for (timeoutCount = 0; WiFi.status() != WL_CONNECTED && timeoutCount < timeoutCountMax; timeoutCount++) //Wait for connection, with max timeoutCountMax tries and delaytime retryDelay
			{  
				delay(retryDelay);  
			}
			if (timeoutCount == timeoutCountMax) //Enter panic mode
			{
				panicmode = true;
				WiFi.mode(WIFI_AP); //Switch mode from client to access point
				WiFi.softAP(fallbackSSID, fallbackWiFiKey); 
			}

		}
		bool panicMode()
		{
			return panicmode;
		}
		
		bool ok()
		{
			if (panicmode)
				return true;
			
			return (WiFi.status() == WL_CONNECTED);
		}
		
		virtual ~Connector()
		{}
		
	private:
		bool panicmode = false;
		
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
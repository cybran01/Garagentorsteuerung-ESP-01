#include "include/Connector.h"
#include "include/Handler.h"

Connector* connection;

void setup() {
  connection = new Connector(100,500,"Garagentorsteuerung_Fallback","123456789");//default fallback IP: 192.168.4.1
  Handler::init(80,connection->panicMode(),1000);
}

void loop() {
    if (!connection->ok())
      ESP.restart();
      
    Handler::handleRequests();//Handling of incoming requests      

    if (Handler::toggleRequestPending()) 
    { 
      Handler::processToggleRequest();
    }
  
}

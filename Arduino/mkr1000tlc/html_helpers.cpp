#include "html_helpers.h"

#include <SPI.h>
#include <WiFi101.h>

#include "html_helpers.h"

char ssid[] = "RCMP14";      // your network SSID (name)
char pass[] = "spatialguru";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)

int status = WL_IDLE_STATUS;

WiFiServer server(80);

String* lastdata; 
int lastdatasize;

void WifiInit(int port){
  // check for the presence of the shield:
  if (WiFi.status() == WL_NO_SHIELD) {
    // don't continue:
    while (true);
  }

  // attempt to connect to Wifi network:
  while ( status != WL_CONNECTED) {
    // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
    status = WiFi.begin(ssid, pass);

    // wait 10 seconds for connection:
    delay(10000);
  }  
  server.begin();
}


bool Listen(){

  WiFiClient client = server.available();   // listen for incoming clients
  if (client) {                             // if you get a client,
    String data = "";                // make a String to hold incoming data from the client
    if (client.connected()) {            // loop while the client's connected
      while(client.available() <= 0); // Wait until we've recived all the data
      while (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        data += c;      // add it to the end of the currentLine
      }
      if(data != "") //Prevent blank "Ghost" requests being passed
        lastdata = parseHTML(data);
      
      // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
      // and a content-type so the client knows what's coming, then a blank line:
      client.println("HTTP/1.1 200 OK");
      client.println("Content-type:text/html");
      client.println();

      // the content of the HTTP response follows the header:
      //If the data was recived successfly 
      if(data != ""){
        client.print("1");
      }
      //If the data was dropped for some reason, return a 0 so the app has a chance to re-send
      else{
        client.print("0");
      }
      // The HTTP response ends with another blank line:
      client.println();
    }
    client.stop();

    if(data != NULL && data != "\n")
      return true;
    else
      return false;
  }else{
    return false;  
  }
}

String* LastData(){
  return   lastdata;
}

int LastDataSize(){
  return   lastdatasize;
}

String* parseHTML(String s){
  //Seperate the headers and the body
  String headers = s.substring(0, s.indexOf("\r\n\r\n"));
  String body = s.substring(s.indexOf("\r\n\r\n")+4);

  //Convert the body to a char array
  char charBuf[body.length()+1];
  body.toCharArray(charBuf, body.length()+1) ;

  //Return an array of all the keyvalue pairs
  return StringSplit(charBuf, "&");
}

String* StringSplit(char* str, char* split){
  lastdatasize = countChars(str, split[0]);
  String* arry = new String[lastdatasize];
  
  char * pch;
  pch = strtok (str, split);
  int i =0;
  while (pch != NULL)
  {
    if(pch != "")
      arry[i] = pch;
    pch = strtok (NULL, split);
    ++i;
  }
  return arry;
}

int countChars( char* s, char c )
{
  int i = 0;
  for(int x = 0; x < strlen(s); ++x){
    if(s[x] == c)
      ++i;  
  }
  return i+1;
}

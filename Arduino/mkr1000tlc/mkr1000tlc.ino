//TLC
#include <MKR1000_TLC5940.h>
#include <MKR1000_Interupts.h>
//END TLC

#include <SPI.h>
#include <WiFi101.h>

#include "html_helpers.h"

int r1 = 0;
int g1 = 0;
int b1 = 0;

//3100
int r2 = 0;
//3800
int g2 = 0;
int b2 = 0;

int r3 = 0;
int g3 = 0;
int b3 = 0;

int r4 = 0;
int g4 = 0;
int b4 = 0;

int r5 = 0;
int g5 = 0;
int b5 = 0;

void setup() {
  //TLC
  TLC_init();
  //END TLC

  WifiInit(80);
}

void loop() {
  //Check if a client is posting data
  if(Listen()){
    //Get the posted data
    String* data = LastData();    
    //Handle the posted data
    for(int i = 0; i < LastDataSize(); ++i){
      String d = data[i];
      //Split the key and value apart by the equals inbetween
      String key = d.substring(0, d.indexOf('='));
      int value = d.substring(d.indexOf('=')+1).toInt();

      //Make sure that the value is in the color range
      if(value > 4095)
        value = 4095;
      if (value < 0)
        value = 0;

      //Set the appropriate colors to the value according to the key
      if(key == "r1")
        r1 = value;
      if(key == "g1")
        g1 = value;
      if(key == "b1")
        b1 = value;

      if(key == "r2")
        r2 = value;
      if(key == "g2")
        g2 = value;
      if(key == "b2")
        b2 = value;

      if(key == "r3")
        r3 = value;
      if(key == "g3")
        g3 = value;
      if(key == "b3")
        b3 = value;

      if(key == "r4")
        r4 = value;
      if(key == "g4")
        g4 = value;
      if(key == "b4")
        b4 = value;

      if(key == "r5")
        r5 = value;
      if(key == "g5")
        g5 = value;
      if(key == "b5")
        b5 = value;

      if(key == "a"){
        r1 = value;
        r2 = value;
        r3 = value;
        r4 = value;
        r5 = value;

        g1 = value;
        g2 = value;
        g3 = value;
        g4 = value;
        g5 = value;

        b1 = value;
        b2 = value;
        b3 = value;
        b4 = value;
        b5 = value;
      }

      if(key == "r"){
        r1 = value;
        r2 = value;
        r3 = value;
        r4 = value;
        r5 = value;
      }
      if(key == "g"){
        g1 = value;
        g2 = value;
        g3 = value;
        g4 = value;
        g5 = value;
      }
      if(key == "b"){
        b1 = value;
        b2 = value;
        b3 = value;
        b4 = value;
        b5 = value;
      }      
    }
    //If there are new colors, send them to the tlc
    UpdateColors();
  }
}  

//Sends all the colors to the TLC
void UpdateColors(){
  TLC_SetChannel(0, r1, g1, b1); 
  TLC_SetChannel(1, r2, g2, b2); 
  TLC_SetChannel(2, r3, g3, b3); 
  TLC_SetChannel(3, r4, g4, b4); 
  TLC_SetChannel(4, r5, g5, b5); 
}



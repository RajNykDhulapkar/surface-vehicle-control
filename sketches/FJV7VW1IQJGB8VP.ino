#include <VirtualWire.h>
#include <string.h>

long tm,t,d; //variables to hold time in seconds

  byte message[VW_MAX_MESSAGE_LEN];
  byte messageLength = VW_MAX_MESSAGE_LEN; 
 
  void setup() 
  {
    Serial.begin(9600);
    vw_set_ptt_inverted(true);
    vw_setup(2000);
    vw_set_rx_pin(7);
    vw_rx_start();
  }
 
  void loop() 
  {
    if (vw_get_message(message, &messageLength)) 
    {
      tm = millis();//needed to keep track of time elapsed since serial monitor has been opened
      t = tm/1000;
      d = tm%1000;
      
      //printing time in seconds and temperature in fahrenheit on the serial monitor
      Serial.print("t = ");
      Serial.print(t);
      Serial.print(".");
      Serial.print(d);
      Serial.print("s    temp = ");
      
      for (int i = 0; i < messageLength; i++)//printing data recieved from RF transission
      {
        Serial.print(message[i] - 48);
      }

      Serial.println(" *F");
    }
  }

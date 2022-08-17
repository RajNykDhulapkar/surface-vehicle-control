#include <VirtualWire.h>

int temp;  //to store surrounding temperature value in fahrenheit
int sensePin = 0; //temperature sensor connected to analog pin A0
char data[5]; //to store the temperature value as a character array form, which is needed while transmitting the temperature value

void setup() 
{
  Serial.begin(9600);
  vw_set_ptt_inverted(true);
  vw_setup(2000);
  
  vw_set_tx_pin(7);
  analogReference(DEFAULT);//optional,not needed
  pinMode(0,INPUT);
}
 
void loop() 
{
  temp = analogRead(sensePin); //analog reading the temperature values from the temperature sensor
  
  changeC(temp,data);
  vw_send((uint8_t*)data,2); //sending data through RF transimission
  delay(3000); //delay of 3 seconds
}

void changeC(int num,char *data) //function to convert temperature data from integers to character array
{
  int k = num,i = 0,j;

  while (k > 0)
  {
    k/=10;
    i++;
  }
  int l = i;

  for (i = 0,j = l-1; i < l; i++,j--)
  {
    data[j] = char(num%10+48);
    num/=10;
  }
}

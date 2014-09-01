#include <Wire.h>
int t,l;
void setup()
{
Wire.begin();
Serial.begin(9600);
}
void loop()
{

//Serial.print("temperature = ");  
Wire.requestFrom(0x48, 2);
while(Wire.available())
{
t = Wire.read(); 
l = Wire.read();      
Serial.print("temperature = ");
Serial.println(t);
Serial.print("lsb = ");
Serial.println(l);
}
delay(500);
}

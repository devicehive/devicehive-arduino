#include <Wire.h> 
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2); 

#define DHT11_PIN 0 

byte read_dht11_dat()
{
  byte i = 0;
  byte result=0;
  for(i=0; i< 8; i++)
  {
    while(!(PINB & _BV(DHT11_PIN)))
    {
    }; 
    delayMicroseconds(30);
    if(PINB & _BV(DHT11_PIN)) 
      result |=(1<<(7-i)); 
    while((PINB & _BV(DHT11_PIN)));
  }
  return result;
}


void setup()
{
//  lcd.init();                     
  lcd.begin(2,16);              // rows, columns.  use 2,16 for a 2x16 LCD, etc.
  lcd.clear();                  // start with a blank screen
  lcd.setCursor(0,0); 
//  lcd.backlight();
  DDRB |= _BV(DHT11_PIN); 
  PORTB |= _BV(DHT11_PIN); 
}

void loop()
{
  byte dht11_dat[5];
  byte dht11_in;
  byte i;
  PORTB &= ~_BV(DHT11_PIN);
  delay(18);
  PORTB |= _BV(DHT11_PIN);
  delayMicroseconds(1);
  DDRB &= ~_BV(DHT11_PIN); 
  delayMicroseconds(40);
  dht11_in = PINB & _BV(DHT11_PIN); 
  if(dht11_in)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("1 not met");
    delay(1000);
    return;
  }
  delayMicroseconds(80);
  dht11_in = PINB & _BV(DHT11_PIN); //
  if(!dht11_in)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("2 not met");
    return;
  }

  delayMicroseconds(80);
  for (i=0; i<5; i++)
  { 
    dht11_dat[i] = read_dht11_dat();
  } 

    DDRB |= _BV(DHT11_PIN); 
  PORTB |= _BV(DHT11_PIN); 
  byte dht11_check_sum = dht11_dat[0]+dht11_dat[1]+dht11_dat[2]+dht11_dat[3];
  if(dht11_dat[4]!= dht11_check_sum)
  {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("checksum");
  }
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("HUM");
  lcd.setCursor(5,0);
  lcd.print(dht11_dat[0], DEC);
  lcd.setCursor(10,0);
  lcd.print("%");
  lcd.setCursor(0,1);
  lcd.print("TEMP");
  lcd.setCursor(5,1);
  lcd.print(dht11_dat[2], DEC);
  lcd.setCursor(10,1);
  lcd.print("C");
  delay(2000); 
}

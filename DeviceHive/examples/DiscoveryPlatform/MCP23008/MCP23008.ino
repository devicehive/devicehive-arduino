#include <Wire.h>
#include "Adafruit_MCP23008.h"
#include <SPI.h>              // We use this library, so it must be called here.
#include <MCP23S17.h>         // Here is the new class to make using the MCP23S17 easy.
#include <OneWire.h>
#include <LiquidCrystal.h>

LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

//int backLight = 13; // pin 13 will control the backlight

OneWire ds(A2); // ds18b20 pin #2 (middle pin) to Arduino pin 8

byte i;
byte present = 0;
byte data[12];
byte addr[8];
 
int HighByte, LowByte, SignBit, Whole, Fract, TReading, Tc_100, FWhole;

Adafruit_MCP23008 mcp;
MCP outputchip(0);            // Instantiate an object called "outputchip" on an MCP23S17 device at address 2


void getTemp() {
  int foo, bar;
 
  ds.reset();
  ds.select(addr);
  ds.write(0x44,1);
 
  present = ds.reset();
  ds.select(addr);   
  ds.write(0xBE);

  for ( i = 0; i < 9; i++) {
    data[i] = ds.read();
  }
 
  LowByte = data[0];
  HighByte = data[1];
  TReading = (HighByte << 8) + LowByte;
  SignBit = TReading & 0x8000;  // test most sig bit
 
  if (SignBit) {
    TReading = -TReading;
  }
  Tc_100 = (6 * TReading) + TReading / 4;    // multiply by (100 * 0.0625) or 6.25
  Whole = Tc_100 / 100;          // separate off the whole and fractional portions
  Fract = Tc_100 % 100;
  if (Fract > 49) {
    if (SignBit) {
      --Whole;
    } else {
      ++Whole;
    }
  }

  if (SignBit) {
    bar = -1;
  } else {
    bar = 1;
  }
  foo = ((Whole * bar) * 18);      // celsius to fahrenheit conversion section
  FWhole = (((Whole * bar) * 18) / 10) + 32;
  if ((foo % 10) > 4) {            // round up if needed
       ++FWhole;
  }
}

void printTemp(void) {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("DHS-2 Temp is: ");
  lcd.setCursor(0,1);  
 
  if (SignBit) { 
     lcd.print("-");
  }
  lcd.print(Whole);
  lcd.print(".");
  lcd.print(Fract);
  lcd.print(" C / ");
  lcd.print(FWhole);
  lcd.print(" F");
}

void setup() {
  
DDRB |= 0b00000110;                      // Direct port manipulation speeds taking Slave Select HIGH after SPI action

outputchip.pinMode(0x0000);    // Use word-write mode to Set all of the pins on outputchip to be outputs

mcp.begin(); // use default address 0

  mcp.pinMode(7, OUTPUT);
  mcp.pinMode(6, OUTPUT);
  mcp.pinMode(5, OUTPUT);
  mcp.pinMode(4, OUTPUT);
  mcp.pinMode(3, OUTPUT);
  mcp.pinMode(2, OUTPUT);
  mcp.pinMode(1, OUTPUT);
  mcp.pinMode(0, OUTPUT);
  
  lcd.begin(2,16);              // rows, columns.  use 2,16 for a 2x16 LCD, etc.
  lcd.clear();                  // start with a blank screen
  lcd.setCursor(0,0);           // set cursor to column 0, row 0
 
    if ( !ds.search(addr)) {
      lcd.clear(); lcd.print("No more addrs");
      delay(1000);
      ds.reset_search();
      return;
  }

  if ( OneWire::crc8( addr, 7) != addr[7]) {
      lcd.clear(); lcd.print("CRC not valid!");
      delay(1000);
      return;
  }
}


void loop() {

  getTemp();
  printTemp();

  int value;                        
  
  mcp.digitalWrite(7, HIGH);
  outputchip.digitalWrite(0xffff); 
  delay(500);
  mcp.digitalWrite(7, LOW);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
  mcp.digitalWrite(6, HIGH);
  outputchip.digitalWrite(0xffff); 
  delay(500);
  mcp.digitalWrite(6, LOW);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
  mcp.digitalWrite(5, HIGH);
  outputchip.digitalWrite(0xffff); 
  delay(500);
  mcp.digitalWrite(5, LOW);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
  mcp.digitalWrite(4, HIGH);
  outputchip.digitalWrite(0xffff); 
  delay(500);
  mcp.digitalWrite(4, LOW);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
  mcp.digitalWrite(3, HIGH);
  outputchip.digitalWrite(0xffff); 
  delay(500);
  mcp.digitalWrite(3, LOW);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
  mcp.digitalWrite(2, HIGH);
  outputchip.digitalWrite(0xffff); 
  delay(500);
  mcp.digitalWrite(2, LOW);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
  mcp.digitalWrite(1, HIGH);
  outputchip.digitalWrite(0xffff); 
  delay(500);
  mcp.digitalWrite(1, LOW);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
  mcp.digitalWrite(0, HIGH);
  outputchip.digitalWrite(0xffff); 
  delay(500);
  mcp.digitalWrite(0, LOW);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
}

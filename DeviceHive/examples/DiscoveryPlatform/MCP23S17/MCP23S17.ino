#include <SPI.h>              // We use this library, so it must be called here.
#include <MCP23S17.h>         // Here is the new class to make using the MCP23S17 easy.

MCP outputchip(0);   

void setup() {
  
DDRB |= 0b00000110;                      // Direct port manipulation speeds taking Slave Select HIGH after SPI action

outputchip.pinMode(0x0000);    // Use word-write mode to Set all of the pins on outputchip to be outputs
}

void loop() {

  int value;                        
  
  outputchip.digitalWrite(0xffff); 
  delay(500);
  outputchip.digitalWrite(0x0000); 
  delay(500);
  
}

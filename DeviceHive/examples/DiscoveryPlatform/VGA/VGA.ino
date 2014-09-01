
    #define NOP asm("nop")
    #define BLACK    PORTD = B00000000;
    #define BLUE     PORTD = B00010000;
    #define GREEN    PORTD = B00100000;
    #define CYAN     PORTD = B00110000;
    #define RED      PORTD = B01000000;
    #define MAGENTA  PORTD = B01010000;
    #define YELLOW   PORTD = B01100000;
    #define WHITE    PORTD = B01110000;
     
    unsigned int linecount = 1;
     
    void setup()
    {
      //Set pins 5 to 10 as outputs
      // 7 - HSYNC
      // 6 - VSYNC
      // 10, 9 e 8 - RGB  
//      DDRD |= B11100000;
      DDRD = 0xff;
//      PORTD |= B11000000;
     
      //set timer  
      TCCR2A = 0x02;                        // WGM22=0 + WGM21=1 + WGM20=0 = Mode2 (CTC)
      TCCR2B |= (1 << CS20);                //
      TCCR2B |= (1 << CS21);                // Set prescaler
      TCCR2B &= ~(1 << CS22);               //
     
      TCNT2 = 0;                            // clean counter
     
      TIMSK2 &= ~(1<<OCIE2A);                // set comparison interrupt  
      TIMSK2 |= (1<<TOIE2);                // set overflow interrupt  
    }
     
    void loop()
    {
      noInterrupts();
      do{
        BLACK;
        if (TCNT2 > 0x0f){
     
          delayMicroseconds(1);
          NOP;NOP;NOP;NOP;
         
          TCNT2 = 0x00;
     
       
                           
          // #### HSYNC ###
          PORTD &= ~(1 << 2);      
          if (++linecount >= 525){ //525 lines
            linecount = 1;
          }      
          PORTD |= (1 << 2);
     
     
     
          // ### VSYNC ###
          if ((linecount == 1)||(linecount == 2)){
            PORTD &= ~(1 << 3);      
          } else {
            PORTD |= (1 << 3);
           
     
            NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
            NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
            NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
            NOP;NOP;NOP;NOP;NOP;
           
            if ((linecount >= 9) && (linecount <= 489)){
             
                    WHITE;
                    delayMicroseconds(3);NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
                    BLACK;
                    delayMicroseconds(3);NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
                    BLUE;
                    delayMicroseconds(3);NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
                    GREEN;
                    delayMicroseconds(3);NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
                    CYAN;
                    delayMicroseconds(3);NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
                    RED;
                    delayMicroseconds(3);NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
                    MAGENTA;
                    delayMicroseconds(3);NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
                    YELLOW;
                    delayMicroseconds(3);NOP;NOP;NOP;NOP;NOP;NOP;NOP;NOP;
                    BLACK;
                    NOP;NOP;NOP;NOP;
            }
               
          }
     
     
    }
     
     
    }while(1);
     
    }



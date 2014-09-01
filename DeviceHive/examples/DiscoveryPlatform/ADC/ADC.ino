    #include <LiquidCrystal.h>
     
    int tempPin = 3;
    int lightPin = 0;
    int ADCPin = 1;

    LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
     
    void setup()
    {
    lcd.begin(20, 2);
    }
     
    void loop()
    {
    int tempReading = analogRead(tempPin);
    int ADCReading = analogRead(ADCPin);
    int lightReading = analogRead(lightPin);
    
    float tempVolts = tempReading * 5.0 / 1024.0;
    float tempC = (tempVolts - 0.5) * 100.0;
    float tempF = tempC * 9.0 / 5.0 + 32.0;
    // ----------------

    //setup();
    
    lcd.clear();
    lcd.print("Temp C=");
    lcd.setCursor(7, 0);
    lcd.print(tempC);
    // Display Light on second row
    
    lcd.setCursor(0, 1);
    lcd.print("Light=");
    lcd.setCursor(6, 1);
    lcd.print(lightReading);

    lcd.setCursor(10, 1);
    lcd.print("ADC=");
    lcd.setCursor(14, 1);
    lcd.print(ADCReading);

    delay(500);
    
    }

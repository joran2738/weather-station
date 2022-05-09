#include  <Adafruit_ST7735.h>
#include  <Adafruit_GFX.h>
#include <Wire.h>

const int menu = 2;
const int omhoog = 3;
const int omlaag = 4;
int ingedrukt = 0;
int terug = 0;
byte neerslag = true;
byte temp = false;
byte luchtvochtigheid = false;
byte druk = false;
byte licht = false;
int scroll = 6;
byte inst = false;      

void setup() {
  pinMode(menu, INPUT);
  pinMode(omhoog, INPUT);
  pinMode(omlaag, INPUT);
}

void loop() {
instellingen();
}

void instellingen(){
  ingedrukt = digitalRead(menu);
  inst = menu;
  if (ingedrukt == HIGH)
  { //start instellingen
    terug = 0;
    while(terug == 0)
    {
      if(digitalRead(omhoog) == 1)
      {//zet de select zone 1 omhoog
        if(scroll > 0){
          scroll = scroll - 1;
          delay(500);//om de knoppen te debouncen
        }
      }
      if(digitalRead(omlaag) == 1)
        {//zet de select zone 1 omlaag
        if(scroll < 7){
          scroll = scroll + 1;
          delay(500);//om de knoppen te debouncen
        }
        }
      switch(scroll){
        case 6: //neerslag mm/m² - %
        if(digitalRead(menu) == 1)
        {//selecteer de optie
          neerslag = !(neerslag); 
          delay(500);
        }
        break;
        case 5: //temperatuur °C - °F
        if(digitalRead(menu) == 1)
        {//selecteer de optie
          temp = !(temp); 
          delay(500);
        }
        break;
        case 4: //luchtvochtigheid % - ppm
        if(digitalRead(menu) == 1)
        {//selecteer de optie
          luchtvochtigheid = !(luchtvochtigheid); 
          delay(500);
        }
        break;
        case 3: //druk bar - P
        if(digitalRead(menu) == 1)
        {//selecteer de optie
          druk = !(druk); 
          delay(500);
        }
        break;
        case 2: //lichtintensiteit lux - % 
        if(digitalRead(menu) == 1)
        {//selecteer de optie
          licht = !(licht); 
          delay(500);
        }
        break;
        case 1: //terug
        terug = 1;
        break;
      }
    }
  }
}

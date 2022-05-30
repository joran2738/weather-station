#include  <Adafruit_ST7735.h>
#include  <Adafruit_GFX.h>
#include <Wire.h>

const int menu = 3;
const int omhoog = 2;
const int omlaag = 4;
byte ingedrukt = 0;
byte terug = 0;
bool neerslag = true;
bool temp = false;
bool luchtvochtigheid = false;
bool druk = false;
int scroll = 4;
bool inst = false;      

void setup() {
  pinMode(menu, INPUT_PULLUP);
  pinMode(omhoog, INPUT_PULLUP);
  pinMode(omlaag, INPUT_PULLUP);
  Serial.begin(9600); while (!Serial); Serial.println();
}

void loop() {
  instellingen();
  Serial.println("out of settings");
  delay(100);
}

void instellingen(){
  scroll = 4;
  ingedrukt = digitalRead(menu);
  inst = menu;
  
  if (ingedrukt == LOW){ //start instellingen
    
    Serial.println("entered settings");
    terug = 0;
    
    while(terug == 0){
      if(digitalRead(omhoog) == 0){//zet de select zone 1 omhoog
        if(scroll > 1){
          scroll = scroll - 1;
          Serial.println(scroll);
          delay(500);//om de knoppen te debouncen
        }
      }
      
      if(digitalRead(omlaag) == 0){//zet de select zone 1 omlaag
          if(scroll < 4){
            scroll = scroll + 1;
            Serial.println(scroll);
            delay(500);//om de knoppen te debouncen
          }
      }
      
      
      switch(scroll){

        case 4: //temperatuur °C - °F
        if(digitalRead(menu) == 0){//selecteer de optie
          temp = !(temp); 
          Serial.println("temp"+String(temp));
          delay(500);
        }
        break;
          
        case 3: //luchtvochtigheid % - ppm
        if(digitalRead(menu) == 0){//selecteer de optie
          luchtvochtigheid = !(luchtvochtigheid); 
          Serial.println("luchtvochtigheid"+String(luchtvochtigheid));
          delay(500);
        }
        break;
          
        case 2: //druk bar - P
        if(digitalRead(menu) == 0){//selecteer de optie
          druk = !(druk); 
          Serial.println("druk"+String(druk));
          delay(500);
        }
        break;

        case 1: //terug
        if(digitalRead(menu) == 0){//selecteer de optie
          terug = 1; 
          Serial.println("going back");
          delay(500);
        }
        break;
      }
      

    }
  }
}

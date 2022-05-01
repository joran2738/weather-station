//initialise libraries

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <EnvironmentCalculations.h>
#include <BME280I2C.h>
#include <Wire.h>

//initialise names/ports

// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     10
#define TFT_RST    9  // you can also connect this to the Arduino reset // in which case, set this #define pin to 0!
#define TFT_DC     8

#define SerialDebugging true
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// color definitions
const uint16_t  Display_Color_Black        = 0x0000;
const uint16_t  Display_Color_Blue         = 0x001F;
const uint16_t  Display_Color_Red          = 0xF800;
const uint16_t  Display_Color_Green        = 0x07E0;
const uint16_t  Display_Color_Cyan         = 0x07FF;
const uint16_t  Display_Color_Magenta      = 0xF81F;
const uint16_t  Display_Color_Yellow       = 0xFFE0;
const uint16_t  Display_Color_White        = 0xFFFF;

const uint16_t  Display_Color_Orange       = 0xff7a05;
const uint16_t  Display_Color_Grey         = 0x852a6;
const uint16_t  Display_Color_Sun          = 0xffb405;
const uint16_t  Display_Color_Dark_Grey    = 0x7d4b4b;


// The colors we actually want to use
uint16_t        Display_Text_Color         = Display_Color_White;
uint16_t        Display_Backround_Color    = Display_Color_Blue;

// assume the display is off until configured in setup()
bool            isDisplayVisible        = false;

// declare size of working string buffers. Basic strlen("d hh:mm:ss") = 10
const size_t    MaxString               = 16;

//LDR init
int LDRPin = A2;    // select the input pin for the potentiometer
int LDRval = 0;  // variable to store the value coming from the sensor
char overcast[10];

// rainsensor init
int rain_pin = A0;
int rain_val = 0;

//values of reference for icons overcast
int suny = 25,sunx = 25,sun = 16;

//time calculation variables
int sunrise_set_times[4];

//Bme sensor settings
BME280I2C::Settings settings(
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::OSR_X1,
   BME280::Mode_Forced,
   BME280::StandbyTime_1000ms,
   BME280::Filter_16,
   BME280::SpiEnable_False,
   BME280I2C::I2CAddr_0x76
);

BME280I2C bme(settings);
float temp(NAN), hum(NAN), pres(NAN);
float last_temp =0,last_heatindex=0,last_hum=0,last_pres;
BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
BME280::PresUnit presUnit(BME280::PresUnit_hPa);
EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;


/////////////////////////////////////////////////////////////////
//setup                                                        //
/////////////////////////////////////////////////////////////////

void setup() {
  #if (SerialDebugging)
    Serial.begin(115200); while (!Serial); Serial.println();
  #endif
  Serial.print("Hello! ST7735 TFT Test");

  delay(250);

  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

// initialise the display
    tft.setFont();
    tft.fillScreen(Display_Backround_Color);
    tft.setTextColor(Display_Text_Color);
    tft.setTextSize(2);

    // the display is now on
    isDisplayVisible = true;

  Serial.println("Initialized");

  lines();
  
  Wire.begin();

  while(!bme.begin())
  {
    Serial.println("Could not find BME280 sensor!");
    delay(1000);
  }

  switch(bme.chipModel())
  {
     case BME280::ChipModel_BME280:
       Serial.println("Found BME280 sensor! Success.");
       break;
     case BME280::ChipModel_BMP280:
       Serial.println("Found BMP280 sensor! No Humidity available.");
       break;
     default:
       Serial.println("Found UNKNOWN sensor! Error!");
  }
  
} 

/////////////////////////////////////////////////////////////////
// loop                                                        //
/////////////////////////////////////////////////////////////////

void loop() {
  // unconditional display, regardless of whether display is visible
  // read the value from the sensor:
  
  char part_of_day[6];
  LDRval = analogRead(LDRPin);
 
  day_or_night_calc(2022,4,30,10,10,part_of_day,LDRval);
  rain_val = analogRead(rain_pin);
  rain_calc(rain_val);
  bme.read(pres, temp, hum, tempUnit, presUnit);
  print_BME280_data(pres,temp,hum);
  print_date(5,1,10,10);
  delay(1000);
}

/////////////////////////////////////////////////////////////////
// this funtion prints lines around the weather icons          //
/////////////////////////////////////////////////////////////////

void lines(){
  tft.drawLine(55, 0, 55,55, Display_Color_White);
  tft.drawLine(55, 55, 0,55, Display_Color_White);
  
}

/////////////////////////////////////////////////////////////////
//this function calculates whether there's rain or not         //
/////////////////////////////////////////////////////////////////

void rain_calc(int rain){
  if (rain < 900){
    rain_icon();
  }
}

/////////////////////////////////////////////////////////////////
//next functions are used to print icons on the screen         //
/////////////////////////////////////////////////////////////////

void rain_icon(){
  int offset_x = 19;
  int offset_drop = 8;
  int offset_y = 0;
  for (int i = 0; i <= 5; i++) {
    offset_y = random(0, 9);
    tft.drawLine(sunx - offset_x - 1 + (i*offset_drop) , suny + 18 + offset_y, sunx - offset_x - 1 + (i*offset_drop) ,suny + 21 + offset_y, Display_Color_Cyan);
    tft.drawLine(sunx - offset_x + (i*offset_drop)     , suny + 15 + offset_y, sunx - offset_x + (i*offset_drop)     ,suny + 22 + offset_y, Display_Color_Cyan);
    tft.drawLine(sunx - offset_x + 1 +(i*offset_drop)  , suny + 18 + offset_y, sunx - offset_x + 1 +(i*offset_drop)  ,suny + 21 + offset_y, Display_Color_Cyan);
  }
}

void overcast_night(){
  overcast_clear();
  tft.fillCircle(sunx, suny, sun, Display_Color_Grey);
  tft.fillCircle(sunx+5, suny, sun-3, Display_Backround_Color);
}
void overcast_day(){
  overcast_clear();
  tft.fillCircle(sunx, suny, sun, Display_Color_Sun);
}
void overcast_dark(){
  overcast_clear();
  
  tft.fillCircle(sunx+12, suny+11, sun-1, Display_Color_Grey);
  tft.fillCircle(sunx-12, suny+11, sun-5, Display_Color_Grey);
  
 
  tft.fillCircle(sunx+12, suny+11, sun-3, Display_Color_Dark_Grey);
  tft.fillCircle(sunx-9, suny+11, sun-7, Display_Color_Dark_Grey);
  tft.fillCircle(sunx-12, suny+11, sun-7, Display_Color_Dark_Grey);
  tft.fillCircle(sunx-3, suny, sun-5, Display_Color_Grey);
  tft.fillCircle(sunx-3, suny, sun-7, Display_Color_Dark_Grey);
  tft.fillCircle(sunx, suny+4, sun-7, Display_Color_Dark_Grey);
  

  tft.fillCircle(sunx+12, suny+11, sun-10, Display_Color_Grey);
  tft.fillCircle(sunx+14, suny+13, sun-9, Display_Color_Dark_Grey);
}
void overcast_office(){
  overcast_clear();
  
  tft.fillCircle(sunx+12, suny+11, sun-1, Display_Color_Grey);
  tft.fillCircle(sunx-12, suny+11, sun-5, Display_Color_Grey);
  
  
  tft.fillCircle(sunx+12, suny+11, sun-3, Display_Color_White);
  tft.fillCircle(sunx-9, suny+11, sun-7, Display_Color_White);
  tft.fillCircle(sunx-12, suny+11, sun-7, Display_Color_White);
  tft.fillCircle(sunx-3, suny, sun-5, Display_Color_Grey);
  tft.fillCircle(sunx-3, suny, sun-7, Display_Color_White);
  
  tft.fillCircle(sunx, suny+4, sun-7, Display_Color_White);
  

  tft.fillCircle(sunx+12, suny+11, sun-10, Display_Color_Grey);
  tft.fillCircle(sunx+14, suny+13, sun-9, Display_Color_White);
}

void overcast_cloudy(){
  
  tft.fillCircle(sunx+10, suny+10, sun-2, Display_Color_Orange);
  tft.fillCircle(sunx-10, suny+10, sun-6, Display_Color_Orange);
  
  tft.fillCircle(sunx+12, suny+11, sun-1, Display_Color_Grey);
  tft.fillCircle(sunx-12, suny+11, sun-5, Display_Color_Grey);
  
  tft.fillCircle(sunx+12, suny+11, sun-3, Display_Color_White);
  tft.fillCircle(sunx-9, suny+11, sun-7, Display_Color_White);
  tft.fillCircle(sunx-12, suny+11, sun-7, Display_Color_White);

  tft.fillCircle(sunx+12, suny+11, sun-10, Display_Color_Grey);
  tft.fillCircle(sunx+14, suny+13, sun-9, Display_Color_White);
}



void overcast_clear(){

  tft.fillRect(0, 0 , 55, 55, Display_Backround_Color);
  
}


/////////////////////////////////////////////////////////////////
//this function determines what type of overcast it is,        //
//based on the Lightintensity                                  //
/////////////////////////////////////////////////////////////////
void calc_overcast_light(int LDR,int day_or_night){
  float voltage;
  float res;
  float lux;

  //translate analog signal to voltage, to resistance of the LDR, to lightintensity
  voltage = ((float)LDR / 1023) * 5;
  res = (voltage /((5 - voltage)/ 4.610));
  lux = 500 / res;
  Serial.print(lux);
  Serial.println(day_or_night);

  // change lux value to overcast type
  if (lux<4 && day_or_night == 1){
    strcpy(overcast,"dark");
    overcast_dark();
  }
  else if (lux<10  && day_or_night == 1){
    strcpy(overcast,"office");
    overcast_office();
  }
  else if (lux<20  && day_or_night == 1){
    strcpy(overcast,"cloudy");
    overcast_cloudy();
  }
  else if (lux<50  && day_or_night == 1){
  }
  else if (lux<200  && day_or_night == 0){
    strcpy(overcast,"dark");
    overcast_dark();
  }
  else if (lux<1250 && day_or_night == 0){
    strcpy(overcast,"office"); 
    overcast_office();
  }
  else if (lux<2500 && day_or_night == 0){
    strcpy(overcast,"cloudy");
    overcast_cloudy();
  }
  else{
  }
 
}

/////////////////////////////////////////////////////////////////
//this function calculates an estimate of when the             //
//sunrise and sunset are going to take place at a certain date //
//the values are based of off Herent, Vlaams Brabant, Belgium  //
/////////////////////////////////////////////////////////////////
void calculate_sunrise_set(int year,int month, int day){

  int isleap;
  int days[]={31,59,90,120,151,181,212,243,273,304,334,365};
  int dayinyear;
  int sunset,sunrise;

  
  //leapyear calc/////////////////
  if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
    isleap = 1;
  }
  else {
    isleap = 0;
  }
  
  //day of year calc//////////////
  if (month==1){
    dayinyear = day;
  }
  else{
    dayinyear = days[month-2]+day;
    if ((isleap == 1)&&(month > 2)){
      dayinyear += 1;
    }
  }
  
  //sunset and sunrise times calc////////
  sunrise = -0.0000000000074008513*pow(dayinyear,6) + 0.00000000857055539697*pow(dayinyear,5) - 0.00000350552622860315*pow(dayinyear,4) + 0.000597221220245117*pow(dayinyear,3) - 0.0445191176009336*pow(dayinyear,2) + 3.00397223016148*dayinyear + 1046.52019079607;
  sunset = 0.00000000000165142095*pow(dayinyear,6) - 0.0000000010058025554*pow(dayinyear,5) - 0.00000022320945673489*pow(dayinyear,4) + 0.000259060197756204*pow(dayinyear,3) - 0.047401259928898*pow(dayinyear,2) + 0.848715494108752*dayinyear + 576.580902452219;

  sunrise_set_times[1] = sunset%60;
  sunrise_set_times[0] = (sunset-sunrise_set_times[1])/60;

  sunrise_set_times[3]= sunrise%60;
  sunrise_set_times[2]= (sunrise-sunrise_set_times[3])/60;
  
}


/////////////////////////////////////////////////////////////////
//calculate whether it's day or night                          //
/////////////////////////////////////////////////////////////////
void day_or_night_calc(int year, int month, int day, int hour, int minute,char part_of_day[6],int LDRval){

  calculate_sunrise_set(year,month,day);
  
  if ((hour < sunrise_set_times[0] ) || (hour == sunrise_set_times[0] && minute < sunrise_set_times[1])||(hour > sunrise_set_times[2] ) || (hour == sunrise_set_times[2] && minute > sunrise_set_times[3])){
   strcpy(part_of_day,"night");
   overcast_night();
   calc_overcast_light(LDRval,1);
  }
  else{
    strcpy(part_of_day,"day");
    overcast_day();
    calc_overcast_light(LDRval,0);
  }
}

/////////////////////////////////////////////////////////////////
//this function gets the BME280 data on the screen             //
/////////////////////////////////////////////////////////////////
void print_BME280_data(int pres, int temp, int hum){
  int x_placement = 3, y_placement = 60, y_offset = 25;
  int heatindex = EnvironmentCalculations::HeatIndex(temp, hum, envTempUnit);


  print_data(last_pres, last_temp, last_hum, last_heatindex,Display_Backround_Color);


  
  
  print_data(pres, temp, hum, heatindex, Display_Text_Color);

  last_temp = temp;
  last_heatindex = heatindex;
  last_hum=hum;
  last_pres=pres;

}

/////////////////////////////////////////////////////////////////
//this function clears the old BME280 data on the screen       //
/////////////////////////////////////////////////////////////////
void print_data(int pres, int temp, int hum, int heatindex,uint16_t color){
  
  int x_placement = 3, y_placement = 60, y_offset = 25;
  tft.setTextColor(color);
  
  tft.setCursor(x_placement, y_placement);
  tft.print(temp+String("C"));
  tft.setCursor(x_placement, y_placement+y_offset);
  tft.setTextSize(1);
  tft.print("feels like ");
  tft.setTextSize(2);
  tft.print(+heatindex+String("C"));
  tft.setCursor(x_placement, y_placement+y_offset*2);
  tft.print(hum+String("% RH"));
  tft.setCursor(x_placement, y_placement+y_offset*3);
  tft.print(pres+String("hPa"));
}
/////////////////////////////////////////////////////////////////
//this function places the new BME280 data on the screen       //
/////////////////////////////////////////////////////////////////
void print_date(int month,int day,int hour, int minute){
  int x_placement = 58, y_placement = 5, y_offset = 25;
  
  tft.setCursor(x_placement, y_placement);
  tft.print(hour+String(":")+minute);
  tft.setCursor(x_placement, y_placement + y_offset);
  tft.print(month+String("/")+day);

  
}

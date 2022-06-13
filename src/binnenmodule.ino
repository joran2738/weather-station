/////////////////////////////////////////////////////////////////
// debugging modes                                             //
/////////////////////////////////////////////////////////////////
bool debug = true;
bool debugscreen = false;
#define SerialDebugging true

//initialise libraries

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <SPI.h>
#include <LoRa.h>

//initialise names/ports

// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     7
#define TFT_RST    6 // you can also connect this to the Arduino reset // in which case, set this #define pin to 0!
#define TFT_DC     5


Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_RST);

// color definitions
const uint16_t  display_color_black        = 0x0000;
const uint16_t  display_color_blue         = 0x001F;
const uint16_t  display_color_red          = 0xF800;
const uint16_t  display_color_green        = 0x07E0;
const uint16_t  display_color_cyan         = 0x07FF;
const uint16_t  display_color_magenta      = 0xF81F;
const uint16_t  display_color_yellow       = 0xFFE0;
const uint16_t  display_color_white        = 0xFFFF;

const uint16_t  display_color_orange       = 0xff7a05;
const uint16_t  display_color_grey         = 0x852a6;
const uint16_t  display_color_sun          = 0xffb405;
const uint16_t  display_color_dark_grey    = 0x7d4b4b;


// The colors we actually want to use
uint16_t        display_text_color          = display_color_white;
uint16_t        display_background_color    = display_color_blue;

//values of reference for icons overcast
uint8_t suny = 25,sunx = 25,sun = 16;




/////////////////////////////////////////////////////////////////
// initialise RTC variables                                    //
/////////////////////////////////////////////////////////////////
const uint8_t button_pin1 = 2,button_pin2 = 3, button_pin3 = 4;
volatile bool is_button_pressed = false;
bool is_display_visible = false;

unsigned long shutdown_time = 5000, runtime; // shut_down_time is defined in millisec because of the millis() function which imports the time the arduino has been running in millisec
unsigned long millis_start_lcd, vorige_millis = 0; // millis_start is saved in the settings tab, this needs to be imported from the second a date and time have been given --OBE NIET VERGETEN

int year = 2022, month = 5, day = 30, hours = 18, minutes = 35, sec = 0;
int last_month, last_day, last_hour, last_minute;

//time calculation variables
int sunrise_set_times[4];

/////////////////////////////////////////////////////////////////
// initialise Lora and weather variables                       //
/////////////////////////////////////////////////////////////////
int counter = 0;
int hold = 0;

String dataset_string = ""; // this is the string that the LORA module receives

// mi and mifeel resemble whether the temperature is negative or not,
//this is needed because the function to get data out of the string doesn't read negative numbers

//temperature
int mi;
int temp = 0;

//feel temperature
int mifeel;
int heatindex = 0;

//other data
int hum = 0;
float pres = 0;
int lux = 0;
char overcast[10];
int rain = 0;

float last_temp =20,last_heatindex=20,last_hum=50,last_pres = 1016;

int data; // is used for a buffer to get data out of the string

/////////////////////////////////////////////////////////////////
// initialise settings variables                               //
/////////////////////////////////////////////////////////////////
const int menu = 3;
const int up = 4;
const int down = 2;
byte pressed = 0;
byte back = 0;
bool temp_setting = false;
bool pres_setting = false;
int scroll = 0;
bool inst = false; 
/////////////////////////////////////////////////////////////////
// error handling                                              //
/////////////////////////////////////////////////////////////////

bool connection_error = false;
bool bme280_error = false;
bool LoRa_error = false;



/////////////////////////////////////////////////////////////////
//setup                                                        //
/////////////////////////////////////////////////////////////////

void setup() {
  if (SerialDebugging){
    Serial.begin(9600); while (!Serial); Serial.println();
  }
  Serial.print("Hello! ST7735 TFT Test");

  delay(250);

  // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB); // initialize a ST7735S chip, black tab

  // initialise the display
  main_background();

  Serial.println("Initialized screen");
  
  
  Serial.println("LoRa test");
  if (!LoRa.begin(433E6)) {
    //Serial.println("Starting LoRa failed!");
    LoRa_error = true;
  }

  pinMode(menu, INPUT_PULLUP);
  pinMode(up, INPUT_PULLUP);
  pinMode(down, INPUT_PULLUP);
} 

/////////////////////////////////////////////////////////////////
// loop                                                        //
/////////////////////////////////////////////////////////////////

void loop() {
  // unconditional display, regardless of whether display is visible
  // read the value from the sensor:
  char part_of_day[6];
  hold = request();
  settings();
  if (hold != 0){
    listen();
    get_data_out();
    day_or_night_calc(part_of_day);
    rain_calc();
    if (!debugscreen){
      print_BME280_data(pres,temp,hum);
      modify_date();
    }
    error_handling();
  }
  time_and_day_upcounter();
  
  delay(2000);
}

//////////////////////////////////////////////////////////////////////////////////////////
// function which keeps track of real time based upon a set time imported from settings //
//////////////////////////////////////////////////////////////////////////////////////////
void time_and_day_upcounter (){
  int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int leap[] = {31,29,31,30,31,30,31,31,30,31,30,31};
  if (sec >= 59) {
    sec = 0;
    minutes++;
    if (minutes >= 60) {
      minutes = 0;
      hours++;
      if (hours >= 24) {
        hours = 0;
        day++;
        if ((year % 4 == 0 && year % 100 != 0) || year % 400 == 0) {
          if (leap[month - 1] < day) {
            day = 1;
            month++;
          }
        }
        else {
          if (days_in_month[month - 1] < day) {
            day = 1;
            month++;
          }
        }
      }
    }
    if (month >= 13 ) {
      month = 1;
      year++;
    }
  }
  else if((millis()-vorige_millis)>=1000) {
    //Serial.println((millis()-vorige_millis));
    sec +=((millis()-vorige_millis)/1000);
  }
  vorige_millis = millis();
  Serial.println(String(year)+"-"+String(month)+"-"+String(day)+" "+String(hours)+":"+String(minutes));
}

/////////////////////////////////////////////////////////////////
// this function requests data from the outside module         //
// it returns 1 so that the loop will only listen for data till//
// it has recieved it                                          //
/////////////////////////////////////////////////////////////////
int request(){
  Serial.print("Sending packet: ");
  Serial.println(counter);

  // send packet
  if (!debug){
    LoRa.beginPacket();
    LoRa.print("hello ");
    LoRa.print(counter);
    LoRa.endPacket();
  }
  counter++;
  return 1;
}

/////////////////////////////////////////////////////////////////
// this function waits for the data returned from the outside  //
// module                                                      //
/////////////////////////////////////////////////////////////////
void listen(){
  int listen_stop = 0, try_stop = 0;
  dataset_string = "";
  
  // try to parse a packet
  while (try_stop < 5){
    while (hold && listen_stop < 30){
      //Serial.println("listening...");
      int packetSize =0; //LoRa.parsePacket(); 
      if (packetSize) {
        // received a packet
        //Serial.print("Received packet ");
    
        // read packet
        while (LoRa.available()) {
          dataset_string += (char)LoRa.read();
        }
        connection_error = false;
        //Serial.println(dataset_string);
        //Serial.println(dataset_string.length());
        // print RSSI of packet
        //Serial.print(" with RSSI ");
        //Serial.println(LoRa.packetRssi());
        
        hold = 0;
      }
      listen_stop++;
    }
    if (listen_stop == 30){
      Serial.println("retrieved nothing");
      settings();
      request();
    }
    listen_stop = 0;
    try_stop++;
  }
  if (try_stop == 5){
    connection_error = true;
  }
  if (debug){
    hold = 0;
    dataset_string = "3,24,2,16,55,1016,3000,2";
  }
}

/////////////////////////////////////////////////////////////////
// this functions gets the data out of the string returned from//
// the outside module                                          //
/////////////////////////////////////////////////////////////////
void get_data_out(){
  uint8_t count = 0;
  char data_array[40];
  dataset_string.toCharArray(data_array,40);

  data = atof(strtok(data_array,","));
  mi = data;
  //Serial.print(String(mi)+String(count));

  while(data != NULL){

    data = atof(strtok(NULL, ","));
    //Serial.println("data"+String(data)); 

    if (count == 0){
      temp = data;
      //Serial.println(String(temp)+";"+String(count));
      count += 1;
    }

    else if (count == 1){
      mifeel = data;
      //Serial.println(String(mifeel)+";"+String(count));
      count += 1;
    }

    else if (count == 2){
      heatindex = data;
      //Serial.println(String(heatindex)+";"+String(count));
      count += 1;
    }

    else if (count == 3){
      hum = data;
      //Serial.println(String(hum)+";"+String(count));
      count += 1;
    }

    else if (count == 4){
      pres = data;
      //Serial.println(String(pres)+";"+String(count));
      count += 1;
    }

    else if (count == 5){
      lux = data;
      //Serial.println(String(lux)+";"+String(count));
      count += 1;
    }

    else if (count == 6){
      rain = data;
      //Serial.println(String(rain)+";"+String(count));
      count += 1;
    }
  }

  //Serial.println("mi"+String(mi));
  if (mi == 3){
    bme280_error = true;
  }
  else{
    bme280_error = false;
    temp = (temp*(((mi-1)*2)-1));
    Serial.println("temp :"+String(temp));
    //Serial.println("mifeel"+String(mifeel));
    heatindex = (heatindex*(((mifeel-1)*2)-1));
    Serial.println("feel :"+String(heatindex));
    Serial.println("hum :"+String(hum));
    Serial.println("pres :"+String(pres));
  }
  if (temp_setting){
    temp = (temp * 1.8) + 32;
    heatindex = (heatindex * 1.8) + 32;
  }
  if (pres_setting){
    pres = pres /1000;
  }
  
  
  Serial.println("lux :"+String(lux));
  Serial.println("rain :"+String(rain));
  
  //Serial.println(String(rain)+String(counter));
  
  //Serial.println(String(rain==counter));
}

/////////////////////////////////////////////////////////////////
// this funtion prints lines around the weather icons          //
/////////////////////////////////////////////////////////////////
void lines(){
  tft.drawLine(55, 0, 55,55, display_color_white);
  tft.drawLine(55, 55, 0,55, display_color_white);

}

/////////////////////////////////////////////////////////////////
//this function calculates whether there's rain or not         //
/////////////////////////////////////////////////////////////////
void rain_calc(){
  if (rain == 2){
    if (!debugscreen){
      rain_icon();
    }
    else{
      Serial.println("it's raining");
    }
  }
  else if (rain == 1){
  }
}

/////////////////////////////////////////////////////////////////
//next functions are used to print icons on the screen         //
/////////////////////////////////////////////////////////////////

void rain_icon(){
  uint8_t offset_x = 19;
  uint8_t offset_drop = 8;
  uint8_t offset_y = 0;
  for (int i = 0; i <= 5; i++) {
    offset_y = random(0, 9);
    tft.drawLine(sunx - offset_x - 1 + (i*offset_drop) , suny + 18 + offset_y, sunx - offset_x - 1 + (i*offset_drop) ,suny + 21 + offset_y, display_color_cyan);
    tft.drawLine(sunx - offset_x + (i*offset_drop)     , suny + 15 + offset_y, sunx - offset_x + (i*offset_drop)     ,suny + 22 + offset_y, display_color_cyan);
    tft.drawLine(sunx - offset_x + 1 +(i*offset_drop)  , suny + 18 + offset_y, sunx - offset_x + 1 +(i*offset_drop)  ,suny + 21 + offset_y, display_color_cyan);
  }
}

void overcast_night(){
  if (!debugscreen){
    overcast_clear();
    tft.fillCircle(sunx, suny, sun, display_color_grey);
    tft.fillCircle(sunx+5, suny, sun-3, display_background_color);
  }
  else{
    Serial.println("it's night");
  }
}
void overcast_day(){
  if (!debugscreen){
    overcast_clear();
    tft.fillCircle(sunx, suny, sun, display_color_sun);
  }
  else{
    Serial.println("it's day");
  }
}
void overcast_dark_and_office(uint16_t color){
  overcast_clear();

  tft.fillCircle(sunx+12, suny+11, sun-1, display_color_grey);
  tft.fillCircle(sunx-12, suny+11, sun-5, display_color_grey);


  tft.fillCircle(sunx+12, suny+11, sun-3, color);
  tft.fillCircle(sunx-9, suny+11, sun-7, color);
  tft.fillCircle(sunx-12, suny+11, sun-7, color);
  tft.fillCircle(sunx-3, suny, sun-5, display_color_grey);
  tft.fillCircle(sunx-3, suny, sun-7, color);

  tft.fillCircle(sunx, suny+4, sun-7, color);

  tft.fillCircle(sunx+12, suny+11, sun-10, display_color_grey);
  tft.fillCircle(sunx+14, suny+13, sun-9, color);
}

void overcast_cloudy(){

  tft.fillCircle(sunx+10, suny+10, sun-2, display_color_orange);
  tft.fillCircle(sunx-10, suny+10, sun-6, display_color_orange);

  tft.fillCircle(sunx+12, suny+11, sun-1, display_color_grey);
  tft.fillCircle(sunx-12, suny+11, sun-5, display_color_grey);

  tft.fillCircle(sunx+12, suny+11, sun-3, display_color_white);
  tft.fillCircle(sunx-9, suny+11, sun-7, display_color_white);
  tft.fillCircle(sunx-12, suny+11, sun-7, display_color_white);

  tft.fillCircle(sunx+12, suny+11, sun-10, display_color_grey);
  tft.fillCircle(sunx+14, suny+13, sun-9, display_color_white);
}



void overcast_clear(){

  tft.fillRect(0, 0 , 55, 55, display_background_color);
  
}


/////////////////////////////////////////////////////////////////
//this function determines what type of overcast it is,        //
//based on the Lightintensity                                  //
/////////////////////////////////////////////////////////////////
void calc_overcast_light(int day_or_night){


  // change lux value to overcast type
  if (lux<4 && day_or_night == 1){
    strcpy(overcast,"dark");
    if (!debugscreen){
      overcast_dark_and_office(display_color_dark_grey);
    }
    else{
      Serial.println("overcast: "+String(overcast));
    }
  }
  else if (lux<10  && day_or_night == 1){
    strcpy(overcast,"office");
    if (!debugscreen){
      overcast_dark_and_office(display_color_white);
    }
    else{
      Serial.println("overcast: "+String(overcast));
    }
  }
  else if (lux<20 && day_or_night == 1){
    strcpy(overcast,"cloudy");
    if (!debugscreen){
      overcast_cloudy();
    }
    else{
      Serial.println("overcast: "+String(overcast));
    }
  }
  else if (lux<50 && day_or_night == 1){
  }
  else if (lux<200 && day_or_night == 0){
    strcpy(overcast,"dark");
    if (!debugscreen){
      overcast_dark_and_office(display_color_dark_grey);
    }
    else{
      Serial.println("overcast: "+String(overcast));
    }
  }
  else if (lux<1250 && day_or_night == 0){
    strcpy(overcast,"office"); 
    if (!debugscreen){
      overcast_dark_and_office(display_color_white);
    }
    else{
      Serial.println("overcast: "+String(overcast));
    }
  }
  else if (lux<2500 && day_or_night == 0){
    strcpy(overcast,"cloudy");
    if (!debugscreen){
      overcast_cloudy();
    }
    else{
      Serial.println("overcast: "+String(overcast));
    }
  }
  else{
  }
 
}

/////////////////////////////////////////////////////////////////
//this function calculates an estimate of when the             //
//sunrise and sunset are going to take place at a certain date //
//the values are based of off Herent, Vlaams Brabant, Belgium  //
/////////////////////////////////////////////////////////////////
void calculate_sunrise_set(){

  uint8_t isleap;
  uint16_t days[]={31,59,90,120,151,181,212,243,273,304,334,365};
  uint16_t dayinyear;
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
void day_or_night_calc(char part_of_day[6]){

  calculate_sunrise_set();
  
  if ((hours < sunrise_set_times[0] ) || (hours == sunrise_set_times[0] && minutes < sunrise_set_times[1])||(hours > sunrise_set_times[2] ) || (hours == sunrise_set_times[2] && minutes > sunrise_set_times[3])){
    strcpy(part_of_day,"night");
    overcast_night();
    calc_overcast_light(1);
  }
  else{
    strcpy(part_of_day,"day");
    overcast_day();
    calc_overcast_light(0);
  }
}

/////////////////////////////////////////////////////////////////
//this function gets the BME280 data on the screen             //
/////////////////////////////////////////////////////////////////
void print_BME280_data(float pres, int temp, int hum){
  int x_placement = 3, y_placement = 60, y_offset = 25;

  print_data(last_pres, last_temp, last_hum, last_heatindex,display_background_color);
  if (connection_error || bme280_error || LoRa_error){
    display_text_color = display_color_red ;
  }

  print_data(pres, temp , hum, heatindex, display_text_color);
  display_text_color = display_color_white;

  last_temp = temp;
  last_heatindex = heatindex;
  last_hum=hum;
  last_pres=pres;

}

/////////////////////////////////////////////////////////////////
//this function clears the old and puts the new BME280 data    //
//on the screen                                                //
/////////////////////////////////////////////////////////////////
void print_data(float pres, int temp, int hum, int heatindex,uint16_t color){

  int x_placement = 3, y_placement = 60, y_offset = 25;
  tft.setTextColor(color);

  tft.setCursor(x_placement, y_placement);
  tft.print(temp);
  if(temp_setting){
    tft.print("F");
  }
  else{
    tft.print("C");
  }
  tft.setCursor(x_placement, y_placement+y_offset);
  tft.setTextSize(1);
  tft.print("feels like ");
  tft.print(heatindex);
  if(temp_setting){
    tft.print("F");
  }
  else{
    tft.print("C");
  }
  tft.setTextSize(2);
  tft.setCursor(x_placement, y_placement+y_offset*2);
  tft.print(hum+String(" %RH"));
  tft.setCursor(x_placement, y_placement+y_offset*3);

  if (pres_setting){
    tft.print(pres);
    tft.print("bar");
  }
  else{
    tft.print((int)pres);
    tft.print("hPa");
  }
}
/////////////////////////////////////////////////////////////////
//this function gets the time and date on the screen           //
/////////////////////////////////////////////////////////////////
void modify_date(){

  print_date(last_month, last_day, last_hour, last_minute, display_background_color);
  print_date(month, day, hours, minutes, display_text_color);

  last_month = month;
  last_day = day;
  last_hour = hours;
  last_minute = minutes;
}

/////////////////////////////////////////////////////////////////
//this function clears the old and puts the new time and date  //
//on the screen                                                //
/////////////////////////////////////////////////////////////////
void print_date(int month,int day,int hour, int minute,uint16_t color){

  int x_placement = 58, y_placement = 5, y_offset = 25;
  tft.setTextColor(color);

  tft.setCursor(x_placement, y_placement);
  if (hour < 10){
    tft.print("0");
  }
  tft.print(hour+String(":"));
  if (minute < 10){
    tft.print("0");
  }
  tft.print(minute);

  tft.setCursor(x_placement, y_placement + y_offset);
  tft.print(month+String("/")+day); 
}

/////////////////////////////////////////////////////////////////
// error handling                                              //
/////////////////////////////////////////////////////////////////
void error_handling(){
  Serial.println(" ");
  if (LoRa_error){
    Serial.println("LoRa module not found");
  }
  else if (connection_error){
    Serial.println("connection error");
  }
  else{
    if(bme280_error){
      Serial.println("temp, hum and pres sensor not found");
    }
  }

}

/////////////////////////////////////////////////////////////////
// this function sets up the main screen                       //
/////////////////////////////////////////////////////////////////
void main_background(){
  tft.setFont();
  tft.fillScreen(display_background_color);
  tft.setTextColor(display_text_color);
  tft.setTextSize(2);
  if (!debugscreen){
    lines();
  }
}

void settings_background(){
  int beginning = 25;
  int offset=25;

  tft.setFont();
  tft.fillScreen(display_color_grey);
  tft.setTextColor(display_color_black);
  tft.setTextSize(2);

  for (int i = 0;i<3;i++){
    tft.drawLine(0, beginning + i*offset, 127,beginning + i*offset, display_color_black);
    tft.setCursor(5,5 + i* offset);
    switch(i){
      case 0: tft.print("temp:");break;
      case 1: tft.print("pres:");break;
      case 2: tft.print("return:");break;
    }
  }
}
void select(int y){
  for (int x = 0;x < 3; x++){
    tft.fillRect(77,(x*25)+1,127,24,display_color_grey);
  }
  tft.fillRect(122,(y*25)+1,127,24,display_color_sun);
}
void choice(){
  int offset=25;
  for (int i = 0;i<2;i++){
    tft.setCursor(78,5 + i* offset);
    switch(i){
      case 0: 
      if(temp_setting){
        tft.print("F");
      }
      else{
        tft.print("C");
      }
      break;
      case 1: 
      if (pres_setting){
        tft.print("bar");
      }
      else{
        tft.print("hPa");
      }
      break;
    }
  } 
}
void settings_errors(){
  int beginning = 80;
  int offset=15;
  int now;

  tft.setTextSize(1);

  for (int i = 0;i<2;i++){
    if (i == 0){
      tft.setCursor(5,beginning);
      tft.print("errors:");
      now = beginning + offset;
    }
    else if (LoRa_error){
      tft.setCursor(5,now);
      tft.print("LoRa not found");
      now = beginning + offset;
    }
    else if (connection_error){
      tft.setCursor(5,now);
      tft.print("connection error");
      now = beginning + offset;
    }
    else{
      if(bme280_error){
        tft.setCursor(5,now);
        tft.print("bme280 not found");
        now = beginning + offset;
      }
    }
  }

  tft.setTextSize(2);
}
void settings(){
  scroll = 0;
  pressed = digitalRead(menu);
  inst = menu;
  if (pressed == LOW) { //start instellingen
    settings_background();
    select(scroll);
    choice();
    settings_errors();
    Serial.println("entered settings");
    delay(150);
    back = 0;
    while(back == 0) {
      if(digitalRead(up) == 0){ //zet de select zone 1 omhoog
        if(scroll > 0){
          scroll = scroll - 1;
          Serial.println(scroll);
          select(scroll);
          choice();
          delay(500); //om de knoppen te debouncen
        }
      }
      if(digitalRead(down) == 0){ //zet de select zone 1 down
        if(scroll < 2){
          scroll = scroll + 1;
          Serial.println(scroll);
          select(scroll);
          choice();
          delay(500);//om de knoppen te debouncen
        }
      }


      switch(scroll){

        case 0: //temperatuur °C - °F
        if(digitalRead(menu) == 0) { //selecteer de optie
          temp_setting = !(temp_setting); 
          Serial.println("temp"+String(temp_setting));
          select(scroll);
          choice();
          delay(500);
        }
        break;
        case 1: //druk bar - P
        if(digitalRead(menu) == 0) { //selecteer de optie
          pres_setting = !(pres_setting); 
          Serial.println("druk"+String(pres_setting));
          select(scroll);
          choice();
          delay(500);
        }
        break;

        case 2: //terug
        if(digitalRead(menu) == 0) { //selecteer de optie
          back = 1; 
          Serial.println("going back");
          main_background();
          delay(500);
        }
        break;
      }
    }
  }
}

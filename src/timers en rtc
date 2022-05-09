const uint8_t button_pin1 = ?,button_pin2 = ?, button_pin3 = ?, button_pin4 =  ?; // assign pin numbers hasn't been done yet
volatile bool is_button_pressed = false;
bool is_display_visible = false;
uint16_t shutdown_time = 60000, runtime;  // shut_down_time is defined in millisec because of the millis() function which imports the time the arduino has been running in millisec
unsigned long millis_start, millis_start_lcd; // millis_start is saved in the settings tab, this needs to be imported from the second a date and time have been given --OBE NIET VERGETEN

/////////////////////////////////////////////////////
// ISR used to detect if a button has been pressed //
/////////////////////////////////////////////////////
void sense_button_pressed () {
  if (!is_button_pressed) {
    is_button_pressed = true;
    millis_start_lcd = millis();
    is_display_visible = true;
  }
}

///////////////////////////////////////////////////////////////////////////////
// timer which turns the lcd off after not interacting with lcd for set time //
///////////////////////////////////////////////////////////////////////////////
void timer_lcd_off (uint16_t millis_start, u ){
  if ((millis_start_lcd + shutdown_time) > runtime) { //check if the lcd's runtime is greater than the shutdown time added to the lcd start time
    runtime = millis(); //put the arduino's uptime in run_time
  }
  else {
    is_display_visible = false; //disable display 
  }
}
//////////////////////////////////////////////////////////////////////////////////////////
// function which keeps track of real time based upon a set time imported from settings //
//////////////////////////////////////////////////////////////////////////////////////////
void time_and_day_upcounter (int year, int month, int day, int minutes, int sec){
  int days_in_month[] = {31,28,31,30,31,30,31,31,30,31,30,31};
  int leap[] = {31,29,31,30,31,30,31,31,30,31,30,31};
  if ((sec + (millis() - millis_start)) >= 60) {
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
    if (month >= 13 ) {
      month = 0;
      year++;
    }
    }
  }
  else {
    sec = (sec + (millis() - millis_start);
  }
}



void setup() {
  // put your setup code here, to run once:
  pinMode(button_pin1,INPUT_PULLUP);
  pinMode(button_pin2,INPUT_PULLUP);
  pinMode(button_pin3,INPUT_PULLUP);
  pinMode(button_pin4,INPUT_PULLUP);
  attachInterrupt(digitalPinToInterrupt(button_pin1), sense_button_pressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_pin2), sense_button_pressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_pin3), sense_button_pressed, FALLING);
  attachInterrupt(digitalPinToInterrupt(button_pin4), sense_button_pressed, FALLING);
  is_button_pressed = false; // Assure is_button_pressed is set to false at power on reset
}


void loop() {
  time_and_day_upcounter(int year, int month, int day, int minutes, int sec);
  timer_lcd_off();
}

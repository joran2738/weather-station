// include libraries
#include <SPI.h>
#include <LoRa.h>
#include <EnvironmentCalculations.h>
#include <BME280I2C.h>
#include <Wire.h>

#define SERIAL_BAUD 115200 //define serial baud rate

// define bme settings and variables
BME280I2C bme;    // Default : forced mode, standby time = 1000 ms
                  // Oversampling = pressure ×1, temperature ×1, humidity ×1, filter off,
float temp(NAN), hum(NAN), pres(NAN);
float heatindex;

//define rain variables
int rain_pin = A0;    // select the input pin for the rain sensor
int rain_value = 0;  // variable to store the value coming from the sensor

//define LDR variables
int LDR_pin = A2;    // select the input pin for the LDR
uint16_t LDR_value = 0;  // variable to store the value coming from the sensor
int lux;

String dataset_string = ""; // this is the string that gets send back to the inside module

//////////////////////////////////////////////////////////////////
// setup                                                        //
//////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  while (!Serial);
  
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

  Serial.println("LoRa Receiver");
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);

  
  }
}

//////////////////////////////////////////////////////////////////
// loop                                                         //
//////////////////////////////////////////////////////////////////
void loop() {
  int hold = 0;
  hold = wait();
  if (hold == 1){
    dataset_string = "";
    read_bme();
    read_rain();
    read_LDR();
    to_string();
    reply();
  }
  
}

//////////////////////////////////////////////////////////////////
// this function waits for a request from the inside module     //
//////////////////////////////////////////////////////////////////
int wait(){
  // try to parse packet
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    // received a packet
    Serial.print("Received packet '");

    // read packet
    while (LoRa.available()) {
      Serial.print((char)LoRa.read());
    }

    // print RSSI of packet
    Serial.print("' with RSSI ");
    Serial.println(LoRa.packetRssi());
    return 1;
  }
  return 0;
}

//////////////////////////////////////////////////////////////////
// this function sends the data back to the inside module       //
//////////////////////////////////////////////////////////////////
void reply(){
  Serial.print("replying: ");
  Serial.println(dataset_string);

  // send packet
  LoRa.beginPacket();
  LoRa.print(dataset_string);
  LoRa.endPacket();
}

//////////////////////////////////////////////////////////////////
// this function reads the data from the BME280 sensor          //
//////////////////////////////////////////////////////////////////
void read_bme() {

   BME280::TempUnit tempUnit(BME280::TempUnit_Celsius);
   BME280::PresUnit presUnit(BME280::PresUnit_hPa);
   EnvironmentCalculations::TempUnit     envTempUnit =  EnvironmentCalculations::TempUnit_Celsius;

   bme.read(pres, temp, hum, tempUnit, presUnit);

   heatindex = EnvironmentCalculations::HeatIndex(temp, hum, envTempUnit);

}

//////////////////////////////////////////////////////////////////
// this function reads the value from the rain sensor           //
//////////////////////////////////////////////////////////////////
void read_rain(){
  rain_value = analogRead(rain_pin);
  //Serial.println(rain_value);
  
}

//////////////////////////////////////////////////////////////////
// this function reads the value from the LDR voltage devider   //
//////////////////////////////////////////////////////////////////
void read_LDR(){
  float voltage;
  float res;
  
  LDR_value = analogRead(LDR_pin);
  
  //translate analog signal to voltage, to resistance of the LDR, to lightintensity
  voltage = ((float)LDR_value / 1023) * 5;
  res = (voltage /((5 - voltage)/ 4.610));
  lux = 500 / res;
  //Serial.print(lux);
  
}
//////////////////////////////////////////////////////////////////
// this function puts all the data together in a string to send //
// to the inside module                                         //
// format: "sign,temperature,sign,feelstemperature,humidity,    //
// pressure,lightintensity,rain"                                //
//////////////////////////////////////////////////////////////////
void to_string(){

  if (temp<0){
    dataset_string += "1,";
  }
  else{
    dataset_string += "2,";
  }
  dataset_string += String(int(temp));
  dataset_string += ",";
  if (heatindex<0){
    dataset_string += "1,";
  }
  else{
    dataset_string += "2,";
  }
  dataset_string += String(int(heatindex));
  dataset_string += ",";

  dataset_string += String(int(hum));
  dataset_string += ",";
  
  dataset_string += String(int(pres));
  dataset_string += ",";
  
  dataset_string += String(lux);
  dataset_string += ",";

  if (rain_value < 950){
    dataset_string += "1,";
  }
  else{
    dataset_string += "2,";
  }
  
}

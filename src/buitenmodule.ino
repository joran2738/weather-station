//initialise libraries
#include <SPI.h>
#include <LoRa.h>

//initialise variables
int counter = 0;
int hold = 0;

String dataset_string = ""; // this is the string that the LORA module receives

// mi and mifeel resemble whether the temperature is negative or not,
//this is needed because the function to get data out of the string doesn't read negative numbers

//temperature
int mi;  
int tempe = 0;

//feel temperature
int mifeel;
int feeling = 0;

//other data
int humi = 0;
int presu = 0;
int light = 0;
int rain = 0;

int data;   // is used for a buffer to get data out of the string

//debugging mode
bool debug = false;

/////////////////////////////////////////////////////////////////
//setup                                                        //
/////////////////////////////////////////////////////////////////
void setup() {
  Serial.begin(9600);
  while (!Serial);

  Serial.println("LoRa Sender");

  if (!LoRa.begin(433E6) && !debug) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
}

/////////////////////////////////////////////////////////////////
// loop                                                        //
/////////////////////////////////////////////////////////////////
void loop() {
  
  hold = request();
  if (hold != 0){
    listen();
    get_data_out();
  }
  
  delay(2000);
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
  
  dataset_string = "";
  
  // try to parse a packet
  while (hold && !debug){
    Serial.println("listening...");
    int packetSize = LoRa.parsePacket(); 
    if (packetSize) {
      // received a packet
      //Serial.print("Received packet '");
  
      // read packet
      while (LoRa.available()) {
        dataset_string += (char)LoRa.read();
      }
      
      //Serial.println(dataset_string);
      //Serial.println(dataset_string.length());
      // print RSSI of packet
      //Serial.print("' with RSSI ");
      //Serial.println(LoRa.packetRssi());
      
      hold = 0;
    }
  }
  if (debug){
    dataset_string = "1,24,1,24,1016,55,3000,920";
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
      tempe = data;
      //Serial.println(String(tempe)+";"+String(count));
      count += 1;
    }
    
    else if (count == 1){
      mifeel = data;
      //Serial.println(String(mifeel)+";"+String(count));
      count += 1;
    }
    else if (count == 2){
      feeling = data;
      //Serial.println(String(feeling)+";"+String(count));
      count += 1;
    }
    else if (count == 3){
      humi = data;
      //Serial.println(String(humi)+";"+String(count));
      count += 1;
    }
    else if (count == 4){
      presu = data;
      //Serial.println(String(presu)+";"+String(count));
      count += 1;
    }
    else if (count == 5){
      light = data;
      //Serial.println(String(light)+";"+String(count));
      count += 1;
    }
    else if (count == 6){
      rain = data;
      //Serial.println(String(rain)+";"+String(count));
      count += 1;
    }  
  }
  
  //Serial.println("mi"+String(mi));
  Serial.println("temp :"+String(tempe*(((mi-1)*2)-1)));
  //Serial.println("mifeel"+String(mifeel));
  Serial.println("feel :"+String(feeling*(((mifeel-1)*2)-1)));
  Serial.println("hum :"+String(humi));
  Serial.println("pres :"+String(presu));
  Serial.println("lux :"+String(light));
  Serial.println("rain :"+String(rain));
  
  //Serial.println(String(rain)+String(counter));
  
  //Serial.println(String(rain==counter));
  
}

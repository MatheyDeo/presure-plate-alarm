//----------------------------------------Sender Code
#include <esp_now.h>
#include <WiFi.h>
#include "HX711.h"

HX711 scale;

uint8_t dataPin = 18; //pins of the Hx711
uint8_t clockPin = 19; //pins of the Hx711
uint8_t buttonPin = 20; //pin for the LOCK button
uint8_t buzzerPin = 21; // pin for the onboard buzzer

volatile float load;
volatile float referenceLoad; //load used for determining object manipulation.
bool locked = false;
bool alert = false;
bool repeat = false; 


// Change these to mac adresses of your esp's
uint8_t alarmAddress[] =     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};// Adress of the alarm device
uint8_t controllerAddress[] ={0x00, 0x00, 0x00, 0x00, 0x00, 0x00};// Adress of the controller
uint8_t scaleAddress[] =     {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};// Adress of the scale/pressure plate device
uint8_t broadcastAddress[] = {0x00, 0x0C, 0x00, 0x00, 0x00, 0x00};// DO NOT CHANGE

String send_rnd_val_1;
String send_rnd_val_2;

String receive_rnd_val_1;
String receive_rnd_val_2;
//----------------------------------------Structure of the data data must match the receiver structure
typedef struct struct_message {
    String rnd_1;
    String rnd_2;
} struct_message;

struct_message receive_Data; // Create a struct_message to receive data.
struct_message send_Data; // Create a struct_message to send data.

//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Callback when data is received
void OnDataRecv(const esp_now_recv_info_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&receive_Data, incomingData, sizeof(receive_Data));
  Serial.print("=========Received Data! ( ");
  Serial.print(len);
  Serial.println(" Bytes)");

  receive_rnd_val_1 = receive_Data.rnd_1;
  receive_rnd_val_2 = receive_Data.rnd_2;
  Serial.print("Data: [");
  Serial.print(receive_rnd_val_1);
  Serial.print("] - [");
  Serial.print(receive_rnd_val_2);
  Serial.println("]");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.println(status == ESP_NOW_SEND_SUCCESS ? "\r\nLast packet was sent corectly." : "\r\nLast packet was NOT sent corectly.");
  Serial.println(">>>>>");
}
//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ Send Data
void SendData(String code, String data, String destination){
  esp_err_t result;
  send_Data.rnd_1 = code;
  send_Data.rnd_2 = data;

  //----------------------------------------Send message via ESP-NOW
  if (destination == "controller") {result = esp_now_send(controllerAddress, (uint8_t *) &send_Data, sizeof(send_Data));}
  else if (destination == "alarm") {result = esp_now_send(alarmAddress, (uint8_t *) &send_Data, sizeof(send_Data));}
  else if (destination == "all") {result = esp_now_send(broadcastAddress, (uint8_t *) &send_Data, sizeof(send_Data));}

  if (result == ESP_OK) { Serial.println("Sent with success");}
  else { Serial.println("Error sending the data");}

}

void setup() { //-------------------------------------- VOID SETUP

  pinMode(buttonPin, INPUT);  // sets lock button pin as input
  pinMode(buzzerPin, OUTPUT); // Set buzzer pin as an output
  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Set device as a Wi-Fi Station.

  scale.begin(dataPin, clockPin);
  scale.set_scale(420.0983);       // TODO you need to calibrate this to your Exact load cell if you want any usable readings
  scale.tare();    // Calibrate the Scale to zero. 
  

  // Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  
  esp_now_register_send_cb(OnDataSent); // Register for Callback receiveing to check if packet was delivered succesfully
  esp_now_register_recv_cb(OnDataRecv); // Register for a callback function that will be called when data is received
  
  //Register peers & Add peers
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  memcpy(peerInfo.peer_addr, controllerAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer Controller");
    return;
  }
  memcpy(peerInfo.peer_addr, alarmAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer alarm");
    return;
  }
    memcpy(peerInfo.peer_addr, broadcastAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer alarm");
    return;
  }
}


//VOID LOOP
void loop() {

  load = scale.get_units(5);
  Serial.print(load);

  //---------receiving
   if (receive_rnd_val_1 == "G33") { // silence the alarm
r :-)
te & lock
    referenceLoad = load;
    locked = true;
    alert = false;
    tone(buzzerPin, 1000); delay(50);       
    noTone(buzzerPin);  delay(50);       
    tone(buzzerPin, 1000); delay(50);       
    noTone(buzzerPin);
   }

  if (receive_rnd_val_1 == "G31") { // unlock
      alert = false;
      locked = false;
    tone(buzzerPin, 523); delay(50);       
    noTone(buzzerPin);  delay(50);       
    tone(buzzerPin, 294); delay(50);       
    noTone(buzzerPin);
  }
      if (receive_rnd_val_1 == "G34") { // scream
      alert = true;
      locked = true;
  }


  if(digitalRead(buttonPin)&& !locked) {
    referenceLoad = scale.get_units(5);
    locked = true;
    tone(buzzerPin, 1000); delay(50);       
    noTone(buzzerPin);  delay(50);       
    tone(buzzerPin, 1000); delay(50);       
    noTone(buzzerPin);  
  }
  if((locked && (abs(load - referenceLoad) > 3)) || alert) { 
    if(repeat) {tone(buzzerPin, 2000);}
    else {tone(buzzerPin, 1000);} 
    repeat = !repeat;
    alert = true;}
  else { 
    noTone(buzzerPin); 
    alert = false;
  }

  if (!alert&& locked)  {SendData("S11", String(referenceLoad-load),"all");}
  if (!alert&& !locked) {SendData("S10", String(load),"all");}
  else if (alert)       {SendData("S12", String("1"),"all");}
  
  delay(500);
}
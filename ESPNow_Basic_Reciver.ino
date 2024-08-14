// Code for the Alarm unit

#include <esp_now.h>
#include <WiFi.h>



const int buzzer = 18; // Alarm pin, should be amplified before entering the speaker
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


// Must match the sender structure
typedef struct struct_message {
    String rnd_1;
    String rnd_2;
} struct_message;

struct_message receive_Data; // Create a struct_message to receive data.
struct_message send_Data; // Create a struct_message to send data.

// Callback when data is received
void OnDataRecv(const esp_now_recv_info_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&receive_Data, incomingData, sizeof(receive_Data));
  Serial.println("");
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
void SendData(int data1, int data2) { //data1 = alert | data2 = load
  send_Data.rnd_1 = data1;
  send_Data.rnd_2 = data2;

  Serial.print(">>>>> ");
  Serial.println("Send data");

  //----------------------------------------Send message via ESP-NOW
  esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &send_Data, sizeof(send_Data));

  if (result == ESP_OK) { Serial.println("Sent with success");}
  else { Serial.println("Error sending the data");}
}


//++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ VOID SETUP
void setup() {
  pinMode(buzzer, OUTPUT); // Set buzzer - pin 9 as an output

  Serial.begin(115200);
  WiFi.mode(WIFI_STA); //--> Set device as a Wi-Fi Station

  //----------------------------------------Init ESP-NOW
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
    Serial.println("Failed to add peer controller");
    return;
  }
  memcpy(peerInfo.peer_addr, alarmAddress, 6);
  if (esp_now_add_peer(&peerInfo) != ESP_OK){
    Serial.println("Failed to add peer alarm");
    return;
  }
}

void loop() {

  if (receive_rnd_val_1 == "S12") {
    if(repeat) { tone(buzzer, 1000);}
    else       { tone(buzzer, 500);}
    repeat = !repeat;
  }
    if (receive_rnd_val_1 == "S10") { noTone(buzzer); }
    if (receive_rnd_val_1 == "S11") { noTone(buzzer); }
delay(250);
}
// Code for the controller
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>


#define OLED_ADDR   0x3C
Adafruit_SSD1306 display(128, 64, &Wire, -1);
int flipcounter = 0; 
int sendTimer = 0; 
int repeat_blocker;
esp_err_t result; //err loggin variable

String first_line = "...";
String second_line = "...";
String callback = "Callback...";
String info_line = "...";
String last_tared_value = "...";
int delayer = 0;

int scale_state = 0; //0- open | 1 - locked | 2 - alerted

int button1 = 10, button2 = 11, button3 = 19, button4 = 18;


// Change these to mac adresses of your esp's
uint8_t alarmAddress[] =     {0x40, 0x4C, 0xCA, 0x51, 0x37, 0x58};// Adress of the alarm device
uint8_t controllerAddress[] ={0x40, 0x4C, 0xCA, 0x44, 0x8F, 0x7C};// Adress of the controller
uint8_t scaleAddress[] =     {0x40, 0x4C, 0xCA, 0x41, 0x8D, 0xB8};// Adress of the scale/pressure plate device
uint8_t broadcastAddress[] = {0x00, 0x0C, 0x00, 0x00, 0x00, 0x00};// DO NOT CHANGE

String send_rnd_val_1;
String send_rnd_val_2;

String receive_rnd_val_1;
String receive_rnd_val_2;

//----------------------------------------Structure of the data
typedef struct struct_message {
    String rnd_1;
    String rnd_2;
} struct_message;

struct_message receive_Data; // Create a struct_message to receive data.
struct_message send_Data; // Create a struct_message to send data.

// Data receiving
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
// Data sending
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  callback = (status == ESP_NOW_SEND_SUCCESS ? "Callback OK" : "ERR_No_reach");
}

// Sending Data
void SendData(String code, String data, String destination){
  esp_err_t result;
  send_Data.rnd_1 = code;
  send_Data.rnd_2 = data;

  //----------------------------------------Send message via ESP-NOW
  if (destination == "scale") {result = esp_now_send(scaleAddress, (uint8_t *) &send_Data, sizeof(send_Data));}
  else if (destination == "alarm") {result = esp_now_send(alarmAddress, (uint8_t *) &send_Data, sizeof(send_Data));}
  else if (destination == "all") {result = esp_now_send(broadcastAddress, (uint8_t *) &send_Data, sizeof(send_Data));}

  if (result == ESP_OK) { Serial.println("Sent with success");}
  else { Serial.println("Error sending the data");}
}

//SETUP
void setup() {
  pinMode(button1, INPUT);
  pinMode(button2, INPUT);
  pinMode(button3, INPUT);
  pinMode(button4, INPUT);
  display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDR);


  Serial.begin(115200);
  WiFi.mode(WIFI_STA); // Set device as a Wi-Fi Station

  //Init ESP-NOW
  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  

  esp_now_register_send_cb(OnDataSent); // Register for Callback receiveing to check if packet was delivered succesfully
  esp_now_register_recv_cb(OnDataRecv); // Register for a callback function that will be called when data is received

  // Register peers & Add peers 
  esp_now_peer_info_t peerInfo;
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;

  memcpy(peerInfo.peer_addr, scaleAddress, 6);
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

//----------- Error translator - formats the errors 
String GetSendResponse(esp_err_t _result) {
    switch (_result) {
        case ESP_OK: return "Data Sent OK"; break;
        case ESP_ERR_ESPNOW_NOT_INIT: return "ESP-NOW not init..."; break; //ESP-NOW not initialized
        case ESP_ERR_ESPNOW_ARG: return "Invalid argument"; break;
        case ESP_ERR_ESPNOW_INTERNAL: return "Internal error"; break;
        case ESP_ERR_ESPNOW_NO_MEM: return "out of memory"; break;
        case ESP_ERR_ESPNOW_NOT_FOUND: return "Peer not found"; break;
        case ESP_ERR_ESPNOW_IF: return "Wifi interface not match"; break; //Wifi interface doesn't match peer
        case ESP_ERR_ESPNOW_FULL: return "Peer list full"; break;
        case ESP_ERR_ESPNOW_EXIST: return "Peer has existed"; break;
        default: return "Unknown error"; break;
    }
}


void loop() {
  String State[4];
  State[0] = String(digitalRead(button1));
  State[1] = String(digitalRead(button2));
  State[2] = String(digitalRead(button3));
  State[3] = String(digitalRead(button4));


  if ((State[0] == "1") && (!repeat_blocker)) {SendData("G33","0","scale");delayer=19; repeat_blocker = true;} //scream
  else if ((State[1] == "1") && (!repeat_blocker)) {SendData("G31","0","scale");delayer=19; repeat_blocker = true;} //lock & recalibrate
  else if ((State[2] == "1") && (!repeat_blocker)) {SendData("G30","0","scale");delayer=19; repeat_blocker = true;} //unlock
  else if ((State[3] == "1") && (!repeat_blocker)) {SendData("G34","0","scale");delayer=19; repeat_blocker = true;} //stfu
  else {
    if(delayer > 20) {SendData("G10","0","scale");delayer = 0;repeat_blocker = false;}
    else{delayer++;repeat_blocker = false;}
  }

    // Formating the data on the oled display
  
    if (receive_rnd_val_1 == "S12") { // aleretd
      scale_state = 2;
    }
    else if (receive_rnd_val_1 == "S10") { // unlocked
      scale_state = 1;
    }
    else if (receive_rnd_val_1 == "S11") { // locked
      scale_state = 0;
    }
    else {scale_state = 3;}


  first_line = "(" + receive_rnd_val_1 + ") + scale1";
  switch (scale_state) {
    case 0: second_line = "READY"; break;
    case 1: second_line = "OFF"; break;
    case 2: second_line = "ALERT"; break;
    case 3: second_line = "NONE"; break;
  } 
  
// Displaying to the oled screen
    String printchars = "";
  for(int i=0; i <= 4; i++) { 
    if(State[i] == "1") {printchars += "#";}
    else {printchars += "-";}
  }

  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println(first_line);
  display.setTextSize(2);

  display.setCursor(0, 10);
  display.println(second_line);
  display.setTextSize(1);
  display.setCursor(0, 30);
  display.println("-");
  display.setCursor(0, 40);
  display.println("-");

  display.setTextSize(1);
  display.setCursor(95, 5);
  display.println("Scrm");
  display.setCursor(95, 20);
  display.println("Tare");
  display.setCursor(95, 35);
  display.println("Dsrm");
  display.setCursor(95, 50);
  display.println("Stfu");
  
  display.setTextSize(1);
  display.setCursor(0, 50);
  display.println(callback);

  display.display();
  delay(250);
}
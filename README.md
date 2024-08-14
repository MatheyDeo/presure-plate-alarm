# Pressure plate alarm
this project uses a load cell to detect weight changes on its pressure plate.

It consists of three individual devices, each with its onw esp32 which comunicate to each other via esp-now.

## The pressure sensor
It is battery powered.

Uses HX711 Analog-Digital Converter and a 5kg load cell to determine changes in the weight
## The Receiver / Alarm
Connected to a speaker trough an amplifier 

If a change in weight is detected by the pressure sensor, the Alarm goes off notifying everyone in range. 
## The controller
Uses Buttons and an OLED display to setup and control both the pressure sensor and the receiver.

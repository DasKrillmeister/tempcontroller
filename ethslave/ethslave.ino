/*

 Secondary part of the tempcontroller, this slave communicates with the master via serial and to a server via Ethernet.
 
 One arduino can't handle everything due to compatibility between libraries.
 
 Serial packet structure
 Status from master to slave: s[temp1 as ascii 00.00],[temp2 as ascii 00.00],[targettemp as ascii 00.00],[Current action as byte - 0 = idle - 1 = cooling - 2 = heating]e
 new temp slave -> master: t[ascii 00.00]e
 
 
 Pinout:
 
 0: RX Master
 1: TX Master
 
 10: Ethernet CS
 11: Ethernet SI
 12: Ethernet SO
 13: Ethernet SCK
 
 TODO: Ethernet!
 
 */

#include <UIPEthernet.h>
EthernetClient tcp;
IPAddress ownIP(172,30,1,50);
IPAddress server(172,30,1,100);


void setup() {
  Serial.begin(115200);
  uint8_t mac[6] = {0xAD,0xAD,0xFA,0x23,0x75,0x22};
  Ethernet.begin(mac, ownIP);
  delay(500);
}

void loop() {
  static float incSerialData[4];
  float newTargetTemp = 100;
  
  readSerial(incSerialData);
  
  // Ethernet read loop goes here
  
  if (newTargetTemp < 100) {
    sendSerial(newTargetTemp);
  }
  
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void readSerial(float incSerialData[4]) {

  if (Serial.available() > 40) { // Serial buffer filling up, clear it.
    while (Serial.available()) {
      Serial.read();
    }
  }


  if (Serial.available() > 0) {
    if (Serial.peek() == 115) {   // Ascii, 115 = s
      Serial.read();   // Throw away start char
      incSerialData[0] = Serial.parseFloat();  // temp 1
      incSerialData[1] = Serial.parseFloat();  // temp 2
      incSerialData[2] = Serial.parseFloat();  // Target temp
      incSerialData[3] = Serial.parseFloat();  // Current action
      Serial.read();   // Throw away end char
    }
  }
  else {

    while (Serial.available()) { // There is serial data available but it does not start with a known char, empty buffer.
      Serial.read();
    }
  }
}

void sendSerial(float newTargetTemp) {
  Serial.print("t");
  Serial.print(newTargetTemp);
  Serial.print("e");
  delay(100);
}



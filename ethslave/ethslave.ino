/*

 Secondary part of the tempcontroller, this slave communicates with the master via serial and to a server via Ethernet.
 
 One arduino can't handle everything due to compatibility between libraries.
 
 Serial packet structure
 Status from master to slave: s[temp1 as ascii 00.00],[temp2 as ascii 00.00],[targettemp as ascii 00.00],[Current action as byte - 0 = idle - 1 = cooling - 2 = heating]e
 new temp slave -> master: t[ascii 00.00]e
 pc to slave new temp -> t[ascii 00.00]e
 
 
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
EthernetClient client;
//IPAddress ownIP(172,30,1,50);
IPAddress server(172,30,1,100);
//IPAddress dns1(172,30,1,5);
//IPAddress gateway(172,30,1,1);
//IPAddress netmask(255,255,240,0);

int dport = 5000;
float incSerialData[4];




void setup() {
  Serial.begin(115200);
  ////////////////////////////
  Serial.println("Begin");
  /////////////////////////
//  uint8_t mac[6] = {0xAD,0xAD,0xFA,0x23,0x75,0x22};
  uint8_t mac[6] = {0x00,0x01,0x02,0x03,0x04,0x05};
  //Ethernet.begin(mac, ownIP, dns1, gateway, netmask);
  Ethernet.begin(mac);
  delay(500);
  ///////////////////////
  Serial.print("localIP: ");
  Serial.println(Ethernet.localIP());
  Serial.print("subnetMask: ");
  Serial.println(Ethernet.subnetMask());
  Serial.print("gatewayIP: ");
  Serial.println(Ethernet.gatewayIP());
  Serial.print("dnsServerIP: ");
  Serial.println(Ethernet.dnsServerIP());
  ////////////////////////
}

void loop() {
  float newTargetTemp;
  
  readSerial();
  
  tcpConnect();    // If connection already established does nothing. Else connects to server.
  
  tcpSend();
  
  newTargetTemp = tcpRead();
  
  if (newTargetTemp < 100) {
    sendSerial(newTargetTemp);
  }
  
  
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

void readSerial() {

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

void tcpConnect() {
  if (client.connected()) {
    return;
  }
  
  client.connect(server, dport);
} 

void tcpSend() {
  client.print("s");
  client.print(incSerialData[0]);
  client.print(",");
  client.print(incSerialData[1]);
  client.print(",");
  client.print(incSerialData[2]);
  client.print(",");
  client.print(incSerialData[3]);
  client.print("e");
}


float tcpRead() {
  if (client.read()) {
    float temp = client.parseFloat();
    client.flush();
    return temp;
  }
  return 100;
}
    
  




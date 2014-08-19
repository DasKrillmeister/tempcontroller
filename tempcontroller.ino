
/*


Temp sensor datasheet:
http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

OneWire on pin 10
*/


#include <OneWire.h>

OneWire onewire(10); // 4.7K pullup on pin

int i;
byte sensAddr[8];

void setup() {
  Serial.begin(9600);

  // Find sensor and select
  onewire.search(sensAddr);
  onewire.select(sensAddr);

  // Upload sensor address to serial, ex 28 FF BA 6E 15 14 0 97
  /*
   for ( i = 0 ; i<8; i++) {
   Serial.print(sensAddr[i], HEX);
   Serial.print(" ");
   }
   */
}


void loop() {
  byte onewireIncData[8];

  onewire.reset();
  onewire.select(sensAddr);
  onewire.write(0x44); // init temp reading

  delay(1000); // Temp reading takes a while to finish, should probably do something useful here

  onewire.reset();
  onewire.select(sensAddr);
  onewire.write(0xBE); //command to read memory


  for (i=0; i<9; i++) {
    onewireIncData[i] = onewire.read();
    //    Serial.print(onewireIncData[i], HEX);
    //    Serial.print(" ");
  }  

  if (OneWire::crc8(onewireIncData,8) != onewireIncData[8]) {
    Serial.println("CRC Mismatch, insert panic");
  }

  byte tempbyte[2];
  tempbyte[0] = onewireIncData[1];  // Having the least significant bit first confuses and enrages me
  tempbyte[1] = onewireIncData[0];

  boolean isNegative;
  isNegative = bitRead(tempbyte[0],7); // Bit 4-8 of the most significant byte are all sign bits indicating a negative number

  tempbyte[0] = tempbyte[0] << 5;
  tempbyte[0] = tempbyte[0] >> 5; // To get rid of the signing bits

  unsigned int tempraw;
  tempraw = tempbyte[1] | tempbyte[0] << 8;

  float currtemp;
  currtemp = tempraw * 0.0625;
  
  if (isNegative) { currtemp = currtemp *-1; }
  
  Serial.println(" ");  
  Serial.print("Current temperature: ");
  Serial.print(currtemp);
  Serial.print("C");


}




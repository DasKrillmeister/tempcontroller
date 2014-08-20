
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
}


void loop() {
  float currtemp;

  initTempReading();

  delay(1000); // Temp reading takes 750ms to finish, should probably do something useful here

  currtemp = readTemp();
  
  Serial.println(" ");  
  Serial.print("Current temperature: ");
  Serial.print(currtemp);
  Serial.print("C");

}


void panic(String x) {
  Serial.println(" ");
  Serial.println("Something went horribly wrong, halting program. Reason: ");
  Serial.print(x);
  while (1) {}
}

// Sends command to initialize a reading
void initTempReading() {
  onewire.reset();
  onewire.select(sensAddr);
  onewire.write(0x44); // init temp reading
}


// Reads the sensor scratchpad and returns temperature in celsius.
float readTemp() {
  byte onewireIncData[8];
  onewire.reset();
  onewire.select(sensAddr);
  onewire.write(0xBE); //command to read memory

  for (i=0; i<9; i++) {
    onewireIncData[i] = onewire.read();
  }  

  if (OneWire::crc8(onewireIncData,8) != onewireIncData[8]) {
    panic("CRC Mismatch");
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
  
  return currtemp;
}


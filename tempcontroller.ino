#include <OneWire.h>

OneWire onewire(10);

byte sensAddr[8];

void setup() {
  Serial.begin(9600);

  // Find sensor and select
  onewire.search(sensAddr);
  onewire.select(sensAddr);

  // Upload sensor address to serial, ex 28 FF BA 6E 15 14 0 97
  int i;
  for ( i = 0 ; i<8; i++) {
    Serial.print(sensAddr[i], HEX);
    Serial.print(" ");
  }
}


void loop() {
  int i;
  byte onewireIncData[8];
  
  onewire.reset();
  onewire.select(sensAddr);
  onewire.write(0x44); // init temp reading
  
  delay(1000); //Temp reading takes a while to finish

  onewire.reset();
  onewire.select(sensAddr);
  onewire.write(0xBE); //command to read memory
   
  Serial.println("Reading:");
  
  
  for (i=0; i<9; i++) {
    onewireIncData[i] = onewire.read();
    Serial.print(onewireIncData[i], HEX);
    Serial.print(" ");
  }  

  while(1) {}

}



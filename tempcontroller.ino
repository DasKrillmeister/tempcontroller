/*

 
 Temp sensor datasheet:
 http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
 
 
 Arduino pinout:
 
 Pin 2: Cooling relay
 Pin 3: Heating
 
 Pin 5: Display E - SCK
 Pin 6: Display RS - CS
 Pin 7: Display R/W - MOSI
 
 Pin 10: Onewire temp sensor
 
 */

#define MOSIpin 7
#define CSpin 6
#define SCKpin 5

#define coolpin 2
#define heatpin 3

#include <OneWire.h>
#include <U8glib.h>


U8GLIB_ST7920_128X64_1X u8g(SCKpin, MOSIpin, CSpin, 8);

OneWire onewire(10); // 4.7K pullup on pin


int i;
byte sensAddr[8];

void setup() {
  pinMode(coolpin, OUTPUT);
  pinMode(heatpin, OUTPUT);

  Serial.begin(9600);

  // Find sensor and select
  onewire.search(sensAddr);
  onewire.select(sensAddr);
}


void loop() {
  static float currtemp = 9001;
  static float targettemp = 9001;

  initTempReading();

  drawloop(currtemp, targettemp);
  

  delay(1000); // Temp reading takes 750ms to finish, should probably do something useful here

  currtemp = readTemp();
  


  Serial.println(" ");  
  Serial.print("Current temperature: ");
  Serial.print(currtemp);
  Serial.print("C");



}


//////////////////////////////////////////////////////////////////////////////////////////////////////


void panic(String x) {
  Serial.println(" ");
  Serial.println("Something went horribly wrong, halting program. Reason: ");
  Serial.print(x);
  while (1) {
  }
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

  if (isNegative) { 
    currtemp = currtemp *-1; 
  }

  return currtemp;
}


void drawloop(float currtemp, float targettemp) {
  u8g.firstPage();  
  do {
    draw(currtemp, targettemp);
  } while( u8g.nextPage() );
}
  

// All draw commands go in here
void draw(float currtemp, float targettemp) {
  int strwidth[2];

  u8g.setFont(u8g_font_helvR08);
  
  u8g.setPrintPos(5, 10);
  u8g.print("Current temp: ");
  u8g.print(currtemp);
  u8g.print("C");
  
  u8g.setPrintPos(5, 20);  
  u8g.print("Target temp: ");
  u8g.print(targettemp);
  u8g.print("C");
}







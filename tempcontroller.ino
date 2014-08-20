/*
 
 todo:
 save target temp to eeprom
 ethernet?
 heat/cooling regulation
 signed int instead of unsigned in data parse
 
 Temp sensor datasheet:
 http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf
 
 
 Arduino pinout:
 
 Pin 2: Cooling relay
 Pin 3: Heating
 
 Pin 5: Display E - SCK
 Pin 6: Display RS - CS
 Pin 7: Display R/W - MOSI
 
 Pin 10: Onewire temp sensor
 
 Pin 11: Switch connected to ground
 Pin 12: Switch connected to ground
 
 */

#define MOSIpin 7
#define CSpin 6
#define SCKpin 5

#define coolpin 2
#define heatpin 3

#include <OneWire.h>
#include <U8glib.h>
#include <EEPROM.h>


U8GLIB_ST7920_128X64_1X u8g(SCKpin, MOSIpin, CSpin, 8);

OneWire onewire(10); // 4.7K pullup on pin


int i;
byte sensAddr[8];

void setup() {
  pinMode(coolpin, OUTPUT);
  pinMode(heatpin, OUTPUT);

  pinMode(11, INPUT_PULLUP);
  pinMode(12, INPUT_PULLUP);

  Serial.begin(9600);

  // Find sensor and select
  onewire.search(sensAddr);
  onewire.select(sensAddr);
}


void loop() {
  static float currtemp = 9001;
  static float targettemp = 14;

  initTempReading();

  drawloop(currtemp, targettemp);

  targettemp = watchInputsFor(750, targettemp);  // temp reading takes 750 ms

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
  // Turn off both relays
  digitalWrite(2, LOW);
  digitalWrite(3, LOW);
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

  int tempraw;
  tempraw = tempbyte[1] | tempbyte[0] << 8;

  float currtemp;
  currtemp = tempraw * 0.0625;

  return currtemp;
}


void drawloop(float currtemp, float targettemp) {
  u8g.firstPage();  
  do {
    draw(currtemp, targettemp);
  } 
  while( u8g.nextPage() );
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


float watchInputsFor(int x, float targettemp) {
  unsigned long starttime = millis();

  while (starttime + x > millis()) {
    if (digitalRead(11) == HIGH) {
      targettemp = targettemp + 1;
      delay(200);
      return targettemp;
    }
    if (digitalRead(12) == HIGH) {
      targettemp = targettemp - 1;
      delay(200);
      return targettemp;
    }
  }
  return targettemp;
}






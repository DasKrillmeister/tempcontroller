/*
 
 
 Temp sensor datasheet:
 http://datasheets.maximintegrated.com/en/ds/DS18B20.pdf

 
 Arduino pinout:
 
 Pin 22: Switch connected to ground
 Pin 9: Switch connected to ground
 
 Pin 2: Cooling relay
 Pin 3: Heating
 
 Pin 5: Display E - SCK
 Pin 6: Display RS - CS
 Pin 7: Display R/W - MOSI
 
 Pin 10: Ethercard CS
 
 Pin 26: Onewire temp sensor
 
 Pin 50: Ethercard SI
 Pin 51: Ethercard SO
 Pin 52: Ethercard SCK
 
 */

#define MOSIpin 7
#define CSpin 6
#define SCKpin 5

#define coolpin 2
#define heatpin 3

#define switchpin1 22
#define switchpin2 9


#include <OneWire.h>
#include <U8glib.h>
#include <EEPROM.h>





// Display lib init
U8GLIB_ST7920_128X64_1X u8g(SCKpin, MOSIpin, CSpin);

// Onewire init
OneWire onewire(26); // 4.7K pullup on pin

int i;
byte sensAddr[8];
String curraction = "Idle";

void setup() {
  pinMode(coolpin, OUTPUT);
  pinMode(heatpin, OUTPUT);

  pinMode(switchpin1, INPUT_PULLUP);
  pinMode(switchpin2, INPUT_PULLUP);


  // Find sensor and select
  
  onewire.search(sensAddr);
  onewire.select(sensAddr);
  
}


void loop() {
  static float targettemp;
  targettemp = readeepromonce(targettemp);
  static float currtemp = targettemp;

  initTempReading();

  drawloop(currtemp, targettemp);

  targettemp = watchInputsFor(750, targettemp);  // temp reading takes 750 ms

  currtemp = readTemp();

  regulateRelays(currtemp, targettemp);
    
}


//////////////////////////////////////////////////////////////////////////////////////////////////////

// Called in case of critical error, stops program and shuts down relays
void panic(String x) {

  // Turn off both relays
  digitalWrite(coolpin, LOW);
  digitalWrite(heatpin, LOW);
  
  u8g.firstPage();  
  do {
    u8g.setFont(u8g_font_helvR08);
    u8g.setPrintPos(5, 10);
    u8g.print("PANIC, Reason:");
    u8g.setPrintPos(5, 25);
    u8g.print(x);    
  } 
  while( u8g.nextPage() ); 
  
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
  
  u8g.setPrintPos(5, 30);
  u8g.print("Current action: ");
  u8g.print(curraction);
  
}


float watchInputsFor(int x, float targettemp) {
  unsigned long starttime = millis();

  while (starttime + x > millis()) {
    if (digitalRead(switchpin1) == HIGH) {
      targettemp = targettemp + 1;
      delay(150);
      if (targettemp > 40) {
        targettemp = 40;
      }
      writeeeprom(targettemp);
      return targettemp;
    }
    if (digitalRead(switchpin2) == HIGH) {
      targettemp = targettemp - 1;
      delay(150);
      if (targettemp < -30) {
        targettemp = -30;
      }
      writeeeprom(targettemp);
      return targettemp;
    }
  }
  return targettemp;
}

float readeepromonce(float currtarget) {
  static boolean read = 0;
  if (read > 0) {
    return currtarget;
  }
  char data;
  data = EEPROM.read(0);
  float retval;
  retval = float(data);
  read++;
  return retval;
}

void writeeeprom(float floattemp) {
  char chartemp;
  chartemp = char(floattemp);
  EEPROM.write(0, chartemp);
}


void regulateRelays(float currtemp, float targettemp) {
  float tolerancestop = 0.4;
  float tolerancestart = 0.7;

  if (currtemp > targettemp + tolerancestop) { // Stop heater
    digitalWrite(heatpin, LOW);
  }  

  if (currtemp < targettemp - tolerancestop) {  // Stop compressor
    digitalWrite(coolpin, LOW);
  }

  if (currtemp < targettemp - tolerancestart) {  //Start heater
    digitalWrite(heatpin, HIGH);
  }

  if (currtemp > targettemp + tolerancestart) {  // Start compressor
    digitalWrite(coolpin, HIGH);
  }


  if (digitalRead(heatpin) == HIGH && digitalRead(coolpin) == HIGH) { // This should never happen, panic
    panic("Both relays active");
  }
  
  if (digitalRead(coolpin) == HIGH) { curraction = "Cooling"; }
  if (digitalRead(heatpin) == HIGH) { curraction = "Heating"; }  
  if (digitalRead(coolpin) == LOW && digitalRead(heatpin) == LOW) { curraction = "Idle"; }
}




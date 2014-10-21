// WS282x programmer
// Takes some commands from the serial and allows you to program WS282x LED modules
//
// Should work with any 168, 328, or 32U4 based Arduino.
//
// Copyright 2014 Matt Mets

#include "DmxSimpleMod.h"

#define DATA_PIN    13      // Pin connected to the data output
#define ADDRESS_PIN  7      // Pin connected to the address programmer output


#define COMMAND_IDENTIFY        'i'    // Idenify one pixel, flashing R-G-B on it
#define COMMAND_PROGRAM         'p'    // Program a channel
#define COMMAND_DISPLAY_PATTERN 'd'    // 1:Identify 2:Rainbow Swirl 3: Flash Mode
#define COMMAND_MAX_PIXELS      'm'    // Number of pixels to send data to
#define COMMAND_CONTROLLER_TYPE 't'    // 1:WS2821S 2:WS2822S


int maxPixel       = 14;    // Maximum pixel to display
int currentPattern = 2;     // Current pattern we are displaying
int currentPixel   = 1;     // Current pixel for identification mode
int controllerType = 1;     // Controller type (1:WS2822S 2:WS2821)


void setup() {
  pinMode(DATA_PIN, OUTPUT);
  digitalWrite(DATA_PIN, LOW);
  
  pinMode(ADDRESS_PIN, OUTPUT);  
  digitalWrite(ADDRESS_PIN, HIGH);

  DmxSimple.usePin(DATA_PIN);
  DmxSimple.maxChannel(maxPixel*3);
  DmxSimple.begin();
  
  Serial.begin(19200);
}


void writePixel(int pixel, int r, int g, int b) {
  DmxSimple.write((pixel - 1)*3 + 1, b);
  DmxSimple.write((pixel - 1)*3 + 2, g);
  DmxSimple.write((pixel - 1)*3 + 3, r);   
}


uint8_t flipEndianness(uint8_t input) {
  uint8_t output = 0;
  for(uint8_t bit = 0; bit < 8; bit++) {
    if(input & (1 << bit)) {
      output |= (1 << (7 - bit));
    }
  }
  
  return output;
}

void programAddress(int address) {
  // Build the output pattern to program this address
  uint8_t pattern[3];
  
  if (controllerType ==  1) {
    // WS2822S (from datasheet)
    int channel = (address-1)*3+1;
    pattern[0] = flipEndianness(channel%256);
    pattern[1] = flipEndianness(240 - (channel/256)*15);
    pattern[2] = flipEndianness(0xD2);
  }
  else {
    // WS2821 (determined experimentally)
    int channel = (address)*3;
    pattern[0] = flipEndianness(channel%256);
    pattern[1] = flipEndianness(240 - (channel/256)*15);
    pattern[2] = flipEndianness(0xAE);    
  }

  DmxSimple.end();                 // Turn off the DMX engine
  delay(50);                       // Wait for the engine to actually stop
  digitalWrite(ADDRESS_PIN, HIGH); // Set the address output pin high if it wasn't already
  digitalWrite(DATA_PIN, LOW);     // Force the data pin low
  delay(50);                       // Let this sit for a bit to signal the chip we are starting
  
  DmxSimple.usePin(ADDRESS_PIN);   // Use the address output pin to generate the DMX signal
  DmxSimple.maxChannel(3);
  DmxSimple.write(1, pattern[0]);  // Pre-load the output pattern (note this might trash the LED output, meh)
  DmxSimple.write(2, pattern[1]);
  DmxSimple.write(3, pattern[2]);

  digitalWrite(ADDRESS_PIN, LOW);   // Set the address pin low to begin transmission
  delay(1000);                      // Delay 1s to signal address transmission begin
  DmxSimple.begin();                // Begin transmission. Only the first one actually counts.

  delay(20);                        // Wait a while for the the signal to be sent
  DmxSimple.end();
  digitalWrite(ADDRESS_PIN, HIGH);  // Set the address output pin high if it wasn't already

  // Reset the output, and set the output to our new pixel address.
  DmxSimple.usePin(DATA_PIN);
  DmxSimple.maxChannel(maxPixel*3);
  DmxSimple.begin();
}

void identify() {
  #define FLASH_SPEED 300
  
  static int brightness;
  static int maxBrightness = 255;
  
  for(int pixel = 1; pixel <= maxPixel; pixel++) {
      int r = 0;
      int g = 0;
      int b = 0;
      if(brightness < FLASH_SPEED/3) {
        r = 1;
      }
      else if(brightness < FLASH_SPEED*2/3) {
        g = 1;
      }
      else {
        b = 1;
      }
    
    if(pixel == currentPixel) {      
      writePixel(pixel, r*maxBrightness, g*maxBrightness, b*maxBrightness);
    }
    else {
      writePixel(pixel, r, g, b);
    }
  }
  
  brightness=(brightness+1)%FLASH_SPEED;
}

void color_loop() {  
  static uint8_t i = 0;
  static int j = 0;
  static int f = 0;
  static int k = 0;
  static int count;

  static int pixelIndex;
  
  for (uint16_t i = 0; i < maxPixel*10; i+=10) {
    writePixel(i/10+1,
      128*(1+sin(i/30.0 + j/1.3       )),
      128*(1+sin(i/10.0 + f/2.9)),
      128*(1+sin(i/25.0 + k/5.6))
//      128*(1+sin(i/10.0 + f/9.0  + 2.1)),
//      128*(1+sin(i/30.0 + k/14.0 + 4.2))
    );
  }
  
  j = j + 1;
  f = f + 1;
  k = k + 2;
}

void marty_loop() {  
  static uint8_t i = 0;
  static int j = 0;
  static int f = 0;
  static int k = 0;
  static int count;

  static int pixelIndex;
  
  for (uint16_t i = 0; i < maxPixel*10; i+=10) {
    f = 48*(1+sin(i/180.0 + j/5));
    writePixel(i/10+1,
      f,
      f,
      f
    );
  }
  
  j = j + 1;
}

void countUp() {
  #define MAX_COUNT 400
  static int counts = 0;
  static int pixel = 0;
  
  for (uint16_t i = 0; i < maxPixel; i+=1) {
    if(pixel == i) {
      writePixel(i+1,
        255,
        255,
        255
      );
    }
    else {
      writePixel(i+1, 0,0,0);
    }
  }
  
  counts++;
  if(counts>MAX_COUNT) {
    counts = 0;
    pixel = (pixel+1)%maxPixel;
  }
}

void pictureFrame() {
  #define MAX_COUNT 10
  static int counts = 0;
  static int pixel = 0;
  
  static uint8_t i = 0;
  static int j = 0;
  static int f = 0;
  static int k = 0;
  static int count;

  static int pixelIndex;
  
  for (uint16_t i = 0; i < 10; i+=1) {
    if(pixel == i) {
      writePixel(i+1,
        0,
        0,
        0
      );
    }
    else {
//      writePixel(i+1, 90,0,255);

    writePixel(i+1,
      128*(1+sin(i/9.0 + j/10.3       )),
      128*(1+sin(i/3.0 + f/20.9)),
      128*(1+sin(i/6.0 + k/50.6))
    );
    
    }
  }
  
  writePixel(11,255,0,0);
  
  counts++;
  if(counts>MAX_COUNT) {
    counts = 0;
    pixel = (pixel+1)%maxPixel;
  }
  
  j = j + 1;
  f = f + 1;
  k = k + 2;
}

void loop() {
  
  if(Serial.available() > 0) {
    char command = Serial.read();
    int parameter = Serial.parseInt();
    
    if(command == COMMAND_IDENTIFY) {
      if(parameter < 1 || parameter > maxPixel) {
        parameter = 1;
      }
      
      Serial.print("Identifying pixel on channel: ");
      Serial.println(parameter);
      currentPixel = parameter;
      currentPattern = 1;
    }
    else if(command == COMMAND_PROGRAM) {
      if(parameter < 1 || parameter > maxPixel) {
        parameter = 1;
      }
      
      Serial.print("Programming pixel to address: ");
      Serial.println(parameter);
      programAddress(parameter);
      currentPixel = parameter;
    }
    else if(command == COMMAND_DISPLAY_PATTERN) {
      Serial.print("Displaying pattern: ");
      Serial.println(parameter);
      currentPattern = parameter;
    }
    else if(command == COMMAND_MAX_PIXELS) {
      Serial.print("Setting max pixel to: ");
      Serial.println(parameter);
      maxPixel = parameter;
    }
    else if(command == COMMAND_CONTROLLER_TYPE) {
      Serial.print("Setting controller type to: ");
      Serial.println(parameter);
      controllerType = parameter;
    } 
  }

  if(currentPattern == 1) {
    identify();
  }
  else if(currentPattern == 2) {
    color_loop();
  }
  else if(currentPattern == 3) {
    marty_loop();
  }
  else if(currentPattern == 4) {
    pictureFrame();
  }
  else {
    countUp();
  }
}

// WS282x programmer
// Takes some commands from the serial and allows you to program WS282x LED modules
//
// Copyright 2014 Matt Mets

#include "DmxSimpleMod.h"

#define ADDRESS_PIN 7
#define DATA_PIN 13
int MAX_PIXEL = 10;

int currentPattern = 1;    // Current pattern we are displaying
int currentPixel = 1;      // Current pixel for identification mode
int controllerType = 1;    // Controller type (1:WS2822S 2:WS2821)

#define COMMAND_IDENTIFY        'i'    // Idenify one pixel, flashing R-G-B on it
#define COMMAND_PROGRAM         'p'    // Program a channel
#define COMMAND_DISPLAY_PATTERN 'd'    // 1:Identify 2:Rainbow Swirl 3: Flash Mode
#define COMMAND_MAX_PIXELS      'm'    // Number of pixels to send data to
#define COMMAND_CONTROLLER_TYPE 't'    // 1:WS2822S 2:WS2821

void setup() {
  pinMode(DATA_PIN, OUTPUT);  
  digitalWrite(DATA_PIN, LOW);
  
  pinMode(ADDRESS_PIN, OUTPUT);  
  digitalWrite(ADDRESS_PIN, HIGH);

  DmxSimple.usePin(DATA_PIN);
  DmxSimple.maxChannel(MAX_PIXEL*3);
  dmxBegin();
  
  Serial.begin(19200);
}


void writePixel(int pixel, int r, int g, int b) {
  DmxSimple.write((pixel - 1)*3 + 1, r);
  DmxSimple.write((pixel - 1)*3 + 2, g);
  DmxSimple.write((pixel - 1)*3 + 3, b);   
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
  
  Serial.print(pattern[0], HEX);
  Serial.print(" ");
  Serial.print(pattern[1], HEX);
  Serial.print(" ");
  Serial.print(pattern[2], HEX);
  Serial.print("\n");


  dmxEnd();                        // Turn off the DMX engine
  delay(200);                      // Wait for the engine to actually stop
  digitalWrite(ADDRESS_PIN, HIGH); // Set the address output pin high if it wasn't already
  digitalWrite(DATA_PIN, LOW);     // Force the data pin low
  delay(200);                      // Let this sit for a bit to signal the chip we are starting
  
  DmxSimple.usePin(ADDRESS_PIN);   // Use the address output pin to generate the DMX signal
  DmxSimple.maxChannel(3);
  DmxSimple.write(1, pattern[0]);  // Pre-load the output pattern (note this might trash the LED output, meh)
  DmxSimple.write(2, pattern[1]);
  DmxSimple.write(3, pattern[2]);

  digitalWrite(ADDRESS_PIN, LOW);   // Set the address pin low to begin transmission
  delay(1000);                      // Delay 1s to signal address transmission begin
  dmxBegin();

  delay(100);                      // Wait a while for the the signal to be sent
  dmxEnd();
  digitalWrite(ADDRESS_PIN, HIGH); // Set the address output pin high if it wasn't already

  // Reset the output, and set the output to our new pixel address.
  DmxSimple.usePin(DATA_PIN);
  DmxSimple.maxChannel(MAX_PIXEL*3);
  dmxBegin();
}

void identify() {
  static int brightness;
  static int maxBrightness = 255;
  
  for(int pixel = 1; pixel <= MAX_PIXEL; pixel++) {    
    if(pixel == currentPixel) {
      int r = 0;
      int g = 0;
      int b = 0;
      if(brightness%10000 < 3333) {
        r = maxBrightness;
      }
      else if(brightness%10000 < 6666) {
        g = maxBrightness;
      }
      else {
        b = maxBrightness;
      }
      
      writePixel(pixel, r, g, b);
    }
    else {
      writePixel(pixel, 1, 2, 3);
    }
  }
  
  brightness++;
}

void color_loop() {  
  static uint8_t i = 0;
  static int j = 0;
  static int f = 0;
  static int k = 0;
  static int count;

  static int pixelIndex;
  
  for (uint16_t i = 0; i < MAX_PIXEL*10; i+=10) {
    writePixel(i/10+1,
      64*(1+sin(i/2.0 + j/4.0       )),
      64*(1+sin(i/1.0 + f/9.0  + 2.1)),
      64*(1+sin(i/3.0 + k/14.0 + 4.2))
    );
  }
  
  j = j + 1;
  f = f + 1;
  k = k + 2;
}


void countUp() {
  static int counts = 0;
  static int pixel = 0;
  
  for (uint16_t i = 0; i < MAX_PIXEL; i+=1) {
    if(pixel == i) {
      writePixel(i+1, 255,255,255);
    }
    else {
      writePixel(i+1, 0,0,0);
    }
  }
  
  counts++;
  if(counts>1000) {
    counts = 0;
    pixel = (pixel+1)%MAX_PIXEL;
  }
}


void loop() {
  
  if(Serial.available() > 0) {
    
    char command = Serial.read();
    int parameter = Serial.parseInt();


    
    if(command == COMMAND_IDENTIFY) {
      if(parameter < 1 || parameter > MAX_PIXEL) {
        parameter = 1;
      }
      
      Serial.print("Identifying pixel on channel: ");
      Serial.println(parameter);
      currentPixel = parameter;
      currentPattern = 1;
    }
    else if(command == COMMAND_PROGRAM) {
      if(parameter < 1 || parameter > MAX_PIXEL) {
        parameter = 1;
      }
      
      Serial.print("Programming pixel to address: ");
      Serial.println(parameter);
      programAddress(parameter);
    }
    else if(command == COMMAND_DISPLAY_PATTERN) {
      Serial.print("Displaying pattern: ");
      Serial.println(parameter);
      currentPattern = parameter;
    }
    else if(command == COMMAND_MAX_PIXELS) {
      Serial.print("Setting max pixel to: ");
      Serial.println(parameter);
      MAX_PIXEL = parameter;
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
  else {
    countUp();
  }
}

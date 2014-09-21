/**
 * DmxSimple - A simple interface to DMX.
 *
 * Modified by Matt Mets for WS2822 LED programming
 *
 * Copyright (c) 2008-2009 Peter Knight, Tinker.it! All rights reserved.                
 */

#ifndef DMX_SIMPLE_MOD_H
#define DMX_SIMPLE_MOD_H

#include <inttypes.h>

#if RAMEND <= 0x4FF
#define DMX_SIZE 128                                                                    
#else                                                                                   
#define DMX_SIZE 512                                                                    
#endif

class DmxSimpleClass                                                                    
{                                                            
  public:
    // Set output pin
    // @param pin (input) Output digital pin to use
    void usePin(uint8_t pin);
  
    // Set DMX maximum channel
    // @param channel (input) The highest DMX channel to use (max DMX_SIZE)
    void maxChannel(int channel);
    
    // Write to a DMX channel
    // @param address (input) DMX address to set (1 - DMX_SIZE)
    // @param value (input) Value to write to the DMX address
    void write(int address, uint8_t value);
    
    void begin();
    void end();
};
extern DmxSimpleClass DmxSimple;
#endif

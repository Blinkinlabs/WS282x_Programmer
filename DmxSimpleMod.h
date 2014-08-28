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
    void maxChannel(int);                                                               
    void write(int, uint8_t);                                                           
    void usePin(uint8_t);
};
extern DmxSimpleClass DmxSimple;

extern void dmxEnd();
extern void dmxBegin();
  
#endif

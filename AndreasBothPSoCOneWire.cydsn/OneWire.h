/* ========================================
 *File Name: OneWire.h
 * 
 * AUTHOR: Andreas BOTH
 * DATE: 2024-04-19
 * COURSE: tx00db04
 * Description: Declares functions to communicate to a OneWire device.
 *
 * ========================================
*/
#ifndef ONEWIRE
#define ONEWIRE
   #include "project.h"
    /* defines the delay timing reccomended for OneWire communication (values in microseconds)*/
    #define ONE_WIRE_DELAY_A    6
    #define ONE_WIRE_DELAY_B    64
    #define ONE_WIRE_DELAY_C    60
    #define ONE_WIRE_DELAY_D    10
    #define ONE_WIRE_DELAY_E    9
    #define ONE_WIRE_DELAY_F    55
    #define ONE_WIRE_DELAY_G    0
    #define ONE_WIRE_DELAY_H    480
    #define ONE_WIRE_DELAY_I    70
    #define ONE_WIRE_DELAY_J    410
    
    
    /*Defines Operations and Command values needed for the DS1822*/
    #define HIGH                0x01
    #define LOW                 0x00
    #define SKIP_ROM            0xCC    //Address all reachable devices
    #define READ_SCRATCHPAD     0xBE    //Command for output of result register from slave
    #define START_TEMP_COMVERSION 0x44  //starts temperature conversion in IC may take up to 750ms
    
    void OneWireWriteBit(uint8 bit);
    void OneWireWriteByte(uint8 byte);
   
    uint8 OneWireReadBit();
    uint8 OneWireReadByte();
    int OneWireReset();
    
    
#endif


/* [] END OF FILE */

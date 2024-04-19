/* ========================================
 *File Name: OneWire.c
 * 
 * AUTHOR: Andreas BOTH
 * DATE: 2024-04-19
 * COURSE: tx00db04
 * Description: Defines functions to communicate to a OneWire device.
 *
 * ========================================
*/
#include "project.h"
#include "OneWire.h"
#define MSB_OF_BYTE 0x80
#define LSB_Of_BYTE 0x01


/*Generates Init Pulse with the necessary delays*/
int OneWireReset()                  //init. pulse generation
{
    int result;
    
    CyDelayUs(ONE_WIRE_DELAY_G);    //0us delay (compatibility with other drive mode, not implemented)
    OneWire_Write(LOW);             
    CyDelayUs(ONE_WIRE_DELAY_H);    //480us delay
    OneWire_Write(HIGH);
    CyDelayUs(ONE_WIRE_DELAY_I);    //70us  delay
    result = OneWire_Read()^0x01;   //check for presence pulse of slave device
    CyDelayUs(ONE_WIRE_DELAY_J);    //410us delay
  
    return result;
}

/*Generates pulse for write function on bus LOGIC ONE or ZERO*/
void OneWireWriteBit(uint8 bit)
{
    OneWire_Write(LOW);
    if(bit)
    {
        /*Case Logic 1*/
        CyDelayUs(ONE_WIRE_DELAY_A);    //6us delay
        OneWire_Write(HIGH);
        CyDelayUs(ONE_WIRE_DELAY_B);    //64us delay
    } 
    else
    {
        /*Case Logic 0*/
        CyDelayUs(ONE_WIRE_DELAY_C);    //60us delay
        OneWire_Write(HIGH);
        CyDelayUs(ONE_WIRE_DELAY_D);    //10us delay
    }
}

/*Writes entire byte to bus*/
void OneWireWriteByte(uint8 byte)
{
    for(int n=0;n<8;n++)
    {
        OneWireWriteBit((byte >> n) & LSB_Of_BYTE);    //shift each iteration and only send last bit
    }
}


/*Genereates READ pulse for Bus system*/
uint8 OneWireReadBit()
{
    uint8 result;
    OneWire_Write(LOW);
    CyDelayUs(ONE_WIRE_DELAY_A);    //6us delay
    OneWire_Write(HIGH);
    CyDelayUs(ONE_WIRE_DELAY_E);    //9us delay   
    result =OneWire_Read();         //read slave response
    CyDelayUs(ONE_WIRE_DELAY_F);    //55us delay
    return result;
}

/*Reads entire Byte from the bus*/
uint8 OneWireReadByte()
{
    uint8 result=0;
    for(uint8 n=0;n<8;n++)
    {
        result >>= 1;               //shifts result each cycle to the right, since slave response is LSB first
        
        //if result is one then set MSB
        if(OneWireReadBit())
        {
            result |= MSB_OF_BYTE;
        }
        
        
        
    }
    return result;
    
}





/* [] END OF FILE */

This README contains the relevant files I edited in this project, the files for the ADC conversion did not changev (you can find them in the Excercise 3). 
This file contains the main.c the OneWire.h and OneWire.c
```
/******************************************************************************
* File Name: main.c
*
* Version: 1.11
*
* Description: This is the source code for the ADC and UART code example.
*
* Related Document: CE195277_ADC_and_UART.pdf
*
* Hardware Dependency: See CE195277_ADC_and_UART.pdf
*
* Note:
* EDITED BY: Andreas BOTH
* DATE: 2024-04-19
* COURSE ID:tx00db04
* DESCRIPTION:  changed the output, such that the ADC temperature value and corresponding
*               temperature in Celsius coming from a LM35DZ temperature
*               sensor is transmitted via UART as JSON data script.
*               As well as implementing an OneWire interface, for communication wit the Dallas18B20
*               temperature sensor also transmitted via UART JSON data script. 
*
*******************************************************************************
* Copyright (2018-2020), Cypress Semiconductor Corporation. All rights reserved.
*******************************************************************************
* This software, including source code, documentation and related materials
* (“Software”), is owned by Cypress Semiconductor Corporation or one of its
* subsidiaries (“Cypress”) and is protected by and subject to worldwide patent
* protection (United States and foreign), United States copyright laws and
* international treaty provisions. Therefore, you may use this Software only
* as provided in the license agreement accompanying the software package from
* which you obtained this Software (“EULA”).
*
* If no EULA applies, Cypress hereby grants you a personal, nonexclusive,
* non-transferable license to copy, modify, and compile the Software source
* code solely for use in connection with Cypress’s integrated circuit products.
* Any reproduction, modification, translation, compilation, or representation
* of this Software except as specified above is prohibited without the express
* written permission of Cypress.
*
* Disclaimer: THIS SOFTWARE IS PROVIDED AS-IS, WITH NO WARRANTY OF ANY KIND, 
* EXPRESS OR IMPLIED, INCLUDING, BUT NOT LIMITED TO, NONINFRINGEMENT, IMPLIED 
* WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE. Cypress 
* reserves the right to make changes to the Software without notice. Cypress 
* does not assume any liability arising out of the application or use of the 
* Software or any product or circuit described in the Software. Cypress does 
* not authorize its products for use in any products where a malfunction or 
* failure of the Cypress product may reasonably be expected to result in 
* significant property damage, injury or death (“High Risk Product”). By 
* including Cypress’s product in a High Risk Product, the manufacturer of such 
* system or application assumes all risk of such use and in doing so agrees to 
* indemnify Cypress against all liability.
*******************************************************************************/

#include <project.h>
#include "OneWire.c"
#include "stdio.h"

/* Project Defines */
#define FALSE  0
#define TRUE   1
#define TRANSMIT_BUFFER_SIZE  50
#define BYTE_SIZE 8
#define HALF_BYTE_SIZE 4




/*Global Variables*/
/*used to store the sum of the input over the duration of one second
64 Bit value that way you can add 16 Bit into it about 2^48 times*/
volatile uint64 gInputSum = 0;
volatile uint32 gAverageData = 0;
volatile uint8 gSendAverage = 0; //Flag set every second

/*******************************************************************************
* Function Name: main
********************************************************************************
*
*
*******************************************************************************/
int main()
{
    /* Variable to store ADC result */
    uint16 temperatureIntPart;      //stores the integer value of the temperature
    uint16 temperatureDecimalPart;  //stores the decimal part of the temperature
    
    uint16 oneWireResult = 0;
    
    /* Transmit Buffer */
    char TransmitBuffer[TRANSMIT_BUFFER_SIZE];
    
    /* Start the components */
    ADC_DelSig_1_Start();
    UART_1_Start();
    
    isr_eoc_getValue_ClearPending();
    isr_eoc_getValue_Start();
    
    isr_averageData_ClearPending();
    isr_averageData_Start();
    
    CyGlobalIntEnable
    /* Initialize Variables */
    gSendAverage = FALSE;
    
    /* Start the ADC conversion */
    ADC_DelSig_1_StartConvert();
    
    /* Send message to verify COM port is connected properly */
    UART_1_PutString("COM Port Open\r\n");
    
    
    for(;;)
    {
            if(gSendAverage)
            {
             /* Format ADC result for transmition */
             CyGlobalIntDisable
             /*start OneWire communication*/
      
                if(!OneWireReset()) //Check for reachable devices
                {
                    UART_1_PutString("\r\nError no device found!"); 
                }else
                {
                    OneWireWriteByte(SKIP_ROM);                         //Address all devices (just one)
                    
                    OneWireWriteByte(READ_SCRATCHPAD);                  //Write read command
                    for(int i=0; i<2;i++)
                    {   
                        oneWireResult |= (OneWireReadByte()<<(i*BYTE_SIZE));
                    }
                    if(!OneWireReset())                                 //check if device is still connected
                    {
                        UART_1_PutString("\r\nError device disconnected!");
                    }
                    else{
     
                    OneWireWriteByte(SKIP_ROM);
                    OneWireWriteByte(START_TEMP_COMVERSION); 
                    }
                    
                }
                
             temperatureIntPart     = gAverageData / 10;    //drops the decimal part of the temperature
             temperatureDecimalPart = gAverageData % 10;    //drops the integer part of the temperature
             sprintf(TransmitBuffer, "{\r\n \"Voltage [mV]\" : %lu,\r\n \"Temperature [C]\" : %hu.%hu \r\n \"Temperature OneWire [C]\" : %i.%i\r\n}\r\n",\
                     gAverageData,
                    temperatureIntPart,
                    temperatureDecimalPart,
                    (oneWireResult>>HALF_BYTE_SIZE), //drop last 4 bits, they are the fractional part
                    (oneWireResult&0x000f));         //drop all bits despite fractional part
            
            CyGlobalIntEnable
             /* Send out the data */
             UART_1_PutString(TransmitBuffer);
             /* Reset the sendAverage flag */
             oneWireResult=0;
             gSendAverage = FALSE;
        }        
    }
}


/* [] END OF FILE */



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

```

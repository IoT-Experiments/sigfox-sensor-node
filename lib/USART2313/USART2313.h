/*
  USART2313.h - Arduino Library for the ATTiny2313 USART.
  Created by Anthony K. Norman, March 25, 2013.
  anthony.k.norman@gmail.com
  Released into the public domain.
*/

#ifndef USART2313_h
#define USART2313_h

#include "Arduino.h"

class USART2313
{
    public:
        USART2313();
        void begin(unsigned int baudrate, bool doubleUSARTSpeed = false);
        void write( unsigned char data );
        void printHexByte( unsigned char data );
        void printBinByte( unsigned char data );
        void print (char *string);
        void println (char *string);
        void waitForEmptyTxBuffer();
};

//extern USART2313 Serial;

#endif

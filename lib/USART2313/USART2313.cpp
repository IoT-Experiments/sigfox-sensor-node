/*
  USART2313.cpp - Arduino Library for the ATTiny2313 USART.
  Created by Anthony K. Norman, March 25, 2013.
  anthony.k.norman@gmail.com
  Released into the public domain.
*/

#include "USART2313.h"


//*****************************************************
//*****************************************************
//************* USART2313 ROUTINES ************************
//*****************************************************
//*****************************************************

USART2313::USART2313(){
}

void USART2313::begin(unsigned int baudrate, bool doubleUSARTSpeed)
{
    // Double the USART Transmission Speed
    if(doubleUSARTSpeed) {
      UCSRA |= _BV(U2X);
    } else {
      UCSRA &= ~_BV(U2X);      
    }

    const unsigned long factor = ( (UCSRA & _BV(U2X)) == 0) ? 16UL: 8UL;
    const unsigned int UBRR = (F_CPU / (factor * baudrate)) - 1;
    UBRRH = (unsigned char) (UBRR >> 8);
    UBRRL = (unsigned char) UBRR;
    UCSRB |= _BV(TXEN); // enable TX
    //UCSRB |= _BV(RXEN);  // enable RX

    // - Set frame format
    // 1 stop bit
    UCSRC &= _BV(USBS);
    // 8 Bit
    UCSRC |= _BV(UCSZ0);
    UCSRC |= _BV(UCSZ1);
    UCSRB &= ~_BV(UCSZ2);
    // Parity disabled
    //UCSRC |= _BV(UPM0);
    //UCSRC |= _BV(UPM1);
    UCSRC &= ~_BV(UPM0);
    UCSRC &= ~_BV(UPM1);
}


void USART2313::write( unsigned char data )
{
    // Wait for empty transmit buffer
    waitForEmptyTxBuffer();
    // Put data into buffer, sends the data
    UDR = data;
}

void USART2313::waitForEmptyTxBuffer( )
{
    while ( !( UCSRA & (1<<UDRE)) );
}

void USART2313::printHexByte( unsigned char data ){
    char nibble = (data >> 4);
    nibble += 0x30;
    if (nibble>0x39) nibble += 7;
    write( nibble );

    nibble = (data & 0x0F);
    nibble += 0x30;
    if (nibble>0x39) nibble += 7;
    write( nibble );
}

void USART2313::printBinByte( unsigned char data ){
    for (byte i=7; i<255; i--){
        char nibble = (data >> i);
        nibble = nibble % 2;
        nibble += 0x30;
        write( nibble );
    }
}

void USART2313::print (char *string)
{
   int count = 0;
   while (string[count] != 0)
   {
        write(string[count]);
        count++;
   }
}

void USART2313::println (char *string)
{
    print(string);
    write(0x0D);
    write(0x0A);
}

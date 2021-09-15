// Copyright (c) 2021 Tara Keeling
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <Arduino.h>
#include "shift.h"

static inline void InitOutputWithLevel( int Pin, int Level ) {
    if ( Pin >= 0 ) {
        pinMode( Pin, OUTPUT );
        digitalWrite( Pin, Level );
    }
}

static inline void WriteWrapper( int Pin, int Level ) {
    if ( Pin >= 0 ) {
        digitalWrite( Pin, Level );
    }
}

ShiftOut::ShiftOut( SPIClass* _Bus, int _Pin_Latch, int _Pin_Enable, int _Pin_Clear ) {
    this->Pin_Enable = _Pin_Enable;
    this->Pin_Clear= _Pin_Clear;
    this->Pin_Latch = _Pin_Latch;
    this->Bus = _Bus;
}

void ShiftOut::Begin( void ) {
    InitOutputWithLevel( this->Pin_Enable, HIGH );
    InitOutputWithLevel( this->Pin_Clear, LOW );
    InitOutputWithLevel( this->Pin_Latch, LOW );

    this->Bus->begin( );
    this->Bus->setClockDivider( 8 );
}

void ShiftOut::Enable( void ) {
    WriteWrapper( this->Pin_Enable, LOW );
}

void ShiftOut::Disable( void ) {
    WriteWrapper( this->Pin_Enable, HIGH );
}

void ShiftOut::Clear( void ) {
    WriteWrapper( this->Pin_Clear, LOW );
        delayMicroseconds( 1 );
    WriteWrapper( this->Pin_Clear, HIGH );
}

void ShiftOut::Latch( void ) {
    WriteWrapper( this->Pin_Latch, LOW );
    delayMicroseconds( 1 );

    WriteWrapper( this->Pin_Latch, HIGH );
    delayMicroseconds( 1 );

    WriteWrapper( this->Pin_Latch, LOW );
    delayMicroseconds( 1 );
}

void ShiftOut::Write( uint8_t* Data, int Length ) {
    Bus->transfer( Data, Length );
    Latch( );
}

ShiftOut& ShiftOut::operator<<( uint8_t Data ) {
    Write( &Data, sizeof( uint8_t ) );
    return *this;
}

ShiftOut& ShiftOut::operator<<( uint16_t Data ) {
    Write( ( uint8_t* ) &Data, sizeof( uint16_t ) );
    return *this;
}

ShiftOut& ShiftOut::operator<<( uint32_t Data ) {
    Write( ( uint8_t* ) &Data, sizeof( uint32_t ) );
    return *this;
}

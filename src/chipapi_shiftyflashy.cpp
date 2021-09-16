// Copyright (c) 2021 Tara Keeling
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#include <Arduino.h>
#include <SPI.h>
#include "chipapi.h"

static bool ShiftyFlashy_Open( void );
static void ShiftyFlashy_Close( void );
static uint8_t ShiftyFlashy_Read( uint32_t Address );
static uint8_t ShiftyFlashy_Write( uint32_t Address, uint8_t Data );
static void InitOutputWithLevel( int Pin, int Level );
static void SetDataBus_Input( void );
static void SetDataBus_Output( void );
static void PinChangeWithDelay( int Pin, int Level, uint32_t DelayUS );
static void SetAddress( uint16_t Address );
static void ShiftyFlashy_SetChip( struct ChipDescription* ChipInfo );
static uint8_t GetDataBus( void );
static void SetDataBus( const uint8_t Data );

static SPIClass SPI_5V( PB15, PB14, PB13 );

// 3.3V Logic pins should still register as logic HIGH
// on the 5v powered ICs.
static const int Pin_Latch = PA0;
static const int Pin_OE = PA1;
static const int Pin_WE = PA2;

static uint32_t LatchDelay = 100; // Actually should be ~20ns

struct ChipDescription* Chip = NULL;

static uint16_t CurrentAddress = 0x0000;

#define NSecsPerClock floor( 1.0 / ( double ) F_CPU )

// These MUST be on 5v tolerant IOs!!!
static const int DataBusPins[ 8 ] = {
    PA8,    // D0
    PA9,    // D1
    PA10,   // D2
    PA15,   // D3
    PB3,    // D4
    PB4,    // D5
    PB6,    // D6
    PB7     // D7
};

struct ParallelChipAPI ShiftyFlashyAPI = {
    ShiftyFlashy_Open,
    ShiftyFlashy_Close,
    ShiftyFlashy_SetChip,
    ShiftyFlashy_Read,
    ShiftyFlashy_Write
};

static void ShiftyFlashy_SetChip( struct ChipDescription* ChipInfo ) {
    Chip = ChipInfo;
}

static bool ShiftyFlashy_Open( void ) {
    SPI_5V.begin( );
    SPI_5V.setClockDivider( 16 );

    InitOutputWithLevel( Pin_Latch, LOW );
    InitOutputWithLevel( Pin_OE, HIGH );
    InitOutputWithLevel( Pin_WE, HIGH );

    SetAddress( CurrentAddress );
    SetDataBus_Input( );

    return true;
}

static void ShiftyFlashy_Close( void ) {
    SPI_5V.end( );
    SetDataBus_Input( );
}

static uint8_t ShiftyFlashy_Read( uint32_t Address ) {
    uint8_t Result = 0;

    noInterrupts( );
    SetDataBus_Input( );

    PinChangeWithDelay( Pin_OE, HIGH, Chip->Timing.OutputDisableTime );
        SetAddress( ( uint16_t ) Address );
    PinChangeWithDelay( Pin_OE, LOW, Chip->Timing.OutputEnableTime );

    interrupts( );

    Result = GetDataBus( );

    return Result;
}

static uint8_t GetDataBus( void ) {
    uint8_t Result = 0;

    Result |= ( digitalRead( DataBusPins[ 7 ] ) == HIGH ) ? 0x80 : 0x00;
    Result |= ( digitalRead( DataBusPins[ 6 ] ) == HIGH ) ? 0x40 : 0x00;
    Result |= ( digitalRead( DataBusPins[ 5 ] ) == HIGH ) ? 0x20 : 0x00;
    Result |= ( digitalRead( DataBusPins[ 4 ] ) == HIGH ) ? 0x10 : 0x00;
    Result |= ( digitalRead( DataBusPins[ 3 ] ) == HIGH ) ? 0x08 : 0x00;
    Result |= ( digitalRead( DataBusPins[ 2 ] ) == HIGH ) ? 0x04 : 0x00;
    Result |= ( digitalRead( DataBusPins[ 1 ] ) == HIGH ) ? 0x02 : 0x00;
    Result |= ( digitalRead( DataBusPins[ 0 ] ) == HIGH ) ? 0x01 : 0x00;

    return Result;
}

static void SetDataBus( const uint8_t Data ) {
    digitalWrite( DataBusPins[ 7 ], ( Data & 0x80 ) ? HIGH : LOW );
    digitalWrite( DataBusPins[ 6 ], ( Data & 0x40 ) ? HIGH : LOW );
    digitalWrite( DataBusPins[ 5 ], ( Data & 0x20 ) ? HIGH : LOW );
    digitalWrite( DataBusPins[ 4 ], ( Data & 0x10 ) ? HIGH : LOW );
    digitalWrite( DataBusPins[ 3 ], ( Data & 0x08 ) ? HIGH : LOW );
    digitalWrite( DataBusPins[ 2 ], ( Data & 0x04 ) ? HIGH : LOW );
    digitalWrite( DataBusPins[ 1 ], ( Data & 0x02 ) ? HIGH : LOW );
    digitalWrite( DataBusPins[ 0 ], ( Data & 0x01 ) ? HIGH : LOW );
}

static uint8_t ShiftyFlashy_Write( uint32_t Address, uint8_t Data ) {
    int Timeout = 100;
    uint8_t Result = 0;
    uint8_t Temp = 0;

    // Disable chip outputs
    PinChangeWithDelay( Pin_OE, HIGH, Chip->Timing.OutputDisableTime );

    // Setup data and address lines
    SetAddress( ( uint16_t ) Address );

    noInterrupts( );
        SetDataBus_Output( );
        SetDataBus( Data );

        // Toggle write pulse
        PinChangeWithDelay( Pin_WE, LOW, Chip->Timing.WritePulseTime );
        PinChangeWithDelay( Pin_WE, HIGH, Chip->Timing.WriteCycleTime );

        SetDataBus_Input( );

        do {
            PinChangeWithDelay( Pin_OE, LOW, Chip->Timing.OutputEnableTime );
                Temp = GetDataBus( );
            PinChangeWithDelay( Pin_OE, HIGH, Chip->Timing.OutputDisableTime );

            PinChangeWithDelay( Pin_OE, LOW, Chip->Timing.OutputEnableTime );
                Result = GetDataBus( );
            PinChangeWithDelay( Pin_OE, HIGH, Chip->Timing.OutputDisableTime );

            delayMicroseconds( 1 );
        } while ( Temp != Result != Data && Timeout-- );
    interrupts( );

    return Result;
}

static void InitOutputWithLevel( int Pin, int Level ) {
    digitalWrite( Pin, Level );
    pinMode( Pin, OUTPUT );
}

static void SetDataBus_Input( void ) {
    int i = 0;

    for ( i = 0; i < 8; i++ ) {
        pinMode( DataBusPins[ i ], INPUT );
    }
}

static void SetDataBus_Output( void ) {
    int i = 0;

    for ( i = 0; i < 8; i++ ) {
        InitOutputWithLevel( DataBusPins[ i ], LOW );
    }
}

static void PinChangeWithDelay( int Pin, int Level, uint32_t DelayUS ) {
    digitalWrite( Pin, Level );

    if ( DelayUS ) {
        delayMicroseconds( DelayUS );
    }
}

static void SetAddress( uint16_t Address ) {
    // Don't unnecessarily shift out an address if it's already
    // in the shift registers.
    if ( CurrentAddress != Address ) {
        //SPI_5V.beginTransaction( SPISettings( Chip->Timing.ClockFreq, MSBFIRST, SPI_MODE0 ) );
            SPI_5V.transfer( ( Address >> 8 ) & 0xFF );
            SPI_5V.transfer( Address & 0xFF );
        //SPI_5V.endTransaction( );

        PinChangeWithDelay( Pin_Latch, HIGH, LatchDelay );
        PinChangeWithDelay( Pin_Latch, LOW, LatchDelay );
    }

    CurrentAddress = Address;
}

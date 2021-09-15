// Copyright (c) 2021 Tara Keeling
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#pragma once

#include <Arduino.h>
#include <SPI.h>

class ShiftOut {
    private:
        SPIClass* Bus;

        int Pin_Enable;
        int Pin_Clear;
        int Pin_Latch;

        void Delay( uint32_t Time );
        void Clock( void );
        void Latch( void );

    public:
        ShiftOut( SPIClass* _Bus, int _Pin_Latch, int _Pin_Enable, int _Pin_Clear );

        void Begin( void );

        void Clear( void );
        void Disable( void );
        void Enable( void );

        void Write( uint8_t* Data, int Length );

        ShiftOut& operator<<( uint8_t Data );
        ShiftOut& operator<<( uint16_t Data );
        ShiftOut& operator<<( uint32_t Data );
};

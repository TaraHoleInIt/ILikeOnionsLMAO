// Copyright (c) 2021 Tara Keeling
// 
// This software is released under the MIT License.
// https://opensource.org/licenses/MIT

#ifndef _CHIPAPI_H_
#define _CHIPAPI_H_

#include <stdbool.h>
#include <stdint.h>

struct ChipDescription {
    int ChipSize;

    struct {
        uint32_t ClockFreq;
        uint32_t WriteCycleTime;
        uint32_t WritePulseTime;
        uint32_t OutputEnableTime;
        uint32_t OutputDisableTime;
        uint32_t AddressToDataValidTime;
    } Timing;
};

struct ParallelChipAPI {
    bool ( *Open ) ( void );
    void ( *Close ) ( void );

    void ( *SetChip ) ( struct ChipDescription* Chip );

    uint8_t ( *Read ) ( uint32_t Address );
    uint8_t ( *Write ) ( uint32_t Address, uint8_t Value );
};

extern struct ParallelChipAPI ShiftyFlashyAPI;

#endif

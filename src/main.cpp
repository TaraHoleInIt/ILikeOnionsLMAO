#include <Arduino.h>
#include <CRC32.h>
#include "chipapi.h"
#include "rom.h"

void setup( void ) {
	Serial.begin( 115200 );

	while ( ! Serial )
	;
}

struct ParallelChipAPI* Programmer = &ShiftyFlashyAPI;
struct ChipDescription ROM = {
	.ChipSize = 8192,
	// FIXME
	// No easy nanosecond level timing
	.Timing = {
		.ClockFreq = 4000000,			// 4MHz (test)
		.WriteCycleTime = 1000,			// 1ms
		.WritePulseTime = 1,			// 1us/1000ns
		.OutputEnableTime = 1,			// 1us/100ns
		.OutputDisableTime = 1,			// 1us/60ns
		.AddressToDataValidTime = 1,	// 1us/250ns
	}
};

void Erase( void ) {
	uint8_t Result = 0;
	bool Pass = true;
	int i = 0;

	for ( i = 0; i < ROM.ChipSize && Pass == true; i++ ) {
		if ( ( i % 64 ) == 0 ) {
			Serial.print( "." );
		}

		Pass = ( ( Result = Programmer->Write( i, 0xFF ) ) == 0xFF );
	}

	Serial.println( );

	if ( Pass == false ) {
		Serial.println( "*** FATAL ***" );
		Serial.print( "Failed to erase location 0x" );
		Serial.println( i, 16 );
		Serial.println( Result, 16 );

		while ( true ) {
		}
	}
}

void Write( const uint8_t* Buffer, int Length ) {
	uint8_t Data = 0;
	bool Pass = true;
	int i = 0;

	Serial.print( "Writing..." );

	for ( i = 0; i < Length && i < ROM.ChipSize && Pass == true; i++ ) {
		if ( ( i % 64 ) == 0 ) {
			Serial.print( "." );
		}

		Pass = ( ( Data = Programmer->Write( i, Buffer[ i ] ) ) == Buffer[ i ] );
	}

	if ( Pass == false ) {
		Serial.println( "\n*** FATAL ***" );
		Serial.print( "Failed to write location 0x" );
		Serial.println( i, 16 );

		while ( true ) {
		}
	}

	Serial.println( "Done." );
}

void loop( void ) {
	CRC32 Result;
	int i = 0;

	Programmer->Open( );
	Programmer->SetChip( &ROM );

#if 0
	Erase( );
	Write( __64kBASIC31A_bin, __64kBASIC31A_bin_len );
#else
	Result.reset( );
	
	for ( i = 0; i < ROM.ChipSize; i++ ) {
		Serial.write( Programmer->Read( i ) );
		//Result.update( Programmer->Read( i ) );
	}

	//Serial.print( "ROM CRC32: 0x" );
	//Serial.println( Result.finalize( ), 16 );
#endif

	while ( true ) {
	}
}

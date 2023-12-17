/*-------------------------------------------------------------------------------------*/

#define CRAFT_NAME "UAVWARE" // Used as filename of the converted logfile.

// General stuff
float dt;
unsigned long current_time, prev_time;

// 100 bytes maximum at 2 Mbit/s UART speed and a logged data frame every 0.5 ms.
static uint8_t bb_buffer[ 93 ] = "FRAME"; // 98 bytes practical limit

void log_buffer( uint8_t buffer[], size_t size )
{
	Serial.write( buffer, size );
}

void blackbox_log( void )
{
	static uint32_t bb_iteration;

	static bool writeCraftOnce = true;
	if( writeCraftOnce ) {
		writeCraftOnce = false;
		static uint8_t buffer[] = "CRAFT" CRAFT_NAME;
		log_buffer( buffer, sizeof( buffer ) );
		return;
	}

	int pos = 5; // size of FRAME

	// iteration
	*(uint32_t*)( &bb_buffer[ pos ] ) = bb_iteration;
	pos += 4;
	// time (not used for FFT, just for time displaying)
	*(uint32_t*)( &bb_buffer[ pos ] ) = micros();
	pos += 4;
	// axisP
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// axisI
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// axisD
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// axisF
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// rcCommand
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(uint16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// setpoint
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// vbatLatest
	*(uint16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// amperageLatest
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// rssi
	*(uint16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// gyroADC
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// accSmooth
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// debug
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// motor
	*(uint16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2; // BR
	*(uint16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2; // FR
	*(uint16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2; // BL
	*(uint16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2; // FL
	// pos is 85
#if 1 // select between logging motor Hz (1) or sdft notch Hz (0)
	// #ifdef RPM_FILTER
	// extern float motor_hz[ 4 ];
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// #endif // RPM_FILTER
#else
	#ifdef BIQUAD_SDFT_NOTCH
	extern float sdft_notch_Hz[ 4 ];
	*(int16_t*)( &bb_buffer[ pos ] ) = sdft_notch_Hz[ 0 ] * 10.0f;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = sdft_notch_Hz[ 1 ] * 10.0f;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = sdft_notch_Hz[ 2 ] * 10.0f;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = sdft_notch_Hz[ 3 ] * 10.0f;
	pos += 2;
	#endif // BIQUAD_SDFT_NOTCH
#endif
	// pos is 93
	++bb_iteration;

	log_buffer( bb_buffer, sizeof( bb_buffer ) );
}
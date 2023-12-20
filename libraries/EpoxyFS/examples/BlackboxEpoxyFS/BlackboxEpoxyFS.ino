/*
Test writing and reading to the filesystem using EpoxyFS on EpoxyDuino, and
LittleFS on ESP8266 and ESP32. The expected output on various platforms is shown
below:

*/

#include <stdio.h> // remove()
#include <Arduino.h>

#if defined( EPOXY_DUINO )
	#define FILE_SYSTEM_NAME "EpoxyFS"
	#include <ftw.h> // nftw()
	#include <EpoxyFS.h>
	#define FILE_SYSTEM fs::EpoxyFS
using fs::Dir;
using fs::File;
#else
	#error Unsupported platform
#endif

#define TEST_FILE_NAME "/LOG000.TXT"

#if defined( EPOXY_DUINO )

int removeFile( const char* fpath, const struct stat* /*sb*/, int typeflag, struct FTW* ftwbuf )
{
	if( typeflag == FTW_F ) {
		printf( "File: %s\n", fpath );
	}
	else if( typeflag == FTW_SL ) {
		printf( "Symlink: %s\n", fpath );
	}
	if( typeflag == FTW_DP ) {
		printf( "Post Dir: %s\n", fpath );
	}
	int status = 0;
	if( ftwbuf->level != 0 ) {
		status = remove( fpath );
	}
	return status;
}

// Maximum number of open file descriptors.
static const int MAX_NOPEN_FD = 5;

// Recursively remove files under 'epoxyfsdata' directory using nftw().
void removeDir()
{
	Serial.println( "== Recursively remove '/' using nftw()" );
	nftw( "epoxyfsdata", removeFile, MAX_NOPEN_FD, FTW_PHYS | FTW_MOUNT | FTW_DEPTH );
}

#else

void removeDir()
{
	Serial.println( "== Recursively remove '/' not implemented" );
}

#endif

void listDir()
{
#if defined( ESP32 )
	Serial.println( "== List '/' not implemented on ESP32" );

#else

	Serial.println( "== List '/' using fs::Dir" );

	// Open dir folder
	Dir dir = FILE_SYSTEM.openDir( "/" );

	int count = 1;
	// Cycle all the content
	while( dir.next() ) {
		Serial.print( "Dir entry #" );
		Serial.println( count );

		// Print info from directory entry
		Serial.print( "Dir: fileName()=" );
		Serial.println( dir.fileName() );
		if( dir.isDirectory() ) {
			Serial.println( "isDirectory=true" );
		}
		else {
			Serial.println( "isDirectory=false" );
		}
		Serial.print( "fileSize()=" );
		Serial.println( dir.fileSize() );

		// Print info from File object.
		Serial.print( "File: " );
		File f = dir.openFile( "r" );
		Serial.print( "name()=" );
		Serial.println( f.name() );
	#if defined( ESP32 )
		Serial.print( "path()=" );
		Serial.println( f.path() );
	#else
		Serial.print( "fullName()=" );
		Serial.println( f.fullName() );
	#endif
		Serial.print( "size()=" );
		Serial.println( f.size() );
		f.close();

		count++;
	}
#endif
}

void writeFile()
{
	Serial.println( "== Writing " TEST_FILE_NAME );

	File f = FILE_SYSTEM.open( TEST_FILE_NAME, "w" );
	f.println( "This is a test" );
	f.println( 42 );
	f.println( 42.0 );
	f.println( 42, 16 );
	f.close();

	bool exists = FILE_SYSTEM.exists( TEST_FILE_NAME );
	if( exists ) {
		Serial.println( "Created " TEST_FILE_NAME );
	}
	else {
		Serial.println( "ERROR creating " TEST_FILE_NAME );
	}
}

void readFile()
{
	Serial.println( "== Reading " TEST_FILE_NAME );

	File f = FILE_SYSTEM.open( TEST_FILE_NAME, "r" );
	Serial.print( "name(): " );
	Serial.println( f.name() );
#if defined( ESP32 )
	Serial.print( "path()=" );
	Serial.println( f.path() );
#else
	Serial.print( "fullName(): " );
	Serial.println( f.fullName() );
#endif
	while( f.available() ) {
		String s = f.readStringUntil( '\r' );
		Serial.print( s );
		f.read();
		Serial.println();
	}
	f.close();
}

/*-------------------------------------------------------------------------------------*/

#define CRAFT_NAME "UAVWARE" // Used as filename of the converted logfile.
File f;

// General stuff
float dt;
unsigned long current_time, prev_time;

// 100 bytes maximum at 2 Mbit/s UART speed and a logged data frame every 0.5 ms.
static uint8_t bb_buffer[ 93 ] = "FRAME"; // 98 bytes practical limit

void log_buffer( uint8_t buffer[], size_t size )
{
	Serial.write( buffer, size );
	f.write( buffer, size );
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
	*(int16_t*)( &bb_buffer[ pos ] ) = 200;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(uint16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// setpoint
	*(int16_t*)( &bb_buffer[ pos ] ) = -100;
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
	// gyroADC - Gyro scale int16_t -1 = 1 deg/sec
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	*(int16_t*)( &bb_buffer[ pos ] ) = 0;
	pos += 2;
	// accSmooth - Acc scale int16_t 2048 = 1g
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
	// motor - scale 1000 = 100%
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

void loopRate( int freq )
{
	// DESCRIPTION: Regulate main loop rate to specified frequency in Hz
	/*
	 * It's good to operate at a constant loop rate for filters to remain stable and whatnot.
	 */
	float invFreq = 1.0 / freq * 1000000.0;
	unsigned long checker = micros();

	// Sit in loop until appropriate time has passed
	while( invFreq > ( checker - current_time ) ) {
		checker = micros();
	}
}

/*---------------------------------------------------------------------------------------------*/

void setup()
{
#if !defined( EPOXY_DUINO )
	delay( 1000 ); // some boards reboot twice
#endif

	Serial.begin( 115200 );
	while( !Serial )
		; // For Leonardo/Micro
#if defined( EPOXY_DUINO )
	Serial.setLineModeUnix();
#endif

	Serial.println( "== Initializing File System: " FILE_SYSTEM_NAME );
	if( !FILE_SYSTEM.begin() ) {
		Serial.println( "ERROR initializing file system." );
		exit( 1 );
	}

	Serial.println( "== Formatting file system" );
	if( !FILE_SYSTEM.format() ) {
		Serial.println( "ERROR formatting file system." );
		exit( 1 );
	}

	// listDir();
	// writeFile();
	// listDir();
	// readFile();
	removeDir();
	listDir();

	Serial.println( "== Writing " TEST_FILE_NAME );

	f = FILE_SYSTEM.open( TEST_FILE_NAME, "w" );

	// Serial.println( F( "== Done" ) );

	// #if defined( EPOXY_DUINO )
	// 	exit(0);
	// #endif
}

void loop()
{
	static uint32_t bb_iteration = 0;

	bb_iteration++;

	// Keep track of what time it is and how much time has elapsed since the last loop
	prev_time = current_time;
	current_time = micros();
	dt = ( current_time - prev_time ) / 1000000.0;

	blackbox_log();

	// Regulate loop rate
	loopRate( 100 ); // Hz

	if( bb_iteration == 100 ) {
		f.close();
		exit( 0 );
	}
}

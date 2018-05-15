/******************************************************************************
 *	Pololu TReX power amplifier API
 *	JG, may 2018
 *	To access serial port as user : sudo usermod -a -G dialout $USER 
 *	(logout needed)
 *****************************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <termios.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <strings.h>

  
#define TREX_BAUDRATE 			B115200
#define TREX_MODEMDEVICE 		"/dev/ttyUSB0"
#define TREX_READ_TIMEOUT		5						// Tenth of second
#define TREX_SIGNATURE_SIZE		9						// Size of TReX signature
#define TREX_SIGNATURE			"TReXJr"				// Prefix of the signature
#define TREX_DEFAULT_ID			7						// Default device ID

#define TREX_EXT_PROTOCOL		0x80
#define TREX_GET_SIGNATURE		0x81

int								trex_fd = -1;			// Serial port file descriptor
struct termios 					trex_oldtio;			// Backup of old configuration

/******************************************************************************
 *	trex_init_port : initialize serial port
 *****************************************************************************/
int trex_init_port( void )	{
	int 			res;
	struct termios 	newtio;

	/* Open device */
	trex_fd = open( TREX_MODEMDEVICE, O_RDWR | O_NOCTTY ); 
	if ( trex_fd < 0 )  { 
		perror( TREX_MODEMDEVICE ); 
		return -1; 
		}

	/* Save current port settings */
	tcgetattr( trex_fd, &trex_oldtio ); 

	/* Define new settings */
	bzero( &newtio, sizeof(newtio) );
	cfmakeraw( &newtio );
	newtio.c_cflag = TREX_BAUDRATE | CS8 | CLOCAL | CREAD;
	newtio.c_iflag = IGNPAR;
	newtio.c_oflag = 0;

	/* Set input mode (non-canonical, no echo,...) */
	newtio.c_lflag = 0;

	/* Inter-character timer  */
	newtio.c_cc[VTIME] = TREX_READ_TIMEOUT;   
	newtio.c_cc[VMIN] = 0;

	/* Apply the settings */
	tcflush( trex_fd, TCIFLUSH );
	tcsetattr( trex_fd, TCSANOW, &newtio );

	return 0;
}

/******************************************************************************
 *	trex_init_port : release serial port
 *****************************************************************************/
void trex_release_port( void )	{

	/* Restore initial settings */
	tcsetattr( trex_fd, TCSANOW, &trex_oldtio );
	close( trex_fd );
	trex_fd = -1;
}

/******************************************************************************
 *	trex_check_presence : send a get signature command and check for the result
 *	0 : no TReX detected
 *	1 : TReX detected
 *****************************************************************************/
int trex_check_presence( unsigned char id )	{
	char 			buf[TREX_SIGNATURE_SIZE+1];
	unsigned char 	cmd;
	int				res;
	
	if ( trex_fd == -1 )
		return 0;
	
	/* Send ext protocol cmd */
	cmd = TREX_EXT_PROTOCOL;
	res = write( trex_fd, &cmd, 1 );
	if ( res < 0 )  { 
		perror( TREX_MODEMDEVICE ); 
		return 0; 
		}
	
	/* Send ID */
	cmd = id;
	res = write( trex_fd, &cmd, 1 );
	if ( res < 0 )  { 
		perror( TREX_MODEMDEVICE ); 
		return 0; 
		}

	/* Send signature cmd */
	cmd = TREX_GET_SIGNATURE;
	res = write( trex_fd, &cmd, 1 );
	if ( res < 0 )  { 
		perror( TREX_MODEMDEVICE ); 
		return 0; 
		}

	/* Read signature */
	res = read( trex_fd, buf, TREX_SIGNATURE_SIZE );
	if ( res != TREX_SIGNATURE_SIZE )  { 
		perror( TREX_MODEMDEVICE ); 
		return 0; 
	}
	else	{
		buf[res] = 0;
		printf( "*** TReX signature: %s\n", buf );
		if ( strstr( buf, TREX_SIGNATURE ) )
			return 1;
		else
			return 0;
	}
}

/******************************************************************************
 *	trex_output : output control
 *	0 : no error
 *****************************************************************************/
int trex_output( int id, int channel, int value )	{

	
	return 0;
}

void main( void )	{
	
	if ( trex_init_port( ) )	{
		printf( "Error while initializing TReX port.\n" );
		exit( -1 );
	}

	if ( trex_check_presence( TREX_DEFAULT_ID ) )
		printf( "TReX detected.\n" );
	
	trex_release_port( );
}

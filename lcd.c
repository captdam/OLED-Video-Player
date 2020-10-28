#define CPU_FREQ 16000000

#define BAUD_RATE 9600

#define LCD_ADDR 0x3C
#define CONTRAST 128 //Default = 128 @ reset

/* Pin assignment
 *	PORT PIN TASK		PORT PIN TASK
 *	C6   1   Reset		C5   28  SCL
 *	D0   2   Rx		C4   27  SDA
 *	D1   3   Tx		C3   26  
 *	D2   4   		C2   25  
 *	D3   5   		C1   24  
 *	D4   6   		C0   23  
 *	VCC  7			GND  22
 *	GND  8			AREF 21
 *	B6   9   Crystal/NC	AVCC 20
 *	B7   10  Crystal/NC	B5   19  SCK
 *	D5   11  		B4   18  MISO
 *	D6   12  		B3   17  MOSI
 *	D7   13  		B2   16
 *	B0   14			B1   15  SDSelect
 */

#include <stdint.h>
#include <avr/io.h>
#include <avr/pgmspace.h>
#include <avr/interrupt.h>

#include "twi.h"

void sendUART(uint8_t data) {
	while (!( UCSR0A & (1<<UDRE0) )); //Wait until buffer empty
	UDR0 = data;
}

uint8_t image[1024]; //128*64 pixels = 128*64 bits = 128*8 bytes

void send1(uint8_t data) {
	startI2CMaster();
	setI2CMaster(LCD_ADDR, 0);
	writeI2CMaster(0x00);
	writeI2CMaster(data);
	stopI2CMaster();
}
void send2(uint8_t data1, uint8_t data2) {
	startI2CMaster();
	setI2CMaster(LCD_ADDR, 0);
	writeI2CMaster(0x00);
	writeI2CMaster(data1);
	writeI2CMaster(data2);
	stopI2CMaster();
}

void drawPage1() {
	uint8_t g = 1;
	for (uint8_t i = 0; i < 8; i++) {
		for (uint8_t j = 0; j < 16; j++) {
			writeI2CMaster(g);
		}
		g = g << 1;
	}
}
void drawPage2() {
	uint8_t g = 128;
	for (uint8_t i = 0; i < 8; i++) {
		for (uint8_t j = 0; j < 16; j++) {
			writeI2CMaster(g);
		}
		g = g >> 1;
	}
}

void startSPI() {
	PORTB &= 0b11111101; //Pull low PORTB1
}
void stopSPI() {
	PORTB |= 0b00000010; //Pull high PORTB1
}


#include "sd.h"

int main() {
	image[0] = 0;
	image[1] = 1;
	image[2] = 3;
	image[3] = 3;
	image[4] = 7;
	image[5] = 7;
	image[6] = 7;
	image[7] = 7;

	UBRR0 = CPU_FREQ/16/BAUD_RATE - 1; //UBRR0 = [UBRR0H:UBRR0L]
	UCSR0B = (1<<RXEN0) | (1<<TXEN0); //Enable Tx, Rx, Tx interrupt and Rx interrupt
	UCSR0C = (0<<USBS0) | (3<<UCSZ00); //Async USART (UART), no parity, 1 stop bit, 8 bits data

	DDRD = 0b00000000;
	DDRC = 0b00110000;
	DDRB = 0b00101010;

	PORTB = 0xFF;
	for (uint16_t i = 0; i < 10000; i++);

	initI2CMaster(40000);
	SPCR = (1<<SPE) | (1<<MSTR) | (2<<SPR0); //Enable SPI as master, no interrupt, MSB first, SCK low on idle, f_SPI = CPU_FREQ / 64 = 250kHz @ 16MHz CPU

	//Init OLED
	send2(0xA8, 64-1);	//MUX = 64, 64 horizontal lines (COM)
	send2(0xD3, 0x00);	//Display offset = 0
	send1(0x40);		//Display RAM start line = 0
	send1(0xA0);		//Segment remap off
	send1(0xC0);		//Scan direction = normal
	send2(0xDA, 0x12);	//COM pins: sequential COM pin config, disable remap

	send2(0x81, CONTRAST);	//Contrast control
	send1(0xA4);		//Turn on display, based on RAM (instead of all white)
	send1(0xA6);		//Normal/Inserse = Normal, turn on pixel if RAM = 1
	send2(0xD5, 0x80);	//Display clock: divide ratio = 1, oscillator frequency = default
	send2(0x8D, 0x14);	//Charge pump on
	send1(0xAF);		//Display on (instead of sleep)

	send1(0x2E);		//No scrolling
	send2(0x20, 2);		//Page addressing mode: vary long (128*64bits) communication is not good. Sending a line at one page at a time and reset segment address to sync the image

	//Clear OLED
	for (uint8_t page = 0; page < 8; page++) {
		send1(0xB0|page); //Set page address
		send1(0x00); //Reset segment pointer
		send1(0x10);

		startI2CMaster();
		setI2CMaster(LCD_ADDR, 0);
		writeI2CMaster(0x40);
		for (uint8_t segment = 0; segment < 128; segment++)
			writeI2CMaster( (segment > 120) ? 0xFF : 0x00 );
		stopI2CMaster();
	}

	uint8_t sdReady = initSDHC();
	switch (sdReady) {
		case 0x80: //CMD0 timeout
			*((uint32_t*)(&image[0])) = 0xFFFF00FF;
			*((uint32_t*)(&image[4])) = 0xFFFFFFFF;
			*((uint32_t*)(&image[8])) = 0xFF000000;
			break;
		case 0x90: //CMD8 timeout
			*((uint32_t*)(&image[0])) = 0xFFFF00FF;
			*((uint32_t*)(&image[4])) = 0x88888888;
			*((uint32_t*)(&image[8])) = 0xFF000000;
			break;
		case 0x91: //CMD8 invalid: SDCv1
			*((uint32_t*)(&image[0])) = 0xFFFF00FF;
			*((uint32_t*)(&image[4])) = 0x88888888;
			*((uint32_t*)(&image[8])) = 0xFF111111;
			break;
		case 0xA0: //CMD55 timeout
			*((uint32_t*)(&image[0])) = 0xFFFF00FF;
			*((uint32_t*)(&image[4])) = 0xF0FFF0FF;
			*((uint32_t*)(&image[8])) = 0xFF000000;
			break;
		case 0xB0: //ACMD41 timeout
			*((uint32_t*)(&image[0])) = 0xFFFF00FF;
			*((uint32_t*)(&image[4])) = 0xFF773311;
			*((uint32_t*)(&image[8])) = 0xFF000000;
			break;
//		default:
//			/* No action */
//			int a = 1 + 1;
	}

	//Read SD card
	
/*	switch (readBlock(image)) {
		case 0x01: //CMD17 timeout
			*((uint32_t*)(&image[0])) = 0xFFFF00FF;
			*((uint32_t*)(&image[4])) = 0x77777777;
			*((uint32_t*)(&image[8])) = 0x01030103;
			break;
		case 0x02: //Read token timeout
			*((uint32_t*)(&image[0])) = 0xFFFF00FF;
			*((uint32_t*)(&image[4])) = 0x77007700;
			*((uint32_t*)(&image[8])) = 0x01030103;
			break;
	}*/

	for (uint16_t frame = 0; frame < 500; frame++) {
		readBlock( (frame<<1) + 0, &(image[0]) );
		readBlock( (frame<<1) + 1, &(image[512]) );


		for (uint8_t page = 0; page < 8; page++) {
			send1(0xB0|page); //Set page address
			send1(0x00); //Reset segment pointer
			send1(0x10);

			startI2CMaster();
			setI2CMaster(LCD_ADDR, 0);
			writeI2CMaster(0x40);
			for (uint8_t segment = 0; segment < 128; segment++)
				writeI2CMaster( image[ page * 128 + segment ] );
			stopI2CMaster();
		}
		
		for (uint16_t d = 0; d < 5000; d++);
	}

	while(1);
}

// TODO: OLED screen sometimes get stall, need to fix this.
// TODO: Increase I2C and SPI speed. Refresh rate too low.
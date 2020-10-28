//Wait SPI: polling
void waitSPI() {
	while ( !(SPSR & (1<<SPIF)) );
}

//Send and receive a byte to SPI bus
uint8_t sendSPI(uint8_t data) {
	SPDR = data;
	while ( !(SPSR & (1<<SPIF)) ); //Wait until data send
	return SPDR;
}

//Send command to SD card (CS = PORTB1), return 0 if success
uint8_t sendCmdSDHC(uint8_t cmd, uint32_t argument, uint8_t crc, uint8_t expectACK) {
//	PORTB |= 0b00000010;
	sendSPI(0xFF);
	PORTB &= 0b11111101;
	sendSPI(0x40 | cmd);

	sendSPI( (argument >> 24) & 0xFF );
	sendSPI( (argument >> 16) & 0xFF );
	sendSPI( (argument >>  8) & 0xFF );
	sendSPI( (argument >>  0) & 0xFF );
	sendSPI(crc);

	while (sendSPI(0xFF) != expectACK); //Wait ACK
}

//Init SDHC card
uint8_t initSDHC() {
	uint8_t res, timeout;

	for (uint8_t i = 0; i < 20; i++) //Clock sync
		sendSPI(0xFF);

	sendSPI(0xFF);
	startSPI();
	sendSPI(0x40 | 0);
	sendSPI(0x00);
	sendSPI(0x00);
	sendSPI(0x00);
	sendSPI(0x00);
	sendSPI(0x95);
	timeout = 250;
	for (;;) {
		res = sendSPI(0xFF);
		if (res == 0x01) {
			break;
		}
		if (!timeout--) {
			return 0x80;
		}
	}
	stopSPI();

	sendSPI(0xFF);
	startSPI();
	sendSPI(0x40 | 8);
	sendSPI(0x00);
	sendSPI(0x00);
	sendSPI(0x01);
	sendSPI(0xAA);
	sendSPI(0x87);
	timeout = 250;
	for (;;) {
		res = sendSPI(0xFF);
		if (res == 0x01) {
			break;
		}
		if (res == 0x05) {
			return 0x91;
		}
		if (!timeout--) {
			return 0x90;
		}
	}
	stopSPI();
	
	uint8_t a1, a2, sdcReady;
	uint8_t acmd41Timeout = 250;
	for(;;) {
		sendSPI(0xFF);
		startSPI();
		sendSPI(0x40 | 55);
		sendSPI(0x00);
		sendSPI(0x00);
		sendSPI(0x00);
		sendSPI(0x00);
		sendSPI(0xFF);
	/*	timeout = 250;			NOTE: SD card return is not 0x01, why? Temp fix: ignor this
		for (;;) {
			res = sendSPI(0xFF);
			sendUART(res);
			if (res == 0x01)
				break;
			if (!timeout--) {
				return 0xA0;
			}
		}*/
		sendSPI(0xFF);
		sendSPI(0xFF);

		stopSPI();

		sendSPI(0xFF);
		startSPI();
		sendSPI(0x40 | 41);
		sendSPI(0x40);
		sendSPI(0x00);
		sendSPI(0x00);
		sendSPI(0x00);
		sendSPI(0xFF);
		for (;;) {
			sdcReady = sendSPI(0xFF);
			if (sdcReady == 0x00)
				return 0x00; //Card ready
			if (!timeout--) {
				break;
			}
		}

		if (!acmd41Timeout--)
			return 0xB0;

		stopSPI();
	}

	return 0xB0; //ACDM41 timeout
}

uint8_t readBlock(uint32_t sector, uint8_t buffer[]) {
	uint8_t res, timeout;

	sendSPI(0xFF);
	startSPI();
	sendSPI(0x40 | 17);
	sendSPI( (sector >> 24) & 0xFF );
	sendSPI( (sector >> 16) & 0xFF );
	sendSPI( (sector >>  8) & 0xFF );
	sendSPI( (sector >>  0) & 0xFF );
	sendSPI(0xFF);

	timeout = 250;
	for (;;) {
		res = sendSPI(0xFF);
		if (res == 0x00)
			break;
		if (!timeout--) {
			return 0x01;
		}
	}

	timeout = 250;
	for (;;) {
		res = sendSPI(0xFF);
		if (res == 0xFE)
			break;
		if (!timeout--) {
			return 0x02;
		}
	}
	for (uint16_t offset = 0; offset < 512; offset++) { //Read a block
		buffer[offset] = sendSPI(0xFF);
	}

	sendSPI(0xFF); //Fetch CRC - not used (placeholder)
	sendSPI(0xFF);
	stopSPI();
	return 0x00;
}
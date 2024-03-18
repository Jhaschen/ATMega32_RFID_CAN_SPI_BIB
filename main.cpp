#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include "ATMega32_utility_bib.h"
#include "rfid.h"
#include "can.h"
#include "lcd.h"


// UID S50 Mifare 1K Chip auslesen
uint8_t mfrc522_get_card_serial(uint8_t * serial_out)
{

    uint8_t status;
    uint8_t i;
    uint8_t serNumCheck=0;
    uint32_t unLen;
    
	mfrc522_write(BitFramingReg, 0x00);		//TxLastBists = BitFramingReg[2..0]
 
    serial_out[0] = PICC_ANTICOLL;
    serial_out[1] = 0x20;
    status = mfrc522_to_card(Transceive_CMD, serial_out, 2, serial_out, &unLen);

    if (status == CARD_FOUND)
	{
		//Check card serial number
		for (i=0; i<4; i++)
		{   
		 	serNumCheck ^= serial_out[i];
		}
		if (serNumCheck != serial_out[i])
		{   
			status = ERROR;    
		}
    }
    return status;
}




int main ()
{
	DDRC = 0xFF;			// LED-Port: output
	PORTC = 0xFF;			// LEDs aus
	
	can_init(BITRATE_500_KBPS);      // CAN init 500 kbit/s
	mfrc522_init();			// RC522 initialisieren 
	USART UART(8,0,1,38400);	// USART init 8 Zeichenbits , keien Paritätsbits , 1 Stoppbit, 9600 Zeichen pro Sekunde
	
	
_delay_ms(1000);	
UART.UsartPuts("AT+UART=9600,1,0");
_delay_ms(1000);
UART.UsartPuts("\n\r");
UART.UsartPuts("AT+RESET");
_delay_ms(1000);
UART.UsartPuts("\n\r");
	
	
	
	for (;;)
	{
	
	if(UART.UsartGetc()=='1') TGL_BIT(LED_PORT,1);
	_delay_ms(1000);
	
	}
	return 0;
}


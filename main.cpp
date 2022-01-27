#include <avr/io.h>
#include <util/delay.h>

#include <stdio.h>
#include "ATMega32_utility_bib.h"
#include "rfid.h"
#include "can.h"


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
	USART UART(8,0,1,9600);	// USART init 8 Zeichenbits , keien Paritätsbits , 1 Stoppbit, 9600 Zeichen pro Sekunde
	
	uint8_t status=0; 		// Statusbyte RFID Reader
	uint8_t str[MAX_LEN];	        // Datenarray für ein Sektor (16 Byte)  (MIFARE S50)
	char buffer[100];		// Buffer zur Zwschischenspeicherung von Zeichenketten
	
	can_t resvmsg;          // Message-Objekt auf dem Stack anlegen
   	can_t sendmsg;          // Message-Objekt auf dem Stack anlegen
        sendmsg.id = 0x27;          // ID setzen, hier: dec 
        sendmsg.flags.rtr = 0;      // Remote-Transmission-Request -> aus
        sendmsg.length = 5;         // Länge der Nachricht: 1 Byte
	
	
	status = mfrc522_read(VersionReg); // Prüfen, ob Reader erreichbar und auslesen der Version 1.0 oder 2.0
	if(status== 0x92) // Prüfen, ob Reader gefunden Versionsnummer 0x92
	{
	sprintf(buffer,"Version: 0x%x   READER FOUND", status);	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben
	UART.UsartPuts(buffer);		   // Versionsnummer ausgeben
	}else // sonst Fehlermeldung ausgeben
	{
	sprintf(buffer,"Version: 0x%x   READER NOT FOUND", status);	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben
	UART.UsartPuts(buffer);		   // Versionsnummer ausgeben
	}
	UART.UsartPuts("\n\r");		   // Neue Zeile
	

	
	
	
	for (;;)
	{
	
	status = mfrc522_request(PICC_REQALL,str);  //Prüfe, ob ein Tag in der nähe ist
	
	if(status== CARD_FOUND)
	{
	UART.UsartPuts("CARD FOUND  ");
	
	if(str[0] == 0x04)
	{
	UART.UsartPuts("Mifare_One_S50 FOUND   ");
	}	
	sprintf(buffer,"TagType: 0x%x%x ", str[0],str[1]);	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben
	UART.UsartPuts(buffer);		   // Versionsnummer ausgeben	  
	
	
	
	uint8_t UID_Status = mfrc522_get_card_serial(str);
	sprintf(buffer,"UID: 0x%x%x%x%x ", str[0],str[1],str[2],str[3]);	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben UID Ausgeben (4 Byte)
	UART.UsartPuts(buffer);		   // UID Seriennummer ausgeben	
	
	sendmsg.data[0]=status;			// Status in das Datenbyte 0 schreiben
	sendmsg.data[1]=str[0];				// UID Byte 1 in das Datenbyte 1 schreiben
	sendmsg.data[2]=str[1];			// UID Byte 2 in das Datenbyte 2 schreiben
	sendmsg.data[3]=str[2];		// UID Byte 3 in das Datenbyte 3 schreiben
	sendmsg.data[4]=str[3];		// UID Byte 4 in das Datenbyte 4 schreiben
	can_send_message(&sendmsg);		// CAN-Nachricht versenden
				
	
	 
	}else{
	UART.UsartPuts("CARD NOT FOUND");
	sendmsg.data[0]=status;			// Status in das Datenbyte 0 schreiben
	can_send_message(&sendmsg);		// CAN-Nachricht versenden
	
	}
	UART.UsartPuts("\n\r");		   // Neue Zeile	
	
	if(can_check_message()) // Prüfe, ob Nachricht empfangen wurde.
	{
	can_get_message(&resvmsg);
	sprintf(buffer,"CAN_Message mit der ID 0x%x empfangen ", resvmsg.id);	// Zeichenkette erzeugen und in dn Zwischenspeicher schreiben
	UART.UsartPuts(buffer);		   // Versionsnummer ausgeben	
	}
	
	_delay_ms(1000);
	
	}
	return 0;
}


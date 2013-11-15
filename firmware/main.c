#include "Framework.h"
#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h> 
#include <util/delay.h>
#include <string.h> 
#include "nmea/nmea.h"
#define USART_BAUD 4800L

// static int uart_putchar(char c); 
static void uart_init (void); 
void send_usb_data(char *s);
bool updating = false;
static void parse_nmea(void);

char *message;
static void uart_init(void) {

	// Set the TxD pin high - set PORTC DIR register bit 3 to 1 
  PORTD.OUTSET = PIN3_bm; 

  // Set the TxD pin as an output - set PORTC OUT register bit 3 to 1 
  PORTD.DIRSET = PIN3_bm; 

	//Baud rate of 4800
	uint16_t BSEL = 12;
	uint8_t BSCALE = 1;
	USARTD0_BAUDCTRLA = BSEL & 0XFF;
	USARTD0_BAUDCTRLB = (BSCALE << 4) | (BSEL & 0xF000) >> 8;

	// no interrupts
	// can't overwrite bits 7:6
	USARTD0.CTRLA = 0x00;

  // Enable transmitter and receiver
  USARTD0.CTRLB = USART_TXEN_bm | USART_RXEN_bm;  

	// async, no parity, 1 stop bit, 8 bit data,
	// 00     00         00          11    
  USARTD0.CTRLC = 0x03;  
}

static void parse_nmea(void) {
	  const char *buff[] = {
        "$GPRMC,173843,A,3349.896,N,11808.521,W,000.0,360.0,230108,013.4,E*69\r\n",
        "$GPGGA,111609.14,5001.27,N,3613.06,E,3,08,0.0,10.2,M,0.0,M,0.0,0000*70\r\n",
        "$GPGSV,2,1,08,01,05,005,80,02,05,050,80,03,05,095,80,04,05,140,80*7f\r\n",
        "$GPGSV,2,2,08,05,05,185,80,06,05,230,80,07,05,275,80,08,05,320,80*71\r\n",
        "$GPGSA,A,3,01,02,03,04,05,06,07,08,00,00,00,00,0.0,0.0,0.0*3a\r\n",
        "$GPRMC,111609.14,A,5001.27,N,3613.06,E,11.2,0.0,261206,0.0,E*50\r\n",
        "$GPVTG,217.5,T,208.8,M,000.00,N,000.01,K*4C\r\n"
    };

    int it;
    nmeaINFO info;
    nmeaPARSER parser;
    nmeaPOS dpos;

    nmea_zero_INFO(&info);
    nmea_parser_init(&parser);

    for(it = 0; it < 6; ++it){
        nmea_parse(&parser, buff[it], (int)strlen(buff[it]), &info);
        nmea_info2pos(&info, &dpos);

        printf(
            "%03d, Lat: %f, Lon: %f, Sig: %d, Fix: %d\n",
            it, dpos.lat, dpos.lon, info.sig, info.fix
        );
    }

    nmea_parser_destroy(&parser);
}

// static int uart_putchar (char c) { 
//     if (c == '\n') 
//         uart_putchar('\r'); 

//     // Wait for the transmit buffer to be empty 
//     while ( !( USARTD0.STATUS & USART_DREIF_bm) ); 

//     // Put our character into the transmit buffer 
//     USARTD0.DATA = c; 

//     return 0; 
// } 

int main(void){
	USB_ConfigureClock();
	USB_Init();
	USB.INTCTRLA = USB_BUSEVIE_bm | USB_INTLVL_MED_gc;
	USB.INTCTRLB = USB_TRNIE_bm | USB_SETUPIE_bm;
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();
	if (false) {
		uart_init();
	}

	parse_nmea();

	message = "012345678901234567";
	// send_usb_data(message);
	for (;;){
		// uart_putchar('a');
	}

}

#define stringify(s) #s

const char PROGMEM hwversion[] = stringify(HW_VERSION);
const char PROGMEM fwversion[] = stringify(FW_VERSION);

uint8_t usb_cmd = 0;
uint8_t cmd_data = 0;

// void send_usb_data(char *message) {
// 	for (uint8_t = 0; i < 64; i++) ep0_buf_in[i] = 0;
// 	int l = strlen(message);
// 	for (int i=0; i<l; i++) {
// 		ep0_buf_in[i] = message[i];
// 	}
// }

/** Event handler for the library USB Control Request reception event. */
bool EVENT_USB_Device_ControlRequest(USB_Request_Header_t* req){
	// zero out ep0_buf_in
	for (uint8_t i = 0; i < 64; i++) ep0_buf_in[i] = 0;
	usb_cmd = 0;
	if ((req->bmRequestType & CONTROL_REQTYPE_TYPE) == REQTYPE_VENDOR){
		switch(req->bRequest){
			case 0x00: // Info
				if (req->wIndex == 0){
					USB_ep0_send_progmem((uint8_t*)hwversion, sizeof(hwversion));
				}else if (req->wIndex == 1){
					USB_ep0_send_progmem((uint8_t*)fwversion, sizeof(fwversion));
				}
				return true;
			case 0x02:
				{
					int l = strlen(message);
					for (uint8_t i=0; i<64; i++) {
						ep0_buf_in[i] = message[i];
					}
					USB_ep0_send(l);
					return true;
				}
			case 0x08:
				* ((uint8_t *) req->wIndex) = req->wValue;
				USB_ep0_send(0);
				return true;
			case 0x09:
				ep0_buf_in[0] = * ((uint8_t *) req->wIndex);
				USB_ep0_send(1);
				return true;
			case 0x16:
				* ((uint16_t *) req->wIndex) = req->wValue;
				USB_ep0_send(0);
				return true;
			case 0x17:{
				uint16_t *addr;
				addr = (uint16_t *) req->wIndex;
				ep0_buf_in[0] = *addr & 0xFF;
				ep0_buf_in[1] = *addr >> 8;
				USB_ep0_send(2);}
				return true;
			// read EEPROM	
			case 0xE0: 
				eeprom_read_block(ep0_buf_in, (void*)(req->wIndex*64), 64);
				USB_ep0_send(64);
				return true;

			// write EEPROM	
			case 0xE1: 
				usb_cmd = req->bRequest;
				cmd_data = req->wIndex;
				USB_ep0_send(0);
				return true; // Wait for OUT data (expecting an OUT transfer)

			// disconnect from USB, jump to bootloader	
			case 0xBB: 
				USB_enter_bootloader();
				return true;
		}
	}
	return false;
}

void EVENT_USB_Device_ControlOUT(uint8_t* buf, uint8_t count){
	switch (usb_cmd){
		case 0xE1: // Write EEPROM
			eeprom_update_block(buf, (void*)(cmd_data*64), count);
			break;
	}
}

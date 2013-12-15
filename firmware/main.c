#include "Framework.h"
#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h> 
#include <stdlib.h>
#include <util/delay.h>
#include <string.h> 
#include <math.h>
#include "utils/utils.h"
#include "nmea/nmea.h"
#include "uart/uart.h"
#include "gps/gps.h"
#include "gprs/gprs.h"
#include "sms/sms.h"

#define HIGH_FREQ 1
#define LOW_FREQ 0

const uint16_t low_freq = 1000*30;
const uint16_t high_freq = 1000*30;
uint8_t freq = 0; // 0 --> low_freq, 1 --> high_freq
char* phonenumber = "+18572080246";
char* twilio_number = "+13042493059";
char *message;
bool updating = false;
uint8_t count;
latlng gps;

int main(void){

  //usb configuration
	USB_ConfigureClock();
	USB_Init();
	USB.INTCTRLA = USB_BUSEVIE_bm | USB_INTLVL_MED_gc;
	USB.INTCTRLB = USB_TRNIE_bm | USB_SETUPIE_bm;
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();

  if (true) {
  	//USART, PORT, TXPIN_bm
    gprs_init(&USARTC0, &PORTC, PIN3_bm);
  }
  
  //uart init code -- change condition to true when we're ready to communicate with the gps
	if (true) {
		//USART, PORT, POWERPIN_bm, TXPIN_bm, RESETPIN_bm
		gps_init(&USARTD0, &PORTD, PIN0_bm, PIN3_bm, PIN4_bm);
	}


  //call break and flush to make sure the buffer is cleared
	// break_and_flush();

  // parse nmea string and send result over usb
  // char* text_message = parse_nmea();
	// char* text_message = "Whatsup?";
  // send_message(phonenumber, text_message, &USARTC0);
  // send_message(twilio_number, text_message, &USARTD0);
  // SimpleReceive(&USARTD0);

	for (;;){
    gps_receive(&USARTD0, &gps);
    if (gps.valid) {
    	if (gps.roaming) {
		  	send_message(phonenumber, gps.sms, &USARTC0);
      	send_message(twilio_number, gps.sms, &USARTC0);
      	freq = HIGH_FREQ;
    	} else {
    		freq = LOW_FREQ;
    	}
    }
    if (freq == HIGH_FREQ) {
    	_delay_ms(high_freq);
    } else {
    	_delay_ms(low_freq);
    }
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

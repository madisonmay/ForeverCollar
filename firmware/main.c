#include "Framework.h"
#include <avr/io.h>
#include <inttypes.h>
#include <stdio.h> 
#include <stdlib.h>
#include <util/delay.h>
#include <string.h> 
#include <math.h>
#include "nmea/nmea.h"

//sample nmea strings for testing parser
double lat = 42.292747;
double lng = -71.264622;
double max_dist = .200;
char* twilio_number = "+13042493059";

typedef struct _latlng {
  double lat;
  double lng;
} latlng;

int uart_putchar(char c, USART_t* USART); 
char uart_getchar(USART_t* USART);

void send_string(char* s);

void gps_init (void);
void grps_init(void);

double distance(double, double);

void turn_on_gps(void);

void wake_up_gps(void);
void wake_up_gprs(void);

void send_usb_data(char *s);
bool updating = false;
char* parse_nmea(void);

void send_message(char* number, char* message);

extern void parse_nmea_string(char *s, latlng *gps);

char *message;

void send_string(char* s) {
  int length = strlen(s);
  for (int i=0; i<length; i++) {
    send_byte(s[i]);
  }
}

void turn_on_gps(void) {
  // wait one second after powering on, as recommended by a2235-h data sheet
  _delay_ms(1000);

  // // May or may not be necessary -- docs on wakeup are a bit sketchy
  // // Does the gps module transmit anything before locking on to a gps position?
  // // Or do we need to wait before querying for location data?
  // Set PD5 direction to output
  PORTD.DIRSET = PIN5_bm;

  // LOW/HIGH transmission of PD5 to wakeup gps module 
  PORTD.OUTTGL = PIN5_bm;
}

void wake_up_gps(void) {

}

void gps_init(void) {
  /*
  
  Port numbers, baud rate, etc will need to be changed when switching 
  to a different port.  This is a bit complex to abstract out to a new 
  code layer at the moment (too many inputs required for things to work right)
  so we'll have to keep it as is.  

  Specifically, we'll have to change PORTD, USARTDO, BSEL, BSCALE, and the Tx bitmask

  */

  turn_on_gps();

  // Set the TxD pin as an output - set PORTD OUT register bit 3 to 1 
  PORTD.DIRSET = PIN3_bm; 

  // Set the TxD pin high - set PORTD DIR register bit 3 to 1 
  PORTD.OUTSET = PIN3_bm; 

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

void wake_up_gprs(void) {
  // 
}

void gprs_init(void) {
  //change port numbers when we get our proto board

  wake_up_gprs();
  //Baud rate of 19200

  // Set the TxD pin as an output - set PORTD OUT register bit 3 to 1 
  PORTD.DIRSET = PIN3_bm; 

  // Set the TxD pin high - set PORTD DIR register bit 3 to 1 
  PORTD.OUTSET = PIN3_bm; 

  uint16_t BSEL = 6;
  uint8_t BSCALE = 0;
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

char* concat(char *s1, char *s2)
{
    //string concatenation -- not needed now, but might prove useful later
    char *result = malloc(strlen(s1)+strlen(s2)+1);
    //should also check for memory allocation errors here
    strcpy(result, s1);
    strcat(result, s2);
    return result;
}

void parse_nmea_string(char *s, latlng *gps)
{
  int i=0; // used to iterate through array
  char *token[20]; //stores the chunks of string after splitting the string on commas

  token[0] = strtok(s, ","); //get pointer to first token found and store in
                             //first element of array
  while(token[i] != NULL) {  //while commas continue to be found
      i++;  
      token[i] = strtok(NULL, ","); //continue to split the string
  }

  //Example: token = [], s = "a,b,c"
  //Iteration 1
  //token --> ["a"], s-->"b,c"
  //token --> ["a", "b"], s-->"c"
  //token --> ["a", "b", "c"], s-->""
  //end

  //when parsing GPRMC data
  //longitude should be stored at index 3
  //latitude should be stord at index 5

  //indices will have to be changed if our gps module
  //speaks a different dialog of NMEA

  char* lat_str = token[3]; //longitude
  char* lng_str = token[5]; //latitude

  //converts string stored in gps->lat_str to double and stores in lat
  gps->lat = atof(lat_str)/100.;

  //converts string stored in gps->lng_str to double and stores in lng
  gps->lng = atof(lng_str)/100.;
}

#define d2r (M_PI / 180.0)

//calculate distance, assuming earth is spherical
double distance(double gpslat, double gpslng) {
    double dlong = (gpslng - lng) * d2r;
    double dlat = (gpslat - lat) * d2r;
    double a = pow(sin(dlat/2.0), 2) + cos(lat*d2r) * cos(gpslat*d2r) * pow(sin(dlong/2.0), 2);
    double c = 2 * atan2(sqrt(a), sqrt(1-a));
    double d = 6367 * c;

    return d;
}



char* parse_nmea(void) {

    char *buff = "$GPRMC,71.132,A,4230.00,N,-7130.00,E,11.2,0.0,261206,0.0,E*50\r\n";

    //latlng struct to store gps data in
    //reused in every iterationif (d
    latlng gps;

    //parse lat and lng out of raw nmea string
    parse_nmea_string(buff, &gps);

    double dist = distance(gps.lat, gps.lng);

    char** result = malloc(30 * sizeof(char*));

    if (dist > max_dist) {

      char lat_buff[100];
      char lng_buff[100];
      char dist_buff[100];

      sprintf(lat_buff, "%f", gps.lat);
      sprintf(lng_buff, "%f", gps.lng);
      sprintf(dist_buff, "%f", dist);

      //determine how many characters are in the gps latitude and longitude strings
      int len_lat = strlen(lat_buff);
      int len_lng = strlen(lng_buff);
      int len_dist = strlen(dist_buff);

      //iterate over lat and lng strings, sending them char by char over usb
      for (int i=0; i<len_lat; i++) {
        send_byte(lat_buff[i]);  
      }

      //lat/lng seperator 
      send_byte(' ');  
      for (int j=0; j<len_lng; j++) {
        send_byte(lng_buff[j]);  
      }

            //lat/lng seperator 
      send_byte(' ');  
      for (int k=0; k<len_dist; k++) {
        send_byte(dist_buff[k]);  
      }

      char* lat_str = &lat_buff[0];
      char* lng_str = &lng_buff[0];
      *result = concat(concat(lat_str, ","), lng_str);
    }

    //presumably has to do with memory management
    //for now, just make sure to call if after you're done sending bytes over usb
    break_and_flush();
    return *result;
}

// code for communicating with the gprs module via uart
int uart_putchar (char c, USART_t* USART) { 
    if (c == '\n') 
        uart_putchar('\r', USART); 

    // Wait for the transmit buffer to be empty 
    while ( !( USART->STATUS & USART_DREIF_bm) ); 

    // Put our character into the transmit buffer 
    USART->DATA = c; 

    return 0; 
} 

// code for communicating with the gprs module via uart
char uart_getchar (USART_t* USART) { 

    // Wait for the receive buffer to be empty 
    while ( !( USART->STATUS & USART_RXCIF_bm) ); 

    // Receive char from receive buffer 
    return USART->DATA; 
} 

void send_message(char* number, char* text_message) {

  uart_putchar('\r', &USARTD0);
  _delay_ms(100);

  char* text_mode = "AT+CMGF=1\r";
  _delay_ms(100);

  char c;

  while (*text_mode != '\0') {  
    uart_putchar(*text_mode, &USARTD0);

    // debugging
    c = uart_getchar(&USARTD0);
    send_byte(c);
    text_mode++;
  }

  _delay_ms(100);
  break_and_flush();


  //build number string
  char* number_string = concat("AT+CMGS=\"", number);
  char* new_number_string = concat(number_string, "\"\r");

  //more sketchiness

  while (*new_number_string != '\0') {
    uart_putchar(*new_number_string, &USARTD0);

    //debugging
    c = uart_getchar(&USARTD0);
    send_byte(c);
    new_number_string++;
  }
  _delay_ms(100);
  break_and_flush();

  char* new_text_message = concat(text_message, "\r");
  while (*new_text_message != '\0') {
    uart_putchar(*new_text_message, &USARTD0);

    //debugging
    c = uart_getchar(&USARTD0);
    send_byte(c);
    new_text_message++;
  }

  _delay_ms(100);
  break_and_flush();

  //check this later to ensure ^Z is being sent
  uart_putchar(26, &USARTD0);
  // c = uart_getchar(&USARTD0);
  // send_byte(c);

  _delay_ms(100);
  break_and_flush();
}

int main(void){

  //usb configuration
	USB_ConfigureClock();
	USB_Init();
	USB.INTCTRLA = USB_BUSEVIE_bm | USB_INTLVL_MED_gc;
	USB.INTCTRLB = USB_TRNIE_bm | USB_SETUPIE_bm;
	PMIC.CTRL = PMIC_LOLVLEN_bm | PMIC_MEDLVLEN_bm;
	sei();

  //uart init code -- change condition to true when we're ready to communicate with the gps
	if (false) {
		gps_init();
	}

  if (true) {
    gprs_init();
  }

  //call break and flush to make sure the buffer is cleared
	break_and_flush();

  // parse nmea string and send result over usb
  char* text_message = parse_nmea();

  char* phonenumber = "+12153166262";
  send_message(phonenumber, text_message);
  send_message(twilio_number, text_message);

	for (;;){
    //heart of the firmware logic goes here
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

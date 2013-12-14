void gps_init (void);
void turn_on_gps(void);
void wake_up_gps(void);

#define ON_OFF 5
#define RESET 4

void turn_on_gps(void) {
  // wait 1+ seconds after powering on, as recommended by a2235-h data sheet
  _delay_ms(1500);

  //Binary logic notes

  //Flip a 0 to a 1
  //00000000
  //   OR
  //00100000
  //   =
  //00100000

  //Flip a 1 to a 0
  //00100000
  //   ~
  //11011111
  //  AND
  //00100000
  //   =
  //00000000

  //76543210



  // PIN 0 -- ON_OFF
  // Set PD0 to low
  PORTD.DIRSET |= PIN0_bm;
  PORTD.OUTSET &= (~PIN0_bm);  

// PIN 4 -- RESET: Active LOW
  PORTD.DIRSET |= PIN4_bm;
  PORTD.OUTSET |= PIN4_bm;

  // Set the TxD pin as an output - set PORTD OUT register bit 3 to 1 
  PORTD.DIRSET |= PIN3_bm; 

  //Baud rate of 4800 for nmea string communication
  int BSEL = 12;
  int BSCALE = 5;
  USARTD0_BAUDCTRLA = BSEL & 0xFF;
  USARTD0_BAUDCTRLB = (BSCALE << 4) | (BSEL & 0xF000) >> 8;

  // no interrupts
  // can't overwrite bits 7:6
  USARTD0.CTRLA = 0x00;

  // Enable transmitter and receiver
  USARTD0.CTRLB = USART_TXEN_bm | USART_RXEN_bm;  

  // async, no parity, 1 stop bit, 8 bit data,
  // 00     00         0           011    
  USARTD0.CTRLC = 0x03;  

  // LOW/HIGH transmission of PD0 to wakeup gps module 
  _delay_ms(2000);
  PORTD.OUTSET |= PIN0_bm;
  _delay_ms(200);
  PORTD.OUTTGL = PIN0_bm;

  // give gps time to boot up and start sending nmea string
  _delay_ms(5000);

  // send_string("Wake up gps");

}

void power_off_gps()
{
  // Set PD5 direction to output
  PORTD.DIRSET |= PIN0_bm;

  // LOW/HIGH transmission of PD5 to wakeup gps module 
  PORTD.OUTTGL |= PIN0_bm;
  _delay_ms(1000);
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

  // send_string("GPS Initialization Sequence Complete");

}

void gps_receive() {
  //parsing out gps coordinates

  //code to look for to indicate start of gps coord string
  char *code = "$GPRMC,";

  //length of code
  int length = 7;

  //keep track of location in code
  int index = 0;

  //indicates whether or not a gps string should be built
  int read = 0;

  //buffer to hold gps string -- liberally sized
  char buff[100];

  //keep track of position in buffer
  int buff_index = 0;

  //gps struct to hold error codes and lat, lng double values
  latlng gps;

  //char to store bytes from uart
  char c; 

  //only terminates when break is hit
  while (1) {
    //pull char from uart
    c = uart_getchar(&USARTD0);
    if (index == length) {
      //if past `length` characters match code set flip read to on
      read = 1;
    }

    if (read == 1) {
      //build string
      buff[buff_index] = c;
      buff_index++;

      if (c == '\r' || c == '\n' || c == 'W') {
        //send full string
        buff[buff_index] = '\0';
        send_string(buff);
        //additional parsing
        parse_nmea_string(buff, &gps);
        break;
      }
    } else if (c == code[index]) {
      //char from matches corresponding char in code
      index++;
    } else {
      //incorrect character, reset counter
      index = 0;
    }
  }
}

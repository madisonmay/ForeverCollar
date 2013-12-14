void gps_init (USART_t* USART, PORT_t* PORT, char PWRPIN_bm, char TXPIN_bm, char RESETPIN_bm);
void toggle_power_gps(PORT_t* PORT, char PWRPIN_bm);

void toggle_power_gps(PORT_t* PORT, char PWRPIN_bm)
{
  // LOW/HIGH transmission of PD0 to wakeup gps module 
  _delay_ms(2000);
  PORT->OUTSET |= PWRPIN_bm;
  _delay_ms(200);
  PORT->OUTTGL = PWRPIN_bm;
  // give gps time to boot up and start sending nmea string
  _delay_ms(5000);
}

void gps_init(USART_t* USART, PORT_t* PORT, char PWRPIN_bm, char TXPIN_bm, char RESETPIN_bm) {
  /*
  
  Port numbers, baud rate, etc will need to be changed when switching 
  to a different port.  This is a bit complex to abstract out to a new 
  code layer, but we'll give it our best shot.

  */

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



  // POWER PIN -- ON_OFF
  // Set PD0 to low
  PORT->DIRSET |= PWRPIN_bm;
  PORT->OUTSET &= (~PWRPIN_bm);  

  // RESET PIN : Active LOW
  // Set to output and send high
  PORT->DIRSET |= RESETPIN_bm;
  PORT->OUTSET |= RESETPIN_bm;

  // Set the TxD pin as an output - set PORT OUT register bit 3 to 1 
  PORT->DIRSET |= TXPIN_bm; 

  //Baud rate of 4800 for nmea string communication
  int BSEL = 12;
  int BSCALE = 5;
  USART->BAUDCTRLA = BSEL & 0xFF;
  USART->BAUDCTRLB = (BSCALE << 4) | (BSEL & 0xF000) >> 8;

  // no interrupts
  // can't overwrite bits 7:6
  USART->CTRLA = 0x00;

  // Enable transmitter and receiver
  USART->CTRLB = USART_TXEN_bm | USART_RXEN_bm;  

  // async, no parity, 1 stop bit, 8 bit data,
  // 00     00         0           011    
  USART->CTRLC = 0x03;  

  // LOW/HIGH transmission of PD0 to wakeup gps module 
  toggle_power_gps(PORT, PWRPIN_bm);

  // send_string("Wake up gps");
}

void gps_receive(USART_t* USART) {
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
    c = uart_getchar(USART);
    if (index == length) {
      //if past `length` characters match code set flip read to on
      read = 1;
    }

    if (read == 1) {
      //build string
      buff[buff_index] = c;
      buff_index++;

      if (c == '\r' || c == '\n' || c == 'W' || c == 'E') {
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

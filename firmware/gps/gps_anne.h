void gps_init (void);
void turn_on_gps(void);
void wake_up_gps(void);

#define ON_OFF 5
#define RESET 4

void turn_on_gps(void) {
  // wait one second after powering on, as recommended by a2235-h data sheet
  _delay_ms(1500);

  // send_string("Begin");
  // pinMode(AXE, OUTPUT);
  // pinMode(LIFESUPPORT , OUTPUT);

  // digitalWrite(AXE , 0);
  // digitalWrite(LIFESUPPORT , 1);

  // Set PD5 direction to output

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

  PORTD.DIRSET |= PIN0_bm;
  // send_byte(PORTD.DIRSET);
  // break_and_flush();

  // PIN 5 -- ON_OFF
  // Set PD5 to low
  PORTD.OUTSET &= (~PIN0_bm);
  // send_byte(PORTD.OUTSET);
  // break_and_flush();

  // PIN 4 -- RESET: Active LOW
  PORTD.DIRSET |= PIN4_bm;
  // send_byte(PORTD.DIRSET);
  // break_and_flush();

  // // Set PD4 to LOW
  // PORTD.OUTSET &= (~PIN4_bm);
  // send_byte(PORTD.OUTSET);
  // break_and_flush();
  // _delay_ms(250);

  // Set PD4 to HIGH
  PORTD.OUTSET |= PIN4_bm;
  // send_byte(PORTD.OUTSET);
  // break_and_flush();

  // Set the TxD pin as an output - set PORTD OUT register bit 3 to 1 
  PORTD.DIRSET |= PIN3_bm; 

  // Set the RxD pin as an input -- set PORT OUT register bit 2 to 1
  PORTD.DIRSET &= (~PIN2_bm);
  // send_byte(PORTD.DIRSET);
  // break_and_flush();

  //Baud rate of 4800 for nmea string communication
  uint8_t BSEL = 12;
  uint8_t BSCALE = 1;
  USARTD0_BAUDCTRLA = BSEL & 0xFF;
  USARTD0_BAUDCTRLB = (BSCALE << 4) | (BSEL & 0xF000) >> 8;

  // no interrupts
  // can't overwrite bits 7:6
  USARTD0.CTRLA = 0x00;

  // Enable transmitter and receiver
  USARTD0.CTRLB = USART_TXEN_bm | USART_RXEN_bm;  

  // async, no parity, 1 stop bit, 8 bit data,
  // 00     00         00          11    
  USARTD0.CTRLC = 0x03;  


  // LOW/HIGH transmission of PD5 to wakeup gps module 
  _delay_ms(2000);
  PORTD.OUTSET |= PIN0_bm;
  _delay_ms(200);
  PORTD.OUTSET &= (~PIN0_bm);
  // send_byte(PORTD.OUTTGL);
  // break_and_flush();
  _delay_ms(10000);

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
  // // while (digitalRead(KILLSWITCH)) {
  //   _delay_ms(10);
  //   // if (GPS_SERIAL.available()) {
  //       _delay_ms(10);
  //       // while(GPS_SERIAL.available()) {
  //           c = uart_getchar(&USARTD0);
  //           send_byte(c);
  //           delay(2);
  //       }
  //   }
  // }
  // send_string("Receive characters");
  
  char c; 
  for (int i=0; i<3; i++) {
    c = uart_getchar(&USARTD0);
    send_byte(c);
  }
  break_and_flush();
}
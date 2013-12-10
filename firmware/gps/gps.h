void gps_init (void);
void turn_on_gps(void);
void wake_up_gps(void);

#define ON_OFF 5
#define RESET 4

void turn_on_gps(void) {
  // wait one second after powering on, as recommended by a2235-h data sheet
  _delay_ms(1000);

  send_string("Begin");
  // pinMode(AXE, OUTPUT);
  // pinMode(LIFESUPPORT , OUTPUT);

  // digitalWrite(AXE , 0);
  // digitalWrite(LIFESUPPORT , 1);

  // Set PD5 direction to output
  PORTD.DIRSET |= PIN5_bm;

  // PIN 5 -- ON_OFF
  // Set PD5 to low
  PORTD.OUTSET &= (~PIN5_bm);

  // PIN 4 -- NRST
  // Set PD5 direction to output
  PORTD.DIRSET |= PIN4_bm;

  // Set PD4 to high
  PORTD.OUTSET |= PIN5_bm;

  _delay_ms(3000);

  send_string("Delay complete");

  // LOW/HIGH transmission of PD5 to wakeup gps module 
  PORTD.OUTTGL = PIN5_bm;
  _delay_ms(1000);

  send_string("Wake up gps");

  // Set the TxD pin as an output - set PORTD OUT register bit 3 to 1 
  PORTD.DIRSET |= PIN3_bm; 

  // Set the TxD pin high - set PORTD DIR register bit 3 to 1 
  PORTD.OUTSET |= PIN3_bm; 

}

void power_off_gps()
{
  // Set PD5 direction to output
  PORTD.DIRSET |= PIN5_bm;

  // LOW/HIGH transmission of PD5 to wakeup gps module 
  PORTD.OUTTGL |= PIN5_bm;
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

  //Baud rate of 115200
  uint16_t BSEL = 11;
  uint8_t BSCALE = -7;
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

  uint8_t characters[32] = {0xA0, 0xA2, 0x00, 0x18, 0x81, 0x02, 0x01, 0x01, 0x00, 0x01, 0x01, 0x01, 0x05, 0x01, 0x01, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x00, 0x01, 0x25, 0x80, 0x01, 0x3A, 0xB0, 0xB3};
  for (int c = 0; c < 32; c++) {
    send_uart((char *) &characters[c], &USARTD0);
  }

  send_string("Characters sent to change baud rate");

  //Baud rate of 9600 for nmea string communication
  BSEL = 12;
  BSCALE = 0;
  USARTD0_BAUDCTRLA = BSEL & 0XFF;
  USARTD0_BAUDCTRLB = (BSCALE << 4) | (BSEL & 0xF000) >> 8;

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
  send_string("Receive characters");
  char c;
  for (int i=0; i<5; i++) {
    c = uart_getchar(&USARTD0);
    send_byte(c);
  }
  break_and_flush();
}
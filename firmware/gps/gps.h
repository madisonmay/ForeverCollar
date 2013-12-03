void gps_init (void);
void turn_on_gps(void);
void wake_up_gps(void);

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
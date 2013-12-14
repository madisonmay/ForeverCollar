void wake_up_gprs(void);
void gprs_init(USART_t* USART, PORT_t* PORT, char TXPIN_bm);

void wake_up_gprs(void) {
  // 
}

void gprs_init(USART_t* USART, PORT_t* PORT, char TXPIN_bm) {
  //change port numbers when we get our proto board

  wake_up_gprs();
  //Baud rate of 19200

  // Set the TxD pin as an output - set PORTD OUT register bit 3 to 1 
  PORT->DIRSET = TXPIN_bm; 

  // Set the TxD pin high - set PORTD DIR register bit 3 to 1 
  PORT->OUTSET = TXPIN_bm; 

  uint16_t BSEL = 6;
  uint8_t BSCALE = 0;
  USART->BAUDCTRLA = BSEL & 0XFF;
  USART->BAUDCTRLB = (BSCALE << 4) | (BSEL & 0xF000) >> 8;

  // no interrupts
  // can't overwrite bits 7:6
  USART->CTRLA = 0x00;

  // Enable transmitter and receiver
  USART->CTRLB = USART_TXEN_bm | USART_RXEN_bm;  

  // async, no parity, 1 stop bit, 8 bit data,
  // 00     00         00          11    
  USART->CTRLC = 0x03;  
}
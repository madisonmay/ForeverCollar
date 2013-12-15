#include "Framework.h"
#include <avr/io.h>
#include <stdlib.h>
#include <string.h>

void send_message(char*, char*, USART_t*);
char* read_message(USART_t*);
void delSMS(USART_t*);
void GprsTextModeSMS(USART_t*);
void ClearGprsMsg(void);
void GprsReadSmsStore(char* SmsStorePos, USART_t* USART);
void ProcessGprsMsg(USART_t*);
void ProcessSms(char* sms);
void ReceiveTextMessage(USART_t*);
void SimpleReceive();

void send_message(char* number, char* text_message, USART_t* USART) {

  uart_putchar('\r', USART);
  _delay_ms(100);

  GprsTextModeSMS(USART);
  _delay_ms(100);
  // send_string("GPS TEXT MODE");

  //build number string
  char* number_string = concat(concat("AT+CMGS=\"", number), "\"");
  send_uart(number_string, USART);
  _delay_ms(100);
  // send_string(number_string);

  send_uart(text_message, USART);
  _delay_ms(100);
  // send_string(text_message);

  uart_putchar(26, USART);
  _delay_ms(100);
  // send_string("COMPLETE");
}

void GprsTextModeSMS(USART_t* USART) {
  send_uart("AT+CMGF=1", USART);
}

void PowerDownModem(USART_t* USART) {
  send_uart("AT+CPOWD=1", USART);
}

void SleepModem(USART_t* USART) {
  send_uart("AT+CFUN=0", USART);
}

void WakeUpModem(USART_t* USART) {
  send_uart("AT+CFUN=1", USART);
}

void SimpleReceive(USART_t* USART) {
  //not currently functional, perhaps
  //because of baud rate issues
  
  char c;
  int n = 1;
  char s[n+1];

  uart_putchar('A', USART);
  send_string("Sync");
  _delay_ms(3000);

  send_uart("AT", USART);
  send_string("PING");
  _delay_ms(100);
  for (uint8_t i=0; i<n; i++) {
    c = uart_getchar(USART);
    s[i] = c;
  }
  s[n] = '\0';

  send_string(s);



  GprsTextModeSMS(USART);
  send_string("Text Mode");
  _delay_ms(800);

  // send_uart("AT+CMGL=\"REC UNREAD\"", USART); // Read Message 
  // send_string("Read Messages");
  // _delay_ms(1000);

  send_uart("AT+CMGF=?", USART); // Read Message 
  send_string("Check Mode");
  _delay_ms(1000);
}
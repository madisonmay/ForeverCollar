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

  //build number string
  char* number_string = concat(concat("AT+CMGS=\"", number), "\"");
  send_uart(number_string, USART);
  _delay_ms(100);

  send_uart(text_message, USART);
  _delay_ms(100);

  uart_putchar(26, USART);
  _delay_ms(100);
  break_and_flush();
}

char* smsStorePos = "";
char* msg = "";
char* snTmp = "";
char* snFull = "";
int SmsContentFlag;
char SerialInByte;


void delSMS(USART_t* USART) {
  char* cmd = "AT+CMGD=";
  char* delete_cmd = concat(cmd, smsStorePos);
  send_uart(delete_cmd, USART);
}

void GprsTextModeSMS(USART_t* USART) {
  send_uart("AT+CMGF=1", USART);
}

void ClearGprsMsg() {
  msg = "";
}

void GprsReadSmsStore( char* SmsStorePos, USART_t* USART){
  char* cmd = concat("AT+CMGR=", SmsStorePos);
  send_uart(cmd, USART);
}

void ProcessSms (char* sms) {

}

void ProcessGprsMsg(USART_t* USART) {

  char* found = strstr(msg, "Call Ready");
  if (found != NULL)                    
  {
    send_string("GPRS Shield registered on Mobile Network");  
    GprsTextModeSMS(USART);      
  }

  found = strstr(msg, "+CMTI");
  if (found != NULL){
    //find comma
    found = strstr(msg, ",");
    smsStorePos = ++found;
    send_string("Message found");

    GprsReadSmsStore(smsStorePos, USART);
  }

  found = strstr(msg, "+CMGR:");
  if (found != NULL) {
    found = strstr(msg, "+1");
    if (found != NULL) {
      snTmp = ++found;
      snFull = "";
      for (int i=0; i<11; i++) {
        snFull = concat(snFull, &snTmp[i]);
      }
      send_string(snFull);

      SmsContentFlag = 1;
      ClearGprsMsg();
      return;
    } 
  }

  if (SmsContentFlag == 1){
    send_string(msg);
    ProcessSms(msg);
    delSMS(USART);
  }

  ClearGprsMsg();
  SmsContentFlag = 0; 
}

void ReceiveTextMessage(USART_t* USART) {
  while(1) {
    SerialInByte = uart_getchar(USART);
    if (SerialInByte == 13){
      ProcessGprsMsg(USART);
    } else if (SerialInByte == 10){
      //pass
    } else{
      msg = concat(msg, &SerialInByte);
    }
  }
}

void SimpleReceive(USART_t* USART) {
  char c;
  uart_putchar('\r', USART);
  _delay_ms(100);
  send_string("PING");
  GprsTextModeSMS(USART);
  _delay_ms(100);
  send_string("Text Mode");
  send_uart("AT+CMGL=\"ALL\"", USART); // Read Message 
  _delay_ms(1000);
  send_string("Read Messages");
  for (int i=0; i<100; i++) {
    c = uart_getchar(USART);  
    send_byte(c);
    break_and_flush();
  }
}
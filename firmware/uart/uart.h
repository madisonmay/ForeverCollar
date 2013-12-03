#include "Framework.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

uint8_t debug = 0;

int uart_putchar(char c, USART_t* USART); 
char uart_getchar(USART_t* USART);
void send_uart(char*, USART_t* USART);

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

void send_uart(char* text, USART_t* USART) {
  char c;
  char* new_text = concat(text, "\r");
  while (*new_text != '\0') {  
    uart_putchar(*new_text, USART);
    new_text++;

    if (debug == 1) {
      c = uart_getchar(USART);
      send_byte(c);
    }
  }
  if (debug == 1) {
    break_and_flush();
  }
}

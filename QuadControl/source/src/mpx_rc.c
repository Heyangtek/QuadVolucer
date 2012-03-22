// Header:    Parses mpx m-link receivers B/D serial servo data
// File Name: mpx_rc.c
// Author:    Konstantin Reichmeyer
// Date:      18.03.2012

/*
    Multiplex M-LINK receivers (e.g. RX-7-DR) can be configured to provide digital servo data over a UART interface.
    mpx_rc.h and mpx_rc.c is an implementation of that interface. 
    
    The UART parameters are 115200 Baud, 8-N-1 
*/


#include "stm32f10x.h"
#include "main.h"
#include "mpx_rc.h"

volatile uint8_t sync_gap = 0;                                      //gets increased by Sys-Tick 
uint8_t rx_data[27];
extern union pulsw pulswidth;




/**
  * @brief  Parses servo data from series of data bytes sent by MPX receiver
  * @param  data byte sent by MPX receiver
  * @retval None
  */
void mpx_parse_data(uint8_t rx_byte)
{
  static uint8_t bytecount = 0;
  static uint16_t crc_ccitt = 0;

  if((sync_gap > 10) && (rx_byte == 0xA1))                          //find start of data-package
  {  
    bytecount = 0;
    crc_ccitt = 0;
  }
  sync_gap = 0;

  crc_ccitt = crc_xmodem_update(crc_ccitt, rx_byte);
  rx_data[bytecount] = rx_byte;
  
  
  if(bytecount == 26 && crc_ccitt == 0)                             //if end of data-package && crc sum is correct
  {
    pulswidth.pw[0] = ((uint16_t)rx_data[1]  << 8) | rx_data[2];
    pulswidth.pw[1] = ((uint16_t)rx_data[3]  << 8) | rx_data[4];
    pulswidth.pw[2] = ((uint16_t)rx_data[5]  << 8) | rx_data[6];
    pulswidth.pw[3] = ((uint16_t)rx_data[7]  << 8) | rx_data[8];
    pulswidth.pw[4] = ((uint16_t)rx_data[9]  << 8) | rx_data[10];
    pulswidth.pw[5] = ((uint16_t)rx_data[11] << 8) | rx_data[12];
  }

  if(bytecount < 26)
    bytecount++;
}


/**
  * @brief  CRC-XMODEM calculation
  * @param  CRC, DATA
  * @retval updated CRC
  */
uint16_t crc_xmodem_update(uint16_t crc, uint8_t data)              
{
  int i;

  crc = crc ^ ((uint16_t)data << 8);
  for (i=0; i<8; i++)
  {
    if (crc & 0x8000)
      crc = (crc << 1) ^ 0x1021;
    else
      crc <<= 1;
  }
  return crc;
}


/*

  u8x8_d_a2printer.c
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2016, olikraus@gmail.com
  All rights reserved.

  Redistribution and use in source and binary forms, with or without modification, 
  are permitted provided that the following conditions are met:

  * Redistributions of source code must retain the above copyright notice, this list 
    of conditions and the following disclaimer.
    
  * Redistributions in binary form must reproduce the above copyright notice, this 
    list of conditions and the following disclaimer in the documentation and/or other 
    materials provided with the distribution.

  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND 
  CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
  INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF 
  MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
  DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, 
  SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT 
  NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
  LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER 
  CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, 
  STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) 
  ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF 
  ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.  


  Use DC2 bitmap command of the A2 Micro panel termal printer
  double stroke
  
  
*/


#include "u8x8.h"

#define LINE_MIN_DELAY_MS 15
/* higher values improve quality */
/* however if the value is too high (>=5) then form feed does not work any more */
#define LINE_EXTRA_8PIXEL_DELAY_MS 3
/* this must be a power of two and between 1 and 8 */
/* best quality only with 1 */
#define NO_OF_LINES_TO_SEND_WITHOUT_DELAY 1

/* calculates the delay, based on the number of black pixel */
/* actually only "none-zero" bytes are calculated which is, of course not so accurate, but should be good enough */
uint16_t get_delay_in_milliseconds(uint8_t cnt, uint8_t *data)
{
  uint8_t i;
  uint16_t time = LINE_MIN_DELAY_MS;
  for ( i = 0; i < cnt; i++ )
    if ( data[i] != 0 )
      time += LINE_EXTRA_8PIXEL_DELAY_MS;
  return time;
}

uint8_t u8x8_d_a2printer_common(u8x8_t *u8x8, uint8_t msg, U8X8_UNUSED uint8_t arg_int, void *arg_ptr)
{
  uint8_t c, i, j;
  uint8_t *ptr;
  uint16_t delay_in_milliseconds;
  switch(msg)
  {
    /* U8X8_MSG_DISPLAY_SETUP_MEMORY is handled by the calling function */
    /*
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      break;
    */
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      // no setup required
      // u8x8_cad_SendSequence(u8x8, u8x8_d_a2printer_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      // no powersave 
      break;
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
      
      u8x8_cad_SendCmd(u8x8, 27);      /* ESC */
      u8x8_cad_SendCmd(u8x8, 55 );      /* parameter command */
      /* increasing the "max printing dots" requires a good power supply, but LINE_EXTRA_8PIXEL_DELAY_MS could be reduced then */
      u8x8_cad_SendCmd(u8x8, 0);      /* Max printing dots,Unit(8dots),Default:7(64 dots) 8*(x+1) ... lower values improve, probably my current supply is not sufficient */
      u8x8_cad_SendCmd(u8x8, 200);      /* 3-255 Heating time,Unit(10us),Default:80(800us) */
      u8x8_cad_SendCmd(u8x8, 2);      /* 0-255 Heating interval,Unit(10us),Default:2(20us) ... does not have much influence */
      
      //c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */
      c = u8x8->display_info->tile_width;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;	/* data ptr to the tiles */
    
      u8x8_cad_SendCmd(u8x8, 18);      /* DC2 */
      u8x8_cad_SendCmd(u8x8, 42 );      /* *  */
      u8x8_cad_SendCmd(u8x8, 8 ); 	/* height */
      u8x8_cad_SendCmd(u8x8, c ); 	/* c, u8x8->display_info->tile_width */
      
      for( j = 0; j < 8 / NO_OF_LINES_TO_SEND_WITHOUT_DELAY; j ++ )
      {

	delay_in_milliseconds = 0;
	for( i = 0; i < NO_OF_LINES_TO_SEND_WITHOUT_DELAY; i++ )
	{
	  u8x8_cad_SendData(u8x8, c, ptr);	/* c, note: SendData can not handle more than 255 bytes, send one line of data */
	  delay_in_milliseconds += get_delay_in_milliseconds(c, ptr);
	  ptr += c;
	}
	
	while( delay_in_milliseconds > 200 )
	{
	  u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_MILLI, 200, NULL);	
	  delay_in_milliseconds -= 200;
	}
	u8x8->gpio_and_delay_cb(u8x8, U8X8_MSG_DELAY_MILLI, delay_in_milliseconds, NULL);	
      }

      /* set parameters back to their default values */
      u8x8_cad_SendCmd(u8x8, 27);      /* ESC */
      u8x8_cad_SendCmd(u8x8, 55 );      /* parameter command */
      u8x8_cad_SendCmd(u8x8, 7);      /* Max printing dots,Unit(8dots),Default:7(64 dots) 8*(x+1)*/
      u8x8_cad_SendCmd(u8x8, 80);      /* 3-255 Heating time,Unit(10us),Default:80(800us) */
      u8x8_cad_SendCmd(u8x8, 2);      /* 0-255 Heating interval,Unit(10us),Default:2(20us)*/

      u8x8_cad_EndTransfer(u8x8);

      break;
    default:
      return 0;
  }
  return 1;
}


static const u8x8_display_info_t u8x8_a2printer_384x240_display_info =
{
  /* most of the settings are not required, because this is a serial RS232 printer */
  
  /* chip_enable_level = */ 1,
  /* chip_disable_level = */ 0,
  
  /* post_chip_enable_wait_ns = */ 5,
  /* pre_chip_disable_wait_ns = */ 5,
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 20,		
  /* sck_pulse_width_ns = */  140,	
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* old: sck_takeover_edge, new: active high (bit 1), rising edge (bit 0) */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,
  /* write_pulse_width_ns = */ 40,
  /* tile_width = */ 48,
  /* tile_hight = */ 30,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 384,
  /* pixel_height = */ 240
};

uint8_t u8x8_d_a2printer_384x240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_a2printer_384x240_display_info);
      break;
    default:
      return u8x8_d_a2printer_common(u8x8, msg, arg_int, arg_ptr);
  }
  return 1;
}



  

  
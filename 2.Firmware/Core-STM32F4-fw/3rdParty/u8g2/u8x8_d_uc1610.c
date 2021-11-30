/*

  u8x8_d_uc1610.c
  
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

  
  cad001
  
*/
#include "u8x8.h"




static const uint8_t u8x8_d_uc1610_dogxl160_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_CA(0x0f1, 0x067),		/* set COM end (display height-1) */
  U8X8_C(0x0c0),            			/* SEG & COM normal */
  U8X8_C(0x040),            			/* set scroll line lsb to zero */
  U8X8_C(0x050),            			/* set scroll line msb to zero */
  U8X8_C(0x02b),            			/* set panelloading */
  U8X8_C(0x0eb),            			/* set bias 1/2 */  
  U8X8_CA(0x081, 0x05f),            	/* set contrast */
  
  /*
    AC0:	0: stop at boundary, 1: increment by one
    AC1: 	0: first column then page, 1: first page, then column increment
    AC2:	0: increment page adr, 1: decrement page adr.
  */
  U8X8_C(0x08b),            			/* set auto increment, low bits are AC2 AC1 AC0 */
  
  /*
    LC0:	0
    MX:	Mirror X
    MY:	Mirror Y
  */  
  U8X8_C(0x0c0),            			/* low bits are MY, MX, LC0 */
  
  U8X8_C(0x0f8),            			// window mode off
  U8X8_C(0x010),		                // col high
  U8X8_C(0x000),		                // col low
  U8X8_C(0x0b0),		                // page
  
  U8X8_C(0x0a6),            			/* set normal pixel mode (not inverse) */
  U8X8_C(0x0a4),            			/* set normal pixel mode (not all on) */

  /* test code 
  U8X8_C(0x0af),		                // display on 
  U8X8_C(0x0f8),            			// window mode off
  U8X8_CA(0x0f4, 0),			// set window
  U8X8_CA(0x0f5, 0),
  U8X8_CA(0x0f6, 4),
  U8X8_CA(0x0f7, 1),
  U8X8_C(0x0f9),            			// window mode on
  U8X8_D1(0x03),
  U8X8_D1(0x0c0),
  U8X8_D1(0x0ff),
  U8X8_D1(0x0ff),
  U8X8_D1(0x0ff),
  U8X8_D1(0x0ff),
  U8X8_D1(0x0ff),
  U8X8_D1(0x0ff),
  */
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1610_dogxl160_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0af),		                /* display on, UC1610 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1610_dogxl160_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off,  UC1610 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1610_dogxl160_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  /*
    LC0:	0
    MX:	Mirror X
    MY:	Mirror Y
  */  
  U8X8_C(0x0c0),            			/* low bits are MY, MX, LC0 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1610_dogxl160_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  /*
    LC0:	0
    MX:	Mirror X
    MY:	Mirror Y
  */  
  U8X8_C(0x0c6),            			/* low bits are MY, MX, LC0 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


/* 
  UC1610 has two chip select inputs (CS0 and CS1).
  CS0 is low active, CS1 is high active. It will depend on the display
  module whether the display has a is low or high active chip select.
*/

static const u8x8_display_info_t u8x8_uc1610_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 15,
  /* pre_chip_disable_wait_ns = */ 15,
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 6, 
  /* sda_setup_time_ns = */ 30,	
  /* sck_pulse_width_ns = */ 63,	/* half of cycle time (125ns cycle time according to datasheet) --> 8MHz clock */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,
  /* write_pulse_width_ns = */ 40,
  /* tile_width = */ 20,		
  /* tile_hight = */ 13,		/* height of 13*8=104 pixel */
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 160,
  /* pixel_height = */ 104
};


/*
  RAM Organization:
  D0  Pix0
  D1
  D2  Pix1
  D3
  D4  Pix2
  D5
  D6  Pix3
  D7    
  D0  Pix4
  D1
  D2  Pix5
  D3
  D4  Pix6
  D5
  D6  Pix7
  D7    


*/
static uint8_t *u8x8_convert_tile_for_uc1610(uint8_t *t)
{
  uint8_t i;
  uint16_t r;
  static uint8_t buf[16];
  uint8_t *pbuf = buf;

  for( i = 0; i < 8; i++ )
  {
    r = u8x8_upscale_byte(*t++);
    *pbuf++ = r & 255;
    r >>= 8;
    *pbuf++ = r;
  }
  return buf;
}

uint8_t u8x8_d_uc1610_ea_dogxl160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c, page;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_uc1610_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_uc1610_dogxl160_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1610_dogxl160_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1610_dogxl160_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1610_dogxl160_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1610_dogxl160_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int  );	/* uc1610 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);

      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
    
      page = (((u8x8_tile_t *)arg_ptr)->y_pos);
      page *= 2;

      u8x8_cad_SendCmd(u8x8, 0x0f8 );	/* window disable */
      
      //u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
      //u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      //u8x8_cad_SendCmd(u8x8, 0x0b0 | page);

      u8x8_cad_SendCmd(u8x8, 0x0f4 );	/* window start column */
      u8x8_cad_SendArg(u8x8, x);
      u8x8_cad_SendCmd(u8x8, 0x0f5 );	/* window start page */
      u8x8_cad_SendArg(u8x8, page);
      u8x8_cad_SendCmd(u8x8, 0x0f6 );	/* window end column */
      u8x8_cad_SendArg(u8x8, 159);		/* end of display */
      u8x8_cad_SendCmd(u8x8, 0x0f7 );	/* window end page */
      u8x8_cad_SendArg(u8x8, page+1);
      u8x8_cad_SendCmd(u8x8, 0x0f9 );	/* window enable */
    
      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	do
	{
	  u8x8_cad_SendData(u8x8, 16, u8x8_convert_tile_for_uc1610(ptr));
	  ptr += 8;
	  x += 8;
	  c--;
	} while( c > 0 );
	
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);

    
      break;
    default:
      return 0;
  }
  return 1;
}



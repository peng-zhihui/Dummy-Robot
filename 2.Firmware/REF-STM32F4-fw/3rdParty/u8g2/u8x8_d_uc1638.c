/*

  u8x8_d_uc1638.c
  
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


  
*/
#include "u8x8.h"





static const uint8_t u8x8_d_uc1638_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0c9, 0x0ad),		                /* display on */   /* UC1638 B/W mode */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1638_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x0c9, 0x0ac),		                /* display off */   /* UC1638 */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1638_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0c4),            	/* LCD Mapping */    /* UC1638*/
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1638_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0c2),            	/* LCD Mapping */    /* UC1638*/
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_uc1638_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, y, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;

      u8x8_cad_SendCmd(u8x8, 0x004);  /* UC1638 */
      u8x8_cad_SendArg(u8x8, x);
    
      y = ((u8x8_tile_t *)arg_ptr)->y_pos;
      y += u8x8->x_offset;
      y *= 2;		/* for B/W mode, use only every second page */

      u8x8_cad_SendCmd(u8x8, 0x060 | (y&15));  /* UC1638 */
      u8x8_cad_SendCmd(u8x8, 0x070 | (y>>4));  /* UC1638 */
    
    
      u8x8_cad_SendCmd(u8x8, 0x001); /* UC1638 */
      c = ((u8x8_tile_t *)arg_ptr)->cnt;
      c *= 8;
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
      do
      {
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    /*	handled in the calling procedure 
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_uc1638_128x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_uc1701_dogs102_init_seq);
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1638_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1638_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1638_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1638_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int  );	/* uc1638 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    default:
      return 0;
  }
  return 1;
}


/*================================================*/
/* uc1638 160x128 */

/* values taken from uc1608 */
static const u8x8_display_info_t u8x8_uc1638_160x128_display_info =
{
  /* chip_enable_level = */ 1,	/* uc1638 has high active CS */
  /* chip_disable_level = */ 0,
  
  /* post_chip_enable_wait_ns = */ 10,	/* */
  /* pre_chip_disable_wait_ns = */ 20,	/* */
  /* reset_pulse_width_ms = */ 5, 	/* */
  /* post_reset_wait_ms = */ 150, 	
  /* sda_setup_time_ns = */ 30,		/* */
  /* sck_pulse_width_ns = */ 65,	/* */
  /* sck_clock_hz = */ 1000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 3,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,	/*  */
  /* write_pulse_width_ns = */ 35,	/*  */
  /* tile_width = */ 20,		/* width of 20*8=160 pixel */
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,	/*  */
  /* flipmode_x_offset = */ 0,	/* */
  /* pixel_width = */ 160,
  /* pixel_height = */ 128
};

static const uint8_t u8x8_d_uc1638_160x128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_CA(0x0e1, 0x0e2),		/* software reset */    /* UC1638*/
  U8X8_DLY(5),					/* 5 ms */	

  U8X8_C(0x024),            		/*	 set temp comp*/
  U8X8_C(0x0c0),            		/*	mirror y and mirror x */  /* WAS: c2 */
  U8X8_C(0x0a2),            		/*	line rate */
  U8X8_C(0x0d6),            		/*	gray scale 2 */
  U8X8_C(0x0eb),            		/*	 set bias*/
  U8X8_C(0x095),            		/*	 set 1 bit per pixel, pattern 0*/
  U8X8_C(0x089),            		/*	 set auto increment, low bits are AC2 AC1 AC0 */  /* WAS 89 */


  U8X8_CA(0x081, 0x0a0),		/* set contrast */    /* UC1638*/
  
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_uc1638_160x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call common procedure first and handle messages there */
  if ( u8x8_d_uc1638_common(u8x8, msg, arg_int, arg_ptr) == 0 )
  {
    /* msg not handled, then try here */
    switch(msg)
    {
      case U8X8_MSG_DISPLAY_SETUP_MEMORY:
	u8x8_d_helper_display_setup_memory(u8x8, &u8x8_uc1638_160x128_display_info);
	break;
      case U8X8_MSG_DISPLAY_INIT:
	u8x8_d_helper_display_init(u8x8);
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1638_160x128_init_seq);
	break;
      default:
	return 0;		/* msg unknown */
    }
  }
  return 1;
}


/*================================================*/

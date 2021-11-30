/*

  u8x8_d_uc1604.c
  
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





static const uint8_t u8x8_d_uc1604_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_DLY(20),			
  U8X8_C(0x0af),		                /* display on */
  U8X8_DLY(20),				/* during setup, it seems that the startup is more reliable when sending this cmd twice */
  U8X8_C(0x0af),		                /* display on */
  U8X8_DLY(50),				/* startup takes some time */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1604_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0ae),		                /* display off, enter sleep mode */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1604_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0c4),            			/* LCD Mapping */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_uc1604_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0x0c2),            			/* LCD Mapping */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_uc1604_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, y, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
   
      u8x8_cad_SendCmd(u8x8, 0x000 | ((x&15)));
      u8x8_cad_SendCmd(u8x8, 0x010 | (x>>4) );
    
      y = ((u8x8_tile_t *)arg_ptr)->y_pos;
      y += u8x8->x_offset;
      u8x8_cad_SendCmd(u8x8, 0x0b0 | (y&15));
    
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
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_uc1604_128x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_uc1701_dogs102_init_seq);
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1604_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1604_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1604_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1604_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int  );	/* uc1604 has range from 0 to 255 */
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    default:
      return 0;
  }
  return 1;
}

/*================================================*/
/* JLX19264 */

/* 
  timings from uc1608 

  UC1604 has two chip select inputs (CS0 and CS1).
  CS0 is low active, CS1 is high active. It will depend on the display
  module whether the display has a is low or high active chip select.

*/
static const u8x8_display_info_t u8x8_uc1604_192x64_display_info =
{
  /* chip_enable_level = */ 0,	/* JLX19264G uses CS0, which is low active CS*/
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,	
  /* pre_chip_disable_wait_ns = */ 20,	
  /* reset_pulse_width_ms = */ 1, 	
  /* post_reset_wait_ms = */ 10, 	
  /* sda_setup_time_ns = */ 30,		
  /* sck_pulse_width_ns = */ 65,	/* half of cycle time  */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,	
  /* write_pulse_width_ns = */ 35,	
  /* tile_width = */ 24,		/* width of 24*8=192 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,	/* reused as y page offset */
  /* flipmode_x_offset = */ 0,	/* reused as y page offset */
  /* pixel_width = */ 192,
  /* pixel_height = */ 64
};

static const uint8_t u8x8_d_uc1604_jlx19264_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */

  U8X8_C(0x0e2),            			/* soft reset */
  U8X8_DLY(200),
  U8X8_DLY(200),

  U8X8_C(0x02f),            			/* power on, Bit 2 PC2=1 (internal charge pump), Bits 0/1: cap of panel */
  U8X8_DLY(200),
  U8X8_DLY(200),
  
  U8X8_CA(0x081, 0x052),		/* set contrast, JLX19264G suggestion: 0x045 */
  U8X8_C(0x0eb),            			/* LCD bias Bits 0/1: 00=6 01=7, 10=8, 11=9 */

  
  //U8X8_C(0x023),            			/* Bit 0/1: Temp compenstation, Bit 2: Multiplex Rate 0=96, 1=128 */
  //U8X8_C(0x027),            			/* Bit 0/1: Temp compenstation, Bit 2: Multiplex Rate 0=96, 1=128 */

  U8X8_C(0x0c4),            			/* Map control, Bit 2: MY=1, Bit 1: MX=0 */
  U8X8_C(0x0a0),            			/* 0xa0: 76Hz FPS, controller default: 0x0a1: 95Hz FPS */
  
  
  U8X8_C(0x040),            			/* set scroll line to 0 */
  U8X8_C(0x089),            			/* RAM access control (controller default: 0x089)*/
  
  
  U8X8_C(0x000),		                /* column low nibble */
  U8X8_C(0x010),		                /* column high nibble */  
  U8X8_C(0x0b0),		                /* page adr  */
  
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_uc1604_jlx19264(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call common procedure first and handle messages there */
  if ( u8x8_d_uc1604_common(u8x8, msg, arg_int, arg_ptr) == 0 )
  {
    /* msg not handled, then try here */
    switch(msg)
    {
      case U8X8_MSG_DISPLAY_SETUP_MEMORY:
	u8x8_d_helper_display_setup_memory(u8x8, &u8x8_uc1604_192x64_display_info);
	break;
      case U8X8_MSG_DISPLAY_INIT:
	u8x8_d_helper_display_init(u8x8);
	u8x8_cad_SendSequence(u8x8, u8x8_d_uc1604_jlx19264_init_seq);
	break;
      default:
	return 0;		/* msg unknown */
    }
  }
  return 1;
}



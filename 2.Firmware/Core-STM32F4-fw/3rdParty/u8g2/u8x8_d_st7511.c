/*

  u8x8_d_st7511.c
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2019, olikraus@gmail.com
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


  20 May 2019:
  https://github.com/olikraus/u8g2/issues/876
  Probably HW Flip does not work 

  
*/
#include "u8x8.h"




static const uint8_t u8x8_d_st7511_320x240_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x015, 0x0a5),		/* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7511_320x240_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(0x014, 0x0a5),		/* display off */
  // maybe use sleep mode here, but it not clear whether sleep mode will reset all the settings
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7511_320x240_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CAAAA(0x24, 0x01, 0xa5, 0xa5, 0xa5),		/* memory control directions */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st7511_320x240_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CAAAA(0x24, 0x02, 0xa5, 0xa5, 0xa5),		/* memory control directions */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};



/*=====================================================*/
/* AV-Display: AVD-TM57QV-NW-001-B, issue 876 */

static const u8x8_display_info_t u8x8_st7511_320x240_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 150,	/* ST7511 Datasheet */
  /* pre_chip_disable_wait_ns = */ 150,	/* ST7511 Datasheet */
  /* reset_pulse_width_ms = */ 1, 
  /* post_reset_wait_ms = */ 1, 
  /* sda_setup_time_ns = */ 120,		/* ST7511 Datasheet */
  /* sck_pulse_width_ns = */ 150,	/* ST7511 Datasheet */
  /* sck_clock_hz = */ 3300000UL,	/* ST7511 Datasheet: 300ns cycle */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 200,	/* */
  /* write_pulse_width_ns = */ 250,	/* ST7511 Datasheet: 500ns */
  /* tile_width = */ 40,		/* width of 17*8=136 pixel */
  /* tile_hight = */ 30,
  /* default_x_offset = */ 160,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 320,
  /* pixel_height = */ 240
};

static const uint8_t u8x8_d_st7511_320x240_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_CA(0xae, 0xa5),						/* SW Reset */
  U8X8_CAAAA(0x61, 0x0f, 0x04, 0x02, 0xa5),	/* all power on */
  U8X8_CAAAA(0x62, 0x0a, 0x06, 0x0f, 0xa5),	/* electronic volumne set 1 */
  U8X8_CAAAA(0x63, 0x0f, 0x0f, 0xa5, 0xa5),		/* electronic volumne set 2 */
  U8X8_CAAAA(0x66, 0x00, 0xa5, 0xa5, 0xa5),		/* electronic volumne set 2 */
  U8X8_CA(0x12, 0xa5),						/* SLeeP OUT */
  U8X8_DLY(50),
  // skiping display on here, deviation from https://github.com/olikraus/u8g2/issues/876
  // will be called later in u8x8_d_st7511_320x240_powersave0_seq
  U8X8_CAAAA(0x22, 0x00, 0xa5, 0xa5, 0xa5),		/* monochrome display */
  U8X8_CAAAA(0x24, 0x01, 0xa5, 0xa5, 0xa5),		/* memory control directions */

  U8X8_DLY(50),
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_st7511_avd_320x240(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint16_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st7511_320x240_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st7511_320x240_init_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7511_320x240_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7511_320x240_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7511_320x240_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_st7511_320x240_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      // not sure how to implement this....
      // u8x8_cad_StartTransfer(u8x8);
      // u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);

      // set page
      u8x8_cad_SendCmd(u8x8, 0x025);
      u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
      u8x8_cad_SendArg(u8x8, 0x09f);		// end page
      u8x8_cad_SendArg(u8x8, 0x000);		// frame 0
      u8x8_cad_SendArg(u8x8, 0x0a5);		
    
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x *= 8;
      x += u8x8->x_offset;
    
      // set column
      u8x8_cad_SendCmd(u8x8, 0x026);
      u8x8_cad_SendArg(u8x8, (x>>8) );
      u8x8_cad_SendArg(u8x8, (x&255) );
      u8x8_cad_SendArg(u8x8, 0x002);
      u8x8_cad_SendArg(u8x8, 0x07f);

      // start data transfer
      u8x8_cad_SendCmd(u8x8, 0x02c);
      u8x8_cad_SendArg(u8x8, 0x0a5 );

      do
      {
	c = ((u8x8_tile_t *)arg_ptr)->cnt;
	c *= 8;
	ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
	
	while ( c > 128 )
	{
	  u8x8_cad_SendData(u8x8, 128, ptr);	/* note: SendData can not handle more than 255 bytes */
	  c -= 128;
	  ptr += 128;
	}
	u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes */
	arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}




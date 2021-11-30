/*

  u8x8_d_s1d15721.c

  240x64 display
  https://github.com/olikraus/u8g2/issues/1473  
  http://datasheet.datasheetarchive.com/originals/library/Datasheets-ISS16/DSAIH000309343.pdf
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)
  
  Copyright (c) 2020, olikraus@gmail.com
  
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

static const uint8_t u8x8_d_s1d15721_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0xA8),		                /* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_s1d15721_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C(0xA8|1),		                /* display off, enter sleep mode */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_s1d15721_flip0_seq[] = {
  U8X8_START_TRANSFER(),            /* enable chip, delay is part of the transfer start */
  U8X8_C(0xA6),            			/* LCD Mapping */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_s1d15721_flip1_seq[] = {
  U8X8_START_TRANSFER(),            /* enable chip, delay is part of the transfer start */
  U8X8_C(0xA7),            			/* LCD Mapping */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_s1d15721_common(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, y, c;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      u8x8_cad_StartTransfer(u8x8);

      x = ((u8x8_tile_t *)arg_ptr)->x_pos;
      x += u8x8->x_offset;
      x *= 8;

	  u8x8_cad_SendCmd(u8x8, 0xB1);	//Page Address - Row
	  u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));

      y = ((u8x8_tile_t *)arg_ptr)->y_pos;

      u8x8_cad_SendCmd(u8x8, 0x13);	/* col */
      u8x8_cad_SendArg(u8x8, x);

	  u8x8_cad_SendCmd(u8x8, 0x1D );	//Data Write

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
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_uc1608_128x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_uc1701_dogs102_init_seq);
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_s1d15721_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_s1d15721_powersave1_seq);
      break;
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_s1d15721_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_s1d15721_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      break;
#endif
    default:
      return 0;
  }
  return 1;
}

/*================================================*/
/* s1d15721 240x64 */


static const u8x8_display_info_t u8x8_s1d15721_240x64_display_info =
{
  /* chip_enable_level = */ 0,	/* low active CS */
  /* chip_disable_level = */ 1,

  /* post_chip_enable_wait_ns = */ 10,	/* */
  /* pre_chip_disable_wait_ns = */ 20,	        /* */
  /* reset_pulse_width_ms = */ 1, 	        /* */
  /* post_reset_wait_ms = */ 10, 	
  /* sda_setup_time_ns = */ 30,		/*  */
  /* sck_pulse_width_ns = */ 65,	/* half of cycle time  */
  /* sck_clock_hz = */ 8000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 30,	         /*  */
  /* write_pulse_width_ns = */ 65,	/* */
  /* tile_width = */ 30,		                /* width of 20*8=160 pixel (30*8 = 240) */
  /* tile_hight = */ 8,                 /* height 8*8 = 64*/
  /* default_x_offset = */ 1,	
  /* flipmode_x_offset = */ 1,	
  /* pixel_width = */ 240,
  /* pixel_height = */ 64
};

static const uint8_t u8x8_d_s1d15721_240x64_init_seq[] = {
  U8X8_START_TRANSFER(),           	/* enable chip, delay is part of the transfer start */
  U8X8_C(0xC4|1),       	    	        /* (5) Common Output Status (Reverse) */
  U8X8_CA(0xA6, 0x01),            	/* (3) Display Normal Reverse (Normal) */
  U8X8_CA(0xA4, 0x00), 	           	/* (4) Display All Light (Normal) */
  U8X8_CAA(0x6D,0x10,0x02), 		/* (18) Duty Set Command */
  U8X8_CA(0x66, 0x01),           	/* (15) Display Mode, Parameter 0 (0 = Gray Scale 1 = Binary */
  U8X8_CA(0x39, 0x36),            	/* (16) Gray Scale Pattern Set, Pattern */
  U8X8_CA(0x2B, 0x07),			/* (27) LCD Drive Mode Voltage Select, Parameter */
  U8X8_CA(0x81, 0x0a),		        /* (28) Electronic Volume, Parameter */
  U8X8_CA(0x5F, 0x00),		        /* (24) Built-in Oscillator Frequency, Parameter  */
  U8X8_C(0xAA|1),			        /* (23) Built-in OSC On */  
  U8X8_CA(0x25, 0x1f),		        /* (25) Power Control Set, Parameter  */
  U8X8_CA(0x8A, 0x00),		        /* (6) Start Line Setup, Parameter  */
  U8X8_CA(0xB1, 0x00),		        /* (7) Page Address Set, Parameter  */
  U8X8_CA(0x13, 0x00),		        /* (8) Column Address Set  */
  U8X8_C(0xAE|1),			        /* (1) Display ON/OFF */  
  U8X8_END_TRANSFER(),           /* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_s1d15721_240x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{

  /* checking for the flip mode cmd first */
  if ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_s1d15721_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_s1d15721_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      return 1;
  }
  /* call the common procedure, this now leads to the effect, that the flip code is executed again */
  /* maybe we should paste the common code here to avoid this */


  if ( u8x8_d_s1d15721_common(u8x8, msg, arg_int, arg_ptr) == 0 )
  {
    /* msg not handled, then try here */
    switch(msg)
    {
      case U8X8_MSG_DISPLAY_SETUP_MEMORY:
	u8x8_d_helper_display_setup_memory(u8x8, &u8x8_s1d15721_240x64_display_info);
	break;
      case U8X8_MSG_DISPLAY_INIT:
	u8x8_d_helper_display_init(u8x8);
	u8x8_cad_SendSequence(u8x8, u8x8_d_s1d15721_240x64_init_seq);
	break;
      default:
	return 0;		/* msg unknown */
    }
  }
  return 1;
}

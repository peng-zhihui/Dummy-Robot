/*

  u8x8_d_lc7981.c
  
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




/* no powersave mode for the LC7981 */
// static const uint8_t u8x8_d_lc7981_powersave0_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };

// static const uint8_t u8x8_d_lc7981_powersave1_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };

/* no hardware flip for the LC7981 */
// static const uint8_t u8x8_d_lc7981_flip0_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };

// static const uint8_t u8x8_d_lc7981_flip1_seq[] = {
//   U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
//   U8X8_END_TRANSFER(),             	/* disable chip */
//   U8X8_END()             			/* end of sequence */
// };


/* http://graphics.stanford.edu/~seander/bithacks.html */
static uint8_t reverse_byte(uint8_t v)
{
  // if ( v != 0 && v != 255 )  does not help much
  {
    // swap odd and even bits
    v = ((v >> 1) & 0x055) | ((v & 0x055) << 1);
    // swap consecutive pairs
    v = ((v >> 2) & 0x033) | ((v & 0x033) << 2);
    // swap nibbles ... 
    v = ((v >> 4) & 0x00F) | ((v & 0x00F) << 4);
  }
  return v;
}

static uint8_t u8x8_d_lc7981_common(u8x8_t *u8x8, uint8_t msg, U8X8_UNUSED uint8_t arg_int, void *arg_ptr)
{
  uint8_t c, i, j;
  uint16_t y;
  uint8_t *ptr;
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      y = (((u8x8_tile_t *)arg_ptr)->y_pos);
      y*=8;
      y*= u8x8->display_info->tile_width;
      /* x = ((u8x8_tile_t *)arg_ptr)->x_pos; x is ignored... no u8x8 support */
      u8x8_cad_StartTransfer(u8x8);
      /* 
	Tile structure is reused here for the t6963, however u8x8 is not supported 
	tile_ptr points to data which has cnt*8 bytes (same as SSD1306 tiles)
	Buffer is expected to have 8 lines of code fitting to the t6963 internal memory
	"cnt" includes the number of horizontal bytes. width is equal to cnt*8
	
	x is assumed to be zero
    
	TODO: Consider arg_int, however arg_int is not used by u8g2
      */
      c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;	/* data ptr to the tiles */
      for( i = 0; i < 8; i++ )
      {
	u8x8_cad_SendCmd(u8x8, 0x0a );	/* display ram (cursor) address low byte */
	u8x8_cad_SendArg(u8x8, y&255);
	u8x8_cad_SendCmd(u8x8, 0x0b );	/* display ram (cursor) address high byte */
	u8x8_cad_SendArg(u8x8, y>>8);
	
	u8x8_cad_SendCmd(u8x8, 0x0c );	/* write start */
	/*
	  The LC7981 has the MSB at the right position, which is exactly the opposite to the T6963.
	  Instead of writing a third hvline procedure for this device, we just revert the bytes before 
	  transmit. This is slow because:
	    - the bit reverse itself
	    - the single byte transfer 
	   The one byte is transmitted via SendArg, which is ok, because CAD = 100
	*/
	for( j = 0; j < c; j++ )
	  u8x8_cad_SendArg(u8x8, reverse_byte(*ptr++));
	
	//u8x8_cad_SendData(u8x8, c, ptr);	/* note: SendData can not handle more than 255 bytes, send one line of data */
	//ptr += u8x8->display_info->tile_width;
	
	y += u8x8->display_info->tile_width;
      }

      u8x8_cad_EndTransfer(u8x8);
      break;
    /*	handled in the calling procedure 
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_lc7981_128x64_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_uc1701_dogs102_init_seq);
      break;
    */
    /* power save is not there... 
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_lc7981_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_lc7981_powersave1_seq);
      break;
    */
    /* hardware flip not is not available
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      if ( arg_int == 0 )
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_lc7981_flip0_seq);
	u8x8->x_offset = u8x8->display_info->default_x_offset;
      }
      else
      {
	u8x8_cad_SendSequence(u8x8, u8x8_d_lc7981_flip1_seq);
	u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
      }	
      break;
    */
#ifdef U8X8_WITH_SET_CONTRAST
    /* no contrast setting :-(
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      u8x8_cad_SendCmd(u8x8, 0x081 );
      u8x8_cad_SendArg(u8x8, arg_int  );
      u8x8_cad_EndTransfer(u8x8);
      break;
  */
#endif
    default:
      return 0;
  }
  return 1;
}

/*================================================*/
/* LC7981 160x80 LCD*/

static const u8x8_display_info_t u8x8_lc7981_160x80_display_info =
{
  /* chip_enable_level = */ 0,	/* LC7981 has a low active CS*/
  /* chip_disable_level = */ 1,
  
  /* from here... */
  /* post_chip_enable_wait_ns = */ 20,	
  /* pre_chip_disable_wait_ns = */ 20,	
  /* reset_pulse_width_ms = */ 1, 	
  /* post_reset_wait_ms = */ 10, 	
  /* sda_setup_time_ns = */ 30,		
  /* sck_pulse_width_ns = */ 65,	/* half of cycle time  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* ... to here, values are ignored, because this is a parallel interface only */
  
  /* data_setup_time_ns = */ 220,	
  /* write_pulse_width_ns = */ 20,	
  /* tile_width = */ 20,		/* width of 20*8=160 pixel */
  /* tile_hight = */ 10,
  /* default_x_offset = */ 0,	
  /* flipmode_x_offset = */ 0,	
  /* pixel_width = */ 160,
  /* pixel_height = */ 80
};

static const uint8_t u8x8_d_lc7981_160x80_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(50),

  U8X8_CA(0x00, 0x32),			/* display on (bit 5), master mode on (bit 4), graphics mode on (bit 1) */
  U8X8_CA(0x01, 0x07),			/* character/bits per pixel pitch */
  U8X8_CA(0x02, 160/8-1),		/* number of chars/byte width of the screen */
  U8X8_CA(0x03, 0x50),			/* time division:  50 (1/80 duty cycle) */
  U8X8_CA(0x08, 0x00),			/* display start low */
  U8X8_CA(0x09, 0x00),			/* display start high */

  U8X8_DLY(10),
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_lc7981_160x80(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call common procedure first and handle messages there */
  if ( u8x8_d_lc7981_common(u8x8, msg, arg_int, arg_ptr) == 0 )
  {
    /* msg not handled, then try here */
    switch(msg)
    {
      case U8X8_MSG_DISPLAY_SETUP_MEMORY:
	u8x8_d_helper_display_setup_memory(u8x8, &u8x8_lc7981_160x80_display_info);
	break;
      case U8X8_MSG_DISPLAY_INIT:
	u8x8_d_helper_display_init(u8x8);
	u8x8_cad_SendSequence(u8x8, u8x8_d_lc7981_160x80_init_seq);
	break;
      default:
	return 0;		/* msg unknown */
    }
  }
  return 1;
}


/*================================================*/
/* LC7981 160x160 LCD*/

static const u8x8_display_info_t u8x8_lc7981_160x160_display_info =
{
  /* chip_enable_level = */ 0,	/* LC7981 has a low active CS*/
  /* chip_disable_level = */ 1,
  
  /* from here... */
  /* post_chip_enable_wait_ns = */ 20,	
  /* pre_chip_disable_wait_ns = */ 20,	
  /* reset_pulse_width_ms = */ 1, 	
  /* post_reset_wait_ms = */ 10, 	
  /* sda_setup_time_ns = */ 30,		
  /* sck_pulse_width_ns = */ 65,	/* half of cycle time  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* ... to here, values are ignored, because this is a parallel interface only */
  
  /* data_setup_time_ns = */ 220,	
  /* write_pulse_width_ns = */ 20,	
  /* tile_width = */ 20,		/* width of 20*8=160 pixel */
  /* tile_hight = */ 20,
  /* default_x_offset = */ 0,	
  /* flipmode_x_offset = */ 0,	
  /* pixel_width = */ 160,
  /* pixel_height = */ 160
};

static const uint8_t u8x8_d_lc7981_160x160_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(50),

  U8X8_CA(0x00, 0x32),			/* display on (bit 5), master mode on (bit 4), graphics mode on (bit 1) */
  U8X8_CA(0x01, 0x07),			/* character/bits per pixel pitch */
  U8X8_CA(0x02, 160/8-1),		/* number of chars/byte width of the screen */
  U8X8_CA(0x03, 159),			/* time division */
  U8X8_CA(0x08, 0x00),			/* display start low */
  U8X8_CA(0x09, 0x00),			/* display start high */

  U8X8_DLY(10),
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_lc7981_160x160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call common procedure first and handle messages there */
  if ( u8x8_d_lc7981_common(u8x8, msg, arg_int, arg_ptr) == 0 )
  {
    /* msg not handled, then try here */
    switch(msg)
    {
      case U8X8_MSG_DISPLAY_SETUP_MEMORY:
	u8x8_d_helper_display_setup_memory(u8x8, &u8x8_lc7981_160x160_display_info);
	break;
      case U8X8_MSG_DISPLAY_INIT:
	u8x8_d_helper_display_init(u8x8);
	u8x8_cad_SendSequence(u8x8, u8x8_d_lc7981_160x160_init_seq);
	break;
      default:
	return 0;		/* msg unknown */
    }
  }
  return 1;
}


/*================================================*/
/* LC7981 240x128 LCD*/

static const u8x8_display_info_t u8x8_lc7981_240x128_display_info =
{
  /* chip_enable_level = */ 0,	/* LC7981 has a low active CS*/
  /* chip_disable_level = */ 1,
  
  /* from here... */
  /* post_chip_enable_wait_ns = */ 20,	
  /* pre_chip_disable_wait_ns = */ 20,	
  /* reset_pulse_width_ms = */ 1, 	
  /* post_reset_wait_ms = */ 10, 	
  /* sda_setup_time_ns = */ 30,		
  /* sck_pulse_width_ns = */ 65,	/* half of cycle time  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* ... to here, values are ignored, because this is a parallel interface only */
  
  /* data_setup_time_ns = */ 220,	
  /* write_pulse_width_ns = */ 20,	
  /* tile_width = */ 30,		/* width of 30*8=240 pixel */
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,	
  /* flipmode_x_offset = */ 0,	
  /* pixel_width = */ 240,
  /* pixel_height = */ 128
};

static const uint8_t u8x8_d_lc7981_240x128_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(50),

  U8X8_CA(0x00, 0x32),			/* display on (bit 5), master mode on (bit 4), graphics mode on (bit 1) */
  U8X8_CA(0x01, 0x07),			/* character/bits per pixel pitch */
  U8X8_CA(0x02, 240/8-1),		/* number of chars/byte width of the screen */
  U8X8_CA(0x03, 0x7f),			/* time division */
  U8X8_CA(0x08, 0x00),			/* display start low */
  U8X8_CA(0x09, 0x00),			/* display start high */

  U8X8_DLY(10),
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_lc7981_240x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call common procedure first and handle messages there */
  if ( u8x8_d_lc7981_common(u8x8, msg, arg_int, arg_ptr) == 0 )
  {
    /* msg not handled, then try here */
    switch(msg)
    {
      case U8X8_MSG_DISPLAY_SETUP_MEMORY:
	u8x8_d_helper_display_setup_memory(u8x8, &u8x8_lc7981_240x128_display_info);
	break;
      case U8X8_MSG_DISPLAY_INIT:
	u8x8_d_helper_display_init(u8x8);
	u8x8_cad_SendSequence(u8x8, u8x8_d_lc7981_240x128_init_seq);
	break;
      default:
	return 0;		/* msg unknown */
    }
  }
  return 1;
}


/*================================================*/
/* LC7981 240x64 LCD*/
/* https://github.com/olikraus/u8g2/issues/642 */

static const u8x8_display_info_t u8x8_lc7981_240x64_display_info =
{
  /* chip_enable_level = */ 0,	/* LC7981 has a low active CS*/
  /* chip_disable_level = */ 1,
  
  /* from here... */
  /* post_chip_enable_wait_ns = */ 20,	
  /* pre_chip_disable_wait_ns = */ 20,	
  /* reset_pulse_width_ms = */ 1, 	
  /* post_reset_wait_ms = */ 10, 	
  /* sda_setup_time_ns = */ 30,		
  /* sck_pulse_width_ns = */ 65,	/* half of cycle time  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* ... to here, values are ignored, because this is a parallel interface only */
  
  /* data_setup_time_ns = */ 220,	
  /* write_pulse_width_ns = */ 20,	
  /* tile_width = */ 30,		/* width of 30*8=240 pixel */
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,	
  /* flipmode_x_offset = */ 0,	
  /* pixel_width = */ 240,
  /* pixel_height = */ 64
};

static const uint8_t u8x8_d_lc7981_240x64_init_seq[] = {
    
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(50),

  U8X8_CA(0x00, 0x32),			/* display on (bit 5), master mode on (bit 4), graphics mode on (bit 1) */
  U8X8_CA(0x01, 0x07),			/* character/bits per pixel pitch */
  U8X8_CA(0x02, 240/8-1),		/* number of chars/byte width of the screen */
  U8X8_CA(0x03, 0x7f),			/* time division */
  U8X8_CA(0x08, 0x00),			/* display start low */
  U8X8_CA(0x09, 0x00),			/* display start high */

  U8X8_DLY(10),
  
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_lc7981_240x64(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  /* call common procedure first and handle messages there */
  if ( u8x8_d_lc7981_common(u8x8, msg, arg_int, arg_ptr) == 0 )
  {
    /* msg not handled, then try here */
    switch(msg)
    {
      case U8X8_MSG_DISPLAY_SETUP_MEMORY:
	u8x8_d_helper_display_setup_memory(u8x8, &u8x8_lc7981_240x64_display_info);
	break;
      case U8X8_MSG_DISPLAY_INIT:
	u8x8_d_helper_display_init(u8x8);
	u8x8_cad_SendSequence(u8x8, u8x8_d_lc7981_240x64_init_seq);
	break;
      default:
	return 0;		/* msg unknown */
    }
  }
  return 1;
}


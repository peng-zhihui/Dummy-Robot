/*

  u8x8_d_max7219.c
  
  Universal 8bit Graphics Library (https://github.com/olikraus/u8g2/)

  Copyright (c) 2017, olikraus@gmail.com
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



static const uint8_t u8x8_d_max7219_init_seq[] = {

  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_END_TRANSFER(),             	/* disable chip */

  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_END_TRANSFER(),             	/* disable chip */
  
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_END_TRANSFER(),             	/* disable chip */
  
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_END_TRANSFER(),             	/* disable chip */
  
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_END_TRANSFER(),             	/* disable chip */


  //U8X8_CA(12, 0),				/* shutdown */
    
  //U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_max7219_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_max7219_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static uint8_t u8x8_d_max7219_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t c, j, i;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_pcf8812_96x65_display_info);
      break;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_init_seq);    
      break;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_powersave1_seq);
      break;
    */
/*  not supported by MAX7219
    case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
      break;
*/
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:
      u8x8_cad_StartTransfer(u8x8);
      for( i = 0; i < u8x8->display_info->tile_width; i++ )
      {
	u8x8_cad_SendCmd(u8x8, 10 );    /* brightness */
	u8x8_cad_SendArg(u8x8, (arg_int>>4) );	/* 0..15 for contrast */
      }
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      /* transfer always has to start at x pos 0 (u8x8 is not supported) */
      /* also y pos has to be 0 */
      /* arg_int is ignored */
      //x = ((u8x8_tile_t *)arg_ptr)->x_pos;    

      c = ((u8x8_tile_t *)arg_ptr)->cnt;	/* number of tiles */
      ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;	/* data ptr to the tiles */
      for( i = 0; i < 8; i++ )
      {
	u8x8_cad_StartTransfer(u8x8);
	for( j = 0; j < c; j++ )
	{
	  u8x8_cad_SendCmd(u8x8, i+1);	/* commands 1..8 select the byte */
	  u8x8_cad_SendArg(u8x8, *ptr );
	  ptr++;
	}
	u8x8_cad_EndTransfer(u8x8);
      }
      
      break;
    default:
      return 0;
  }
  return 1;
}

/*==============================*/

static const u8x8_display_info_t u8x8_max7219_32x8_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 100,
  /* pre_chip_disable_wait_ns = */ 100,
  /* reset_pulse_width_ms = */ 100, 
  /* post_reset_wait_ms = */ 100, 
  /* sda_setup_time_ns = */ 100,	
  /* sck_pulse_width_ns = */ 100,	
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	
  /* tile_width = */ 4,
  /* tile_hight = */ 1,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 32,
  /* pixel_height = */ 8
};

uint8_t u8x8_d_max7219_32x8(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY :
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_max7219_32x8_display_info);
      return 1;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_init_seq);    
      return 1;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_powersave1_seq);
      return 1;
  }
  return u8x8_d_max7219_generic(u8x8, msg, arg_int, arg_ptr);
}

/*==============================*/

static const u8x8_display_info_t u8x8_max7219_16x16_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 100,
  /* pre_chip_disable_wait_ns = */ 100,
  /* reset_pulse_width_ms = */ 100, 
  /* post_reset_wait_ms = */ 100, 
  /* sda_setup_time_ns = */ 100,	
  /* sck_pulse_width_ns = */ 100,	
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	
  /* tile_width = */ 2,
  /* tile_hight = */ 2,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 16,
  /* pixel_height = */ 16
};

/*
  Multiple page rows are not supported, so 16x16 will not work.
  Due to the hardware structure of such displays all tiles of the display
  must be written at once. 
  This is not possible with the current u8g2 structure.
  So u8x8_d_max7219_16x16 will not work.
*/

uint8_t u8x8_d_max7219_16x16(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY :
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_max7219_16x16_display_info);
      return 1;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_init_seq);    
      return 1;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_powersave1_seq);
      return 1;
  }
  return u8x8_d_max7219_generic(u8x8, msg, arg_int, arg_ptr);
}

/*==============================*/

static const u8x8_display_info_t u8x8_max7219_8x8_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 100,
  /* pre_chip_disable_wait_ns = */ 100,
  /* reset_pulse_width_ms = */ 100, 
  /* post_reset_wait_ms = */ 100, 
  /* sda_setup_time_ns = */ 100,	
  /* sck_pulse_width_ns = */ 100,	
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	
  /* tile_width = */ 1,
  /* tile_hight = */ 1,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 8,
  /* pixel_height = */ 8
};

uint8_t u8x8_d_max7219_8x8(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY :
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_max7219_8x8_display_info);
      return 1;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_init_seq);    
      return 1;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_powersave1_seq);
      return 1;
  }
    return u8x8_d_max7219_generic(u8x8, msg, arg_int, arg_ptr);
}


/*==============================*/

static const uint8_t u8x8_d_max7219_8_init_seq[] = {

  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_CA(15, 0),				/* test mode off */
  U8X8_END_TRANSFER(),             	/* disable chip */

  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_CA(12, 0),				/*  */
  U8X8_END_TRANSFER(),             	/* disable chip */
  
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_CA(9, 0),				/* decode mode: graphics */
  U8X8_END_TRANSFER(),             	/* disable chip */
  
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_CA(10, 10),				/* medium high intensity */
  U8X8_END_TRANSFER(),             	/* disable chip */
  
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */  
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_CA(11, 7),				/* scan limit: display all digits (assuming a 8x8 matrix) */
  U8X8_END_TRANSFER(),             	/* disable chip */


  //U8X8_CA(12, 0),				/* shutdown */
    
  //U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_max7219_8_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_CA(12, 1),				/* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_max7219_8_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_CA(12, 0),				/* shutdown */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static const u8x8_display_info_t u8x8_max7219_64x8_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 100,
  /* pre_chip_disable_wait_ns = */ 100,
  /* reset_pulse_width_ms = */ 100, 
  /* post_reset_wait_ms = */ 100, 
  /* sda_setup_time_ns = */ 100,	
  /* sck_pulse_width_ns = */ 100,	
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,
  /* data_setup_time_ns = */ 40,
  /* write_pulse_width_ns = */ 150,	
  /* tile_width = */ 8,
  /* tile_hight = */ 1,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 0,
  /* pixel_width = */ 64,
  /* pixel_height = */ 8
};

uint8_t u8x8_d_max7219_64x8(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  switch(msg)
  {
    case U8X8_MSG_DISPLAY_SETUP_MEMORY :
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_max7219_64x8_display_info);
      return 1;
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_8_init_seq);    
      return 1;
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_8_powersave0_seq);
      else
	u8x8_cad_SendSequence(u8x8, u8x8_d_max7219_8_powersave1_seq);
      return 1;
  }
  return u8x8_d_max7219_generic(u8x8, msg, arg_int, arg_ptr);
}



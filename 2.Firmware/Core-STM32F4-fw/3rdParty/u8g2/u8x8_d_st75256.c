/*

  u8x8_d_st75256.c

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

  0x030	ext 00
  0x031	ext 01
  0x038	ext 10
  0x039	ext 11
  
  cad 011
  
  
  code examples:
  http://www.it610.com/article/2601023.htm
  
  normal mode:
	0x00c	bit format
  U8X8_CA( 0xbc, 0x00 ),	data scan dir 
  U8X8_A( 0xa6 ),				
  y: 0 offset
  
  flip mode:
	0x008	bit format
  U8X8_CA( 0xbc, 0x03 ),	data scan dir 
  U8X8_A( 0xa6 ),				
  y: 5 offset
	
  
*/


#include "u8x8.h"


/* not a real power down for the st75256... just a display off */
static const uint8_t u8x8_d_st75256_256x128_powersave0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C( 0x030 ),				/* select 00 commands */  
  U8X8_C( 0x94 ),				/* sleep out */
  U8X8_DLY(10),
  U8X8_C( 0xaf ),				/* display on */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75256_256x128_powersave1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0xae ),				/* display off */
  U8X8_C( 0x95 ),				/* sleep in */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75256_jlx256128_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x00 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75256_jlx256128_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x03 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x008 ),				/* data format MSB top */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75256_jlx172104_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x02 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75256_jlx172104_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x01 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x008 ),				/* data format MSB top */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75256_jlx256160_flip0_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x00 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

static const uint8_t u8x8_d_st75256_jlx256160_flip1_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x03 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x008 ),				/* data format MSB top */
  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


static uint8_t u8x8_d_st75256_256x128_generic(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;
  switch(msg)
  {
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_SETUP_MEMORY:
      u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_256x128_display_info);
      break;
    */
    /* handled by the calling function
    case U8X8_MSG_DISPLAY_INIT:
      u8x8_d_helper_display_init(u8x8);
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_init_seq);    
      break;
    */
    case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
      if ( arg_int == 0 )
        u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave0_seq);
      else
        u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave1_seq);

      break;
#ifdef U8X8_WITH_SET_CONTRAST
    case U8X8_MSG_DISPLAY_SET_CONTRAST:

      u8x8_cad_StartTransfer(u8x8);
      
      u8x8_cad_SendCmd(u8x8, 0x030 );
      u8x8_cad_SendCmd(u8x8, 0x081 );  /* there are 9 bit for the volume control */
      u8x8_cad_SendArg(u8x8, (arg_int & 0x1f)<<1 );	/* lower 6 bit */
      u8x8_cad_SendArg(u8x8, (arg_int>>5));		/* upper 3 bit */
      
      u8x8_cad_EndTransfer(u8x8);
      break;
#endif
    case U8X8_MSG_DISPLAY_DRAW_TILE:
      
      u8x8_cad_StartTransfer(u8x8);
      x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
      x *= 8;
      
      u8x8_cad_SendCmd(u8x8, 0x030 );	/* select command set */
      u8x8_cad_SendCmd(u8x8, 0x075 );	/* row */
      u8x8_cad_SendArg(u8x8, u8x8->x_offset + (((u8x8_tile_t *)arg_ptr)->y_pos));	/* x offset is used as y offset */
      u8x8_cad_SendArg(u8x8, 0x04f);
      //u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
      u8x8_cad_SendCmd(u8x8, 0x015 );	/* col */
      u8x8_cad_SendArg(u8x8, x);
      u8x8_cad_SendArg(u8x8, 255);
      u8x8_cad_SendCmd(u8x8, 0x05c );	
          
      do
      {
        c = ((u8x8_tile_t *)arg_ptr)->cnt;
        ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
        /* SendData can not handle more than 255 bytes, treat c > 31 correctly  */
        if ( c > 31 )
        {
          u8x8_cad_SendData(u8x8, 248, ptr); 	/* 31*8=248 */
          ptr+=248;
          c -= 31;
        }
        
        u8x8_cad_SendData(u8x8, c*8, ptr); 	
        arg_int--;
      } while( arg_int > 0 );
      
      u8x8_cad_EndTransfer(u8x8);
      break;
    default:
      return 0;
  }
  return 1;
}

/*=============================================*/
/* JLX256128 */

static const u8x8_display_info_t u8x8_st75256_256x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 32,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 0,	/* must be 0, because this is checked also for normal mode */
  /* flipmode_x_offset = */ 5,		/* used as y offset */
  /* pixel_width = */ 256,
  /* pixel_height = */ 128
};


static const uint8_t u8x8_d_st75256_256x128_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x000 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x4f),		/* row range */
  U8X8_CAA(0x15, 0, 255),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x00 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 0x7f ),				/* 1/160 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 0x36, 0x05 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st75256_jlx256128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_st75256_256x128_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_256x128_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_init_seq);    
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx256128_flip0_seq);
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx256128_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}



/*=============================================*/
/* WO256X128, https://github.com/olikraus/u8g2/issues/891  */

static const u8x8_display_info_t u8x8_st75256_wo256x128_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 32,
  /* tile_hight = */ 16,
  /* default_x_offset = */ 5,	/* must be 0, because this is checked also for normal mode */
  /* flipmode_x_offset = */ 0,		/* used as y offset */
  /* pixel_width = */ 256,
  /* pixel_height = */ 128
};


static const uint8_t u8x8_d_st75256_wo256x128_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x000 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x4f),		/* row range */
  U8X8_CAA(0x15, 0, 255),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x01 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x008 ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 0x7f ),				/* 1/160 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 0x36, 0x05 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st75256_wo256x128(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_st75256_256x128_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_wo256x128_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_wo256x128_init_seq);    
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip1_seq);	// this matches the init sequence
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip0_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}


/*=============================================*/
/* JLX25664 */

static const u8x8_display_info_t u8x8_st75256_256x64_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 32,
  /* tile_hight = */ 8,
  /* default_x_offset = */ 0,	/* must be 0, because this is checked also for normal mode */
  /* flipmode_x_offset = */ 13,		/* used as y offset */
  /* pixel_width = */ 256,
  /* pixel_height = */ 64
};


static const uint8_t u8x8_d_st75256_256x64_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x005 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x1f),		/* row range */
  U8X8_CAA(0x15, 0, 255),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x00 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 0x3f ),				/* 64 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 012, 0x02 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st75256_jlx25664(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  if ( u8x8_d_st75256_256x128_generic(u8x8, msg, arg_int, arg_ptr) != 0 )
    return 1;
  if ( msg == U8X8_MSG_DISPLAY_SETUP_MEMORY )
  {
    //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_256x64_display_info);
    return 1;
  }
  else if ( msg == U8X8_MSG_DISPLAY_INIT )
  {
    u8x8_d_helper_display_init(u8x8);
    u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x64_init_seq);    
    return 1;
  }
  else if  ( msg == U8X8_MSG_DISPLAY_SET_FLIP_MODE )
  {
    if ( arg_int == 0 )
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx256128_flip0_seq);
      u8x8->x_offset = u8x8->display_info->default_x_offset;
    }
    else
    {
      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx256128_flip1_seq);
      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
    }
    return 1;
  }
  return 0;
}


/*=============================================*/
/* JLX172104 LCD */

static const u8x8_display_info_t u8x8_st75256_172x104_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 22,			/* 22=176 */
  /* tile_hight = */ 13,
  /* default_x_offset = */ 84,	/*  */
  /* flipmode_x_offset = */ 0,		
  /* pixel_width = */ 172,
  /* pixel_height = */ 104
};

static const uint8_t u8x8_d_st75256_jlx172104_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x003 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x4f),		/* row range */
  U8X8_CAA(0x15, 0, 255),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x02 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 0x9f ),				/* 1/160 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 0x08, 0x04 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};    


uint8_t u8x8_d_st75256_jlx172104(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;

  switch(msg)
  {
            case U8X8_MSG_DISPLAY_DRAW_TILE:
              
              u8x8_cad_StartTransfer(u8x8);
              x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
              x *= 8;
              
              u8x8_cad_SendCmd(u8x8, 0x030 );	/* select command set */
              u8x8_cad_SendCmd(u8x8, 0x075 );	/* row */
	      if ( u8x8->x_offset == 0 )		/* 0 means flip mode 1, then adjust y value */
		u8x8_cad_SendArg(u8x8, 8+(((u8x8_tile_t *)arg_ptr)->y_pos));
	      else
		u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendArg(u8x8, 0x04f);
              //u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendCmd(u8x8, 0x015 );	/* col */
              u8x8_cad_SendArg(u8x8, x+u8x8->x_offset);
              u8x8_cad_SendArg(u8x8, 255);
              u8x8_cad_SendCmd(u8x8, 0x05c );	
            
              
              /* this procedure assumes, that the overall width is 172 */
              do
              {
                c = ((u8x8_tile_t *)arg_ptr)->cnt;
                ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
                c *= 8;

                if ( c + x > 172u )
                {
                        c = 172u;
                        c -= x;
                }
                      
                u8x8_cad_SendData(u8x8, c, ptr); 	
                x += c;
                arg_int--;
              } while( arg_int > 0 );
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
            u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_172x104_display_info);
            return 1;
        case U8X8_MSG_DISPLAY_INIT:
            u8x8_d_helper_display_init(u8x8);
            u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_init_seq);
            return 1;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
              if ( arg_int == 0 )
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave0_seq);
              else
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave1_seq);

              return 1;
	case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
	    if ( arg_int == 0 )
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip0_seq);
	      u8x8->x_offset = u8x8->display_info->default_x_offset;
	    }
	    else
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip1_seq); 
	      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
	    }
	    return 1;
		
#ifdef U8X8_WITH_SET_CONTRAST
        case U8X8_MSG_DISPLAY_SET_CONTRAST:

              u8x8_cad_StartTransfer(u8x8);
              
              u8x8_cad_SendCmd(u8x8, 0x030 );
              u8x8_cad_SendCmd(u8x8, 0x081 );  /* there are 9 bit for the volume control */
              u8x8_cad_SendArg(u8x8, (arg_int & 0x1f)<<1 );	/* lower 6 bit */
              u8x8_cad_SendArg(u8x8, (arg_int>>5));		/* upper 3 bit */
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
#endif
  }
  return 0;
}

/*=============================================*/
/* JLX240160 */

static const u8x8_display_info_t u8x8_st75256_240x160_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 30,
  /* tile_hight = */ 20,
  /* default_x_offset = */ 16,	/*  x offset in flipmode 0 */
  /* flipmode_x_offset = */ 0,		/* */
  /* pixel_width = */ 240,
  /* pixel_height = */ 160
};


static const uint8_t u8x8_d_st75256_240x160_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x000 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x4f),		/* row range */
  U8X8_CAA(0x15, 0, 239),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x02 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 159 ),				/* 1/160 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 0x18, 0x04 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st75256_jlx240160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;

  switch(msg)
  {
            case U8X8_MSG_DISPLAY_DRAW_TILE:
              
              u8x8_cad_StartTransfer(u8x8);
              x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
              x *= 8;
              
              u8x8_cad_SendCmd(u8x8, 0x030 );	/* select command set */
              u8x8_cad_SendCmd(u8x8, 0x075 );	/* row */
	      if ( u8x8->x_offset == 0 )		/* 0 means flip mode 1 */
		u8x8_cad_SendArg(u8x8, 1+(((u8x8_tile_t *)arg_ptr)->y_pos));
	      else
		u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendArg(u8x8, 0x04f);
              //u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendCmd(u8x8, 0x015 );	/* col */
              u8x8_cad_SendArg(u8x8, x+u8x8->x_offset);
              u8x8_cad_SendArg(u8x8, 255);
              u8x8_cad_SendCmd(u8x8, 0x05c );	
            
              
              do
              {
                c = ((u8x8_tile_t *)arg_ptr)->cnt;
                ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
                c *= 8;

                if ( c + x > 240u )
                {
                        c = 240u;
                        c -= x;
                }
                      
                u8x8_cad_SendData(u8x8, c, ptr); 	
                x += c;
                arg_int--;
              } while( arg_int > 0 );
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
	    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_240x160_display_info);
            return 1;
        case U8X8_MSG_DISPLAY_INIT:
	    u8x8_d_helper_display_init(u8x8);
	    u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_240x160_init_seq);    
            return 1;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
              if ( arg_int == 0 )
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave0_seq);
              else
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave1_seq);

              return 1;
	case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
	    if ( arg_int == 0 )
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip0_seq);
	      u8x8->x_offset = u8x8->display_info->default_x_offset;
	    }
	    else
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip1_seq);
	      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
	    }
	    return 1;
		
#ifdef U8X8_WITH_SET_CONTRAST
        case U8X8_MSG_DISPLAY_SET_CONTRAST:

              u8x8_cad_StartTransfer(u8x8);
              
              u8x8_cad_SendCmd(u8x8, 0x030 );
              u8x8_cad_SendCmd(u8x8, 0x081 );  /* there are 9 bit for the volume control */
              u8x8_cad_SendArg(u8x8, (arg_int & 0x1f)<<1 );	/* lower 6 bit */
              u8x8_cad_SendArg(u8x8, (arg_int>>5));		/* upper 3 bit */
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
#endif
  }
  return 0;
  
  
  
}


/*=============================================*/
/* JLX256160 */

static const u8x8_display_info_t u8x8_st75256_256x160_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 32,
  /* tile_hight = */ 20,
  /* default_x_offset = */ 0,
  /* flipmode_x_offset = */ 1,	/* x offset is used as y offset in flipmode */
  /* pixel_width = */ 256,
  /* pixel_height = */ 160
};


static const uint8_t u8x8_d_st75256_256x160_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x000 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x28),		/* row range */
  U8X8_CAA(0x15, 0, 0xFF),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x00 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 159 ),				/* 1/160 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 0x18, 0x05 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st75256_jlx256160(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;

  switch(msg)
  {
            case U8X8_MSG_DISPLAY_DRAW_TILE:
              
              u8x8_cad_StartTransfer(u8x8);
              x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
              x *= 8;
              
              u8x8_cad_SendCmd(u8x8, 0x030 );	/* select command set */
              u8x8_cad_SendCmd(u8x8, 0x075 );	/* row */
	      if ( u8x8->x_offset == 1 )		/* 1 means flip mode 1 */
		u8x8_cad_SendArg(u8x8, 1+(((u8x8_tile_t *)arg_ptr)->y_pos));
	      else
		u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendArg(u8x8, 0x04f);
              //u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendCmd(u8x8, 0x015 );	/* col */
              u8x8_cad_SendArg(u8x8, x+u8x8->display_info->default_x_offset);
              u8x8_cad_SendArg(u8x8, 255);
              u8x8_cad_SendCmd(u8x8, 0x05c );	
            
              
              do
              {
                c = ((u8x8_tile_t *)arg_ptr)->cnt;
                ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
		/* SendData can not handle more than 255 bytes, treat c > 31 correctly  */
		if ( c > 31 )
		{
		  u8x8_cad_SendData(u8x8, 248, ptr); 	/* 31*8=248 */
		  ptr+=248;
		  c -= 31;
		}
		
		u8x8_cad_SendData(u8x8, c*8, ptr); 	
                arg_int--;
              } while( arg_int > 0 );
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
	    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_256x160_display_info);
            return 1;
        case U8X8_MSG_DISPLAY_INIT:
	    u8x8_d_helper_display_init(u8x8);
	    u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x160_init_seq);    
            return 1;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
              if ( arg_int == 0 )
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave0_seq);
              else
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave1_seq);

              return 1;
	case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
	    if ( arg_int == 0 )
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx256160_flip0_seq);
	      u8x8->x_offset = u8x8->display_info->default_x_offset;
	    }
	    else
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx256160_flip1_seq);
	      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
	    }
	    return 1;
		
#ifdef U8X8_WITH_SET_CONTRAST
        case U8X8_MSG_DISPLAY_SET_CONTRAST:

              u8x8_cad_StartTransfer(u8x8);
              
              u8x8_cad_SendCmd(u8x8, 0x030 );
              u8x8_cad_SendCmd(u8x8, 0x081 );  /* there are 9 bit for the volume control */
              u8x8_cad_SendArg(u8x8, (arg_int & 0x1f)<<1 );	/* lower 6 bit */
              u8x8_cad_SendArg(u8x8, (arg_int>>5));		/* upper 3 bit */
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
#endif
  }
  return 0;
}


/*=============================================*/
/* JLX256160 mirror version #930 */


static const uint8_t u8x8_d_st75256_256x160m_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x000 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x28),		/* row range */
  U8X8_CAA(0x15, 0, 0xFF),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x02 ),			/* data scan dir  ( CHANGED FOR MIRROR VERSION ) */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 159 ),				/* 1/160 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 0x18, 0x05 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};

uint8_t u8x8_d_st75256_jlx256160m(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;

  switch(msg)
  {
            case U8X8_MSG_DISPLAY_DRAW_TILE:
              
              u8x8_cad_StartTransfer(u8x8);
              x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
              x *= 8;
              
              u8x8_cad_SendCmd(u8x8, 0x030 );	/* select command set */
              u8x8_cad_SendCmd(u8x8, 0x075 );	/* row */
	      if ( u8x8->x_offset == 1 )		/* 1 means flip mode 1 */
		u8x8_cad_SendArg(u8x8, 1+(((u8x8_tile_t *)arg_ptr)->y_pos));
	      else
		u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendArg(u8x8, 0x04f);
              //u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendCmd(u8x8, 0x015 );	/* col */
              u8x8_cad_SendArg(u8x8, x+u8x8->display_info->default_x_offset);
              u8x8_cad_SendArg(u8x8, 255);
              u8x8_cad_SendCmd(u8x8, 0x05c );	
            
              
              do
              {
                c = ((u8x8_tile_t *)arg_ptr)->cnt;
                ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
		/* SendData can not handle more than 255 bytes, treat c > 31 correctly  */
		if ( c > 31 )
		{
		  u8x8_cad_SendData(u8x8, 248, ptr); 	/* 31*8=248 */
		  ptr+=248;
		  c -= 31;
		}
		
		u8x8_cad_SendData(u8x8, c*8, ptr); 	
                arg_int--;
              } while( arg_int > 0 );
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
	    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_256x160_display_info);
            return 1;
        case U8X8_MSG_DISPLAY_INIT:
	    u8x8_d_helper_display_init(u8x8);
	    u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x160m_init_seq);    
            return 1;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
              if ( arg_int == 0 )
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave0_seq);
              else
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave1_seq);

              return 1;
	case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
	    if ( arg_int == 0 )
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip0_seq);
	      u8x8->x_offset = u8x8->display_info->default_x_offset;
	    }
	    else
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip1_seq);
	      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
	    }
	    return 1;
		
#ifdef U8X8_WITH_SET_CONTRAST
        case U8X8_MSG_DISPLAY_SET_CONTRAST:

              u8x8_cad_StartTransfer(u8x8);
              
              u8x8_cad_SendCmd(u8x8, 0x030 );
              u8x8_cad_SendCmd(u8x8, 0x081 );  /* there are 9 bit for the volume control */
              u8x8_cad_SendArg(u8x8, (arg_int & 0x1f)<<1 );	/* lower 6 bit */
              u8x8_cad_SendArg(u8x8, (arg_int>>5));		/* upper 3 bit */
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
#endif
  }
  return 0;
}




/*=============================================*/
/* JLX256160 alternative version from issue #561 */

static const u8x8_display_info_t u8x8_st75256_256x160_alt_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 32,
  /* tile_hight = */ 20,
  /* default_x_offset = */ 0,	/*  x offset in flipmode 0 */
  /* flipmode_x_offset = */ 0,		/* */
  /* pixel_width = */ 256,
  /* pixel_height = */ 160
};


static const uint8_t u8x8_d_st75256_256x160_alt_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x000 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x4f),		/* row range */
  U8X8_CAA(0x15, 0, 255),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x02 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 159 ),				/* 1/160 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 0x18, 0x05 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};


uint8_t u8x8_d_st75256_jlx256160_alt(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;

  switch(msg)
  {
            case U8X8_MSG_DISPLAY_DRAW_TILE:
              
              u8x8_cad_StartTransfer(u8x8);
              x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
              x *= 8;
              
              u8x8_cad_SendCmd(u8x8, 0x030 );	/* select command set */
              u8x8_cad_SendCmd(u8x8, 0x075 );	/* row */
	      if ( u8x8->x_offset == 0 )		/* 0 means flip mode 1 */
		u8x8_cad_SendArg(u8x8, 1+(((u8x8_tile_t *)arg_ptr)->y_pos));
	      else
		u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendArg(u8x8, 0x04f);
              //u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendCmd(u8x8, 0x015 );	/* col */
              u8x8_cad_SendArg(u8x8, x+u8x8->x_offset);
              u8x8_cad_SendArg(u8x8, 255);
              u8x8_cad_SendCmd(u8x8, 0x05c );	
            
              
              do
              {
                c = ((u8x8_tile_t *)arg_ptr)->cnt;
                ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
		/* SendData can not handle more than 255 bytes, treat c > 31 correctly  */
		if ( c > 31 )
		{
		  u8x8_cad_SendData(u8x8, 248, ptr); 	/* 31*8=248 */
		  ptr+=248;
		  c -= 31;
		}
		
		u8x8_cad_SendData(u8x8, c*8, ptr); 	
                arg_int--;
              } while( arg_int > 0 );
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
	    u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_256x160_alt_display_info);
            return 1;
        case U8X8_MSG_DISPLAY_INIT:
	    u8x8_d_helper_display_init(u8x8);
	    u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x160_alt_init_seq);    
            return 1;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
              if ( arg_int == 0 )
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave0_seq);
              else
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave1_seq);

              return 1;
	case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
	    if ( arg_int == 0 )
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip0_seq);
	      u8x8->x_offset = u8x8->display_info->default_x_offset;
	    }
	    else
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip1_seq);
	      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
	    }
	    return 1;
		
#ifdef U8X8_WITH_SET_CONTRAST
        case U8X8_MSG_DISPLAY_SET_CONTRAST:

              u8x8_cad_StartTransfer(u8x8);
              
              u8x8_cad_SendCmd(u8x8, 0x030 );
              u8x8_cad_SendCmd(u8x8, 0x081 );  /* there are 9 bit for the volume control */
              u8x8_cad_SendArg(u8x8, (arg_int & 0x1f)<<1 );	/* lower 6 bit */
              u8x8_cad_SendArg(u8x8, (arg_int>>5));		/* upper 3 bit */
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
#endif
  }
  return 0;

} 


/*=============================================*/
/* JLX19296 LCD */

static const u8x8_display_info_t u8x8_st75256_192x96_display_info =
{
  /* chip_enable_level = */ 0,
  /* chip_disable_level = */ 1,
  
  /* post_chip_enable_wait_ns = */ 20,
  /* pre_chip_disable_wait_ns = */ 20,
  /* reset_pulse_width_ms = */ 5, 	
  /* post_reset_wait_ms = */ 5, 		/**/
  /* sda_setup_time_ns = */ 20,		/* */
  /* sck_pulse_width_ns = */ 40,	/*  */
  /* sck_clock_hz = */ 4000000UL,	/* since Arduino 1.6.0, the SPI bus speed in Hz. Should be  1000000000/sck_pulse_width_ns */
  /* spi_mode = */ 0,		/* active high, rising edge */
  /* i2c_bus_clock_100kHz = */ 4,	/* 400KHz */
  /* data_setup_time_ns = */ 15,
  /* write_pulse_width_ns = */ 70,	
  /* tile_width = */ 24,	
  /* tile_hight = */ 12,
  /* default_x_offset = */ 0,	/*  */
  /* flipmode_x_offset = */ 64,
  /* pixel_width = */ 192,
  /* pixel_height = */ 96
};

static const uint8_t u8x8_d_st75256_jlx19296_init_seq[] = {
  U8X8_START_TRANSFER(),             	/* enable chip, delay is part of the transfer start */
  
  U8X8_DLY(20),

  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x094 ),				/* sleep out */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x0ae ),				/* display off */

  U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_CA( 0x0d7, 0x09f ),		/* disable auto read */  

  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x032 ),				/* analog circuit set */
  U8X8_A( 0x000 ),				/* code example: OSC Frequency adjustment */
  U8X8_A( 0x001 ),				/* Frequency on booster capacitors 1 = 6KHz? */
  U8X8_A( 0x003 ),				/* Bias: 1: 1/13, 2: 1/12, 3: 1/11, 4:1/10, 5:1/9 */
    
  //U8X8_C( 0x031 ),				/* select 01 commands */
  U8X8_C( 0x020 ),				/* gray levels */
  U8X8_A( 0x01 ),
  U8X8_A( 0x03 ),
  U8X8_A( 0x05 ),
  U8X8_A( 0x07 ),
  U8X8_A( 0x09),
  U8X8_A( 0x0b ),
  U8X8_A( 0x0d ),
  U8X8_A( 0x10 ),
  U8X8_A( 0x11 ),
  U8X8_A( 0x13 ),
  U8X8_A( 0x15 ),
  U8X8_A( 0x17 ),
  U8X8_A( 0x19 ),
  U8X8_A( 0x1b ),
  U8X8_A( 0x1d ),
  U8X8_A( 0x1f ),
 
  
  U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA(0x75, 0, 0x4f),		/* row range */
  U8X8_CAA(0x15, 0, 255),		/* col range */
  
  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0xbc, 0x00 ),			/* data scan dir */
  U8X8_A( 0xa6 ),				/* ??? */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_C( 0x00c ),				/* data format LSB top */

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_C( 0xca ),				/* display control, 3 args follow  */
  U8X8_A( 0x00 ),				/* 0x00: no clock division, 0x04: devide clock */
  U8X8_A( 0x9f ),				/* 1/160 duty value from the DS example code */
  U8X8_A( 0x20 ),				/* nline off */ 

  //U8X8_C( 0x030 ),				/* select 00 commands */ 
  U8X8_CA( 0x0f0, 0x010 ),		/* monochrome mode  = 0x010*/

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CAA( 0x81, 0x2e, 0x03 ),	/* Volume control */

  //U8X8_C( 0x030 ),				/* select 00 commands */
  U8X8_CA( 0x020, 0x00b ),		/* Power control: Regulator, follower & booster on */
  U8X8_DLY(100),

  U8X8_END_TRANSFER(),             	/* disable chip */
  U8X8_END()             			/* end of sequence */
};    


uint8_t u8x8_d_st75256_jlx19296(u8x8_t *u8x8, uint8_t msg, uint8_t arg_int, void *arg_ptr)
{
  uint8_t x, c;
  uint8_t *ptr;

  switch(msg)
  {
            case U8X8_MSG_DISPLAY_DRAW_TILE:
              
              u8x8_cad_StartTransfer(u8x8);
              x = ((u8x8_tile_t *)arg_ptr)->x_pos;    
              x *= 8;
              
              u8x8_cad_SendCmd(u8x8, 0x030 );	/* select command set */
              u8x8_cad_SendCmd(u8x8, 0x075 );	/* row */
	      if ( u8x8->x_offset == 0 )		/* 0 means flip mode 1, then adjust y value */
		u8x8_cad_SendArg(u8x8, 8+(((u8x8_tile_t *)arg_ptr)->y_pos));
	      else
		u8x8_cad_SendArg(u8x8, 1+(((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendArg(u8x8, 0x04f);
              //u8x8_cad_SendArg(u8x8, (((u8x8_tile_t *)arg_ptr)->y_pos));
              u8x8_cad_SendCmd(u8x8, 0x015 );	/* col */
              u8x8_cad_SendArg(u8x8, x+u8x8->x_offset);
              u8x8_cad_SendArg(u8x8, 255);
              u8x8_cad_SendCmd(u8x8, 0x05c );	
            
              
              do
              {
                c = ((u8x8_tile_t *)arg_ptr)->cnt;
                ptr = ((u8x8_tile_t *)arg_ptr)->tile_ptr;
                c *= 8;

                if ( c + x > 192u )
                {
                        c = 192u;
                        c -= x;
                }
                      
                u8x8_cad_SendData(u8x8, c, ptr); 	
                x += c;
                arg_int--;
              } while( arg_int > 0 );
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
        case U8X8_MSG_DISPLAY_SETUP_MEMORY:
            //u8x8_SetI2CAddress(u8x8, 0x078);		/* lowest I2C adr of the ST75256 */
            u8x8_d_helper_display_setup_memory(u8x8, &u8x8_st75256_192x96_display_info);
            return 1;
        case U8X8_MSG_DISPLAY_INIT:
            u8x8_d_helper_display_init(u8x8);
            u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx19296_init_seq);
            return 1;
        case U8X8_MSG_DISPLAY_SET_POWER_SAVE:
              if ( arg_int == 0 )
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave0_seq);
              else
                u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_256x128_powersave1_seq);

              return 1;
	case U8X8_MSG_DISPLAY_SET_FLIP_MODE:
	    if ( arg_int == 0 )
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx256160_flip0_seq);
	      u8x8->x_offset = u8x8->display_info->default_x_offset;
	    }
	    else
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx256160_flip1_seq);
	      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
	    }
	    return 1;
	    /*
	    if ( arg_int == 0 )
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip0_seq);
	      u8x8->x_offset = u8x8->display_info->default_x_offset;
	    }
	    else
	    {
	      u8x8_cad_SendSequence(u8x8, u8x8_d_st75256_jlx172104_flip1_seq); 
	      u8x8->x_offset = u8x8->display_info->flipmode_x_offset;
	    }
	    return 1;
	    */
		
#ifdef U8X8_WITH_SET_CONTRAST
        case U8X8_MSG_DISPLAY_SET_CONTRAST:

              u8x8_cad_StartTransfer(u8x8);
              
              u8x8_cad_SendCmd(u8x8, 0x030 );
              u8x8_cad_SendCmd(u8x8, 0x081 );  /* there are 9 bit for the volume control */
              u8x8_cad_SendArg(u8x8, (arg_int & 0x1f)<<1 );	/* lower 6 bit */
              u8x8_cad_SendArg(u8x8, (arg_int>>5));		/* upper 3 bit */
              
              u8x8_cad_EndTransfer(u8x8);
              return 1;
#endif
  }
  return 0;
}

